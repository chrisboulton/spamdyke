/*
  dnsns -- an example of making DNS NS queries by sending UDP packets
  Copyright (C) 2010 Sam Clippinger (samc (at) silence (dot) org)

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License version 2 as
  published by the Free Software Foundation.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/nameser.h>
#include <arpa/inet.h>
#include <resolv.h>
#include <unistd.h>
#include <stdlib.h>
#include <netdb.h>
#include <errno.h>
#include <strings.h>
#include "config.h"

#ifdef TIME_WITH_SYS_TIME

#include <sys/time.h>
#include <time.h>

#else /* TIME_WITH_SYS_TIME */
#ifdef HAVE_SYS_TIME_H

#include <sys/time.h>

#else /* HAVE_SYS_TIME_H */

#include <time.h>

#endif /* HAVE_SYS_TIME_H */
#endif /* TIME_WITH_SYS_TIME */

#ifdef HAVE_NAMESER_COMPAT
#include <arpa/nameser_compat.h>
#endif /* HAVE_NAMESER_COMPAT */

extern int opterr;

#define MINVAL(a,b)                     ({ typeof (a) _a = (a); typeof (b) _b = (b); _a < _b ? _a : _b; })

#define MAX_HOSTNAME            127
#define MAX_RDNS                29
#define MAX_BUF_SOCKET          32768
#define TIMEOUT_DNS_QUERY_SECS  2
#define MAX_DNS_QUERIES         5
#define MAX_DNS_QUERIES_PREFERRED       2

#define PROTOCOL_NAME_UDP       "udp"

#define ERROR_RES_INIT          "ERROR: unable to initialize resolver library.\n"
#define ERROR_GETPROTO          "ERROR: unable to find protocol number with getprotobyname()\n"
#define ERROR_SOCKET            "ERROR: unable to create socket: %s\n"
#define ERROR_BIND              "ERROR: unable to bind socket: %s\n"
#define ERROR_OPTION            "ERROR: unable to set socket option: %s\n"
#define ERROR_SENDTO_INCOMPLETE "ERROR: unable to send complete data packet, tried to send %d bytes, actually sent %d bytes\n"
#define ERROR_SENDTO            "ERROR: unable to send data packet: %s\n"
#define ERROR_MKQUERY           "ERROR: unable to construct data packet: %s\n"
#define ERROR_DN_EXPAND         "ERROR: dn_expand failed for %s; this could indicate a problem with the nameserver.\n"
#define ERROR_DN_SKIPNAME       "ERROR: dn_skipname failed for %s; this could indicate a problem with the nameserver.\n"
#define ERROR_DNS_RESPONSE      "ERROR: bad or invalid dns response to %s; this could indicate a problem with the name server.\n"
#define ERROR_TYPE              "ERROR: answer type (%d) does not match query type (%d).\n"

#define MSG_SEARCHING           "Searching for NS record: %s\n"
#define MSG_FOUND               "Found NS record for %s: "
#define MSG_FOUND_CNAME         "Found CNAME record for %s: %s\n"
#define MSG_FAILURE             "No records found: %s\n"
#define MSG_SENDING             "Attempt %d: sending DNS query packet to %s:%d\n"
#define MSG_RECEIVING           "Received DNS response packet: %d bytes\n"
#define MSG_MISMATCH            "Response packet does not match query -- discarding.\n"

struct previous_action
  {
  char *data;
  struct previous_action *prev;
  };

