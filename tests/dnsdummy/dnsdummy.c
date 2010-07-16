#include <stdio.h>
#include <string.h>
#include <netdb.h>
#include <sys/socket.h>
#include <time.h>
#include <sys/select.h>
#include <unistd.h>
#include <getopt.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <ctype.h>
#include <netinet/in.h>
#include <errno.h>

#define _STRINGIFY(X)                   #X
#define STRINGIFY(X)                    _STRINGIFY(X)
#define MAXVAL(a,b)                     ({ typeof (a) _a = (a); typeof (b) _b = (b); _a > _b ? _a : _b; })
#define MINVAL(a,b)                     ({ typeof (a) _a = (a); typeof (b) _b = (b); _a < _b ? _a : _b; })

#define MAX_BUF                         1023
#define MAX_BIND_ATTEMPTS               32
#define DEFAULT_TIMEOUT_SECS            120

#define DNS_A                           1
#define DNS_MX                          15
#define DNS_NS                          2
#define DNS_CNAME                       5
#define DNS_PTR                         12
#define DNS_TXT                         16

/*
 * NOTE: This code performs almost zero buffer length checking.  Crafting a
 * query or an input file to cause a buffer overflow would be extremely trivial.
 *
 * In other words, if you use this program for any production purpose, you're
 * asking for trouble and you will very likely get it.
 */

void usage()
  {
  fprintf(stderr, "USAGE: dnsdummy [ -v[v[v]] ] [ -p PORT ] [ -t TIMEOUT_SECS ] [ -n ] -f FILE\n"
                  "\n"
                  "dnsdummy functions is an extremely limited DNS server that responds to UDP\n"
                  "queries only.  Normally, it will run in the background and automatically exit\n"
                  "after a set time.\n"
                  "\n"
                  "dnsdummy will always print its port number on stdout as it starts up.\n"
                  "\n"
                  "WARNING: dnsdummy is meant only for testing DNS client code. It is NOT to be\n"
                  "used in production settings, PERIOD. In addition to being extremely\n"
                  "inefficient, it also performs almost no buffer length checking. Crafting either\n"
                  "a config file or a query to trigger a buffer overflow is TRIVIAL.\n"
                  "If you choose to use dnsdummy for anything \"real\", you will be asking for\n"
                  "(and will certainly receive) more trouble than you can handle.\n"
                  "\n"
                  "-v\n"
                  "  Print some debugging messages. If given multiple times, more debugging output\n"
                  "  will be produced.\n"
                  "\n"
                  "-p PORT\n"
                  "  Listen for incoming connections on PORT. Default: select a port randomly.\n"
                  "\n"
                  "-n\n"
                  "  Do not fork into the background and do not automatically exit, even if -t is\n"
                  "  given.\n"
                  "\n"
                  "-t TIMEOUT_SECS\n"
                  "  Exit after TIMEOUT_SECS seconds. Default: " STRINGIFY(DEFAULT_TIMEOUT_SECS) "\n"
                  "\n"
                  "-f FILE\n"
                  "  Read responses from FILE. The file format is very simple.\n"
                  "  Blank lines are ignored. Lines beginning with # are ignored as comments.\n"
                  "  Otherwise, each line should contain:\n"
                  "    QUERY TYPE FLAG ANSWER\n"
                  "  Where QUERY is the name queried by the remote client, TYPE is the type\n"
                  "  of query and ANSWER is the response dnsdummy should send.  FLAG determines\n"
                  "  extra behavior dnsdummy should use when responding:\n"
                  "    NORMAL: send a normal response using the same protocol as the request\n"
                  "    TRUNCATE: set the truncation flag when responding via UDP; the client\n"
                  "      should retry the request via TCP\n"
                  "    IGNORE: send no response at all (as opposed to NXDOMAIN); the ANSWER\n"
                  "      field may be left blank\n"
                  "    SPOOF: reply to UDP requests from a different port than the request\n"
                  "  The fields should be separated by whitespace (tabs/spaces). Trailing periods\n"
                  "  should not be used; all names are assumed to reference the root scope.\n"
                  "  \"A\" records must always contain IP addresses.\n"
                  "  Examples:\n"
                  "    mail.foo.com             A       NORMAL          11.22.33.44\n"
                  "    www.foo.com              CNAME   TRUNCATE        foo.com\n"
                  "    foo.com                  NS      NORMAL          ns.foo.com\n"
                  "    foo.com                  TXT     SPOOF           Some text response.\n"
                  "    foo.com                  MX      IGNORE\n"
                  "    44.33.22.11.foo.com      PTR     NORMAL          www.foo.com\n"
                  "  If a match is not found, dnsdummy will respond with NXDOMAIN.\n"
                  );
                 /********************************************************************************/

  exit(0);

  return;
  }

