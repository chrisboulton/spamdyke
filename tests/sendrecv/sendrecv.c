/*
  sendrecv -- a simple program for piping data to and from spamdyke
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
#include <errno.h>
#include <getopt.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/select.h>
#include <time.h>
#include <stdint.h>
#include <ctype.h>
#include <openssl/ssl.h>
#include <openssl/err.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>
#include "base64.h"
#include "md5.h"

#define STRLEN(X)               (sizeof(X) - 1)
#define _STRINGIFY(X)           #X
#define STRINGIFY(X)            _STRINGIFY(X)


#define DEFAULT_TIMEOUT_SECS            10
#define DEFAULT_WRITE_DELAY_SECS        2

#define STDIN_FD                0
#define STDOUT_FD               1
#define LINE_TERMINATOR_CR      '\r'
#define LINE_TERMINATOR         '\n'
#define LINE_TERMINATOR_STR     "\r\n"
#define MAX_BUF                 4095
#define MAX_INPUT_BUF           65535
#define MINVAL(a,b)             ({ typeof (a) _a = (a); typeof (b) _b = (b); _a < _b ? _a : _b; })
#define MAXVAL(a,b)             ({ typeof (a) _a = (a); typeof (b) _b = (b); _a > _b ? _a : _b; })
#define SMTP_DATA               "DATA"
#define SMTP_DATA_END           "."
#define SMTP_DATA_END_MULTILINE "\r\n.\r\n"
#define SMTP_STARTTLS           "STARTTLS"
#define IPAD_BYTE               0x36
#define OPAD_BYTE               0x5C
#define CODE_CRAMMD5_CHALLENGE  "334 "
#define CODE_STARTTLS_SUCCESS   "220 "
#define CODE_STARTTLS_ERROR4    "4"
#define CODE_STARTTLS_ERROR5    "5"

#define TLS_STATE_NONE          0
#define TLS_STATE_REQUESTED     1
#define TLS_STATE_ACTIVE        2

#define RW_FROM_CHILD           0
#define RW_TO_CHILD             1
#define RW_FIRST_AVAILABLE      2

#define ENVIRONMENT_LOCAL_PORT  "TCPLOCALPORT"
#define ENVIRONMENT_REMOTE_IP   "TCPREMOTEIP"
#define ENVIRONMENT_REMOTE_NAME "TCPREMOTEHOST"

#define SETENV_LOCAL_PORT       "TCPLOCALPORT=25"
#define SETENV_REMOTE_IP        "TCPREMOTEIP=127.0.0.1"
#define SETENV_REMOTE_NAME      "TCPREMOTEHOST=localhost"

#define TLS_CORRUPT_COUNT       256
#define TLS_CORRUPT_DATA        "foobarbazqux"

void usage()
  {
  printf(
    "USAGE: sendrecv [ -s ] [ -S ] [ -b DATA_BURST_BYTES ] [ -B POST_DATA_BURST_BYTES ] [ -c UNCORRUPTED_BYTES_TO_SEND ] [ -d INITIAL_DELAY ] [ -M MAX_DATA_BYTES_TO_SEND ] [ -t TIMEOUT_SECS ] [ -r DESIRED_RESPONSE ] [ -u USERNAME -p PASSWORD ] [ -w WRITE_DELAY_SECS ] [ -W BURST_WRITE_DELAY_SECS ] [ -- ] COMMANDLINE [ ARG1 ARG2 ... ]\n"
    "\n"
    "-s\n"
    "  Start SSL at the beginning of the session (SMTPS).\n"
    "\n"
    "-S\n"
    "  Do not start a TLS session, even if the input includes the STARTTLS command.\n"
    "\n"
    "-b DATA_BURST_BYTES\n"
    "  After the DATA command but before the end of the message data, send message\n"
    "  data in bursts of DATA_BURST_BYTES bytes instead of one line at a time.\n"
    "\n"
    "-B POST_DATA_BURST_BYTES\n"
    "  After the end of the message data, send all remaining commands in bursts of\n"
    "  POST_DATA_BURST_BYTES bytes instead of one line at a time.\n"
    "\n"
    "-c UNCORRUPTED_BYTES_TO_SEND\n"
    "  After a TLS/SSL session has been established and UNCORRUPTED_BYTES_TO_SEND\n"
    "  bytes have been sent, send a batch of garbage data to deliberately corrupt\n"
    "  the TLS/SSL stream.  Has no effect if TLS/SSL is not in use.\n"
    "\n"
    "-d INITIAL_DELAY\n"
    "  Wait INITIAL_DELAY seconds before sending any data to the child process.\n"
    "  If missing, sendrecv will wait for a greeting banner.  Set INITIAL_DELAY to 0\n"
    "  to send data before the child process sends a greeting banner.\n"
    "\n"
    "-M MAX_DATA_BYTES_TO_SEND\n"
    "  Exit after sending MAX_DATA_BYTES_TO_SEND bytes to the child process, total.\n"
    "\n"
    "-t TIMEOUT_SECS\n"
    "  Kill the child process if it sends no data for TIMEOUT_SECS seconds (or more).\n"
    "  Defaults to " STRINGIFY(DEFAULT_TIMEOUT_SECS) ".\n"
    "\n"
    "-r DESIRED_RESPONSE\n"
    "  Exit if the child process sends a line that starts with DESIRED_RESPONSE.\n"
    "\n"
    "-u USERNAME\n"
    "  When using CRAM_MD5 authentication, use USERNAME as the username.\n"
    "\n"
    "-p PASSWORD\n"
    "  When using CRAM_MD5 authentication, use PASSWORD as the password.\n"
    "\n"
    "-w WRITE_DELAY_SECS\n"
    "  Send data to the child process no more often than every WRITE_DELAY_SECS\n"
    "  seconds, to simulate a slow link.  Defaults to " STRINGIFY(DEFAULT_WRITE_DELAY_SECS) ".\n"
    "\n"
    "-W BURST_WRITE_DELAY_SECS\n"
    "  When bursting data to the child process, send a burst no more often than\n"
    "  BURST_WRITE_DELAY_SECS seconds, to simulate a slow link.\n"
    );

  exit(0);

  return;
  }

int cram_md5(char *return_text, char *username, char *password, char *encoded_challenge)
  {
  unsigned char challenge[MAX_BUF + 1];
  int strlen_challenge;
  unsigned char ipad[MAX_BUF + 1];
  unsigned char opad[MAX_BUF + 1];
  unsigned char result[16];
  unsigned char prepend[MAX_BUF + 1];
  int strlen_prepend;
  unsigned char secret[64];
  int i;

  for (i = 0; i < 64; i++)
    {
    ipad[i] = IPAD_BYTE;
    opad[i] = OPAD_BYTE;
    }

  if (strlen(password) > 64)
    {
    md5(secret, (unsigned char *)password, strlen(password));
    for (i = 16; i < 64; i++)
      secret[i] = '\0';
    }
  else
    {
    strncpy((char *)secret, password, strlen(password));
    for (i = strlen(password); i < 64; i++)
      secret[i] = '\0';
    }

  for (i = 0; i < 64; i++)
    {
    ipad[i] ^= secret[i];
    opad[i] ^= secret[i];
    }

  strlen_challenge = base64_decode(challenge, MAX_BUF, (unsigned char *)encoded_challenge, strlen(encoded_challenge));

  for (i = 0; i < strlen_challenge; i++)
    ipad[i + 64] = challenge[i];

  md5(opad + 64, ipad, strlen_challenge + 64);
  md5(result, opad, 80);

  strlen_prepend = snprintf((char *)prepend, MAX_BUF, "%s ", username);
  for (i = 0; i < 16; i++)
    snprintf((char *)(prepend + strlen_prepend + (i * 2)), MAX_BUF, "%.2x", result[i]);
  strlen_prepend += 32;

  return(base64_encode((unsigned char *)return_text, MAX_BUF, prepend, strlen_prepend));
  }

void tls_error(SSL *tls_session, int return_code)
  {
  char error_text[MAX_BUF + 1];
  int strlen_error_text;
  int saved_errno;
  int tls_error;
  int get_error;

  saved_errno = errno;
  strlen_error_text = 0;

  get_error = SSL_get_error(tls_session, return_code);

  error_text[0] = '\0';
  while (((tls_error = ERR_get_error()) != 0) &&
         (strlen_error_text < MAX_BUF))
    {
    if (strlen_error_text > 0)
      strlen_error_text += snprintf(error_text + strlen_error_text, MAX_BUF - strlen_error_text, ", ");

    ERR_error_string_n(tls_error, error_text + strlen_error_text, MAX_BUF - strlen_error_text);
    strlen_error_text += strlen(error_text + strlen_error_text);
    }

  switch (get_error)
    {
    case SSL_ERROR_NONE:
      // No error occurred.
      printf("(No SSL error)\n");
      break;
    case SSL_ERROR_ZERO_RETURN:
      // SSL connection closed
      printf("(SSL connection closed: %s)\n", error_text);
      break;
    case SSL_ERROR_WANT_READ:
    case SSL_ERROR_WANT_WRITE:
      // operation did not complete, call it again
    case SSL_ERROR_WANT_CONNECT:
    case SSL_ERROR_WANT_ACCEPT:
      // operation did not complete, call it again
    case SSL_ERROR_WANT_X509_LOOKUP:
      // callback function wants another callback.  Call the SSL function again.
      printf("(SSL function wants a callback: %s)\n", error_text);
      break;
    case SSL_ERROR_SYSCALL:
      // check the SSL error queue.  If return_code == 0, EOF found.  If return_code == -1, check errno.
      if (return_code == 0)
        strlen_error_text += snprintf(error_text + strlen_error_text, MAX_BUF - strlen_error_text, ", %s", "TLS EOF found");
      else if (return_code == -1)
        strlen_error_text += snprintf(error_text + strlen_error_text, MAX_BUF - strlen_error_text, ", %s", strerror(saved_errno));

      printf("(SSL failed due to a syscall: %s)\n", error_text);

      break;
    case SSL_ERROR_SSL:
      // Library failure, check the SSL error queue.
      printf("(SSL library failed: %s)\n", error_text);
      break;
    default:
      printf("(SSL unknown error: %s)\n", error_text);
      break;
    }

  fflush(NULL);

  return;
  }

int main(int argc, char *argv[], char *envp[])
  {
  int return_value;
  int i;
  int child_pid;
  int child_stdout_pipe[2];
  int child_stdin_pipe[2];
  int timeout_secs;
  int opt;
  int tmp_int;
  char *desired_response;
  int strlen_desired_response;
  fd_set read_fds;
  fd_set loop_read_fds;
  struct timeval tmp_timeout;
  char *username;
  char *password;
  int tls_state;
  SSL_CTX *tls_context;
  SSL *tls_session;
  int initial_delay_secs;
  int write_delay_secs;
  int write_delay_secs_saved;
  char stdin_buf[MAX_INPUT_BUF + 1];
  char *stdin_ptr;
  int strlen_stdin_buf;
  char insert_buf[MAX_INPUT_BUF + 1];
  int strlen_insert_buf;
  time_t last_child_read_time;
  time_t last_write_time;
  time_t start_time;
  int next_readwrite;
  int exit_loop;
  int read_result;
  char stdout_buf[MAX_INPUT_BUF + 1];
  char *stdout_ptr;
  char *next_terminator;
  int inside_data;
  int post_data;
  int stdin_closed;
  int ssl_return;
  int max_fd;
  struct timespec tmp_timespec;
  time_t current_time;
  int burst_data;
  int burst_post_data;
  int tls_bytes_sent;
  int corrupt_tls;
  int burst_delay_secs;
  int start_smtps;
  int max_data_bytes;
  int total_data_bytes;
  int tmp_bytes;
  int start_tls;

  opterr = 0;

  return_value = 0;
  timeout_secs = DEFAULT_TIMEOUT_SECS;
  write_delay_secs = DEFAULT_WRITE_DELAY_SECS;
  strlen_desired_response = 0;
  desired_response = NULL;
  username = NULL;
  password = NULL;
  tls_state = TLS_STATE_NONE;
  tls_context = NULL;
  tls_session = NULL;
  initial_delay_secs = -1;
  burst_data = 0;
  corrupt_tls = 0;
  tls_bytes_sent = 0;
  burst_post_data = 0;
  post_data = 0;
  burst_delay_secs = 0;
  max_data_bytes = 0;
  total_data_bytes = 0;
  start_tls = 1;
  start_smtps = 0;

  while ((opt = getopt(argc, argv, "b:B:c:d:M:p:r:sSt:u:w:W:")) != -1)
    {
    switch (opt)
      {
      case 'b':
        if ((sscanf(optarg, "%d", &tmp_int) == 1) &&
            (tmp_int >= 0))
          burst_data = tmp_int;

        break;
      case 'B':
        if ((sscanf(optarg, "%d", &tmp_int) == 1) &&
            (tmp_int >= 0))
          burst_post_data = tmp_int;

        break;
      case 'c':
        if (sscanf(optarg, "%d", &tmp_int) == 1)
          corrupt_tls = tmp_int;

        break;
      case 'd':
        if ((sscanf(optarg, "%d", &tmp_int) == 1) &&
            (tmp_int >= 0))
          initial_delay_secs = tmp_int;

        break;
      case 'M':
        if ((sscanf(optarg, "%d", &tmp_int) == 1) &&
            (tmp_int >= 0))
          max_data_bytes = tmp_int;

        break;
      case 'p':
        password = optarg;
        break;
      case 's':
        start_smtps = 1;
        break;
      case 'S':
        start_tls = 0;
        break;
      case 't':
        if ((sscanf(optarg, "%d", &tmp_int) == 1) &&
            (tmp_int >= 0))
          timeout_secs = tmp_int;

        break;
      case 'r':
        strlen_desired_response = ((desired_response = optarg) != NULL) ? strlen(desired_response) : 0;

        break;
      case 'u':
        username = optarg;
        break;
      case 'w':
        if ((sscanf(optarg, "%d", &tmp_int) == 1) &&
            (tmp_int >= 0))
          write_delay_secs = tmp_int;

        break;
      case 'W':
        if ((sscanf(optarg, "%d", &tmp_int) == 1) &&
            (tmp_int >= 0))
          burst_delay_secs = tmp_int;

        break;
      default:
        usage();
        break;
      }
    }

  if (argv[optind] == NULL)
    usage();

  if (!pipe(child_stdout_pipe) != -1)
    if (!pipe(child_stdin_pipe) != -1)
      if ((child_pid = fork()) > 0)
        {
        close(child_stdout_pipe[1]);
        close(child_stdin_pipe[0]);

        stdin_closed = 0;
        current_time = time(NULL);
        start_time = current_time;
        last_child_read_time = current_time;
        last_write_time = 0;
        exit_loop = 0;
        inside_data = 0;
        switch (initial_delay_secs)
          {
          case -1:
            next_readwrite = RW_FROM_CHILD;
            break;
          case 0:
            next_readwrite = RW_TO_CHILD;
            break;
          default:
            next_readwrite = RW_FIRST_AVAILABLE;
          }

        stdin_ptr = NULL;
        strlen_stdin_buf = 0;

        if (start_smtps)
          {
          printf("(Negotiating SSL session. This may take a moment.)\n");
          fflush(NULL);

          SSL_library_init();
          if (((tls_context = SSL_CTX_new(SSLv23_client_method())) != NULL) &&
              ((tls_session = SSL_new(tls_context)) != NULL) &&
              SSL_set_rfd(tls_session, child_stdout_pipe[0]) &&
              SSL_set_wfd(tls_session, child_stdin_pipe[1]) &&
              ((ssl_return = SSL_connect(tls_session)) == 1))
            {
            printf("(SSL session started.)\n");
            fflush(NULL);
            tls_state = TLS_STATE_ACTIVE;
            }
          else
            {
            tls_error(tls_session, ssl_return);

            if (tls_session != NULL)
              {
              SSL_free(tls_session);
              tls_session = NULL;
              }

            if (tls_context != NULL)
              {
              SSL_CTX_free(tls_context);
              tls_context = NULL;
              }

            tls_state = TLS_STATE_NONE;
            }
          }

        tmp_timeout.tv_usec = 0;
        tmp_timeout.tv_sec = (initial_delay_secs > -1) ? initial_delay_secs : timeout_secs;

        while ((current_time - last_child_read_time) < timeout_secs)
          {
          FD_ZERO(&read_fds);
          max_fd = -1;
          if (!stdin_closed)
            {
            FD_SET(STDIN_FD, &read_fds);
            max_fd = STDIN_FD;
            }
          if ((tls_state != TLS_STATE_ACTIVE) ||
              (SSL_pending(tls_session) <= 0))
            {
            FD_SET(child_stdout_pipe[0], &read_fds);
            max_fd = child_stdout_pipe[0];
            }

          if (((tls_state == TLS_STATE_ACTIVE) &&
               (SSL_pending(tls_session) > 0)) ||
              (select(max_fd + 1, &read_fds, NULL, NULL, &tmp_timeout) != -1))
            {
            /* Any time input is available from the child process, read and print it. */
            if (((tls_state == TLS_STATE_ACTIVE) &&
                 (SSL_pending(tls_session) > 0)) ||
                FD_ISSET(child_stdout_pipe[0], &read_fds))
              {
              do
                {
                if (((tls_state == TLS_STATE_ACTIVE) &&
                     ((read_result = SSL_read(tls_session, stdout_buf, MAX_INPUT_BUF)) > 0)) ||
                    ((tls_state == TLS_STATE_REQUESTED) &&
                     ((read_result = read(child_stdout_pipe[0], stdout_buf, STRLEN(CODE_STARTTLS_SUCCESS))) > 0)) ||
                    ((tls_state != TLS_STATE_ACTIVE) &&
                     ((read_result = read(child_stdout_pipe[0], stdout_buf, MAX_INPUT_BUF)) > 0)))
                  {
                  stdout_buf[read_result] = '\0';

                  /* This stops sendrecv from reading too much data from the
                   * pipe.  Some of it may be SSL data. */
                  if (tls_state == TLS_STATE_REQUESTED)
                    {
                    if ((read_result >= STRLEN(CODE_STARTTLS_SUCCESS)) &&
                        (strncasecmp(stdout_buf, CODE_STARTTLS_SUCCESS, STRLEN(CODE_STARTTLS_SUCCESS)) == 0))
                      while (stdout_buf[read_result - 1] != LINE_TERMINATOR)
                        read_result += read(child_stdout_pipe[0], stdout_buf + read_result, 1);
                    else
                      read_result += read(child_stdout_pipe[0], stdout_buf + read_result, MAX_INPUT_BUF - read_result);
                    }

                  stdout_buf[read_result] = '\0';
                  printf("%.*s", read_result, stdout_buf);
                  fflush(NULL);

                  if ((desired_response != NULL) &&
                      ((stdout_ptr = strstr(stdout_buf, desired_response)) != NULL) &&
                      ((stdout_ptr == stdout_buf) ||
                       ((stdout_ptr - 1)[0] == LINE_TERMINATOR)))
                    {
                    printf("(Desired response found.)\n");
                    fflush(NULL);
                    return_value = 1;
                    exit_loop = 1;
                    break;
                    }
                  /* NOTE: This assumes the entire response is grabbed with one read().  It's possible multiple read()s are required. */
                  else if ((tls_state == TLS_STATE_REQUESTED) &&
                           (read_result >= STRLEN(CODE_STARTTLS_SUCCESS)) &&
                           (strncasecmp(stdout_buf, CODE_STARTTLS_SUCCESS, STRLEN(CODE_STARTTLS_SUCCESS)) == 0) &&
                           (stdout_buf[read_result - 1] == LINE_TERMINATOR))
                    {
                    if (corrupt_tls >= 0)
                      {
                      printf("(Negotiating TLS session. This may take a moment.)\n");
                      fflush(NULL);

                      SSL_library_init();
                      if (((tls_context = SSL_CTX_new(SSLv23_client_method())) != NULL) &&
                          ((tls_session = SSL_new(tls_context)) != NULL) &&
                          SSL_set_rfd(tls_session, child_stdout_pipe[0]) &&
                          SSL_set_wfd(tls_session, child_stdin_pipe[1]) &&
                          ((ssl_return = SSL_connect(tls_session)) == 1))
                        {
                        printf("(TLS session started.)\n");
                        fflush(NULL);
                        tls_state = TLS_STATE_ACTIVE;
                        }
                      else
                        {
                        tls_error(tls_session, ssl_return);

                        if (tls_session != NULL)
                          {
                          SSL_free(tls_session);
                          tls_session = NULL;
                          }

                        if (tls_context != NULL)
                          {
                          SSL_CTX_free(tls_context);
                          tls_context = NULL;
                          }

                        tls_state = TLS_STATE_NONE;
                        }
                      }
                    else
                      {
                      printf("(Corrupting TLS session. This may take a moment.)\n");
                      fflush(NULL);

                      for (i = 0; i < TLS_CORRUPT_COUNT; i++)
                        write(child_stdin_pipe[1], TLS_CORRUPT_DATA, STRLEN(TLS_CORRUPT_DATA));
                      }
                    }
                  else if ((tls_state == TLS_STATE_REQUESTED) &&
                           (((read_result >= STRLEN(CODE_STARTTLS_ERROR4)) &&
                             (strncasecmp(stdout_buf, CODE_STARTTLS_ERROR4, STRLEN(CODE_STARTTLS_ERROR4)) == 0)) ||
                            ((read_result >= STRLEN(CODE_STARTTLS_ERROR5)) &&
                             (strncasecmp(stdout_buf, CODE_STARTTLS_ERROR5, STRLEN(CODE_STARTTLS_ERROR5)) == 0))))
                    {
                    printf("(Unable to start TLS session -- child process returned error code.)\n");
                    fflush(NULL);
                    tls_state = TLS_STATE_NONE;
                    }
                  else if ((username != NULL) &&
                           (password != NULL) &&
                           (read_result >= STRLEN(CODE_CRAMMD5_CHALLENGE)) &&
                           (strncasecmp(stdout_buf, CODE_CRAMMD5_CHALLENGE, STRLEN(CODE_CRAMMD5_CHALLENGE)) == 0) &&
                           (stdout_buf[read_result - 1] == LINE_TERMINATOR))
                    {
                    for (; read_result > 0; read_result--)
                      if (!isspace((int)stdout_buf[read_result - 1]))
                        {
                        stdout_buf[read_result] = '\0';
                        break;
                        }

                    strlen_insert_buf = cram_md5(insert_buf, username, password, stdout_buf + STRLEN(CODE_CRAMMD5_CHALLENGE));
                    strncpy(insert_buf + strlen_insert_buf, LINE_TERMINATOR_STR, STRLEN(LINE_TERMINATOR_STR));
                    strlen_insert_buf += STRLEN(LINE_TERMINATOR_STR);
                    insert_buf[strlen_insert_buf] = '\0';
                    }
                  }
                else if (read_result == 0)
                  {
                  exit_loop = 1;
                  break;
                  }
                else if (read_result == -1)
                  {
                  printf("(Unable to read from stdin: %s)\n", strerror(errno));
                  fflush(NULL);
                  exit_loop = 1;
                  break;
                  }

                FD_ZERO(&loop_read_fds);
                FD_SET(child_stdout_pipe[0], &loop_read_fds);

                tmp_timeout.tv_sec = 0;
                tmp_timeout.tv_usec = 0;
                }
              while (select(child_stdout_pipe[0] + 1, &loop_read_fds, NULL, NULL, &tmp_timeout) > 0);

              if (exit_loop)
                break;

              last_child_read_time = time(NULL);
              next_readwrite = RW_TO_CHILD;
              }

            /* Any time input is available from stdin, read and buffer it. */
            read_result = -2;
            if (FD_ISSET(STDIN_FD, &read_fds) &&
                (strlen_stdin_buf < MAX_INPUT_BUF) &&
                ((read_result = read(STDIN_FD, stdin_buf + strlen_stdin_buf, MAX_INPUT_BUF - strlen_stdin_buf)) > 0))
              {
              strlen_stdin_buf += read_result;
              stdin_buf[strlen_stdin_buf] = '\0';

              if (stdin_ptr == NULL)
                stdin_ptr = stdin_buf;
              }
            else if (read_result == 0)
              stdin_closed = 1;
            else if (read_result == -1)
              {
              printf("(Unable to read from stdin: %s)\n", strerror(errno));
              fflush(NULL);
              break;
              }
            }
          else
            {
            printf("(ERROR: Unable to select: %s)\n", strerror(errno));
            fflush(NULL);
            break;
            }

          /*
           * Write to child if:
           *   Data has been buffered to write
           *   The child is expecting data
           *   Enough time has passed since program start (initial delay)
           *   Enough time has passed since last write (write delay)
           */
          current_time = time(NULL);
          if (((strlen_insert_buf > 0) ||
               (stdin_ptr != NULL)) &&
              ((next_readwrite != RW_FROM_CHILD) ||
               inside_data) &&
              (current_time >= (start_time + initial_delay_secs)) &&
              ((current_time - last_write_time) >= write_delay_secs))
            {
            if (strlen_insert_buf > 0)
              {
              if (tls_state == TLS_STATE_ACTIVE)
                if (!corrupt_tls)
                  SSL_write(tls_session, insert_buf, strlen_insert_buf);
                else if (corrupt_tls >= 0)
                  {
                  if (corrupt_tls > tls_bytes_sent)
                    tls_bytes_sent += SSL_write(tls_session, insert_buf, MAXVAL(strlen_insert_buf, corrupt_tls - tls_bytes_sent));

                  if (corrupt_tls <= tls_bytes_sent)
                    tls_bytes_sent += write(child_stdin_pipe[1], insert_buf, strlen_insert_buf);
                  }
                else
                  write(child_stdin_pipe[1], insert_buf, strlen_insert_buf);
              else
                write(child_stdin_pipe[1], insert_buf, strlen_insert_buf);

              next_readwrite = RW_FROM_CHILD;
              strlen_insert_buf = 0;
              last_write_time = time(NULL);
              }
            else
              {
              if ((inside_data &&
                   (burst_data > 0) &&
                   (((burst_post_data == 0) &&
                     ((next_terminator = strstr(stdin_ptr, SMTP_DATA_END_MULTILINE)) != NULL)) ||
                    ((next_terminator = (stdin_buf + strlen_stdin_buf)) != NULL)) &&
                   ((next_terminator < (stdin_ptr + burst_data + burst_post_data)) ||
                    ((next_terminator = (stdin_ptr + burst_data + burst_post_data)) != NULL))) ||
                  (post_data &&
                   (burst_post_data > 0) &&
                   ((next_terminator = (stdin_buf + strlen_stdin_buf)) != NULL) &&
                   ((next_terminator < (stdin_ptr + burst_post_data)) ||
                    ((next_terminator = (stdin_ptr + burst_post_data)) != NULL))) ||
                  ((next_terminator = memchr(stdin_ptr, LINE_TERMINATOR, strlen_stdin_buf - (stdin_ptr - stdin_buf))) != NULL) ||
                  (((strlen_stdin_buf >= MAX_INPUT_BUF) &&
                    (stdin_ptr == stdin_buf)) &&
                   ((next_terminator = (stdin_buf + strlen_stdin_buf)) != NULL)))
                {
                next_terminator++;

                if (tls_state == TLS_STATE_ACTIVE)
                  if (!corrupt_tls)
                    {
                    tmp_bytes = (inside_data && (max_data_bytes > 0)) ? (((total_data_bytes + (next_terminator - stdin_ptr)) > max_data_bytes) ? (max_data_bytes - total_data_bytes) : (next_terminator - stdin_ptr)) : (next_terminator - stdin_ptr);
                    if (inside_data)
                      total_data_bytes += tmp_bytes;
                    SSL_write(tls_session, stdin_ptr, tmp_bytes);
                    if ((max_data_bytes > 0) &&
                        (total_data_bytes >= max_data_bytes))
                      {
                      printf("(Sent %d data bytes total)\n", total_data_bytes);
                      fflush(NULL);
                      exit(0);
                      }
                    }
                  else if (corrupt_tls >= 0)
                    {
                    if (corrupt_tls > tls_bytes_sent)
                      {
                      tmp_bytes = (inside_data && (max_data_bytes > 0)) ? (((total_data_bytes + MAXVAL(next_terminator - stdin_ptr, corrupt_tls - tls_bytes_sent)) > max_data_bytes) ? (max_data_bytes - total_data_bytes) : MAXVAL(next_terminator - stdin_ptr, corrupt_tls - tls_bytes_sent)) : MAXVAL(next_terminator - stdin_ptr, corrupt_tls - tls_bytes_sent);
                      if (inside_data)
                        total_data_bytes += tmp_bytes;
                      tls_bytes_sent += SSL_write(tls_session, stdin_ptr, tmp_bytes);
                      if ((max_data_bytes > 0) &&
                          (total_data_bytes >= max_data_bytes))
                        {
                        printf("(Sent %d data bytes total)\n", total_data_bytes);
                        fflush(NULL);
                        exit(0);
                        }
                      }

                    if (corrupt_tls <= tls_bytes_sent)
                      {
                      tmp_bytes = (inside_data && (max_data_bytes > 0)) ? (((total_data_bytes + (next_terminator - stdin_ptr)) > max_data_bytes) ? (max_data_bytes - total_data_bytes) : (next_terminator - stdin_ptr)) : (next_terminator - stdin_ptr);
                      if (inside_data)
                        total_data_bytes += tmp_bytes;
                      tls_bytes_sent += write(child_stdin_pipe[1], stdin_ptr, tmp_bytes);
                      if ((max_data_bytes > 0) &&
                          (total_data_bytes >= max_data_bytes))
                        {
                        printf("(Sent %d data bytes total)\n", total_data_bytes);
                        fflush(NULL);
                        exit(0);
                        }
                      }
                    }
                  else
                    {
                    tmp_bytes = (inside_data && (max_data_bytes > 0)) ? (((total_data_bytes + strlen_stdin_buf) > max_data_bytes) ? (max_data_bytes - total_data_bytes) : strlen_stdin_buf) : strlen_stdin_buf;
                    if (inside_data)
                      total_data_bytes += tmp_bytes;
                    write(child_stdin_pipe[1], stdin_buf, tmp_bytes);
                    if ((max_data_bytes > 0) &&
                        (total_data_bytes >= max_data_bytes))
                      {
                      printf("(Sent %d data bytes total)\n", total_data_bytes);
                      fflush(NULL);
                      exit(0);
                      }
                    }
                else
                  {
                  tmp_bytes = (inside_data && (max_data_bytes > 0)) ? (((total_data_bytes + (next_terminator - stdin_ptr)) > max_data_bytes) ? (max_data_bytes - total_data_bytes) : (next_terminator - stdin_ptr)) : (next_terminator - stdin_ptr);
                  if (inside_data)
                    total_data_bytes += tmp_bytes;
                  write(child_stdin_pipe[1], stdin_ptr, tmp_bytes);
                  if ((max_data_bytes > 0) &&
                      (total_data_bytes >= max_data_bytes))
                    {
                    printf("(Sent %d data bytes total)\n", total_data_bytes);
                    fflush(NULL);
                    exit(0);
                    }
                  }

                if ((strlen_stdin_buf < MAX_INPUT_BUF) ||
                    (stdin_ptr != stdin_buf))
                  next_readwrite = RW_FROM_CHILD;
                current_time = time(NULL);

                if (!inside_data &&
                    ((next_terminator - stdin_ptr) >= (STRLEN(SMTP_DATA) + 1)) &&
                    (strncasecmp(stdin_ptr, SMTP_DATA, STRLEN(SMTP_DATA)) == 0) &&
                    ((stdin_ptr[STRLEN(SMTP_DATA)] == LINE_TERMINATOR_CR) ||
                     (stdin_ptr[STRLEN(SMTP_DATA)] == LINE_TERMINATOR)))
                  {
                  inside_data = 1;
                  next_readwrite = RW_TO_CHILD;

                  if (burst_data > 0)
                    {
                    write_delay_secs_saved = write_delay_secs;
                    write_delay_secs = burst_delay_secs;
                    }
                  }
                else if (inside_data &&
                         ((next_terminator - stdin_ptr) >= (STRLEN(SMTP_DATA_END) + 1)) &&
                         (strncasecmp(stdin_ptr, SMTP_DATA_END, STRLEN(SMTP_DATA_END)) == 0) &&
                         ((stdin_ptr[STRLEN(SMTP_DATA_END)] == LINE_TERMINATOR_CR) ||
                          (stdin_ptr[STRLEN(SMTP_DATA_END)] == LINE_TERMINATOR)))
                  {
                  inside_data = 0;
                  post_data = 1;

                  if (burst_post_data > 0)
                    write_delay_secs = burst_delay_secs;
                  else if (burst_data > 0)
                    write_delay_secs = write_delay_secs_saved;
                  }
                else if (inside_data)
                  last_child_read_time = current_time;
                else if (start_tls &&
                         ((next_terminator - stdin_ptr) >= (STRLEN(SMTP_STARTTLS) + 1)) &&
                         (strncasecmp(stdin_ptr, SMTP_STARTTLS, STRLEN(SMTP_STARTTLS)) == 0) &&
                         ((stdin_ptr[STRLEN(SMTP_STARTTLS)] == LINE_TERMINATOR_CR) ||
                          (stdin_ptr[STRLEN(SMTP_STARTTLS)] == LINE_TERMINATOR)))
                  tls_state = TLS_STATE_REQUESTED;

                last_write_time = current_time;
                stdin_ptr = next_terminator;
                }

              if ((stdin_ptr - stdin_buf) >= strlen_stdin_buf)
                {
                /* Used entire buffer, reset pointers */
                stdin_ptr = NULL;
                strlen_stdin_buf = 0;
                }
              else if (strlen_stdin_buf >= MAX_INPUT_BUF)
                {
                /* Buffer is full */
                if (stdin_ptr > stdin_buf)
                  {
                  /* Unused space at the start, move data from the end to the start */
                  strlen_stdin_buf -= stdin_ptr - stdin_buf;
                  memmove(stdin_buf, stdin_ptr, strlen_stdin_buf);
                  stdin_ptr = stdin_buf;
                  }
                /* Buffer cannot be completely full because above if() clause handles that case. */
                }
              else if ((next_terminator == NULL) &&
                       (stdin_ptr != stdin_buf))
                {
                /* No terminator found and buffer is not full, move data */
                strlen_stdin_buf -= stdin_ptr - stdin_buf;
                memmove(stdin_buf, stdin_ptr, strlen_stdin_buf);
                stdin_ptr = NULL;
                }
              }
            }

          current_time = time(NULL);
          tmp_timeout.tv_usec = 0;
          tmp_timeout.tv_sec = (start_time + initial_delay_secs) - current_time;
          if (tmp_timeout.tv_sec <= 0)
            {
            tmp_timeout.tv_sec = MAXVAL(0, timeout_secs - (current_time - last_child_read_time));
            if ((stdin_ptr != NULL) &&
                ((next_readwrite != RW_FROM_CHILD) ||
                 inside_data) &&
                ((write_delay_secs - (current_time - last_write_time)) < tmp_timeout.tv_sec))
              tmp_timeout.tv_sec = MAXVAL(0, write_delay_secs - (current_time - last_write_time));
            }

          tmp_timespec.tv_sec = 0;
          tmp_timespec.tv_nsec = 10000000;
          nanosleep(&tmp_timespec, NULL);
          }

        if ((current_time - last_child_read_time) >= timeout_secs)
          {
          kill(child_pid, SIGKILL);
          printf("(Timeout.  Child process killed.)\n");
          fflush(NULL);
          }
        else if (!return_value &&
                 (waitpid(child_pid, NULL, WNOHANG) == child_pid))
          {
          printf("(Child process exited, desired response not found.)\n");
          fflush(NULL);
          }

        if (tls_state == TLS_STATE_ACTIVE)
          {
          if ((SSL_get_shutdown(tls_session) & SSL_RECEIVED_SHUTDOWN) == 0)
            {
            if (waitpid(child_pid, NULL, WNOHANG) == 0)
              SSL_shutdown(tls_session);
            else
              {
              printf("(Child process exited before SSL session could be properly shut down.)\n");
              fflush(NULL);
              }
            }

          if (tls_session != NULL)
            {
            SSL_free(tls_session);
            tls_session = NULL;
            }

          if (tls_context != NULL)
            {
            SSL_CTX_free(tls_context);
            tls_context = NULL;
            }

          printf("(TLS session closed.)\n");
          fflush(NULL);
          }

        close(child_stdout_pipe[0]);
        close(child_stdin_pipe[1]);

        waitpid(child_pid, NULL, WNOHANG);
        }
      else if (child_pid == 0)
        {
        close(child_stdout_pipe[0]);
        close(child_stdin_pipe[1]);

        if ((dup2(child_stdout_pipe[1], STDOUT_FD) != -1) &&
            (dup2(child_stdin_pipe[0], STDIN_FD) != -1))
          {
          if (execve(argv[optind], argv + optind, envp) == -1)
            {
            printf("(Unable to exec %s: %s)\n", argv[optind], strerror(errno));
            fflush(NULL);

            close(child_stdout_pipe[1]);
            close(child_stdin_pipe[0]);
            }
          }
        else
          {
          printf("(Unable to reassign file descriptor: %s)\n", strerror(errno));
          fflush(NULL);

          close(child_stdout_pipe[1]);
          close(child_stdin_pipe[0]);
          }
        }
      else
        {
        printf("(Unable to fork: %s)\n", strerror(errno));
        fflush(NULL);

        close(child_stdout_pipe[0]);
        close(child_stdout_pipe[1]);
        close(child_stdin_pipe[0]);
        close(child_stdin_pipe[1]);
        }
    else
      {
      printf("(Unable to open pipe: %s)\n", strerror(errno));
      fflush(NULL);

      close(child_stdout_pipe[0]);
      close(child_stdout_pipe[1]);
      }
  else
    {
    printf("(Unable to open pipe: %s)\n", strerror(errno));
    fflush(NULL);
    }

  return(return_value);
  }