/*
 * The DNS packet format is not well documented outside of the RFCs.  There is
 * almost no sample or tutorial code to be found on the internet outside of the
 * sendmail source code, which is pretty hard to read.
 *
 * Basically, each DNS packet starts with a HEADER structure, defined in
 * arpa/nameser.h (or arpa/nameser_compat.h on Linux).  Most of the time, the
 * header can be skipped.
 *
 * After the header, the nameserver returns all of the "questions" it was asked,
 * so the answers will make sense.  If you're asking more than one "question"
 * per query, this is important.  Otherwise, skip them by finding the size of
 * each question with dn_skipname() and advancing past them.  The number of
 * questions is found in the qdcount field of the header.
 *
 * Next is the answer section, which can contain many answers, though multiple
 * answers may not make much sense for all query types.  The number of answers
 * is found in the ancount field of the header.  Within each answer, the first
 * field is the name that was queried, for reference.  It can be skipped with
 * dn_skipname().
 *
 * After that comes the type in a 16 bit field, then the class in a 16 bit
 * field, then the time-to-live (TTL) in a 32 bit field, then the answer size
 * in a 16 bit field.  The type and size are important; the class and the ttl
 * can usually be ignored.  The format of the rest of the answer field is
 * different depending on the type.
 *
 * IF THE TYPE IS A:
 * The first 4 bytes are the four octets of the IP address.
 *
 * IF THE TYPE IS T_TXT:
 * The first 8 bits are an unsigned integer indicating the total length of
 * the text response.  The following bytes are the ASCII text of the response.
 *
 * IF THE TYPE IS T_PTR OR T_NS:
 * All of the bytes are the compressed name of the result.  They can be
 * decoded with dn_expand().
 *
 * IF THE TYPE IS T_CNAME:
 * All of the bytes are the compressed name of the CNAME entry.  They can be
 * decoded with dn_expand().
 *
 * IF THE TYPE IS T_MX:
 * Each answer begins with an unsigned 16 bit integer indicating the preference
 * of the mail server (lower preferences should be contacted first).  The
 * remainder of the answer is the mail server name.  It can be decoded with
 * dn_expand().
 *
 * IF THE TYPE IS T_SOA:
 * The first section of bytes are the compressed name of the primary NS server.
 * They can be decoded with dn_expand().  The second section of bytes are the
 * compressed name of the administrator's mailbox.  They can be decoded with
 * dn_expand().  After the end of the mailbox data, five 32 bit integers give
 * the serial number, the refresh interval, the retry interval, the expiration
 * limit and the minimum time to live, in that order.
 *
 * SEE ALSO:
 *   RFC 1035
 *   http://www.zytrax.com/books/dns/ch15/
 *   "DNS and BIND" from O'Reilly
 */
int dns_initialize(int close_socket)
  {
  static int dns_socket = -1;
  struct protoent *tmp_protoent;
  struct sockaddr_in tmp_sockaddr;
  int max_buf_socket = MAX_BUF_SOCKET;

  if (!close_socket)
    {
    if (dns_socket == -1)
      {
      if (res_init() >= 0)
        {
        tmp_sockaddr.sin_family = AF_INET;
        tmp_sockaddr.sin_port = 0;
        tmp_sockaddr.sin_addr.s_addr = htonl(INADDR_ANY);
        bzero(&tmp_sockaddr.sin_zero, 8);

        if ((tmp_protoent = getprotobyname(PROTOCOL_NAME_UDP)) != NULL)
          {
          if ((dns_socket = socket(PF_INET, SOCK_DGRAM, tmp_protoent->p_proto)) != -1)
            {
            if (bind(dns_socket, (struct sockaddr *)&tmp_sockaddr, sizeof(struct sockaddr)) == 0)
              {
              if (setsockopt(dns_socket, SOL_SOCKET, SO_RCVBUF, (char *)&max_buf_socket, sizeof(int)) != 0)
                printf(ERROR_OPTION, strerror(errno));

              if (setsockopt(dns_socket, SOL_SOCKET, SO_SNDBUF, (char *)&max_buf_socket, sizeof(int)) != 0)
                printf(ERROR_OPTION, strerror(errno));
              }
            else
              {
              printf(ERROR_BIND, strerror(errno));
              close(dns_socket);
              dns_socket = -1;
              }
            }
          else
            printf(ERROR_SOCKET, strerror(errno));
          }
        else
          printf(ERROR_GETPROTO);
        }
      else
        printf(ERROR_RES_INIT);
      }
    }
  else if (dns_socket != -1)
    {
    close(dns_socket);
    dns_socket = -1;
    }

  return(dns_socket);
  }