char *type_name(int target_type)
  {
  char *return_value;

  return_value = NULL;

  switch (target_type)
    {
    case DNS_A:
      return_value = "A";
      break;
    case DNS_MX:
      return_value = "MX";
      break;
    case DNS_NS:
      return_value = "NS";
      break;
    case DNS_CNAME:
      return_value = "CNAME";
      break;
    case DNS_PTR:
      return_value = "PTR";
      break;
    case DNS_TXT:
      return_value = "TXT";
      break;
    }

  return(return_value);
  }

/*
 * RETURNS:
 *  -1: error occurred
 *   0: No match found (NXDOMAIN)
 *   1: Send no response
 *   2: Found match
 *   3: Found match + truncation flag
 *   4: Found match + spoof flag
 */
int search_file(char *return_answer, int strlen_return_answer, char *target_file, char *target_name, int target_type, int verbose)
  {
  int return_value;
  int i;
  char input_line[MAX_BUF + 1];
  char tmp_name[MAX_BUF + 1];
  char query_name[MAX_BUF + 1];
  char query_type[MAX_BUF + 1];
  char flag[MAX_BUF + 1];
  char answer_name[MAX_BUF + 1];
  FILE *input_file;
  int line_num;
  int strlen_line;

  return_value = 0;
  input_file = NULL;
  line_num = 0;

  if ((input_file = fopen(target_file, "r")) != NULL)
    {
    for (i = 0; (i < MAX_BUF) && (target_name[i] != '\0') && !isspace(target_name[i]); i++)
      tmp_name[i] = tolower(target_name[i]);
    tmp_name[i] = '\0';

    if (verbose)
      fprintf(stderr, "dnsdummy: searching %s for name: %s (%s)\n", target_file, tmp_name, type_name(target_type));

    while (!feof(input_file) &&
           (line_num < 65536))
      {
      if ((fscanf(input_file, "%" STRINGIFY(MAX_BUF) "[^\r\n]", input_line) == 1) &&
          (input_line[0] != '#') &&
          ((strlen_line = strlen(input_line)) > 0))
        {
        answer_name[0] = '\0';
        if (sscanf(input_line, "%" STRINGIFY(MAX_BUF) "[^\r\n\t ]%*[\r\n\t ]%" STRINGIFY(MAX_BUF) "[^\r\n\t ]%*[\r\n\t ]%" STRINGIFY(MAX_BUF) "[^\r\n\t ]%*[\r\n\t ]%" STRINGIFY(MAX_BUF) "[^\r\n]", query_name, query_type, flag, answer_name) >= 3)
          {
          for (i = 0; query_name[i] != '\0'; i++)
            query_name[i] = tolower(query_name[i]);
          for (i = 0; query_type[i] != '\0'; i++)
            query_type[i] = tolower(query_type[i]);
          for (i = 0; flag[i] != '\0'; i++)
            flag[i] = tolower(flag[i]);

          if (verbose >= 2)
            fprintf(stderr, "dnsdummy: read config line: query_name = %s, query_type = %s, flag = %s, answer = %s\n", query_name, query_type, flag, answer_name);

          if (!strcmp(tmp_name, query_name) &&
              (((target_type == DNS_A) &&
                !strcmp("a", query_type)) ||
               ((target_type == DNS_NS) &&
                !strcmp("ns", query_type)) ||
               ((target_type == DNS_CNAME) &&
                !strcmp("cname", query_type)) ||
               ((target_type == DNS_PTR) &&
                !strcmp("ptr", query_type)) ||
               ((target_type == DNS_MX) &&
                !strcmp("mx", query_type)) ||
               ((target_type == DNS_TXT) &&
                !strcmp("txt", query_type))))
            {
            if (strcmp(flag, "ignore") != 0)
              {
              if (answer_name[0] != '\0')
                {
                if (verbose)
                  fprintf(stderr, "dnsdummy: matched %s on line %d, returning: %s\n", tmp_name, line_num + 1, answer_name);

                memcpy(return_answer, answer_name, MINVAL(255, MINVAL(strlen_return_answer, strlen(answer_name))));
                return_answer[MINVAL(255, MINVAL(strlen_return_answer, strlen(answer_name)))] = '\0';

                if (!strcmp(flag, "normal"))
                  return_value = 2;
                else if (!strcmp(flag, "truncate"))
                  {
                  if (verbose)
                    fprintf(stderr, "dnsdummy: truncation flag found\n");

                  return_value = 3;
                  }
                else if (!strcmp(flag, "spoof"))
                  {
                  if (verbose)
                    fprintf(stderr, "dnsdummy: spoof flag found\n");

                  return_value = 4;
                  }
                else if (verbose)
                  fprintf(stderr, "dnsdummy: malformed configuration file on line %d: unknown flag %s\n", line_num + 1, flag);
                }
              else if (verbose)
                fprintf(stderr, "dnsdummy: malformed configuration file on line %d: no answer supplied but flag is not \"ignore\"\n", line_num + 1);
              }
            else
              {
              if (verbose)
                fprintf(stderr, "dnsdummy: matched %s on line %d, ignoring query\n", tmp_name, line_num + 1);

              return_value = 1;
              }

            break;
            }
          }
        else
          fprintf(stderr, "ERROR: unparsable DNS record in %s at line %d: bad format\n", target_file, line_num + 1);
        }

      fscanf(input_file, "%*1[\r\n]");
      line_num++;
      }

    if ((return_value == 0) &&
        verbose)
      fprintf(stderr, "dnsdummy: no match found\n");

    fclose(input_file);
    }
  else
    {
    fprintf(stderr, "ERROR: unable to open file %s: %s\n", target_file, strerror(errno));
    return_value = -1;
    }

  return(return_value);
  }

/* FIXME: This doesn't handle pointers */
int decode(char *return_name, int strlen_return_name, unsigned char *name_start, unsigned char *query_end, int verbose)
  {
  int return_value;
  int i;
  unsigned char *cur_ptr;

  return_value = 0;
  cur_ptr = name_start;

  if ((cur_ptr = name_start) != NULL)
    while (((int)cur_ptr[0] != 0) &&
           (cur_ptr < query_end) &&
           (return_value < strlen_return_name))
      {
      for (i = 0; i < (int)cur_ptr[0]; i++)
        return_name[return_value++] = cur_ptr[i + 1];

      cur_ptr += (int)cur_ptr[0] + 1;

      if ((int)cur_ptr[0] != 0)
        return_name[return_value++] = '.';
      else
        return_name[return_value++] = '\0';
      }

  return_value++;

  if (verbose >= 2)
    fprintf(stderr, "dnsdummy: decoded %s (%d bytes)\n", return_name, return_value);

  return(return_value);
  }

/*
 * RETURN VALUE:
 *   number of characters encoded into return_name
 */
int encode(unsigned char *return_name, int strlen_return_name, char *target_name)
  {
  int i;
  int strlen_cur;
  int strlen_name;

  strlen_name = 0;

  if ((return_name != NULL) &&
      (target_name != NULL) &&
      (strlen_return_name > 2))
    {
    strlen_name = MINVAL(strlen_return_name - 2, strlen(target_name));
    memcpy(return_name + 1, target_name, strlen_name);
    return_name[0] = '.';
    return_name[strlen_name + 1] = '\0';

    strlen_cur = 0;
    for (i = strlen_name; i >= 0; i--)
      if (return_name[i] == '.')
        {
        return_name[i] = strlen_cur;
        strlen_cur = 0;
        }
      else
        strlen_cur++;

    strlen_name += 2;
    }

  return(strlen_name);
  }