int dns_get(char *target_name, int type, unsigned char *return_answer, int return_answer_length, int verbose)
  {
  static int query_id = 0;
  int return_value;
  int i;
  int dns_socket;
  unsigned char question[PACKETSZ];
  unsigned char answer[PACKETSZ];
  int question_length;
  unsigned short tmp_id;
  int sendto_result;
  int select_result;
  int response_length;
  fd_set read_fds;
  struct timeval tmp_timeval;
  time_t start_time;
  int num_queries;

  return_value = 0;

  if ((dns_socket = dns_initialize(0)) != -1)
    {
    if ((question_length = res_mkquery(QUERY, target_name, C_IN, type, NULL, 0, NULL, question, PACKETSZ)) >= 0)
      {
      tmp_id = htons(query_id);
      query_id++;

      question[0] = ((char *)&tmp_id)[0];
      question[1] = ((char *)&tmp_id)[1];

      start_time = time(NULL);
      num_queries = 0;

      do
        {
        sendto_result = 0;

        if (num_queries < MAX_DNS_QUERIES_PREFERRED)
          {
          if (verbose)
            printf(MSG_SENDING, num_queries, inet_ntoa(_res.nsaddr_list[0].sin_addr), ntohs(_res.nsaddr_list[0].sin_port));

          sendto_result = sendto(dns_socket, question, question_length, 0, (struct sockaddr *)&_res.nsaddr_list[0], sizeof(struct sockaddr));
          }
        else
          for (i = 0; i < _res.nscount; i++)
            {
            if (verbose)
              printf(MSG_SENDING, num_queries, inet_ntoa(_res.nsaddr_list[i].sin_addr), ntohs(_res.nsaddr_list[i].sin_port));

            sendto_result = sendto(dns_socket, question, question_length, 0, (struct sockaddr *)&_res.nsaddr_list[i], sizeof(struct sockaddr));
            }

        num_queries++;

        if (sendto_result == question_length)
          {
          FD_ZERO(&read_fds);
          FD_SET(dns_socket, &read_fds);

          tmp_timeval.tv_sec = MINVAL((TIMEOUT_DNS_QUERY_SECS * MAX_DNS_QUERIES) - (time(NULL) - start_time), TIMEOUT_DNS_QUERY_SECS);
          tmp_timeval.tv_usec = 0;

          while ((tmp_timeval.tv_sec > 0) &&
                 ((select_result = select(dns_socket + 1, &read_fds, NULL, NULL, &tmp_timeval)) > 0))
            {
            if (((response_length = recvfrom(dns_socket, answer, PACKETSZ, 0, NULL, NULL)) > 0) &&
                (answer[0] == question[0]) &&
                (answer[1] == question[1]))
              {
              if (verbose)
                printf(MSG_RECEIVING, response_length);

              memcpy(return_answer, answer, MINVAL(response_length, return_answer_length));
              return_value = MINVAL(response_length, return_answer_length);
              break;
              }
            else if (verbose)
              printf(MSG_MISMATCH);
            }

          if (return_value > 0)
            break;
          }
        else if (sendto_result >= 0)
          printf(ERROR_SENDTO_INCOMPLETE, question_length, sendto_result);
        else
          {
          printf(ERROR_SENDTO, strerror(errno));
          break;
          }
        }
      while (num_queries < MAX_DNS_QUERIES);
      }
    else
      printf(ERROR_MKQUERY, strerror(errno));
    }

  return(return_value);
  }