int main(int argc, char *argv[])
  {
  char answer_template[] = { /* ID */ 0x00, 0x00,
                             /* QR, OPCODE, AA, TC, RD, RA, RCODE */ 0x84, 0x00,
                             /* QDCOUNT */ 0x00, 0x00,
                             /* ANCOUNT */ 0x00, 0x00,
                             /* NSCOUNT */ 0x00, 0x00,
                             /* ARCOUNT */ 0x00, 0x00 };
  int i;
  int udp_socket;
  int tcp_socket;
  int accept_socket;
  int target_socket;
  struct sockaddr_in tmp_sockaddr;
  time_t start_time;
  int max_secs;
  int query_len;
  unsigned char query[MAX_BUF + 1];
  int tcp_query_len;
  unsigned char tcp_query[MAX_BUF + 1];
  int answer_len;
  int answer_start;
  unsigned char answer[MAX_BUF + 1];
  char query_name[MAX_BUF + 1];
  char answer_name[MAX_BUF + 1];
  int listen_port;
  fd_set read_fds;
  struct timeval tmp_timeval;
  char opt;
  int tmp_int;
  int target_port;
  int bind_attempts;
  int tmp_ptr;
  long tmp_count;
  long tmp_num;
  int type_num;
  struct sockaddr_in sender_address;
  int sender_address_len;
  int name_len;
  int num_answers;
  char *config_file;
  int verbose;
  int ip_ints[4];
  int search_result;
  int do_fork;
  int select_value;

  max_secs = DEFAULT_TIMEOUT_SECS;
  target_port = 0;
  verbose = 0;
  config_file = NULL;
  do_fork = 1;

  while ((opt = getopt(argc, argv, "f:np:t:v")) != -1)
    {
    switch (opt)
      {
      case 'f':
        config_file = optarg;
        break;
      case 'n':
        do_fork = 0;
        break;
      case 'p':
        if ((sscanf(optarg, "%d", &tmp_int) == 1) &&
            (tmp_int > 0) &&
            (tmp_int < 65536))
          target_port = tmp_int;
        else
          usage();

        break;
      case 't':
        if ((sscanf(optarg, "%d", &tmp_int) == 1) &&
            (tmp_int > 0))
          max_secs = tmp_int;

        break;
      case 'v':
        verbose++;
        break;
      }
    }

  if (config_file == NULL)
    usage();

  srandom(time(NULL));

  tmp_sockaddr.sin_family = AF_INET;
  tmp_sockaddr.sin_addr.s_addr = htonl(INADDR_ANY);
  bzero(&tmp_sockaddr.sin_zero, 8);

  bind_attempts = 0;

  while (bind_attempts < MAX_BIND_ATTEMPTS)
    {
    listen_port = (target_port > 0) ? target_port : ((random() % (65535 - 1024)) + 1024);
    tmp_sockaddr.sin_port = htons(listen_port);

    udp_socket = -1;
    tcp_socket = -1;
    accept_socket = -1;

    if ((udp_socket = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP)) != -1)
      if ((tcp_socket = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) != -1)
        if ((bind(udp_socket, (struct sockaddr *)&tmp_sockaddr, sizeof(struct sockaddr)) == 0) &&
            (bind(tcp_socket, (struct sockaddr *)&tmp_sockaddr, sizeof(struct sockaddr)) == 0) &&
            (listen(tcp_socket, 10) == 0))
          {
          fprintf(stdout, "%d\n", listen_port);
          fflush(NULL);

          if (!do_fork ||
              (fork() == 0))
            {
            if (do_fork)
              fclose(stdout);

            start_time = time(NULL);

            FD_ZERO(&read_fds);
            FD_SET(udp_socket, &read_fds);
            FD_SET(tcp_socket, &read_fds);
            select_value = MAXVAL(udp_socket, tcp_socket) + 1;

            tmp_timeval.tv_sec = max_secs - (time(NULL) - start_time);
            tmp_timeval.tv_usec = 0;

            query_len = 0;
            tcp_query_len = 0;
            accept_socket = -1;

            while (!do_fork ||
                   (tmp_timeval.tv_sec > 0))
              {
              if (select(select_value, &read_fds, NULL, NULL, (do_fork) ? &tmp_timeval : NULL) > 0)
                {
                sender_address_len = sizeof(struct sockaddr_in);

                if (FD_ISSET(udp_socket, &read_fds))
                  {
                  if ((query_len = recvfrom(udp_socket, query, MAX_BUF, 0, (struct sockaddr *)&sender_address, (socklen_t *)&sender_address_len)) >= 12)
                    {
                    if (verbose)
                      fprintf(stderr, "dnsdummy: received %d bytes via UDP, ID: %d/%d\n", query_len, query[0], query[1]);
                    }
                  else
                    {
                    fprintf(stderr, "DNSDUMMY ERROR: received malformed UDP query packet: %d bytes, minimum 12\n", query_len);
                    query_len = 0;
                    }
                  }
                else if (FD_ISSET(tcp_socket, &read_fds))
                  {
                  if ((accept_socket = accept(tcp_socket, (struct sockaddr *)&sender_address, (socklen_t *)&sender_address_len)) != -1)
                    {
                    if (verbose)
                      fprintf(stderr, "dnsdummy: accepted TCP connection on socket %d\n", accept_socket);
                    }
                  else
                    fprintf(stderr, "DNSDUMMY ERROR: unable to accept incoming TCP connection: %s\n", strerror(errno));
                  }
                else if ((accept_socket != -1) &&
                         FD_ISSET(accept_socket, &read_fds))
                  {
                  if ((query_len = recv(accept_socket, tcp_query + tcp_query_len, MAX_BUF - tcp_query_len, 0)) > 0)
                    {
                    if (verbose >= 2)
                      fprintf(stderr, "dnsdummy: read %d bytes from TCP socket\n", query_len);

                    if ((tcp_query_len += query_len) > 2)
                      {
                      ((char *)&tmp_count)[0] = tcp_query[0];
                      ((char *)&tmp_count)[1] = tcp_query[1];

                      if (ntohs(tmp_count) == (tcp_query_len - 2))
                        {
                        memcpy(query, tcp_query + 2, tcp_query_len - 2);
                        query_len = tcp_query_len;
                        }
                      else
                        query_len = 0;
                      }
                    else
                      query_len = 0;
                    }
                  else
                    {
                    if (verbose)
                      fprintf(stderr, "dnsdummy: closed TCP socket\n");

                    close(accept_socket);
                    accept_socket = -1;
                    query_len = 0;
                    }
                  }
                else
                  query_len = 0;

                if (query_len > 0)
                  {
                  if (verbose >= 3)
                    for (i = 0; i < query_len; i++)
                      fprintf(stderr, "dnsdummy: query byte %d:\t%d\t%c\n", i, query[i], query[i]);
  
                  answer_len = sizeof(answer_template);
                  memcpy(answer, answer_template, answer_len);
  
                  answer[0] = query[0];
                  answer[1] = query[1];
  
                  /* Copy the number of questions into the response */
                  answer[4] = query[4];
                  answer[5] = query[5];
  
                  num_answers = 0;
  
                  /* Decode the number of questions */
                  ((char *)&tmp_count)[0] = query[4];
                  ((char *)&tmp_count)[1] = query[5];
  
                  /* Set the pointer to the end of the header */
                  tmp_ptr = 12;
  
                  /*
                   * Question structure:
                   *   text of unknown length
                   *   type (16 bits)
                   *   class (16 bits)
                   */
                  for (i = 0; i < ntohs(tmp_count); i++)
                    {
                    name_len = decode(query_name, MAX_BUF, query + tmp_ptr, query + query_len, verbose);
  
                    /* Copy the question into the answer */
                    if ((answer_len + name_len + 4) < MAX_BUF)
                      {
                      memcpy(answer + answer_len, query + tmp_ptr, name_len + 4);
                      answer_len += name_len + 4;
                      }
                    else
                      {
                      fprintf(stderr, "DNSDUMMY ERROR: questions are too long to copy: %d bytes\n", answer_len + name_len + 4);
                      break;
                      }
  
                    tmp_ptr += name_len + 4;
                    }
  
                  /* Reset the pointer to the end of the header */
                  tmp_ptr = 12;
  
                  /*
                   * Examine the questions:
                   *   text of unknown length
                   *   type (16 bits)
                   *   class (16 bits)
                   */
                  for (i = 0; i < ntohs(tmp_count); i++)
                    {
                    name_len = decode(query_name, MAX_BUF, query + tmp_ptr, query + query_len, verbose);
  
                    if ((answer_len + name_len + 4) >= MAX_BUF)
                      {
                      fprintf(stderr, "DNSDUMMY ERROR: questions are too long to copy: %d bytes\n", answer_len + name_len + 4);
                      break;
                      }
  
                    /* Decode the type */
                    ((char *)&tmp_num)[0] = query[tmp_ptr + name_len];
                    ((char *)&tmp_num)[1] = query[tmp_ptr + name_len + 1];
                    type_num = ntohs(tmp_num);
  
                    if (((search_result = search_file(answer_name, MAX_BUF, config_file, query_name, type_num, verbose)) == 2) ||
                        ((search_result == 3) &&
                         (tcp_query_len > 0)) ||
                        (search_result == 4))
                      {
                      answer_start = answer_len;
  
                      /* Copy the name into the answer */
                      if ((answer_len + name_len) < MAX_BUF)
                        {
                        memcpy(answer + answer_len, query + tmp_ptr, name_len);
                        answer_len += name_len;
                        }
                      else
                        {
                        fprintf(stderr, "DNSDUMMY ERROR: name is too long to copy: %d bytes\n", answer_len + name_len + 4);
                        break;
                        }
  
                      /* Copy the type */
                      answer[answer_len++] = query[tmp_ptr + name_len];
                      answer[answer_len++] = query[tmp_ptr + name_len + 1];
                      /* Copy the class */
                      answer[answer_len++] = query[tmp_ptr + name_len + 2];
                      answer[answer_len++] = query[tmp_ptr + name_len + 3];
                      /* Set the TTL to 0 */
                      answer[answer_len++] = 0;
                      answer[answer_len++] = 0;
                      answer[answer_len++] = 0;
                      answer[answer_len++] = 0;
  
                      if (type_num == DNS_A)
                        {
                        /* Set the answer length */
                        tmp_num = htons(4);
                        answer[answer_len++] = ((char *)&tmp_num)[0];
                        answer[answer_len++] = ((char *)&tmp_num)[1];
  
                        if (sscanf(answer_name, "%d.%d.%d.%d", &ip_ints[0], &ip_ints[1], &ip_ints[2], &ip_ints[3]) == 4)
                          {
                          /* Set the IP address */
                          answer[answer_len++] = ip_ints[0];
                          answer[answer_len++] = ip_ints[1];
                          answer[answer_len++] = ip_ints[2];
                          answer[answer_len++] = ip_ints[3];
                          }
  
                        if ((tcp_query_len > 0) ||
                            (answer_len <= 512))
                          num_answers++;
                        else
                          {
                          if (verbose)
                            fprintf(stderr, "dnsdummy: answer is too long for UDP, setting truncation bit: %d bytes\n", answer_len);
  
                          answer_len = answer_start;
                          answer[2] |= 0x02;
                          }
                        }
                      else if ((type_num == DNS_MX) ||
                               (type_num == DNS_NS) ||
                               (type_num == DNS_CNAME) ||
                               (type_num == DNS_PTR))
                        {
                        if (type_num == DNS_MX)
                          tmp_int = 2;
                        else
                          tmp_int = 0;
  
                        if (verbose >= 4)
                          fprintf(stderr, "dnsdummy: encoding answer starting at byte %d: %s\n", answer_len + 2, answer_name);
  
                        tmp_int += encode(answer + answer_len + tmp_int + 2, MAX_BUF - (answer_len + tmp_int + 2), answer_name);
  
                        /* Set the answer length */
                        tmp_num = htons(tmp_int);
                        answer[answer_len++] = ((char *)&tmp_num)[0];
                        answer[answer_len++] = ((char *)&tmp_num)[1];
  
                        if (type_num == DNS_MX)
                          {
                          /* Set the preference */
                          tmp_num = htons(10);
                          answer[answer_len++] = ((char *)&tmp_num)[0];
                          answer[answer_len++] = ((char *)&tmp_num)[1];
                          }
  
                        answer_len += tmp_int;
  
                        if ((tcp_query_len > 0) ||
                            (answer_len <= 512))
                          num_answers++;
                        else
                          {
                          if (verbose)
                            fprintf(stderr, "dnsdummy: answer is too long for UDP, setting truncation bit: %d bytes\n", answer_len);
  
                          answer_len = answer_start;
                          answer[2] |= 0x02;
                          }
                        }
                      else if (type_num == DNS_TXT)
                        {
                        /* Set the answer length */
                        tmp_num = htons(strlen(answer_name) + 1);
                        answer[answer_len++] = ((char *)&tmp_num)[0];
                        answer[answer_len++] = ((char *)&tmp_num)[1];
                        answer[answer_len++] = (char)strlen(answer_name);
  
                        memcpy(answer + answer_len, answer_name, strlen(answer_name));
                        answer_len += strlen(answer_name);
  
                        if ((tcp_query_len > 0) ||
                            (answer_len <= 512))
                          num_answers++;
                        else
                          {
                          if (verbose)
                            fprintf(stderr, "dnsdummy: answer is too long for UDP, setting truncation bit: %d bytes\n", answer_len);
  
                          answer_len = answer_start;
                          answer[2] |= 0x02;
                          }
                        }
                      }
                    else if ((search_result == 3) &&
                             (tcp_query_len == 0))
                      {
                      if (verbose)
                        fprintf(stderr, "dnsdummy: setting truncation bit\n");

                      answer[2] |= 0x02;
                      }
                    else if (search_result == 0)
                      if ((type_num == DNS_A) ||
                          (type_num == DNS_MX) ||
                          (type_num == DNS_NS) ||
                          (type_num == DNS_CNAME) ||
                          (type_num == DNS_PTR) ||
                          (type_num == DNS_TXT))
                        answer[3] = 3;
                      else
                        answer[3] = 4;
                    else if (verbose)
                      fprintf(stderr, "dnsdummy: ignoring query for DNS type %s\n", type_name(type_num));
  
                    tmp_ptr += name_len + 4;
                    }
  
                  if (((answer[2] & 0x02) == 0x02) ||
                      (answer[3] != 0) ||
                      (num_answers > 0))
                    {
                    /* Set the number of answers */
                    tmp_num = htons(num_answers);
                    answer[6] = ((char *)&tmp_num)[0];
                    answer[7] = ((char *)&tmp_num)[1];
  
                    if (tcp_query_len > 0)
                      {
                      memmove(answer + 2, answer, answer_len);
                      tmp_num = htons(answer_len);
                      answer[0] = ((char *)&tmp_num)[0];
                      answer[1] = ((char *)&tmp_num)[1];

                      if (send(accept_socket, answer, answer_len + 2, 0) > 0)
                        {
                        if (verbose)
                          fprintf(stderr, "dnsdummy: sent %d bytes to sender via TCP\n", answer_len + 2);
                        if (verbose >= 3)
                          for (i = 0; i < (answer_len + 2); i++)
                            fprintf(stderr, "dnsdummy: answer byte %d:\t%d\t%c\n", i, answer[i], answer[i]);
                        }
                      else
                        fprintf(stderr, "DNSDUMMY ERROR: unable to send %d bytes to sender via TCP: %s\n", answer_len, strerror(errno));

                      close(accept_socket);
                      accept_socket = -1;
                      tcp_query_len = 0;
                      }
                    else
                      {
                      if (answer_len > 512)
                        {
                        fprintf(stderr, "DNSDUMMY ERROR: answer is too long for UDP: %d bytes\n", answer_len);
                        answer_len = 512;
                        answer[2] |= 0x02;
                        }
                      else if (search_result == 3)
                        answer[2] = 0x02;

                      if (search_result == 4)
                        {
                        if ((target_socket = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1)
                          {
                          fprintf(stderr, "DNSDUMMY ERROR: unable to create new socket for spoofed response: %s\n", strerror(errno));
                          target_socket = udp_socket;
                          }
                        }
                      else
                        target_socket = udp_socket;
  
                      if (sendto(target_socket, answer, answer_len, 0, (struct sockaddr *)&sender_address, sender_address_len) > 0)
                        {
                        if (verbose)
                          fprintf(stderr, "dnsdummy: sent %d bytes to sender via UDP\n", answer_len);
                        if (verbose >= 3)
                          for (i = 0; i < answer_len; i++)
                            fprintf(stderr, "dnsdummy: answer byte %d:\t%d\t%c\n", i, answer[i], answer[i]);
                        }
                      else
                        fprintf(stderr, "DNSDUMMY ERROR: unable to send %d bytes to sender via UDP: %s\n", answer_len, strerror(errno));

                      if (target_socket != udp_socket)
                        close(target_socket);
                      }
                    }

                  query_len = 0;
                  }
                }

              FD_ZERO(&read_fds);
              FD_SET(udp_socket, &read_fds);
              if (accept_socket == -1)
                {
                FD_SET(tcp_socket, &read_fds);
                select_value = MAXVAL(udp_socket, tcp_socket) + 1;
                }
              else
                {
                FD_SET(accept_socket, &read_fds);
                select_value = MAXVAL(udp_socket, accept_socket) + 1;
                }

              tmp_timeval.tv_sec = max_secs - (time(NULL) - start_time);
              tmp_timeval.tv_usec = 0;
              }

            if (verbose >= 2)
              fprintf(stderr, "dnsdummy: child exiting\n");
            }
          else if (verbose >= 2)
            fprintf(stderr, "dnsdummy: parent exiting\n");

          break;
          }
        else if ((target_port == 0) &&
                 (errno == EADDRINUSE))
          bind_attempts++;
        else
          {
          fprintf(stderr, "DNSDUMMY ERROR: unable to bind socket: %s\n", strerror(errno));
          break;
          }
      else
        {
        fprintf(stderr, "DNSDUMMY ERROR: unable to create TCP socket: %s\n", strerror(errno));
        break;
        }
    else
      {
      fprintf(stderr, "DNSDUMMY ERROR: unable to create UDP socket: %s\n", strerror(errno));
      break;
      }

    if (udp_socket != -1)
      close(udp_socket);
    if (tcp_socket != -1)
      close(tcp_socket);
    if (accept_socket != -1)
      close(accept_socket);
    }

  return(0);
  }