int dns_ns(char *target_name, struct previous_action *history, int verbose)
  {
  int return_value;
  int i;
  unsigned char answer[PACKETSZ];
  unsigned char host[MAX_HOSTNAME + 1];
  unsigned char *answer_ptr;
  unsigned char *answer_start;
  int answer_length;
  int size;
  int type;
  int num_questions;
  int num_answers;
  struct previous_action current_lookup;
  struct previous_action *tmp_lookup;

  return_value = 0;

  memset(answer, 0, PACKETSZ);

  if (target_name != NULL)
    {
    if (verbose)
      printf(MSG_SEARCHING, target_name);

    if ((answer_length = dns_get(target_name, T_NS, answer, PACKETSZ, verbose)) >= 0)
      {
      answer_ptr = answer + sizeof(HEADER);

      // Skip the questions
      num_questions = ntohs((unsigned short)((HEADER *)&answer)->qdcount);
      for (i = 0; i < num_questions; i++)
        if ((size = dn_skipname(answer_ptr, answer + answer_length)) >= 0)
          answer_ptr += size + QFIXEDSZ;
        else
          break;

      if (i == num_questions)
        {
        num_answers = ntohs((unsigned short)((HEADER *)&answer)->ancount);
        answer_start = answer_ptr;

        for (i = 0; (i < num_answers) && !return_value; i++)
          if ((size = dn_skipname(answer_ptr, answer + answer_length)) >= 0)
            {
            answer_ptr += size;
            GETSHORT(type, answer_ptr);
            answer_ptr += INT16SZ; // class
            answer_ptr += INT32SZ; // ttl
            answer_ptr += INT16SZ; // size

            if (type == T_NS)
              {
              if (verbose)
                printf(MSG_FOUND, target_name);

              if ((size = dn_expand(answer, answer + answer_length, answer_ptr, (char *)host, MAX_HOSTNAME)) >= 0)
                {
                printf("%s\n", host);
                answer_ptr += size;

                return_value = 1;
                }
              else
                {
                printf(ERROR_DNS_RESPONSE, target_name);
                break;
                }
              }
            else if (type == T_CNAME)
              {
              if ((size = dn_expand(answer, answer + answer_length, answer_ptr, (char *)host, MAX_HOSTNAME)) >= 0)
                {
                if (verbose)
                  printf(MSG_FOUND_CNAME, target_name, host);

                current_lookup.data = target_name;
                current_lookup.prev = history;

                tmp_lookup = &current_lookup;
                while (tmp_lookup != NULL)
                  if (strcasecmp((char *)host, tmp_lookup->data) == 0)
                    break;
                  else
                    tmp_lookup = tmp_lookup->prev;

                return_value = (tmp_lookup == NULL) ? dns_ns((char *)host, &current_lookup, verbose) : 0;
                answer_ptr += size;
                }
              else
                {
                printf(ERROR_DNS_RESPONSE, target_name);
                break;
                }
              }
            else
              {
              printf(ERROR_TYPE, type, T_NS);
              break;
              }
            }
          else
            {
            printf(ERROR_DN_SKIPNAME, target_name);
            break;
            }
        }
      else
        printf(ERROR_DN_SKIPNAME, target_name);
      }
    else if (verbose)
      printf(MSG_FAILURE, target_name);
    }

  return(return_value);
  }

void usage()
  {
  printf(
    PACKAGE_NAME " " PACKAGE_VERSION " (C)2010 Sam Clippinger, " PACKAGE_BUGREPORT "\n"
    "http://www.spamdyke.org/\n"
    "\n"
    "USAGE: dnsns [ -v ] FQDN [ FQDN ... ]\n"
    "\n"
    "Performs a DNS lookup for the NS records associated with FQDN and prints the results.\n"
    "\n"
    "-v\n"
    "  Give more verbose output\n"
    );

  return;
  }

int main(int argc, char *argv[])
  {
  int i;
  int verbose;
  int opt;
  int error;

  opterr = 0;
  verbose = 0;
  error = 0;

  while ((opt = getopt(argc, argv, "v")) != -1)
    switch (opt)
      {
      case 'v':
        verbose = 1;
        break;
      default:
        usage();
        error = 1;
        break;
      }

  if (!error)
    {
    if ((argc - optind) > 0)
      for (i = optind; i < argc; i++)
        dns_ns(argv[i], NULL, verbose);
    else
      usage();
    }

  dns_initialize(1);

  return(0);
  }
