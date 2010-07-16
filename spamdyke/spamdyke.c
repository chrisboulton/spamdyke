/*
  spamdyke -- a filter for stopping spam at connection time.
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
#include <ctype.h>
#include <errno.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/select.h>
#include <sys/wait.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <signal.h>
#include <grp.h>
#include <pwd.h>
#include <fcntl.h>

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

#include "spamdyke.h"
#include "tls.h"
#include "dns.h"
#include "environment.h"
#include "configuration.h"
#include "usage.h"
#include "search_fs.h"
#include "exec.h"
#include "base64.h"
#include "log.h"
#include "config_test.h"
#include "filter.h"

/*
 * Expects:
 *   strlen_haystack == strlen(haystack)
 *   pos <= strlen(haystack) - 1
 *
 * Return value:
 *   ERROR: not possible
 *   SUCCESS: the position of the first character beyond the foldable whitespace that begins at pos
 */
int skip_fws(char *haystack, int strlen_haystack, int pos)
  {
  int return_value;

  return_value = pos;
  while ((return_value < strlen_haystack) &&
         ((haystack[return_value] == ' ') ||
          (haystack[return_value] == '\t') ||
          (haystack[return_value] == '\r') ||
          (haystack[return_value] == '\n')))
    return_value++;

  return(return_value);
  }

/*
 * Expects:
 *   strlen_haystack == strlen(haystack)
 *   pos <= strlen(haystack) - 1
 *
 * Return value:
 *   ERROR: not possible
 *   SUCCESS: the position of the first character beyond the comment/foldable whitespace that begins at pos
 */
int skip_cfws(char *haystack, int strlen_haystack, int pos)
  {
  int return_value;
  int new_pos;
  int comment_start;

  return_value = pos;

  while (return_value < strlen_haystack)
    if (((return_value = skip_fws(haystack, strlen_haystack, return_value)) < strlen_haystack) &&
        (haystack[return_value] == '('))
      {
      comment_start = return_value;

      while (return_value < strlen_haystack)
        {
        new_pos = return_value;

        if ((new_pos = skip_fws(haystack, strlen_haystack, new_pos)) < strlen_haystack)
          {
          while (new_pos < strlen_haystack)
            if ((((int)haystack[new_pos] >= 1) &&
                 ((int)haystack[new_pos] <= 8)) ||
                ((int)haystack[new_pos] == 11) ||
                ((int)haystack[new_pos] == 12) ||
                (((int)haystack[new_pos] >= 14) &&
                 ((int)haystack[new_pos] <= 31)) ||
                (((int)haystack[new_pos] >= 33) &&
                 ((int)haystack[new_pos] <= 39)) ||
                (((int)haystack[new_pos] >= 42) &&
                 ((int)haystack[new_pos] <= 91)) ||
                (((int)haystack[new_pos] >= 93) &&
                 ((int)haystack[new_pos] <= 127)))
              new_pos++;
            else
              break;
          }

        if (new_pos != return_value)
          return_value = new_pos;
        else
          break;
        }

      return_value = skip_fws(haystack, strlen_haystack, return_value);

      if (haystack[return_value] == ')')
        return_value++;
      else
        {
        return_value = comment_start;
        break;
        }
      }
    else
      break;

  return(return_value);
  }

/*
 * See RFC 2822 for all the painful details of parsing addresses.
 * Also see http://tools.ietf.org/html/draft-levine-smtp-batv-01 for details on
 * BATV.  Example BATV addresses:
 *   prvs=xxxxxx=USER@DOMAIN.COM
 *   prvs=USER/xxxxxx@DOMAIN.COM
 *
 * Expects:
 *   strlen_haystack == strlen(haystack)
 *   return_username is a preallocated buffer - must not be NULL
 *   max_return_username is the size of the return_username buffer - must be greater than 0
 *   return_domain is a preallocated buffer - must not be NULL
 *   max_return_domain is the size of the return_domain buffer - must be greater than 0
 *   return_strlen_domain may be NULL
 *
 * Return value:
 *   ERROR: 0
 *   SUCCESS: length of the returned username
 */
int find_address(struct filter_settings *current_settings, char *haystack, int strlen_haystack, char *return_username, int max_return_username, char *return_domain, int max_return_domain, int *return_strlen_domain)
  {
  int return_value;
  int i;
  int j;
  int tmp_strlen_domain;
  int pos;
  int saved_pos;
  int search_start;
  int search_end;
  int quoted_string;
  int bracketed_domain;
  char tmp_username[MAX_BUF + 1];
  int strlen_username;
  char tmp_domain[MAX_BUF + 1];
  int strlen_domain;

  return_value = 0;
  tmp_username[0] = '\0';
  strlen_username = 0;
  tmp_domain[0] = '\0';
  strlen_domain = 0;
  search_end = strlen_haystack;

  if ((return_username != NULL) &&
      (max_return_username > 0) &&
      (return_domain != NULL) &&
      (max_return_domain > 0))
    {
    if ((haystack != NULL) &&
        (strlen_haystack > 0))
      {
      for (pos = 0; pos < strlen_haystack; pos++)
        if (haystack[pos] == ':')
          {
          pos++;

          while (pos < strlen_haystack)
            if (haystack[pos] == ' ')
              pos++;
            else if (haystack[pos] == '<')
              {
              while (search_end >= pos)
                if (haystack[search_end] == '>')
                  {
                  pos++;
                  break;
                  }
                else
                  search_end--;

              if (search_end < pos)
                search_end = strlen_haystack;

              break;
              }
            else
              break;

          break;
          }

      quoted_string = 0;
      bracketed_domain = 0;

      while (pos < search_end)
        {
        if (((haystack[pos] >= 'A') &&
             (haystack[pos] <= 'Z')) ||
            ((haystack[pos] >= 'a') &&
             (haystack[pos] <= 'z')) ||
            ((haystack[pos] >= '0') &&
             (haystack[pos] <= '9')) ||
            (haystack[pos] == '.') ||
            (haystack[pos] == '!') ||
            (haystack[pos] == '#') ||
            (haystack[pos] == '$') ||
            (haystack[pos] == '%') ||
            (haystack[pos] == '&') ||
            (haystack[pos] == '\'') ||
            (haystack[pos] == '*') ||
            (haystack[pos] == '+') ||
            (haystack[pos] == '-') ||
            (haystack[pos] == '/') ||
            (haystack[pos] == '=') ||
            (haystack[pos] == '?') ||
            (haystack[pos] == '^') ||
            (haystack[pos] == '_') ||
            (haystack[pos] == '`') ||
            (haystack[pos] == '{') ||
            (haystack[pos] == '}') ||
            (haystack[pos] == '|') ||
            (haystack[pos] == '~') ||
            (haystack[pos] == '"') ||
            (haystack[pos] == '@'))
          break;
        else if ((haystack[pos] == ',') ||
                 (haystack[pos] == ':') ||
                 (haystack[pos] == ';') ||
                 (haystack[pos] == '[') ||
                 (haystack[pos] == ']') ||
                 (haystack[pos] == '\\'))
          {
          SPAMDYKE_LOG_EXCESSIVE(current_settings, LOG_DEBUGX_ADDRESS_ILLEGAL_CHAR, pos, haystack);
          break;
          }

        pos++;
        }

      search_start = pos;

      if (haystack[pos] == '"')
        {
        SPAMDYKE_LOG_EXCESSIVE(current_settings, LOG_DEBUGX_ADDRESS_FOUND_QUOTE_OPEN, pos, haystack);

        pos++;
        while (pos < search_end)
          {
          saved_pos = pos;

          pos = skip_fws(haystack, search_end, pos);

          while ((pos < search_end) &&
                 (strlen_username < MAX_BUF))
            if ((((int)haystack[pos] >= 1) &&
                 ((int)haystack[pos] <= 8)) ||
                ((int)haystack[pos] == 11) ||
                ((int)haystack[pos] == 12) ||
                (((int)haystack[pos] >= 14) &&
                 ((int)haystack[pos] <= 31)))
              {
              SPAMDYKE_LOG_DEBUG(current_settings, LOG_DEBUG_ADDRESS_CONTROL_CHAR, pos, (int)haystack[pos]);

              tmp_username[strlen_username] = haystack[pos];
              tmp_username[strlen_username + 1] = '\0';
              strlen_username++;
              pos++;
              }
            else if (((int)haystack[pos] == 33) ||
                     (((int)haystack[pos] >= 35) &&
                      ((int)haystack[pos] <= 91)) ||
                     (((int)haystack[pos] >= 93) &&
                      ((int)haystack[pos] <= 127)))
              {
              tmp_username[strlen_username] = haystack[pos];
              tmp_username[strlen_username + 1] = '\0';
              strlen_username++;
              pos++;
              }
            else if (((int)haystack[pos] == '\\') &&
                     (pos < (search_end - 1)))
              {
              tmp_username[strlen_username] = haystack[pos + 1];
              tmp_username[strlen_username + 1] = '\0';
              strlen_username++;
              pos += 2;
              }
            else
              break;

          if (saved_pos == pos)
            break;
          }

        pos = skip_fws(haystack, search_end, pos);

        if (haystack[pos] == '"')
          {
          SPAMDYKE_LOG_EXCESSIVE(current_settings, LOG_DEBUGX_ADDRESS_FOUND_QUOTE_CLOSE, pos, haystack);

          pos++;
          quoted_string = 1;
          }
        else
          {
          SPAMDYKE_LOG_EXCESSIVE(current_settings, LOG_DEBUGX_ADDRESS_NO_QUOTE_CLOSE, search_start, haystack);

          tmp_username[0] = '\0';
          strlen_username = 0;
          pos = search_start;
          }
        }

      if (!quoted_string)
        {
        while ((pos < search_end) &&
               (strlen_username < MAX_BUF))
          if (((haystack[pos] >= 'A') &&
               (haystack[pos] <= 'Z')) ||
              ((haystack[pos] >= 'a') &&
               (haystack[pos] <= 'z')) ||
              ((haystack[pos] >= '0') &&
               (haystack[pos] <= '9')) ||
              (haystack[pos] == '.') ||
              (haystack[pos] == '!') ||
              (haystack[pos] == '#') ||
              (haystack[pos] == '$') ||
              (haystack[pos] == '%') ||
              (haystack[pos] == '&') ||
              (haystack[pos] == '\'') ||
              (haystack[pos] == '*') ||
              (haystack[pos] == '+') ||
              (haystack[pos] == '-') ||
              (haystack[pos] == '/') ||
              (haystack[pos] == '=') ||
              (haystack[pos] == '?') ||
              (haystack[pos] == '^') ||
              (haystack[pos] == '_') ||
              (haystack[pos] == '`') ||
              (haystack[pos] == '{') ||
              (haystack[pos] == '}') ||
              (haystack[pos] == '|') ||
              (haystack[pos] == '~'))
            {
            tmp_username[strlen_username] = haystack[pos];
            tmp_username[strlen_username + 1] = '\0';
            strlen_username++;
            pos++;
            }
          else if ((haystack[pos] == '"') ||
                   (haystack[pos] == ',') ||
                   (haystack[pos] == ':') ||
                   (haystack[pos] == ';') ||
                   (haystack[pos] == '[') ||
                   (haystack[pos] == ']') ||
                   (haystack[pos] == '\\'))
            {
            SPAMDYKE_LOG_EXCESSIVE(current_settings, LOG_DEBUGX_ADDRESS_ILLEGAL_CHAR, pos, haystack);

            tmp_username[strlen_username] = haystack[pos];
            tmp_username[strlen_username + 1] = '\0';
            strlen_username++;
            pos++;
            }
          else
            break;

        if ((strlen_username > 0) &&
            ((tmp_username[0] == '.') ||
             (tmp_username[strlen_username - 1] == '.')))
          SPAMDYKE_LOG_EXCESSIVE(current_settings, LOG_DEBUGX_ADDRESS_ILLEGAL_DOT, tmp_username);
        }

      if (strlen_username > 0)
        {
        if (strncmp(tmp_username, BATV_PREFIX, STRLEN(BATV_PREFIX)) == 0)
          {
          for (i = (strlen_username - 1); i >= 0; i--)
            if (tmp_username[i] == '=')
              {
              i++;

              for (j = i; (j < strlen_username) && ((j - i) < max_return_username); j++)
                if (tmp_username[j] != '/')
                  {
                  return_username[j - i] = tmp_username[j];
                  return_value++;
                  }
                else
                  break;

              return_username[return_value] = '\0';
              break;
              }
          }
        else
          {
          return_value = MINVAL(strlen_username, max_return_username);
          memcpy(return_username, tmp_username, sizeof(char) * return_value);
          return_username[return_value] = '\0';
          }
        }
      else
        {
        SPAMDYKE_LOG_DEBUG(current_settings, LOG_DEBUG_ADDRESS_EMPTY_USERNAME, haystack);
        return_username[0] = '\0';
        return_value = 0;
        }

      while ((pos < search_end) &&
             (haystack[pos] != '@'))
        pos++;

      pos++;

      search_start = pos;

      if (haystack[pos] == '[')
        {
        SPAMDYKE_LOG_EXCESSIVE(current_settings, LOG_DEBUGX_ADDRESS_FOUND_BRACKET_OPEN, pos, haystack);

        pos = skip_fws(haystack, search_end, pos);

        while ((pos < search_end) &&
               (strlen_domain < MAX_BUF))
          {
          if ((((int)haystack[pos] >= 1) &&
               ((int)haystack[pos] <= 9)) ||
              ((int)haystack[pos] == 11) ||
              ((int)haystack[pos] == 12) ||
              (((int)haystack[pos] >= 14) &&
               ((int)haystack[pos] <= 31)))
            {
            SPAMDYKE_LOG_DEBUG(current_settings, LOG_DEBUG_ADDRESS_CONTROL_CHAR, pos, (int)haystack[pos]);

            tmp_domain[strlen_domain] = haystack[pos];
            tmp_domain[strlen_domain + 1] = '\0';
            strlen_domain++;
            }
          else if ((((int)haystack[pos] >= 33) &&
                    ((int)haystack[pos] <= 90)) ||
                   (((int)haystack[pos] >= 94) &&
                    ((int)haystack[pos] <= 126)))
            {
            tmp_domain[strlen_domain] = haystack[pos];
            tmp_domain[strlen_domain + 1] = '\0';
            strlen_domain++;
            }
          else if (((int)haystack[pos] == '\\') &&
                   (pos < (search_end - 1)))
            {
            tmp_domain[strlen_domain] = haystack[pos + 1];
            tmp_domain[strlen_domain + 1] = '\0';
            strlen_domain++;
            pos += 2;
            }
          else
            break;
          }

        pos = skip_fws(haystack, search_end, pos);

        if (haystack[pos] == ']')
          {
          SPAMDYKE_LOG_EXCESSIVE(current_settings, LOG_DEBUGX_ADDRESS_FOUND_BRACKET_CLOSE, pos, haystack);

          pos++;
          bracketed_domain = 1;
          }
        else
          {
          SPAMDYKE_LOG_EXCESSIVE(current_settings, LOG_DEBUGX_ADDRESS_NO_BRACKET_CLOSE, search_start, haystack);

          tmp_username[0] = '\0';
          strlen_username = 0;
          pos = search_start;
          }
        }

      if (!bracketed_domain)
        {
        while ((pos < search_end) &&
               (strlen_domain < MAX_BUF))
          if (((haystack[pos] >= 'A') &&
               (haystack[pos] <= 'Z')) ||
              ((haystack[pos] >= 'a') &&
               (haystack[pos] <= 'z')) ||
              ((haystack[pos] >= '0') &&
               (haystack[pos] <= '9')) ||
              (haystack[pos] == '.') ||
              (haystack[pos] == '!') ||
              (haystack[pos] == '#') ||
              (haystack[pos] == '$') ||
              (haystack[pos] == '%') ||
              (haystack[pos] == '&') ||
              (haystack[pos] == '\'') ||
              (haystack[pos] == '*') ||
              (haystack[pos] == '+') ||
              (haystack[pos] == '-') ||
              (haystack[pos] == '/') ||
              (haystack[pos] == '=') ||
              (haystack[pos] == '?') ||
              (haystack[pos] == '^') ||
              (haystack[pos] == '_') ||
              (haystack[pos] == '`') ||
              (haystack[pos] == '{') ||
              (haystack[pos] == '}') ||
              (haystack[pos] == '|') ||
              (haystack[pos] == '~'))
            {
            tmp_domain[strlen_domain] = haystack[pos];
            tmp_domain[strlen_domain + 1] = '\0';
            strlen_domain++;
            pos++;
            }
          else if ((haystack[pos] == '"') ||
                   (haystack[pos] == ',') ||
                   (haystack[pos] == ':') ||
                   (haystack[pos] == ';') ||
                   (haystack[pos] == '[') ||
                   (haystack[pos] == ']') ||
                   (haystack[pos] == '\\'))
            {
            SPAMDYKE_LOG_EXCESSIVE(current_settings, LOG_DEBUGX_ADDRESS_ILLEGAL_CHAR, pos, haystack);
            pos++;
            }
          else
            break;

        if ((strlen_domain > 0) &&
            (tmp_domain[0] == '.'))
          {
          SPAMDYKE_LOG_EXCESSIVE(current_settings, LOG_DEBUGX_ADDRESS_ILLEGAL_DOT_START, tmp_domain);

          for (i = 0; i < strlen_domain; i++)
            if (tmp_domain[i] != '.')
              break;
          i++;

          memmove(tmp_domain, tmp_domain + i, strlen_domain - i);
          strlen_domain -= i;
          tmp_domain[strlen_domain] = '\0';
          }

        if ((strlen_domain > 0) &&
            (tmp_domain[strlen_domain - 1] == '.'))
          {
          SPAMDYKE_LOG_EXCESSIVE(current_settings, LOG_DEBUGX_ADDRESS_ILLEGAL_DOT_END, tmp_domain);

          while ((strlen_domain > 0) &&
                 (tmp_domain[strlen_domain - 1] == '.'))
            strlen_domain--;

          tmp_domain[strlen_domain] = '\0';
          }
        }

      if (strlen_domain > 0)
        {
        tmp_strlen_domain = MINVAL(strlen_domain, max_return_domain);
        memcpy(return_domain, tmp_domain, sizeof(char) * tmp_strlen_domain);
        return_domain[tmp_strlen_domain] = '\0';

        if (return_strlen_domain != NULL)
          *return_strlen_domain = tmp_strlen_domain;

        SPAMDYKE_LOG_DEBUG(current_settings, LOG_DEBUG_FIND_ADDRESS, return_username, return_domain);
        }
      else
        {
        SPAMDYKE_LOG_DEBUG(current_settings, LOG_DEBUG_ADDRESS_EMPTY_DOMAIN, haystack);
        return_domain[0] = '\0';

        if (return_strlen_domain != NULL)
          *return_strlen_domain = 0;
        }
      }
    else
      {
      return_username[0] = '\0';
      return_domain[0] = '\0';

      if (return_strlen_domain != NULL)
        *return_strlen_domain = 0;
      }
    }

  return(return_value);
  }

/*
 * Expects:
 *   target_fd == file descriptor destination or -1 for no output
 *   substitute_reject_severity == SMTP status code or NULL to use status from target_rejection
 *
 * Return value:
 *   ERROR: -1
 *   SUCCESS: bytes output to target_fd
 */
int output_write_rejection(struct filter_settings *current_settings, struct rejection_data *target_rejection, int target_fd, char *substitute_reject_severity)
  {
  int return_value;
  int strlen_buf;
  char output_buf[MAX_BUF + 1];

  if ((current_settings->current_options->policy_location != NULL) &&
      target_rejection->append_policy)
    if (current_settings->current_options->policy_location[current_settings->current_options->strlen_policy_location - 1] == ERROR_URL_DELIMITER_DYNAMIC)
      strlen_buf = snprintf(output_buf, MAX_BUF, "%.*s%.*s" ERROR_URL "%.*s%s" REJECT_CRLF, STRLEN_REJECT_SEVERITY, (substitute_reject_severity == NULL) ? target_rejection->reject_severity : substitute_reject_severity, target_rejection->strlen_reject_message, target_rejection->reject_message, current_settings->current_options->strlen_policy_location, current_settings->current_options->policy_location, target_rejection->short_reject_message);
    else
      strlen_buf = snprintf(output_buf, MAX_BUF, "%.*s%.*s" ERROR_URL "%.*s" ERROR_URL_DELIMITER_STATIC "%s" REJECT_CRLF, STRLEN_REJECT_SEVERITY, (substitute_reject_severity == NULL) ? target_rejection->reject_severity : substitute_reject_severity, target_rejection->strlen_reject_message, target_rejection->reject_message, current_settings->current_options->strlen_policy_location, current_settings->current_options->policy_location, target_rejection->short_reject_message);
  else
    strlen_buf = snprintf(output_buf, MAX_BUF, "%.*s%.*s" REJECT_CRLF, STRLEN_REJECT_SEVERITY, (substitute_reject_severity == NULL) ? target_rejection->reject_severity : substitute_reject_severity, target_rejection->strlen_reject_message, target_rejection->reject_message);

  return_value = output_writeln(current_settings, LOG_ACTION_FILTER_FROM, target_fd, output_buf, strlen_buf);

  fflush(NULL);

  return(return_value);
  }

/*
 * Expects:
 *   inbound_fd == inbound file descriptor from remote server
 *   outbound_fd == outbound file descriptor to remote server
 *
 * Return value:
 *   bitwise OR of FILTER_FLAG values
 */
int smtp_filter(int inbound_fd, int outbound_fd, char *input_line, int strlen_input_line, struct filter_settings *current_settings)
  {

#ifdef HAVE_LIBSSL
  static struct rejection_data tls_success = SUCCESS_TLS;
#endif /* HAVE_LIBSSL */

  static char *environment_hostname[] = ENVIRONMENT_HOSTNAME;
  static int strlen_environment_hostname[] = STRLEN_ENVIRONMENT_HOSTNAME;
  static struct rejection_data smtp_auth_success = SUCCESS_AUTH;
  static char *smtp_auth_types[] = SMTP_AUTH_TYPES;
  static int strlen_smtp_auth_types[] = STRLEN_SMTP_AUTH_TYPES;
  int return_value;
  int i;
  int j;
  int strlen_target_username;
  int strlen_target_domain;
  char decoded_content[MAX_BUF + 1];
  char *content_ptr;
  int strlen_content;
  char username[MAX_BUF + 1];
  char password[MAX_BUF + 1];
  char *tmp_hostname;
  char *tmp_char;
  int tmp_strlen;
  char tmp_sender_address[MAX_ADDRESS + 1];
  char tmp_recipient_address[MAX_ADDRESS + 1];

  return_value = FILTER_FLAG_PASS;

  if (current_settings->inside_data)
    {
    if ((strlen_input_line == STRLEN(SMTP_DATA_END)) &&
        (strncasecmp(SMTP_DATA_END, input_line, STRLEN(SMTP_DATA_END)) == 0))
      current_settings->inside_data = 0;
    else
      return_value = FILTER_FLAG_PASS | FILTER_FLAG_CHILD_RESPONSE_NOT_NEEDED;
    }
  else if (((current_settings->current_options->smtp_auth_level & SMTP_AUTH_LEVEL_MASK) >= SMTP_AUTH_LEVEL_VALUE_OBSERVE) &&
           (current_settings->smtp_auth_state == SMTP_AUTH_STATE_CHALLENGE_1_SENT))
    {
    current_settings->smtp_auth_state = SMTP_AUTH_STATE_RESPONSE_1_SEEN;

    switch (current_settings->smtp_auth_type)
      {
      case SMTP_AUTH_PLAIN:
        strlen_content = base64_decode((unsigned char *)decoded_content, MAX_BUF, (unsigned char *)input_line, strlen_input_line);
        decoded_content[strlen_content] = '\0';

        for (i = 0; (i < strlen_content) && (decoded_content[i] != '\0'); i++);
        content_ptr = decoded_content + i;
        strlen_content -= i;

        if ((current_settings->current_options->smtp_auth_command != NULL) &&
            ((((current_settings->current_options->smtp_auth_level & SMTP_AUTH_LEVEL_MASK) >= SMTP_AUTH_LEVEL_VALUE_ON_DEMAND) &&
              (current_settings->smtp_auth_origin == SMTP_AUTH_ORIGIN_SPAMDYKE)) ||
             ((current_settings->current_options->smtp_auth_level & SMTP_AUTH_LEVEL_MASK) >= SMTP_AUTH_LEVEL_VALUE_ALWAYS)))
          {
          if ((strlen_content >= 3) &&
              (content_ptr[0] == '\0'))
            {
            snprintf(username, MAX_BUF, "%s", content_ptr + 1);
            for (i = 1; ((i < strlen_content) && (content_ptr[i] != '\0')); i++);
            if (i < strlen_content)
              {
              snprintf(password, MAX_BUF, "%s", content_ptr + i + 1);

              for (i = 0; current_settings->current_options->smtp_auth_command[i] != NULL; i++)
                if (exec_checkpassword(current_settings, current_settings->current_options->smtp_auth_command[i], username, password, NULL))
                  {
                  output_write_rejection(current_settings, &smtp_auth_success, outbound_fd, NULL);
                  return_value = FILTER_FLAG_INTERCEPT;

                  current_settings->smtp_auth_state = SMTP_AUTH_STATE_AUTHENTICATED;
                  snprintf(current_settings->smtp_auth_username, MAX_BUF, "%s", username);
                  if (!current_settings->current_options->filter_action_locked)
                    {
                    current_settings->current_options->filter_action = FILTER_DECISION_DO_NOT_FILTER;
                    current_settings->current_options->rejection = NULL;
                    }

                  output_writeln(current_settings, LOG_ACTION_AUTH_SUCCESS, -1, NULL, -1);
                  break;
                  }
              }
            }

          if (current_settings->smtp_auth_state != SMTP_AUTH_STATE_AUTHENTICATED)
            {
            output_writeln(current_settings, LOG_ACTION_AUTH_FAILURE, -1, username, -1);

            if ((current_settings->smtp_auth_origin == SMTP_AUTH_ORIGIN_SPAMDYKE) ||
                ((current_settings->current_options->smtp_auth_level & SMTP_AUTH_LEVEL_MASK) >= SMTP_AUTH_LEVEL_VALUE_ALWAYS))
              {
              set_rejection(current_settings, REJECTION_SMTP_AUTH_FAILURE, &current_settings->current_options->transient_rejection, &current_settings->current_options->transient_rejection_buf, current_settings->current_options->transient_reject_message_buf, MAX_BUF);
              output_write_rejection(current_settings, current_settings->current_options->transient_rejection, outbound_fd, NULL);
              current_settings->current_options->transient_rejection = NULL;
              return_value = FILTER_FLAG_INTERCEPT | FILTER_FLAG_CHILD_CONTINUE;
              }
            else
              return_value = FILTER_FLAG_PASS;

            current_settings->smtp_auth_state = SMTP_AUTH_STATE_NONE;
            }
          }
        else
          {
          if ((strlen_content >= 3) &&
              (content_ptr[0] == '\0'))
            snprintf(current_settings->smtp_auth_response, MAX_BUF, "%s", content_ptr + 1);

          current_settings->smtp_auth_state = SMTP_AUTH_STATE_UNKNOWN;
          return_value = FILTER_FLAG_PASS | FILTER_FLAG_AUTH_CAPTURE;
          }

        break;
      case SMTP_AUTH_LOGIN:
        memcpy(current_settings->smtp_auth_response, input_line, sizeof(char) * MINVAL(MAX_BUF, strlen_input_line));
        current_settings->smtp_auth_response[MINVAL(MAX_BUF, strlen_input_line)] = '\0';

        if ((current_settings->smtp_auth_origin == SMTP_AUTH_ORIGIN_SPAMDYKE) ||
            ((current_settings->current_options->smtp_auth_level & SMTP_AUTH_LEVEL_MASK) >= SMTP_AUTH_LEVEL_VALUE_ALWAYS))
          {
          strlen_content = snprintf(current_settings->smtp_auth_challenge, MAX_BUF, "%s%s%s", REJECT_SEVERITY_AUTH_CHALLENGE, SMTP_AUTH_LOGIN_CHALLENGE_2, REJECT_CRLF);
          output_writeln(current_settings, LOG_ACTION_FILTER_FROM, outbound_fd, current_settings->smtp_auth_challenge, strlen_content);
          current_settings->smtp_auth_state = SMTP_AUTH_STATE_CHALLENGE_2_SENT;

          return_value = FILTER_FLAG_INTERCEPT;
          }
        else
          return_value = FILTER_FLAG_PASS | FILTER_FLAG_AUTH_CAPTURE;

        break;
      case SMTP_AUTH_CRAM_MD5:
        strlen_content = base64_decode((unsigned char *)decoded_content, MAX_BUF, (unsigned char *)input_line, strlen_input_line);
        decoded_content[strlen_content] = '\0';

        if ((current_settings->current_options->smtp_auth_command != NULL) &&
            ((((current_settings->current_options->smtp_auth_level & SMTP_AUTH_LEVEL_MASK) >= SMTP_AUTH_LEVEL_VALUE_ON_DEMAND) &&
              (current_settings->smtp_auth_origin == SMTP_AUTH_ORIGIN_SPAMDYKE)) ||
             ((current_settings->current_options->smtp_auth_level & SMTP_AUTH_LEVEL_MASK) >= SMTP_AUTH_LEVEL_VALUE_ALWAYS)))
          {
          if (strlen_content > 0)
            {
            for (i = 0; (i < strlen_content) && (decoded_content[i] != ' '); i++);

            if (i < strlen_content)
              {
              snprintf(username, MAX_BUF, "%.*s", i, decoded_content);
              snprintf(password, MAX_BUF, "%.*s", (strlen_content - i) - 1, decoded_content + i + 1);

              strlen_content = strlen(current_settings->smtp_auth_challenge);
              for (i = STRLEN(SMTP_AUTH_CHALLENGE); (i < strlen_content) && (current_settings->smtp_auth_challenge[i] != ' '); i++);
              if (i < strlen_content)
                {
                strlen_content = base64_decode((unsigned char *)decoded_content, MAX_BUF, (unsigned char *)current_settings->smtp_auth_challenge + i, strlen_content - i);
                decoded_content[strlen_content] = '\0';

                for (i = 0; current_settings->current_options->smtp_auth_command[i] != NULL; i++)
                  if (exec_checkpassword(current_settings, current_settings->current_options->smtp_auth_command[i], username, password, decoded_content))
                    {
                    output_write_rejection(current_settings, &smtp_auth_success, outbound_fd, NULL);
                    return_value = FILTER_FLAG_INTERCEPT;

                    current_settings->smtp_auth_state = SMTP_AUTH_STATE_AUTHENTICATED;
                    snprintf(current_settings->smtp_auth_username, MAX_BUF, "%s", username);
                    if (!current_settings->current_options->filter_action_locked)
                      {
                      current_settings->current_options->filter_action = FILTER_DECISION_DO_NOT_FILTER;
                      current_settings->current_options->rejection = NULL;
                      }

                    output_writeln(current_settings, LOG_ACTION_AUTH_SUCCESS, -1, NULL, -1);
                    break;
                    }
                }
              }
            }

          if (current_settings->smtp_auth_state != SMTP_AUTH_STATE_AUTHENTICATED)
            {
            output_writeln(current_settings, LOG_ACTION_AUTH_FAILURE, -1, username, -1);

            if ((current_settings->smtp_auth_origin == SMTP_AUTH_ORIGIN_SPAMDYKE) ||
                ((current_settings->current_options->smtp_auth_level & SMTP_AUTH_LEVEL_MASK) >= SMTP_AUTH_LEVEL_VALUE_ALWAYS))
              {
              set_rejection(current_settings, REJECTION_SMTP_AUTH_FAILURE, &current_settings->current_options->transient_rejection, &current_settings->current_options->transient_rejection_buf, current_settings->current_options->transient_reject_message_buf, MAX_BUF);
              output_write_rejection(current_settings, current_settings->current_options->transient_rejection, outbound_fd, NULL);
              current_settings->current_options->transient_rejection = NULL;
              return_value = FILTER_FLAG_INTERCEPT | FILTER_FLAG_CHILD_CONTINUE;
              }
            else
              return_value = FILTER_FLAG_PASS;

            current_settings->smtp_auth_state = SMTP_AUTH_STATE_NONE;
            }
          }
        else
          {
          if (strlen_content > 0)
            {
            for (i = 0; (i < strlen_content) && (decoded_content[i] != ' '); i++);

            if (i < strlen_content)
              snprintf(current_settings->smtp_auth_response, MAX_BUF, "%.*s", i, decoded_content);
            }

          current_settings->smtp_auth_state = SMTP_AUTH_STATE_UNKNOWN;
          return_value = FILTER_FLAG_PASS | FILTER_FLAG_AUTH_CAPTURE;
          }

        break;
      default:
        /* The client is using a protocol spamdyke doesn't support. Continue listening for the server's response and trust it. */
        current_settings->smtp_auth_state = SMTP_AUTH_STATE_UNKNOWN;
        return_value = FILTER_FLAG_PASS | FILTER_FLAG_AUTH_CAPTURE;
        break;
      }
    }
  else if (((current_settings->current_options->smtp_auth_level & SMTP_AUTH_LEVEL_MASK) >= SMTP_AUTH_LEVEL_VALUE_OBSERVE) &&
           (current_settings->smtp_auth_state == SMTP_AUTH_STATE_CHALLENGE_2_SENT))
    {
    switch (current_settings->smtp_auth_type)
      {
      case SMTP_AUTH_LOGIN:
        strlen_content = base64_decode((unsigned char *)username, MAX_BUF, (unsigned char *)current_settings->smtp_auth_response, strlen(current_settings->smtp_auth_response));
        username[strlen_content] = '\0';

        for (j = 0; (j < strlen_input_line) && !isalnum((int)input_line[j]); j++);
        strlen_content = base64_decode((unsigned char *)password, MAX_BUF, (unsigned char *)(input_line + j), strlen_input_line - j);
        password[strlen_content] = '\0';

        if ((current_settings->current_options->smtp_auth_command != NULL) &&
            ((((current_settings->current_options->smtp_auth_level & SMTP_AUTH_LEVEL_MASK) >= SMTP_AUTH_LEVEL_VALUE_ON_DEMAND) &&
              (current_settings->smtp_auth_origin == SMTP_AUTH_ORIGIN_SPAMDYKE)) ||
             ((current_settings->current_options->smtp_auth_level & SMTP_AUTH_LEVEL_MASK) >= SMTP_AUTH_LEVEL_VALUE_ALWAYS)))
          {
          for (i = 0; current_settings->current_options->smtp_auth_command[i] != NULL; i++)
            if (exec_checkpassword(current_settings, current_settings->current_options->smtp_auth_command[i], username, password, NULL))
              {
              output_write_rejection(current_settings, &smtp_auth_success, outbound_fd, NULL);
              return_value = FILTER_FLAG_INTERCEPT;

              current_settings->smtp_auth_state = SMTP_AUTH_STATE_AUTHENTICATED;
              snprintf(current_settings->smtp_auth_username, MAX_BUF, "%s", username);
              if (!current_settings->current_options->filter_action_locked)
                {
                current_settings->current_options->filter_action = FILTER_DECISION_DO_NOT_FILTER;
                current_settings->current_options->rejection = NULL;
                }

              output_writeln(current_settings, LOG_ACTION_AUTH_SUCCESS, -1, NULL, -1);
              break;
              }

          if (current_settings->smtp_auth_state != SMTP_AUTH_STATE_AUTHENTICATED)
            {
            output_writeln(current_settings, LOG_ACTION_AUTH_FAILURE, -1, username, -1);

            if ((current_settings->smtp_auth_origin == SMTP_AUTH_ORIGIN_SPAMDYKE) ||
                ((current_settings->current_options->smtp_auth_level & SMTP_AUTH_LEVEL_MASK) >= SMTP_AUTH_LEVEL_VALUE_ALWAYS))
              {
              set_rejection(current_settings, REJECTION_SMTP_AUTH_FAILURE, &current_settings->current_options->transient_rejection, &current_settings->current_options->transient_rejection_buf, current_settings->current_options->transient_reject_message_buf, MAX_BUF);
              output_write_rejection(current_settings, current_settings->current_options->transient_rejection, outbound_fd, NULL);
              current_settings->current_options->transient_rejection = NULL;
              return_value = FILTER_FLAG_INTERCEPT | FILTER_FLAG_CHILD_CONTINUE;
              }
            else
              return_value = FILTER_FLAG_PASS;

            current_settings->smtp_auth_state = SMTP_AUTH_STATE_NONE;
            }
          }
        else
          {
          snprintf(current_settings->smtp_auth_response, MAX_BUF, "%s", username);
          current_settings->smtp_auth_state = SMTP_AUTH_STATE_UNKNOWN;
          return_value = FILTER_FLAG_PASS | FILTER_FLAG_AUTH_CAPTURE;
          }

        break;
      default:
        /* The client is using a protocol spamdyke doesn't support. Continue listening for the server's response and trust it. */
        current_settings->smtp_auth_state = SMTP_AUTH_STATE_UNKNOWN;
        return_value = FILTER_FLAG_PASS | FILTER_FLAG_AUTH_CAPTURE;
        break;
      }
    }
  else if ((strlen_input_line >= STRLEN(SMTP_HELO)) &&
           (strncasecmp(SMTP_HELO, input_line, STRLEN(SMTP_HELO)) == 0))
    {
    if ((current_settings->current_options->filter_action == FILTER_DECISION_DO_FILTER) &&
        (current_settings->current_options->filter_grace <= FILTER_GRACE_NONE) &&
        (current_settings->current_options->rejection != NULL))
      {
      output_write_rejection(current_settings, current_settings->current_options->rejection, outbound_fd, REJECT_SEVERITY_NONE);
      current_settings->current_options->filter_grace = FILTER_GRACE_EXPIRED;
      return_value = FILTER_FLAG_INTERCEPT | FILTER_FLAG_CHILD_QUIT;
      }
    }
  else if ((strlen_input_line >= STRLEN(SMTP_EHLO)) &&
           (strncasecmp(SMTP_EHLO, input_line, STRLEN(SMTP_EHLO)) == 0))
    {
    if ((current_settings->current_options->filter_action == FILTER_DECISION_DO_FILTER) &&
        (current_settings->current_options->filter_grace <= FILTER_GRACE_NONE) &&
        (current_settings->current_options->rejection != NULL))
      {
      output_write_rejection(current_settings, current_settings->current_options->rejection, outbound_fd, REJECT_SEVERITY_NONE);
      current_settings->current_options->filter_grace = FILTER_GRACE_EXPIRED;
      return_value = FILTER_FLAG_INTERCEPT | FILTER_FLAG_CHILD_QUIT;
      }
    else
      {
      if (((current_settings->current_options->smtp_auth_level & SMTP_AUTH_LEVEL_MASK) >= SMTP_AUTH_LEVEL_VALUE_ALWAYS) &&
          (current_settings->current_options->smtp_auth_command != NULL))
        {
        SPAMDYKE_LOG_EXCESSIVE(current_settings, LOG_DEBUGX_SMTP_AUTH_REPLACE, NULL);
        return_value |= FILTER_FLAG_AUTH_ADD | FILTER_FLAG_AUTH_REMOVE;
        }
      else if (((current_settings->current_options->smtp_auth_level & SMTP_AUTH_LEVEL_MASK) >= SMTP_AUTH_LEVEL_VALUE_ON_DEMAND) &&
               (current_settings->current_options->smtp_auth_command != NULL))
        {
        SPAMDYKE_LOG_EXCESSIVE(current_settings, LOG_DEBUGX_SMTP_AUTH_ADD, NULL);
        return_value |= FILTER_FLAG_AUTH_ADD;
        }
      else if ((current_settings->current_options->smtp_auth_level & SMTP_AUTH_LEVEL_MASK) == SMTP_AUTH_LEVEL_VALUE_NONE)
        {
        SPAMDYKE_LOG_EXCESSIVE(current_settings, LOG_DEBUGX_SMTP_AUTH_REMOVE, NULL);
        return_value |= FILTER_FLAG_AUTH_REMOVE;
        }

#ifdef HAVE_LIBSSL

      if ((current_settings->tls_state == TLS_STATE_ACTIVE_SPAMDYKE) ||
          (current_settings->current_options->tls_level == TLS_LEVEL_NONE))
        {
        SPAMDYKE_LOG_EXCESSIVE(current_settings, LOG_DEBUGX_TLS_REMOVE, NULL);
        return_value |= FILTER_FLAG_TLS_REMOVE;
        }
      else if (current_settings->current_options->tls_certificate_file != NULL)
        {
        SPAMDYKE_LOG_EXCESSIVE(current_settings, LOG_DEBUGX_TLS_ADD, NULL);
        return_value |= FILTER_FLAG_TLS_ADD;
        }

#else /* HAVE_LIBSSL */

      if (current_settings->current_options->tls_level == TLS_LEVEL_NONE)
        {
        SPAMDYKE_LOG_EXCESSIVE(current_settings, LOG_DEBUGX_TLS_REMOVE, NULL);
        return_value = FILTER_FLAG_PASS | FILTER_FLAG_TLS_REMOVE;
        }

#endif /* HAVE_LIBSSL */

      }
    }
  else if (((current_settings->current_options->smtp_auth_level & SMTP_AUTH_LEVEL_MASK) >= SMTP_AUTH_LEVEL_VALUE_OBSERVE) &&
           (strlen_input_line >= STRLEN(SMTP_AUTH)) &&
           (strncasecmp(SMTP_AUTH, input_line, STRLEN(SMTP_AUTH)) == 0))
    {
    for (j = STRLEN(SMTP_AUTH); (j < strlen_input_line) && isspace((int)input_line[j]); j++);
    if (j < strlen_input_line)
      for (i = 0; smtp_auth_types[i] != NULL; i++)
        if ((strlen_input_line >= (strlen_smtp_auth_types[i] + j)) &&
            (strncasecmp(input_line + j, smtp_auth_types[i], strlen_smtp_auth_types[i]) == 0))
          {
          current_settings->smtp_auth_type = i;
          break;
          }

    current_settings->smtp_auth_state = SMTP_AUTH_STATE_CMD_SEEN;

    if ((current_settings->current_options->smtp_auth_command != NULL) &&
        ((((current_settings->current_options->smtp_auth_level & SMTP_AUTH_LEVEL_MASK) >= SMTP_AUTH_LEVEL_VALUE_ON_DEMAND) &&
          (current_settings->smtp_auth_origin == SMTP_AUTH_ORIGIN_SPAMDYKE)) ||
         ((current_settings->current_options->smtp_auth_level & SMTP_AUTH_LEVEL_MASK) >= SMTP_AUTH_LEVEL_VALUE_ALWAYS)))
      {
      switch (current_settings->smtp_auth_type)
        {
        case SMTP_AUTH_PLAIN:
          //FIXME: This code appears 3 times for processing AUTH PLAIN.  Can it be moved to a function?
          for (i = j + strlen_smtp_auth_types[SMTP_AUTH_PLAIN]; (i < strlen_input_line) && !isalnum((int)input_line[i]); i++);
          if ((i < strlen_input_line) &&
              isalnum((int)input_line[i]))
            {
            strlen_content = base64_decode((unsigned char *)decoded_content, MAX_BUF, (unsigned char *)(input_line + i), strlen_input_line - i);
            decoded_content[strlen_content] = '\0';

            for (i = 0; (i < strlen_content) && (decoded_content[i] != '\0'); i++);
            content_ptr = decoded_content + i;
            strlen_content -= i;

            if ((strlen_content >= 3) &&
                (content_ptr[0] == '\0'))
              {
              snprintf(username, MAX_BUF, "%s", content_ptr + 1);
              for (i = 1; ((i < strlen_content) && (content_ptr[i] != '\0')); i++);
              if (i < strlen_content)
                {
                snprintf(password, MAX_BUF, "%s", content_ptr + i + 1);

                for (i = 0; current_settings->current_options->smtp_auth_command[i] != NULL; i++)
                  if (exec_checkpassword(current_settings, current_settings->current_options->smtp_auth_command[i], username, password, NULL))
                    {
                    output_write_rejection(current_settings, &smtp_auth_success, outbound_fd, NULL);
                    return_value = FILTER_FLAG_INTERCEPT;

                    current_settings->smtp_auth_state = SMTP_AUTH_STATE_AUTHENTICATED;
                    snprintf(current_settings->smtp_auth_username, MAX_BUF, "%s", username);
                    if (!current_settings->current_options->filter_action_locked)
                      {
                      current_settings->current_options->filter_action = FILTER_DECISION_DO_NOT_FILTER;
                      current_settings->current_options->rejection = NULL;
                      }

                    output_writeln(current_settings, LOG_ACTION_AUTH_SUCCESS, -1, NULL, -1);
                    break;
                    }
                }
              }

            if (current_settings->smtp_auth_state != SMTP_AUTH_STATE_AUTHENTICATED)
              {
              output_writeln(current_settings, LOG_ACTION_AUTH_FAILURE, -1, username, -1);

              if ((current_settings->smtp_auth_origin == SMTP_AUTH_ORIGIN_SPAMDYKE) ||
                  ((current_settings->current_options->smtp_auth_level & SMTP_AUTH_LEVEL_MASK) >= SMTP_AUTH_LEVEL_VALUE_ALWAYS))
                {
                set_rejection(current_settings, REJECTION_SMTP_AUTH_FAILURE, &current_settings->current_options->transient_rejection, &current_settings->current_options->transient_rejection_buf, current_settings->current_options->transient_reject_message_buf, MAX_BUF);
                output_write_rejection(current_settings, current_settings->current_options->transient_rejection, outbound_fd, NULL);
                current_settings->current_options->transient_rejection = NULL;
                return_value = FILTER_FLAG_INTERCEPT | FILTER_FLAG_CHILD_CONTINUE;
                }
              else
                return_value = FILTER_FLAG_PASS;

              current_settings->smtp_auth_state = SMTP_AUTH_STATE_NONE;
              }
            }
          else
            {
            strlen_content = snprintf(current_settings->smtp_auth_challenge, MAX_BUF, "%s%s", REJECT_SEVERITY_AUTH_CHALLENGE, REJECT_CRLF);
            output_writeln(current_settings, LOG_ACTION_FILTER_FROM, outbound_fd, current_settings->smtp_auth_challenge, strlen_content);
            current_settings->smtp_auth_state = SMTP_AUTH_STATE_CHALLENGE_1_SENT;
            }

          break;
        case SMTP_AUTH_LOGIN:
          for (i = j + strlen_smtp_auth_types[SMTP_AUTH_LOGIN]; (i < strlen_input_line) && !isalnum((int)input_line[i]); i++);
          if ((i < strlen_input_line) &&
              isalnum((int)input_line[i]))
            {
            memcpy(current_settings->smtp_auth_response, input_line + i, sizeof(char) * MINVAL(MAX_BUF, strlen_input_line - i));
            current_settings->smtp_auth_response[MINVAL(MAX_BUF, strlen_input_line - i)] = '\0';

            strlen_content = snprintf(current_settings->smtp_auth_challenge, MAX_BUF, "%s%s%s", REJECT_SEVERITY_AUTH_CHALLENGE, SMTP_AUTH_LOGIN_CHALLENGE_2, REJECT_CRLF);
            output_writeln(current_settings, LOG_ACTION_FILTER_FROM, outbound_fd, current_settings->smtp_auth_challenge, strlen_content);
            current_settings->smtp_auth_state = SMTP_AUTH_STATE_CHALLENGE_2_SENT;
            }
          else
            {
            strlen_content = snprintf(current_settings->smtp_auth_challenge, MAX_BUF, "%s%s%s", REJECT_SEVERITY_AUTH_CHALLENGE, SMTP_AUTH_LOGIN_CHALLENGE_1, REJECT_CRLF);
            output_writeln(current_settings, LOG_ACTION_FILTER_FROM, outbound_fd, current_settings->smtp_auth_challenge, strlen_content);
            current_settings->smtp_auth_state = SMTP_AUTH_STATE_CHALLENGE_1_SENT;
            }

          break;
        case SMTP_AUTH_CRAM_MD5:
          if ((current_settings->current_options->local_server_name == NULL) &&
              (current_settings->current_options->local_server_name_file != NULL))
            read_file_first_line(current_settings, current_settings->current_options->local_server_name_file, &current_settings->current_options->local_server_name);

          if ((current_settings->current_options->local_server_name == NULL) &&
              (current_settings->current_options->local_server_name_command != NULL))
            exec_command(current_settings, current_settings->current_options->local_server_name_command, NULL, &current_settings->current_options->local_server_name, -1);

          if (current_settings->current_options->local_server_name == NULL)
            for (i = 0; environment_hostname[i] != NULL; i++)
              if ((tmp_hostname = find_environment_variable(current_settings, current_settings->current_environment, environment_hostname[i], strlen_environment_hostname[i], NULL)) != NULL)
                {
                tmp_strlen = strlen(tmp_hostname);
                if ((tmp_char = realloc(current_settings->current_options->local_server_name, sizeof(char) * (tmp_strlen + 1))) != NULL)
                  {
                  memcpy(tmp_char, tmp_hostname, sizeof(char) * tmp_strlen);
                  tmp_char[tmp_strlen] = '\0';
                  current_settings->current_options->local_server_name = tmp_char;
                  }
                else
                  {
                  SPAMDYKE_LOG_ERROR(current_settings, LOG_ERROR_MALLOC, sizeof(char) * (tmp_strlen + 1));
                  current_settings->current_options->filter_action = FILTER_DECISION_ERROR;
                  return_value = FILTER_FLAG_QUIT;
                  }

                break;
                }

          if (current_settings->current_options->filter_action != FILTER_DECISION_ERROR)
            {
            strlen_content = snprintf(decoded_content, MAX_BUF, "<%ld.%ld@%s>", random(), (long)time(NULL), (current_settings->current_options->local_server_name != NULL) ? current_settings->current_options->local_server_name : MISSING_LOCAL_SERVER_NAME);
            snprintf(current_settings->smtp_auth_challenge, MAX_BUF - STRLEN(REJECT_CRLF), "%s ", SMTP_AUTH_CHALLENGE);

            strlen_content = base64_encode((unsigned char *)(current_settings->smtp_auth_challenge + STRLEN(SMTP_AUTH_CHALLENGE) + 1), MAX_BUF - (STRLEN(SMTP_AUTH_CHALLENGE) + STRLEN(REJECT_CRLF) + 1), (unsigned char *)decoded_content, strlen_content) + STRLEN(SMTP_AUTH_CHALLENGE) + 1;
            memcpy(current_settings->smtp_auth_challenge + strlen_content, REJECT_CRLF, sizeof(char) * STRLEN(REJECT_CRLF));
            current_settings->smtp_auth_challenge[strlen_content + STRLEN(REJECT_CRLF)] = '\0';

            output_writeln(current_settings, LOG_ACTION_FILTER_FROM, outbound_fd, current_settings->smtp_auth_challenge, strlen_content + STRLEN(REJECT_CRLF));

            current_settings->smtp_auth_challenge[strlen_content] = '\0';

            current_settings->smtp_auth_state = SMTP_AUTH_STATE_CHALLENGE_1_SENT;
            }

          break;
        default:
          /* The client requested an algorithm spamdyke didn't advertise. */
          set_rejection(current_settings, REJECTION_SMTP_AUTH_UNKNOWN, &current_settings->current_options->transient_rejection, &current_settings->current_options->transient_rejection_buf, current_settings->current_options->transient_reject_message_buf, MAX_BUF);
          output_write_rejection(current_settings, current_settings->current_options->transient_rejection, outbound_fd, NULL);
          current_settings->current_options->transient_rejection = NULL;
          current_settings->smtp_auth_state = SMTP_AUTH_STATE_NONE;
          break;
        }

      return_value = FILTER_FLAG_INTERCEPT;
      }
    else
      switch (current_settings->smtp_auth_type)
        {
        case SMTP_AUTH_PLAIN:
          for (i = j + strlen_smtp_auth_types[SMTP_AUTH_PLAIN]; (i < strlen_input_line) && !isalnum((int)input_line[i]); i++);
          if ((i < strlen_input_line) &&
              isalnum((int)input_line[i]))
            {
            strlen_content = base64_decode((unsigned char *)decoded_content, MAX_BUF, (unsigned char *)(input_line + i), strlen_input_line - i);
            decoded_content[strlen_content] = '\0';

            for (i = 0; (i < strlen_content) && (decoded_content[i] != '\0'); i++);
            content_ptr = decoded_content + i;
            strlen_content -= i;

            if ((strlen_content >= 3) &&
                (content_ptr[0] == '\0'))
              snprintf(current_settings->smtp_auth_response, MAX_BUF, "%s", content_ptr + 1);

            current_settings->smtp_auth_state = SMTP_AUTH_STATE_UNKNOWN;
            return_value = FILTER_FLAG_PASS | FILTER_FLAG_AUTH_CAPTURE;
            }
          else
            return_value = FILTER_FLAG_PASS | FILTER_FLAG_AUTH_CAPTURE;

          break;
        case SMTP_AUTH_LOGIN:
          for (i = j + strlen_smtp_auth_types[SMTP_AUTH_LOGIN]; (i < strlen_input_line) && !isalnum((int)input_line[i]); i++);
          if ((i < strlen_input_line) &&
              isalnum((int)input_line[i]))
            {
            memcpy(current_settings->smtp_auth_response, input_line + i, sizeof(char) * MINVAL(MAX_BUF, strlen_input_line - i));
            current_settings->smtp_auth_response[MINVAL(MAX_BUF, strlen_input_line - i)] = '\0';
            current_settings->smtp_auth_state = SMTP_AUTH_STATE_RESPONSE_1_SEEN;
            }

          return_value = FILTER_FLAG_PASS | FILTER_FLAG_AUTH_CAPTURE;

          break;
        case SMTP_AUTH_CRAM_MD5:
          return_value = FILTER_FLAG_PASS | FILTER_FLAG_AUTH_CAPTURE;
          break;
        default:
          current_settings->smtp_auth_state = SMTP_AUTH_STATE_UNKNOWN;
          return_value = FILTER_FLAG_PASS | FILTER_FLAG_AUTH_CAPTURE;
        }
    }
  else if ((strlen_input_line >= STRLEN(SMTP_TLS)) &&
           (strncasecmp(SMTP_TLS, input_line, STRLEN(SMTP_TLS)) == 0))
    {
    if (current_settings->tls_state == TLS_STATE_INACTIVE)
      if ((current_settings->current_options->filter_action == FILTER_DECISION_DO_FILTER) &&
          (current_settings->current_options->filter_grace <= FILTER_GRACE_NONE) &&
          (current_settings->current_options->rejection != NULL))
        {
        output_write_rejection(current_settings, current_settings->current_options->rejection, outbound_fd, NULL);
        current_settings->current_options->filter_grace = FILTER_GRACE_EXPIRED;
        return_value = FILTER_FLAG_INTERCEPT | FILTER_FLAG_CHILD_QUIT;
        }
      else

#ifdef HAVE_LIBSSL
        if (current_settings->current_options->tls_certificate_file != NULL)
          {
          output_write_rejection(current_settings, &tls_success, outbound_fd, NULL);
          if (tls_start(current_settings, inbound_fd, outbound_fd))
            output_writeln(current_settings, LOG_ACTION_TLS_START, -1, NULL, -1);
          else
            {
            set_rejection(current_settings, FAILURE_TLS, &current_settings->current_options->transient_rejection, &current_settings->current_options->transient_rejection_buf, current_settings->current_options->transient_reject_message_buf, MAX_BUF);
            output_write_rejection(current_settings, current_settings->current_options->transient_rejection, outbound_fd, NULL);
            current_settings->current_options->transient_rejection = NULL;
            }

          return_value = FILTER_FLAG_INTERCEPT;
          }
        else
#endif /* HAVE_LIBSSL */

          return_value = FILTER_FLAG_PASS | FILTER_FLAG_TLS_CAPTURE;
    else
      {
      set_rejection(current_settings, FAILURE_TLS, &current_settings->current_options->transient_rejection, &current_settings->current_options->transient_rejection_buf, current_settings->current_options->transient_reject_message_buf, MAX_BUF);
      output_write_rejection(current_settings, current_settings->current_options->transient_rejection, outbound_fd, REJECT_SEVERITY_PERMANENT);
      current_settings->current_options->transient_rejection = NULL;
      return_value = FILTER_FLAG_INTERCEPT;
      }
    }
  else if ((strlen_input_line >= STRLEN(SMTP_MAIL_FROM)) &&
           (strncasecmp(SMTP_MAIL_FROM, input_line, STRLEN(SMTP_MAIL_FROM)) == 0))
    {
    if ((strlen_target_username = find_address(current_settings, input_line, strlen_input_line, current_settings->sender_username, MAX_ADDRESS, current_settings->sender_domain, MAX_ADDRESS, &strlen_target_domain)) > 0)
      {
      for (i = 0; i < strlen_target_username; i++)
        current_settings->sender_username[i] = tolower((int)current_settings->sender_username[i]);

      for (i = 0; i < strlen_target_domain; i++)
        current_settings->sender_domain[i] = tolower((int)current_settings->sender_domain[i]);

      if (!current_settings->local_sender &&
          (current_settings->current_options->local_domains != NULL))
        for (i = 0; current_settings->current_options->local_domains[i] != NULL; i++)
          if (examine_entry(current_settings->sender_domain, strlen_target_domain, current_settings->current_options->local_domains[i], strlen(current_settings->current_options->local_domains[i]), '.', ".", '\0', NULL) > 0)
            {
            current_settings->local_sender = 1;
            break;
            }

      if (!current_settings->local_sender &&
          (current_settings->current_options->local_domains_file != NULL))
        for (i = 0; current_settings->current_options->local_domains_file[i] != NULL; i++)
          if (search_file(current_settings, current_settings->current_options->local_domains_file[i], current_settings->sender_domain, strlen_target_domain, '.', ".", '\0', NULL) > 0)
            {
            current_settings->local_sender = 1;
            break;
            }

      if (current_settings->current_options->configuration_dir == NULL)
        {
        filter_level(current_settings, &current_settings->current_options->filter_action, &current_settings->current_options->filter_action_locked, &current_settings->current_options->rejection, &current_settings->current_options->rejection_buf, current_settings->current_options->reject_message_buf, MAX_BUF);
        filter_sender_whitelist(current_settings, &current_settings->current_options->filter_action, &current_settings->current_options->filter_action_locked, &current_settings->current_options->rejection);
        filter_sender_rhswl(current_settings, &current_settings->current_options->filter_action, &current_settings->current_options->filter_action_locked, &current_settings->current_options->rejection);
        filter_sender_blacklist(current_settings, &current_settings->current_options->filter_action, &current_settings->current_options->filter_action_locked, &current_settings->current_options->rejection, &current_settings->current_options->rejection_buf, current_settings->current_options->reject_message_buf, MAX_BUF);
        filter_sender_rhsbl(current_settings, &current_settings->current_options->filter_action, &current_settings->current_options->filter_action_locked, &current_settings->current_options->rejection, &current_settings->current_options->rejection_buf, current_settings->current_options->reject_message_buf, MAX_BUF);
        filter_sender_no_mx(current_settings, &current_settings->current_options->filter_action, &current_settings->current_options->filter_action_locked, &current_settings->current_options->rejection, &current_settings->current_options->rejection_buf, current_settings->current_options->reject_message_buf, MAX_BUF);
        }
      }

    if ((current_settings->current_options->filter_action == FILTER_DECISION_DO_FILTER) &&
        (current_settings->current_options->filter_grace <= FILTER_GRACE_AFTER_FROM) &&
        (current_settings->current_options->rejection != NULL))
      {
      output_write_rejection(current_settings, current_settings->current_options->rejection, outbound_fd, REJECT_SEVERITY_NONE);
      current_settings->current_options->filter_grace = FILTER_GRACE_EXPIRED;
      return_value = FILTER_FLAG_INTERCEPT | FILTER_FLAG_CHILD_QUIT;
      }
    }
  else if ((strlen_input_line >= STRLEN(SMTP_RCPT_TO)) &&
           (strncasecmp(SMTP_RCPT_TO, input_line, STRLEN(SMTP_RCPT_TO)) == 0))
    {
    if ((strlen_target_username = find_address(current_settings, input_line, strlen_input_line, current_settings->recipient_username, MAX_ADDRESS, current_settings->recipient_domain, MAX_ADDRESS, &strlen_target_domain)) > 0)
      {
      for (i = 0; i < strlen_target_username; i++)
        current_settings->recipient_username[i] = tolower((int)current_settings->recipient_username[i]);

      for (i = 0; i < strlen_target_domain; i++)
        current_settings->recipient_domain[i] = tolower((int)current_settings->recipient_domain[i]);

      if ((current_settings->current_options->local_domains != NULL) ||
          (current_settings->current_options->local_domains_file != NULL))
        current_settings->local_recipient = 0;

      if (!current_settings->local_recipient &&
          (current_settings->current_options->local_domains != NULL))
        for (i = 0; current_settings->current_options->local_domains[i] != NULL; i++)
          if (examine_entry(current_settings->recipient_domain, strlen_target_domain, current_settings->current_options->local_domains[i], strlen(current_settings->current_options->local_domains[i]), '.', ".", '\0', NULL) > 0)
            {
            current_settings->local_recipient = 1;
            break;
            }

      if (!current_settings->local_recipient &&
          (current_settings->current_options->local_domains_file != NULL))
        {
        for (i = 0; current_settings->current_options->local_domains_file[i] != NULL; i++)
          if (search_file(current_settings, current_settings->current_options->local_domains_file[i], current_settings->recipient_domain, strlen_target_domain, '.', ".", '\0', NULL) > 0)
            {
            current_settings->local_recipient = 1;
            break;
            }
        }
      else
        current_settings->local_recipient = 1;
      }
    else
      current_settings->local_recipient = 1;

    current_settings->current_options->prev_filter_action = current_settings->current_options->filter_action;

    if ((current_settings->current_options->configuration_dir != NULL) &&
        ((current_settings->current_options->filter_action = copy_base_options(current_settings, current_settings->current_options->filter_action)) != FILTER_DECISION_ERROR))
      {
      for (i = 0; current_settings->current_options->configuration_dir[i] != NULL; i++)
        if ((current_settings->current_options->filter_action = process_config_dir(current_settings, current_settings->current_options->configuration_dir[i], current_settings->server_ip, current_settings->server_name, current_settings->sender_username, current_settings->sender_domain, current_settings->recipient_username, current_settings->recipient_domain, current_settings->current_options->filter_action, NULL)) == FILTER_DECISION_ERROR)
          break;

      if (current_settings->current_options->filter_action != FILTER_DECISION_ERROR)
        {
        filter_level(current_settings, &current_settings->current_options->filter_action, &current_settings->current_options->filter_action_locked, &current_settings->current_options->rejection, &current_settings->current_options->rejection_buf, current_settings->current_options->reject_message_buf, MAX_BUF);

        /* Post-connect filters */
        filter_rdns_missing(current_settings, &current_settings->current_options->filter_action, &current_settings->current_options->filter_action_locked, &current_settings->current_options->rejection, &current_settings->current_options->rejection_buf, current_settings->current_options->reject_message_buf, MAX_BUF);
        filter_ip_in_rdns_cc(current_settings, &current_settings->current_options->filter_action, &current_settings->current_options->filter_action_locked, &current_settings->current_options->rejection, &current_settings->current_options->rejection_buf, current_settings->current_options->reject_message_buf, MAX_BUF);
        filter_rdns_whitelist(current_settings, &current_settings->current_options->filter_action, &current_settings->current_options->filter_action_locked, &current_settings->current_options->rejection);
        filter_rdns_whitelist_file(current_settings, &current_settings->current_options->filter_action, &current_settings->current_options->filter_action_locked, &current_settings->current_options->rejection);
        filter_rdns_whitelist_dir(current_settings, &current_settings->current_options->filter_action, &current_settings->current_options->filter_action_locked, &current_settings->current_options->rejection);
        filter_rdns_blacklist(current_settings, &current_settings->current_options->filter_action, &current_settings->current_options->filter_action_locked, &current_settings->current_options->rejection, &current_settings->current_options->rejection_buf, current_settings->current_options->reject_message_buf, MAX_BUF);
        filter_rdns_blacklist_file(current_settings, &current_settings->current_options->filter_action, &current_settings->current_options->filter_action_locked, &current_settings->current_options->rejection, &current_settings->current_options->rejection_buf, current_settings->current_options->reject_message_buf, MAX_BUF);
        filter_rdns_blacklist_dir(current_settings, &current_settings->current_options->filter_action, &current_settings->current_options->filter_action_locked, &current_settings->current_options->rejection, &current_settings->current_options->rejection_buf, current_settings->current_options->reject_message_buf, MAX_BUF);
        filter_ip_whitelist(current_settings, &current_settings->current_options->filter_action, &current_settings->current_options->filter_action_locked, &current_settings->current_options->rejection);
        filter_ip_blacklist(current_settings, &current_settings->current_options->filter_action, &current_settings->current_options->filter_action_locked, &current_settings->current_options->rejection, &current_settings->current_options->rejection_buf, current_settings->current_options->reject_message_buf, MAX_BUF);
        filter_ip_in_rdns_whitelist(current_settings, &current_settings->current_options->filter_action, &current_settings->current_options->filter_action_locked, &current_settings->current_options->rejection);
        filter_ip_in_rdns_blacklist(current_settings, &current_settings->current_options->filter_action, &current_settings->current_options->filter_action_locked, &current_settings->current_options->rejection, &current_settings->current_options->rejection_buf, current_settings->current_options->reject_message_buf, MAX_BUF);
        filter_rdns_resolve(current_settings, &current_settings->current_options->filter_action, &current_settings->current_options->filter_action_locked, &current_settings->current_options->rejection, &current_settings->current_options->rejection_buf, current_settings->current_options->reject_message_buf, MAX_BUF);
        filter_dns_rwl(current_settings, &current_settings->current_options->filter_action, &current_settings->current_options->filter_action_locked, &current_settings->current_options->rejection);
        filter_dns_rhswl(current_settings, &current_settings->current_options->filter_action, &current_settings->current_options->filter_action_locked, &current_settings->current_options->rejection);
        filter_dns_rbl(current_settings, &current_settings->current_options->filter_action, &current_settings->current_options->filter_action_locked, &current_settings->current_options->rejection, &current_settings->current_options->rejection_buf, current_settings->current_options->reject_message_buf, MAX_BUF);
        filter_dns_rhsbl(current_settings, &current_settings->current_options->filter_action, &current_settings->current_options->filter_action_locked, &current_settings->current_options->rejection, &current_settings->current_options->rejection_buf, current_settings->current_options->reject_message_buf, MAX_BUF);
        filter_earlytalker(current_settings, 0, &current_settings->current_options->filter_action, &current_settings->current_options->filter_action_locked, &current_settings->current_options->rejection, &current_settings->current_options->rejection_buf, current_settings->current_options->reject_message_buf, MAX_BUF);

        /* Post-MAIL FROM filters */
        filter_sender_whitelist(current_settings, &current_settings->current_options->filter_action, &current_settings->current_options->filter_action_locked, &current_settings->current_options->rejection);
        filter_sender_rhswl(current_settings, &current_settings->current_options->filter_action, &current_settings->current_options->filter_action_locked, &current_settings->current_options->rejection);
        filter_sender_blacklist(current_settings, &current_settings->current_options->filter_action, &current_settings->current_options->filter_action_locked, &current_settings->current_options->rejection, &current_settings->current_options->rejection_buf, current_settings->current_options->reject_message_buf, MAX_BUF);
        filter_sender_rhsbl(current_settings, &current_settings->current_options->filter_action, &current_settings->current_options->filter_action_locked, &current_settings->current_options->rejection, &current_settings->current_options->rejection_buf, current_settings->current_options->reject_message_buf, MAX_BUF);
        filter_sender_no_mx(current_settings, &current_settings->current_options->filter_action, &current_settings->current_options->filter_action_locked, &current_settings->current_options->rejection, &current_settings->current_options->rejection_buf, current_settings->current_options->reject_message_buf, MAX_BUF);
        }
      }

    if (current_settings->current_options->filter_action != FILTER_DECISION_ERROR)
      filter_recipient_whitelist(current_settings, &current_settings->current_options->filter_action, &current_settings->current_options->filter_action_locked, &current_settings->current_options->transient_rejection);

    filter_recipient_relay(current_settings, &current_settings->current_options->filter_action, &current_settings->current_options->filter_action_locked, &current_settings->current_options->transient_rejection, &current_settings->current_options->transient_rejection_buf, current_settings->current_options->transient_reject_message_buf, MAX_BUF);

    if (current_settings->current_options->filter_action <= FILTER_DECISION_DO_FILTER)
      {
      if ((current_settings->current_options->rejection != NULL) &&
          (current_settings->current_options->rejection->rejection_index == REJECTION_ACCESS_DENIED) &&
          (current_settings->current_options->rejection_text[REJECTION_ACCESS_DENIED] != NULL))
        set_rejection(current_settings, REJECTION_ACCESS_DENIED, &current_settings->current_options->rejection, &current_settings->current_options->rejection_buf, current_settings->current_options->reject_message_buf, MAX_BUF);

      if ((current_settings->current_options->filter_action == FILTER_DECISION_DO_FILTER) &&
          (current_settings->current_options->filter_grace <= FILTER_GRACE_AFTER_TO) &&
          (current_settings->current_options->rejection != NULL))
        {
        output_write_rejection(current_settings, current_settings->current_options->rejection, outbound_fd, NULL);
        current_settings->current_options->filter_grace = FILTER_GRACE_EXPIRED;
        return_value = FILTER_FLAG_INTERCEPT;
        }
      else
        {
        /* Post-RCPT TO filters */
        filter_recipient_local(current_settings, &current_settings->current_options->filter_action, &current_settings->current_options->filter_action_locked, &current_settings->current_options->transient_rejection, &current_settings->current_options->transient_rejection_buf, current_settings->current_options->transient_reject_message_buf, MAX_BUF);
        filter_recipient_max(current_settings, &current_settings->current_options->filter_action, &current_settings->current_options->filter_action_locked, &current_settings->current_options->transient_rejection, &current_settings->current_options->transient_rejection_buf, current_settings->current_options->transient_reject_message_buf, MAX_BUF);
        filter_recipient_blacklist(current_settings, &current_settings->current_options->filter_action, &current_settings->current_options->filter_action_locked, &current_settings->current_options->transient_rejection, &current_settings->current_options->transient_rejection_buf, current_settings->current_options->transient_reject_message_buf, MAX_BUF);
        filter_identical_from_to(current_settings, &current_settings->current_options->filter_action, &current_settings->current_options->filter_action_locked, &current_settings->current_options->transient_rejection, &current_settings->current_options->transient_rejection_buf, current_settings->current_options->transient_reject_message_buf, MAX_BUF);
        filter_recipient_graylist(current_settings, &current_settings->current_options->filter_action, &current_settings->current_options->filter_action_locked, &current_settings->current_options->transient_rejection, &current_settings->current_options->transient_rejection_buf, current_settings->current_options->transient_reject_message_buf, MAX_BUF);

        if (current_settings->current_options->filter_action == FILTER_DECISION_TRANSIENT_DO_FILTER)
          {
          output_write_rejection(current_settings, current_settings->current_options->transient_rejection, outbound_fd, NULL);
          return_value = FILTER_FLAG_INTERCEPT;
          }
        }
      }

    free_current_options(current_settings);

    if ((current_settings->current_options->filter_action == FILTER_DECISION_TRANSIENT_DO_NOT_FILTER) ||
        (current_settings->current_options->filter_action == FILTER_DECISION_TRANSIENT_DO_FILTER))
      current_settings->current_options->filter_action = current_settings->current_options->prev_filter_action;

    if ((return_value & FILTER_MASK_PASS) == FILTER_FLAG_PASS)
      return_value = FILTER_FLAG_PASS | FILTER_FLAG_RCPT_CAPTURE;
    else
      {
      SPAMDYKE_LOG_INFO(current_settings, "%s from: %s to: %s origin_ip: %s origin_rdns: %s auth: %s encryption: %s", (current_settings->current_options->transient_rejection != NULL) ? current_settings->current_options->transient_rejection->short_reject_message : ((current_settings->current_options->rejection != NULL) ? current_settings->current_options->rejection->short_reject_message : SHORT_SUCCESS), reassemble_address(current_settings->sender_username, current_settings->sender_domain, LOG_MISSING_DATA, tmp_sender_address, MAX_ADDRESS, NULL), reassemble_address(current_settings->recipient_username, current_settings->recipient_domain, LOG_MISSING_DATA, tmp_recipient_address, MAX_ADDRESS, NULL), (current_settings->server_ip != NULL) ? current_settings->server_ip : LOG_MISSING_DATA, (current_settings->strlen_server_name > 0) ? current_settings->server_name : LOG_MISSING_DATA, (current_settings->smtp_auth_username[0] != '\0') ? current_settings->smtp_auth_username : LOG_MISSING_DATA, tls_state_desc(current_settings));
      current_settings->current_options->transient_rejection = NULL;
      }
    }
  else if ((strlen_input_line >= STRLEN(SMTP_DATA)) &&
           (strncasecmp(SMTP_DATA, input_line, STRLEN(SMTP_DATA)) == 0))
    {
    if ((current_settings->current_options->filter_action == FILTER_DECISION_DO_FILTER) &&
        (current_settings->current_options->filter_grace <= FILTER_GRACE_AFTER_DATA) &&
        (current_settings->current_options->rejection != NULL))
      {
      output_write_rejection(current_settings, current_settings->current_options->rejection, outbound_fd, NULL);
      return_value = FILTER_FLAG_INTERCEPT | FILTER_FLAG_CHILD_QUIT;
      }
    else if (current_settings->num_rcpt_to == 0)
      {
      set_rejection(current_settings, REJECTION_ZERO_RECIPIENTS, &current_settings->current_options->transient_rejection, &current_settings->current_options->transient_rejection_buf, current_settings->current_options->transient_reject_message_buf, MAX_BUF);
      output_write_rejection(current_settings, current_settings->current_options->transient_rejection, STDOUT_FD, NULL);
      current_settings->current_options->transient_rejection = NULL;
      return_value = FILTER_FLAG_INTERCEPT | FILTER_FLAG_CHILD_CONTINUE;
      }
    else
      current_settings->inside_data = 1;
    }
  else if ((strlen_input_line >= STRLEN(SMTP_QUIT)) &&
           (strncasecmp(SMTP_QUIT, input_line, STRLEN(SMTP_QUIT)) == 0))
    {
    if ((current_settings->current_options->filter_action == FILTER_DECISION_DO_FILTER) &&
        (current_settings->current_options->filter_grace <= FILTER_GRACE_NONE) &&
        (current_settings->current_options->rejection != NULL))
      {
      output_write_rejection(current_settings, current_settings->current_options->rejection, outbound_fd, REJECT_SEVERITY_QUIT);
      return_value = FILTER_FLAG_QUIT;
      }
    }
  else if ((strlen_input_line >= STRLEN(SMTP_RSET)) &&
           (strncasecmp(SMTP_RSET, input_line, STRLEN(SMTP_RSET)) == 0))
    {
    if ((current_settings->current_options->filter_action != FILTER_DECISION_DO_FILTER) ||
        (current_settings->current_options->filter_grace != FILTER_GRACE_EXPIRED) ||
        (current_settings->current_options->rejection == NULL))
      {
      current_settings->num_rcpt_to = 0;
      current_settings->local_sender = 1;
      current_settings->sender_username[0] = '\0';
      current_settings->sender_domain[0] = '\0';
      current_settings->recipient_username[0] = '\0';
      current_settings->recipient_domain[0] = '\0';
      }
    else
      {
      output_write_rejection(current_settings, current_settings->current_options->rejection, outbound_fd, REJECT_SEVERITY_PERMANENT);
      return_value = FILTER_FLAG_INTERCEPT | FILTER_FLAG_CHILD_QUIT;
      }
    }
  else if ((strlen_input_line < STRLEN(SMTP_QUIT)) ||
           (strncasecmp(SMTP_QUIT, input_line, STRLEN(SMTP_QUIT)) != 0))
    {
    if ((current_settings->current_options->filter_action == FILTER_DECISION_DO_FILTER) &&
        (current_settings->current_options->filter_grace == FILTER_GRACE_EXPIRED) &&
        (current_settings->current_options->rejection != NULL))
      {
      output_write_rejection(current_settings, current_settings->current_options->rejection, outbound_fd, REJECT_SEVERITY_PERMANENT);
      return_value = FILTER_FLAG_INTERCEPT | FILTER_FLAG_CHILD_QUIT;
      }
    //FIXME: Not sure I like this here.  What if input for an unknown auth protocol looks like an SMTP command?
    else if (current_settings->smtp_auth_state == SMTP_AUTH_STATE_UNKNOWN)
      return_value = FILTER_FLAG_PASS | FILTER_FLAG_AUTH_CAPTURE;
    }

  return(return_value);
  }

/*
 * Expects:
 *   inbound_fd == pointer to input file descriptor from child process
 *   outbound_fd == pointer to output file descriptor to child process
 */
void middleman(int *inbound_fd, int *outbound_fd, char line_terminator, struct filter_settings *current_settings, int child_pid)
  {
  char buf_input[MAX_NETWORK_BUF + 1];
  char buf_childinput[MAX_NETWORK_BUF + 1];
  int strlen_buf_input;
  int strlen_buf_childinput;
  int strlen_buf_trim;
  int tmp_strlen;
  fd_set listen_set;
  struct timeval listen_timeout;
  int more_stdinput;
  int more_childinput;
  int discard_childinput;
  long current_time;
  int filter_return;
  int network_ready;
  int added_ehlo_continuation;
  int read_result;
  /* start_buf_input must never ==NULL unless strlen_buf_input==0 */
  char *start_buf_input;
  /* start_buf_childinput must never ==NULL unless strlen_buf_childinput==0 */
  char *start_buf_childinput;
  char *next_terminator;
  char *tmp_terminator;
  int child_response_needed;
  int max_fd;
  int select_result;
  int usable_buf_input;
  int timeout_printed;
  int wait_status;
  int child_stopped;
  char tmp_sender_address[MAX_ADDRESS + 1];
  char tmp_recipient_address[MAX_ADDRESS + 1];

  strlen_buf_input = 0;
  usable_buf_input = 0;
  buf_input[0] = '\0';
  start_buf_input = NULL;
  buf_childinput[0] = '\0';
  start_buf_childinput = NULL;
  strlen_buf_childinput = 0;
  filter_return = FILTER_FLAG_PASS;
  child_response_needed = (child_pid != -1) ? 1 : 0;
  timeout_printed = 0;
  child_stopped = 0;

  more_stdinput = 1;
  more_childinput = ((*inbound_fd >= 0) && (*outbound_fd >= 0)) ? 1 : 0;
  discard_childinput = !more_childinput;

  current_settings->connection_start = time(NULL);
  current_settings->command_start = time(NULL);

#ifdef HAVE_LIBSSL
  if ((current_settings->current_options->tls_certificate_file != NULL) &&
      (current_settings->current_options->tls_level == TLS_LEVEL_SMTPS))
    {
    if (tls_start(current_settings, STDIN_FD, STDOUT_FD))
      output_writeln(current_settings, LOG_ACTION_TLS_START, -1, NULL, -1);
    else
      {
      if ((*inbound_fd) != -1)
        {
        SPAMDYKE_LOG_EXCESSIVE(current_settings, LOG_DEBUGX_CHILD_IN_CLOSE, *inbound_fd);

        close(*inbound_fd);
        *inbound_fd = -1;
        }
      if ((*outbound_fd) != -1)
        {
        SPAMDYKE_LOG_EXCESSIVE(current_settings, LOG_DEBUGX_CHILD_OUT_CLOSE, *outbound_fd);

        close(*outbound_fd);
        *outbound_fd = -1;
        }

      SPAMDYKE_LOG_VERBOSE(current_settings, LOG_VERBOSE_SMTPS_FAILURE);

      more_stdinput = 0;
      more_childinput = 0;
      discard_childinput = 1;
      child_response_needed = 0;
      }
    }
  else
#endif /* HAVE_LIBSSL */

    if (current_settings->current_options->tls_level == TLS_LEVEL_SMTPS)
      {
      SPAMDYKE_LOG_ERROR(current_settings, LOG_ERROR_SMTPS_SUPPORT);

      more_stdinput = 0;
      more_childinput = 0;
      discard_childinput = 1;
      child_response_needed = 0;
      }

  /*
   * Deciding when to stop looping and disconnect is tricky and this code gets
   * updated with almost every version.  more_childinput should indicate if
   * the qmail child process is still running and thus if spamdyke should
   * continue to accept input from it.  discard_childinput should indicate if
   * spamdyke is discarding qmail's output.  This happens when a filter is
   * triggered and qmail is cut off.  more_stdinput should indicate if the
   * remote host is still connected and sending data.
   */
  while (more_childinput ||
         (discard_childinput &&
          more_stdinput))
    {
    current_time = time(NULL);

    if (!timeout_printed &&
        ((!child_response_needed &&
          (current_settings->current_options->timeout_connection > 0) &&
          ((current_time - current_settings->connection_start) > current_settings->current_options->timeout_connection)) ||
         ((current_settings->current_options->timeout_command > 0) &&
          ((current_time - current_settings->command_start) > current_settings->current_options->timeout_command))))
      {
      if (*outbound_fd >= 0)
        {
        SPAMDYKE_LOG_EXCESSIVE(current_settings, LOG_DEBUGX_CHILD_OUT_CLOSE, *outbound_fd);

        close(*outbound_fd);
        *outbound_fd = -1;
        }

      current_settings->current_options->timeout_connection = 0;
      current_settings->current_options->timeout_command = 0;

      set_rejection(current_settings, REJECTION_TIMEOUT, &current_settings->current_options->rejection, &current_settings->current_options->rejection_buf, current_settings->current_options->reject_message_buf, MAX_BUF);
      output_write_rejection(current_settings, current_settings->current_options->rejection, STDOUT_FD, NULL);

      more_stdinput = 0;
      discard_childinput = 1;
      child_response_needed = 0;

      SPAMDYKE_LOG_INFO(current_settings, "%s from: %s to: %s origin_ip: %s origin_rdns: %s auth: %s encryption: %s reason: %s", current_settings->current_options->rejection->short_reject_message, reassemble_address(current_settings->sender_username, current_settings->sender_domain, LOG_MISSING_DATA, tmp_sender_address, MAX_ADDRESS, NULL), reassemble_address(current_settings->recipient_username, current_settings->recipient_domain, LOG_MISSING_DATA, tmp_recipient_address, MAX_ADDRESS, NULL), current_settings->server_ip, (current_settings->strlen_server_name > 0) ? current_settings->server_name : LOG_MISSING_DATA, (current_settings->smtp_auth_username[0] != '\0') ? current_settings->smtp_auth_username : LOG_MISSING_DATA, tls_state_desc(current_settings), (current_settings->current_options->rejection != NULL) ? current_settings->current_options->rejection->short_reject_message : LOG_MISSING_DATA);

      timeout_printed = 1;
      }

    listen_timeout.tv_sec = 0;
    listen_timeout.tv_usec = 0;

    if ((current_settings->current_options->timeout_connection > 0) ||
        (current_settings->current_options->timeout_command > 0))
      {
      if (current_settings->current_options->timeout_command > 0)
        listen_timeout.tv_sec = MAXVAL(0, current_settings->current_options->timeout_command - (current_time - current_settings->command_start));

      if (current_settings->current_options->timeout_connection > 0)
        listen_timeout.tv_sec = MAXVAL(0, MINVAL(listen_timeout.tv_sec, current_settings->current_options->timeout_connection - (current_time - current_settings->connection_start)));

      if (listen_timeout.tv_sec > MAX_SELECT_SECS_TIMEOUT)
        {
        listen_timeout.tv_sec = MAX_SELECT_SECS_TIMEOUT;
        listen_timeout.tv_usec = MAX_SELECT_USECS_TIMEOUT;
        }
      else if ((listen_timeout.tv_sec == 0) ||
               (listen_timeout.tv_sec < MIN_SELECT_SECS_TIMEOUT))
        {
        listen_timeout.tv_sec = MIN_SELECT_SECS_TIMEOUT;
        listen_timeout.tv_usec = MIN_SELECT_USECS_TIMEOUT;
        }
      }
    else
      {
      listen_timeout.tv_sec = SELECT_SECS_NO_TIMEOUT;
      listen_timeout.tv_usec = SELECT_USECS_NO_TIMEOUT;
      }

    FD_ZERO(&listen_set);
    max_fd = 0;
    if (more_stdinput &&
        ((child_pid == -1) ||
         !child_response_needed))
      {
      FD_SET(STDIN_FD, &listen_set);
      max_fd = STDIN_FD;
      }
    if (more_childinput)
      {
      FD_SET(*inbound_fd, &listen_set);
      max_fd = *inbound_fd;
      }

    /*
     * Block here if no data is available for processing, either from the remote
     * server or from qmail.
     *
     * The NETWORK_CAN_READ macro only has meaning when TLS is available;
     * sometimes OpenSSL collects traffic in its buffer and requires multiple
     * reads to deliver it all.  select()ing on the socket in that state will
     * block because the data is in OpenSSL's buffer, not the network layer.
     */
    network_ready = 0;
    select_result = -1;
    if ((usable_buf_input &&
         !child_response_needed) ||
        (strlen_buf_childinput > 0) ||
        (!child_response_needed &&
         ((network_ready = NETWORK_CAN_READ(current_settings)) > 0)) ||
        ((select_result = select(max_fd + 1, &listen_set, NULL, NULL, &listen_timeout)) > 0))
      {
      if (select_result == -1)
        {
        if (network_ready == 0)
          {
          listen_timeout.tv_sec = 0;
          listen_timeout.tv_usec = 0;
          select(max_fd + 1, &listen_set, NULL, NULL, &listen_timeout);
          }
        else
          FD_ZERO(&listen_set);
        }

      next_terminator = NULL;

      /*
       * Accept and buffer input from qmail.
       */
      if ((*inbound_fd >= 0) &&
          FD_ISSET(*inbound_fd, &listen_set))
        {
        if ((read_result = read(*inbound_fd, buf_childinput + strlen_buf_childinput, MAX_NETWORK_BUF - strlen_buf_childinput)) > 0)
          {
          SPAMDYKE_LOG_EXCESSIVE(current_settings, LOG_DEBUGX_CHILD_READ, read_result, *inbound_fd, strlen_buf_childinput + read_result, (start_buf_childinput != NULL) ? (start_buf_childinput - buf_childinput) : 0);
          strlen_buf_childinput += read_result;
          buf_childinput[strlen_buf_childinput] = '\0';

          if (start_buf_childinput == NULL)
            start_buf_childinput = buf_childinput;
          }
        else
          {
          SPAMDYKE_LOG_EXCESSIVE(current_settings, LOG_DEBUGX_CHILD_FD_EOF, *inbound_fd, strlen_buf_childinput, (start_buf_childinput != NULL) ? (start_buf_childinput - buf_childinput) : 0);
          SPAMDYKE_LOG_EXCESSIVE(current_settings, LOG_DEBUGX_CHILD_IN_CLOSE, *inbound_fd);

          more_childinput = 0;
          close(*inbound_fd);
          *inbound_fd = -1;
          }
        }

      /*
       * Examine qmail's input and take action if necessary.
       */
      if (strlen_buf_childinput > 0)
        {
        while ((line_terminator != '\0') &&
               (current_settings->tls_state != TLS_STATE_ACTIVE_PASSTHROUGH) &&
               ((next_terminator = memchr(start_buf_childinput, line_terminator, strlen_buf_childinput - (start_buf_childinput - buf_childinput))) != NULL))
          {
          next_terminator++;
          child_response_needed = 0;
          current_settings->command_start = time(NULL);

          strlen_buf_trim = next_terminator - start_buf_childinput;
          while ((strlen_buf_trim > 0) &&
                 ((start_buf_childinput[strlen_buf_trim - 1] == '\r') ||
                  (start_buf_childinput[strlen_buf_trim - 1] == '\n')))
            strlen_buf_trim--;

          if (!discard_childinput)
            {
            if ((line_terminator != '\0') &&
                (((filter_return & FILTER_MASK_AUTH) != FILTER_FLAG_AUTH_NONE) ||
                 ((filter_return & FILTER_MASK_TLS) != FILTER_FLAG_TLS_NONE) ||
                 ((filter_return & FILTER_MASK_RCPT) != FILTER_FLAG_RCPT_NONE)) &&
                (strlen_buf_trim > 0))
              {
              if ((filter_return & FILTER_MASK_RCPT) == FILTER_FLAG_RCPT_CAPTURE)
                {
                /* The client said "RCPT TO" and spamdyke didn't block it.  Capture qmail's response. */
                if ((strlen_buf_trim >= STRLEN(SMTP_RCPT_SUCCESS)) &&
                    (strncasecmp(start_buf_childinput, SMTP_RCPT_SUCCESS, STRLEN(SMTP_RCPT_SUCCESS)) == 0))
                  {
                  /* qmail said "250".  Log success. */
                  current_settings->num_rcpt_to++;
                  current_settings->current_options->rejection = NULL;
                  }
                else
                  {
                  /* qmail did not say "250".  Log a rejection. */
                  SPAMDYKE_LOG_VERBOSE(current_settings, LOG_VERBOSE_FILTER_OTHER_REJECTION, strlen_buf_trim, start_buf_childinput);
                  set_rejection(current_settings, REJECTION_OTHER, &current_settings->current_options->transient_rejection, &current_settings->current_options->transient_rejection_buf, current_settings->current_options->transient_reject_message_buf, MAX_BUF);
                  }

                output_writeln(current_settings, LOG_ACTION_CHILD_FROM, STDOUT_FD, start_buf_childinput, next_terminator - start_buf_childinput);
                filter_return ^= FILTER_FLAG_RCPT_CAPTURE;

                SPAMDYKE_LOG_INFO(current_settings, "%s from: %s to: %s origin_ip: %s origin_rdns: %s auth: %s encryption: %s", (current_settings->current_options->transient_rejection != NULL) ? current_settings->current_options->transient_rejection->short_reject_message : ((current_settings->current_options->rejection != NULL) ? current_settings->current_options->rejection->short_reject_message : SHORT_SUCCESS), reassemble_address(current_settings->sender_username, current_settings->sender_domain, LOG_MISSING_DATA, tmp_sender_address, MAX_ADDRESS, NULL), reassemble_address(current_settings->recipient_username, current_settings->recipient_domain, LOG_MISSING_DATA, tmp_recipient_address, MAX_ADDRESS, NULL), (current_settings->server_ip != NULL) ? current_settings->server_ip : LOG_MISSING_DATA, (current_settings->strlen_server_name > 0) ? current_settings->server_name : LOG_MISSING_DATA, (current_settings->smtp_auth_username[0] != '\0') ? current_settings->smtp_auth_username : LOG_MISSING_DATA, tls_state_desc(current_settings));
                current_settings->current_options->transient_rejection = NULL;
                }
              else if ((((filter_return & FILTER_FLAG_AUTH_ADD) == FILTER_FLAG_AUTH_ADD) ||
                        ((filter_return & FILTER_FLAG_AUTH_REMOVE) == FILTER_FLAG_AUTH_REMOVE) ||
                        ((filter_return & FILTER_MASK_TLS) == FILTER_FLAG_TLS_ADD) ||
                        ((filter_return & FILTER_MASK_TLS) == FILTER_FLAG_TLS_REMOVE)) &&
                       (strlen_buf_trim >= STRLEN(SMTP_EHLO_SUCCESS)) &&
                       (strncasecmp(start_buf_childinput, SMTP_EHLO_SUCCESS, STRLEN(SMTP_EHLO_SUCCESS)) == 0))
                {
                /* The server said "250". */
                added_ehlo_continuation = 0;

                if (((filter_return & FILTER_FLAG_AUTH_ADD) == FILTER_FLAG_AUTH_ADD) &&
                    (((strlen_buf_trim >= (STRLEN(SMTP_EHLO_SUCCESS) + STRLEN(SMTP_EHLO_AUTH_CORRECT) + 1)) &&
                      (strncasecmp(start_buf_childinput + STRLEN(SMTP_EHLO_SUCCESS) + 1, SMTP_EHLO_AUTH_CORRECT, STRLEN(SMTP_EHLO_AUTH_CORRECT)) == 0)) ||
                     ((strlen_buf_trim >= (STRLEN(SMTP_EHLO_SUCCESS) + STRLEN(SMTP_EHLO_AUTH_INCORRECT) + 1)) &&
                      (strncasecmp(start_buf_childinput + STRLEN(SMTP_EHLO_SUCCESS) + 1, SMTP_EHLO_AUTH_INCORRECT, STRLEN(SMTP_EHLO_AUTH_INCORRECT)) == 0))))
                  {
                  /* The server said "250 AUTH " or "250 AUTH=". */
                  if ((current_settings->current_options->smtp_auth_level & SMTP_AUTH_LEVEL_MASK) >= SMTP_AUTH_LEVEL_VALUE_ALWAYS)
                    {
                    /* spamdyke is supposed to offer its own AUTH, so remove the child's response line. */
                    output_writeln(current_settings, LOG_ACTION_CHILD_FROM_DISCARDED, -1, start_buf_childinput, next_terminator - start_buf_childinput);

                    current_settings->smtp_auth_origin = SMTP_AUTH_ORIGIN_SPAMDYKE;

                    /* Add a "250 AUTH" (or "250-AUTH") line. */
                    output_writeln(current_settings, LOG_ACTION_FILTER_FROM, STDOUT_FD, SMTP_EHLO_SUCCESS, STRLEN(SMTP_EHLO_SUCCESS));

                    if (start_buf_childinput[STRLEN(SMTP_EHLO_SUCCESS)] == SMTP_CONTINUATION)
                      output_writeln(current_settings, LOG_ACTION_FILTER_FROM, STDOUT_FD, SMTP_STR_CONTINUATION, STRLEN(SMTP_STR_CONTINUATION));
                    else
                      output_writeln(current_settings, LOG_ACTION_FILTER_FROM, STDOUT_FD, SMTP_STR_DONE, STRLEN(SMTP_STR_DONE));

                    if (((current_settings->current_options->smtp_auth_level & SMTP_AUTH_LEVEL_MASK) == SMTP_AUTH_LEVEL_VALUE_ON_DEMAND_ENCRYPTED) ||
                        ((current_settings->current_options->smtp_auth_level & SMTP_AUTH_LEVEL_MASK) == SMTP_AUTH_LEVEL_VALUE_ALWAYS_ENCRYPTED))
                      output_writeln(current_settings, LOG_ACTION_FILTER_FROM, STDOUT_FD, SMTP_EHLO_AUTH_INSERT_ENCRYPTION, STRLEN(SMTP_EHLO_AUTH_INSERT_ENCRYPTION));
                    else
                      output_writeln(current_settings, LOG_ACTION_FILTER_FROM, STDOUT_FD, SMTP_EHLO_AUTH_INSERT_CLEAR, STRLEN(SMTP_EHLO_AUTH_INSERT_CLEAR));
                    }
                  else
                    {
                    /* The server said "250 AUTH ".  Hopefully it doesn't support an algorithm spamdyke can't handle... */
                    if (((filter_return & FILTER_MASK_TLS) == FILTER_FLAG_TLS_ADD) &&
                        (start_buf_childinput[STRLEN(SMTP_EHLO_SUCCESS)] != SMTP_CONTINUATION))
                      {
                      start_buf_childinput[STRLEN(SMTP_EHLO_SUCCESS)] = SMTP_CONTINUATION;
                      added_ehlo_continuation = 1;
                      }

                    current_settings->smtp_auth_origin = SMTP_AUTH_ORIGIN_CHILD;
                    output_writeln(current_settings, LOG_ACTION_CHILD_FROM, STDOUT_FD, start_buf_childinput, next_terminator - start_buf_childinput);
                    }

                  strlen_buf_trim = 0;
                  filter_return ^= FILTER_FLAG_AUTH_ADD;
                  }
                else if (((filter_return & FILTER_FLAG_AUTH_REMOVE) == FILTER_FLAG_AUTH_REMOVE) &&
                         (((strlen_buf_trim >= (STRLEN(SMTP_EHLO_SUCCESS) + STRLEN(SMTP_EHLO_AUTH_CORRECT) + 1)) &&
                           (strncasecmp(start_buf_childinput + STRLEN(SMTP_EHLO_SUCCESS) + 1, SMTP_EHLO_AUTH_CORRECT, STRLEN(SMTP_EHLO_AUTH_CORRECT)) == 0)) ||
                          ((strlen_buf_trim >= (STRLEN(SMTP_EHLO_SUCCESS) + STRLEN(SMTP_EHLO_AUTH_INCORRECT) + 1)) &&
                           (strncasecmp(start_buf_childinput + STRLEN(SMTP_EHLO_SUCCESS) + 1, SMTP_EHLO_AUTH_INCORRECT, STRLEN(SMTP_EHLO_AUTH_INCORRECT)) == 0))))
                  {
                  /* The server said "250 AUTH".  Remove it. */
                  output_writeln(current_settings, LOG_ACTION_CHILD_FROM_DISCARDED, -1, start_buf_childinput, next_terminator - start_buf_childinput);

                  if (((filter_return & FILTER_MASK_TLS) != FILTER_FLAG_TLS_ADD) &&
                      (start_buf_childinput[STRLEN(SMTP_EHLO_SUCCESS)] != SMTP_CONTINUATION))
                    {
                    /* The server said "250 AUTH".  Remove it and replace it with something bogus because the previous line was a continuation. */
                    output_writeln(current_settings, LOG_ACTION_FILTER_FROM, STDOUT_FD, SMTP_EHLO_SUCCESS, STRLEN(SMTP_EHLO_SUCCESS));
                    output_writeln(current_settings, LOG_ACTION_FILTER_FROM, STDOUT_FD, SMTP_STR_DONE, STRLEN(SMTP_STR_DONE));
                    output_writeln(current_settings, LOG_ACTION_FILTER_FROM, STDOUT_FD, SMTP_EHLO_NOTHING_INSERT, STRLEN(SMTP_EHLO_NOTHING_INSERT));
                    }

                  strlen_buf_trim = 0;
                  }
                else if (((filter_return & FILTER_MASK_TLS) == FILTER_FLAG_TLS_ADD) &&
                        (strlen_buf_trim >= (STRLEN(SMTP_EHLO_SUCCESS) + STRLEN(SMTP_EHLO_TLS) + 1)) &&
                        (strncasecmp(start_buf_childinput + STRLEN(SMTP_EHLO_SUCCESS) + 1, SMTP_EHLO_TLS, STRLEN(SMTP_EHLO_TLS)) == 0))
                  {
                  /* The server said "250 STARTTLS". */
                  if (((filter_return & FILTER_FLAG_AUTH_ADD) == FILTER_FLAG_AUTH_ADD) &&
                      (start_buf_childinput[STRLEN(SMTP_EHLO_SUCCESS)] != SMTP_CONTINUATION))
                    {
                    start_buf_childinput[STRLEN(SMTP_EHLO_SUCCESS)] = SMTP_CONTINUATION;
                    added_ehlo_continuation = 1;
                    }

                  strlen_buf_trim = 0;
                  filter_return ^= FILTER_FLAG_TLS_ADD;
                  output_writeln(current_settings, LOG_ACTION_CHILD_FROM, STDOUT_FD, start_buf_childinput, next_terminator - start_buf_childinput);
                  }
                else if (((filter_return & FILTER_MASK_TLS) == FILTER_FLAG_TLS_REMOVE) &&
                        (strlen_buf_trim >= (STRLEN(SMTP_EHLO_SUCCESS) + STRLEN(SMTP_EHLO_TLS) + 1)) &&
                        (strncasecmp(start_buf_childinput + STRLEN(SMTP_EHLO_SUCCESS) + 1, SMTP_EHLO_TLS, STRLEN(SMTP_EHLO_TLS)) == 0))
                  {
                  /* The server said "250 STARTTLS".  Remove it. */
                  output_writeln(current_settings, LOG_ACTION_CHILD_FROM_DISCARDED, -1, start_buf_childinput, next_terminator - start_buf_childinput);

                  if (((filter_return & FILTER_FLAG_AUTH_ADD) != FILTER_FLAG_AUTH_ADD) &&
                      (start_buf_childinput[STRLEN(SMTP_EHLO_SUCCESS)] != SMTP_CONTINUATION))
                    {
                    /* The server said "250 STARTTLS".  Remove it and replace it with something bogus because the previous line was a continuation. */
                    output_writeln(current_settings, LOG_ACTION_FILTER_FROM, STDOUT_FD, SMTP_EHLO_SUCCESS, STRLEN(SMTP_EHLO_SUCCESS));
                    output_writeln(current_settings, LOG_ACTION_FILTER_FROM, STDOUT_FD, SMTP_STR_DONE, STRLEN(SMTP_STR_DONE));
                    output_writeln(current_settings, LOG_ACTION_FILTER_FROM, STDOUT_FD, SMTP_EHLO_NOTHING_INSERT, STRLEN(SMTP_EHLO_NOTHING_INSERT));
                    }

                  strlen_buf_trim = 0;
                  filter_return ^= FILTER_FLAG_TLS_REMOVE;
                  }

                if (strlen_buf_trim > 0)
                  {
                  /* This line is not "AUTH" or "STARTTLS". */
                  if ((strlen_buf_trim >= (STRLEN(SMTP_EHLO_SUCCESS) + 1)) &&
                      (start_buf_childinput[STRLEN(SMTP_EHLO_SUCCESS)] != SMTP_CONTINUATION) &&
                      (((filter_return & FILTER_FLAG_AUTH_ADD) == FILTER_FLAG_AUTH_ADD) ||
                       ((filter_return & FILTER_MASK_TLS) == FILTER_FLAG_TLS_ADD)))
                    {
                    /* The server said "250 " without saying "AUTH" and/or "STARTTLS", so change the last line to "250-". */
                    start_buf_childinput[STRLEN(SMTP_EHLO_SUCCESS)] = SMTP_CONTINUATION;
                    added_ehlo_continuation = 1;
                    }

                  output_writeln(current_settings, LOG_ACTION_CHILD_FROM, STDOUT_FD, start_buf_childinput, next_terminator - start_buf_childinput);
                  strlen_buf_trim = 0;
                  }

                if ((((filter_return & FILTER_FLAG_AUTH_ADD) == FILTER_FLAG_AUTH_ADD) ||
                     ((filter_return & FILTER_FLAG_AUTH_REMOVE) == FILTER_FLAG_AUTH_REMOVE) ||
                     ((filter_return & FILTER_MASK_TLS) == FILTER_FLAG_TLS_ADD) ||
                     ((filter_return & FILTER_MASK_TLS) == FILTER_FLAG_TLS_REMOVE)) &&
                     added_ehlo_continuation)
                  {
                  if ((filter_return & FILTER_FLAG_AUTH_ADD) == FILTER_FLAG_AUTH_ADD)
                    {
                    filter_return ^= FILTER_FLAG_AUTH_ADD;
                    current_settings->smtp_auth_origin = SMTP_AUTH_ORIGIN_SPAMDYKE;

                    /* Add a "250 AUTH" (or "250-AUTH") line. */
                    output_writeln(current_settings, LOG_ACTION_FILTER_FROM, STDOUT_FD, SMTP_EHLO_SUCCESS, STRLEN(SMTP_EHLO_SUCCESS));

                    if ((filter_return & FILTER_MASK_TLS) == FILTER_FLAG_TLS_ADD)
                      output_writeln(current_settings, LOG_ACTION_FILTER_FROM, STDOUT_FD, SMTP_STR_CONTINUATION, STRLEN(SMTP_STR_CONTINUATION));
                    else
                      output_writeln(current_settings, LOG_ACTION_FILTER_FROM, STDOUT_FD, SMTP_STR_DONE, STRLEN(SMTP_STR_DONE));

                    if (((current_settings->current_options->smtp_auth_level & SMTP_AUTH_LEVEL_MASK) == SMTP_AUTH_LEVEL_VALUE_ON_DEMAND_ENCRYPTED) ||
                        ((current_settings->current_options->smtp_auth_level & SMTP_AUTH_LEVEL_MASK) == SMTP_AUTH_LEVEL_VALUE_ALWAYS_ENCRYPTED))
                      output_writeln(current_settings, LOG_ACTION_FILTER_FROM, STDOUT_FD, SMTP_EHLO_AUTH_INSERT_ENCRYPTION, STRLEN(SMTP_EHLO_AUTH_INSERT_ENCRYPTION));
                    else
                      output_writeln(current_settings, LOG_ACTION_FILTER_FROM, STDOUT_FD, SMTP_EHLO_AUTH_INSERT_CLEAR, STRLEN(SMTP_EHLO_AUTH_INSERT_CLEAR));
                    }
                  else if ((filter_return & FILTER_FLAG_AUTH_REMOVE) == FILTER_FLAG_AUTH_REMOVE)
                    filter_return ^= FILTER_FLAG_AUTH_REMOVE;

                  if ((filter_return & FILTER_MASK_TLS) == FILTER_FLAG_TLS_ADD)
                    {
                    filter_return ^= FILTER_FLAG_TLS_ADD;

                    /* Add a "250 STARTTLS" line. */
                    output_writeln(current_settings, LOG_ACTION_FILTER_FROM, STDOUT_FD, SMTP_EHLO_SUCCESS, STRLEN(SMTP_EHLO_SUCCESS));
                    output_writeln(current_settings, LOG_ACTION_FILTER_FROM, STDOUT_FD, SMTP_STR_DONE, STRLEN(SMTP_STR_DONE));
                    output_writeln(current_settings, LOG_ACTION_FILTER_FROM, STDOUT_FD, SMTP_EHLO_TLS_INSERT, STRLEN(SMTP_EHLO_TLS_INSERT));
                    }
                  else if ((filter_return & FILTER_MASK_TLS) == FILTER_FLAG_TLS_REMOVE)
                    filter_return ^= FILTER_FLAG_TLS_REMOVE;
                  }
                else if (strlen_buf_trim > 0)
                  output_writeln(current_settings, LOG_ACTION_CHILD_FROM, STDOUT_FD, start_buf_childinput, next_terminator - start_buf_childinput);
                }
              else if (((filter_return & FILTER_FLAG_AUTH_CAPTURE) == FILTER_FLAG_AUTH_CAPTURE) &&
                       (current_settings->smtp_auth_state == SMTP_AUTH_STATE_CMD_SEEN) &&
                       (strlen_buf_trim >= STRLEN(SMTP_AUTH_CHALLENGE)) &&
                       (strncasecmp(start_buf_childinput, SMTP_AUTH_CHALLENGE, STRLEN(SMTP_AUTH_CHALLENGE)) == 0))
                {
                memcpy(current_settings->smtp_auth_challenge, start_buf_childinput, sizeof(char) * MINVAL(MAX_BUF, strlen_buf_trim));
                current_settings->smtp_auth_challenge[MINVAL(MAX_BUF, strlen_buf_trim)] = '\0';
                current_settings->smtp_auth_state = SMTP_AUTH_STATE_CHALLENGE_1_SENT;
                output_writeln(current_settings, LOG_ACTION_CHILD_FROM, STDOUT_FD, start_buf_childinput, next_terminator - start_buf_childinput);
                }
              else if (((filter_return & FILTER_FLAG_AUTH_CAPTURE) == FILTER_FLAG_AUTH_CAPTURE) &&
                       (current_settings->smtp_auth_state == SMTP_AUTH_STATE_RESPONSE_1_SEEN) &&
                       (strlen_buf_trim >= STRLEN(SMTP_AUTH_CHALLENGE)) &&
                       (strncasecmp(start_buf_childinput, SMTP_AUTH_CHALLENGE, STRLEN(SMTP_AUTH_CHALLENGE)) == 0))
                {
                current_settings->smtp_auth_state = SMTP_AUTH_STATE_CHALLENGE_2_SENT;
                output_writeln(current_settings, LOG_ACTION_CHILD_FROM, STDOUT_FD, start_buf_childinput, next_terminator - start_buf_childinput);
                }
              else if (((filter_return & FILTER_FLAG_AUTH_CAPTURE) == FILTER_FLAG_AUTH_CAPTURE) &&
                       (current_settings->smtp_auth_state == SMTP_AUTH_STATE_RESPONSE_2_SEEN) &&
                       (strlen_buf_trim >= STRLEN(SMTP_AUTH_SUCCESS)) &&
                       (strncasecmp(start_buf_childinput, SMTP_AUTH_SUCCESS, STRLEN(SMTP_AUTH_SUCCESS)) == 0))
                {
                current_settings->smtp_auth_state = SMTP_AUTH_STATE_AUTHENTICATED;
                if (!current_settings->current_options->filter_action_locked)
                  {
                  current_settings->current_options->filter_action = FILTER_DECISION_DO_NOT_FILTER;
                  current_settings->current_options->rejection = NULL;
                  }

                filter_return ^= FILTER_FLAG_AUTH_CAPTURE;
                output_writeln(current_settings, LOG_ACTION_AUTH_SUCCESS, -1, NULL, -1);
                output_writeln(current_settings, LOG_ACTION_CHILD_FROM, STDOUT_FD, start_buf_childinput, next_terminator - start_buf_childinput);
                }
              else if (((filter_return & FILTER_FLAG_AUTH_CAPTURE) == FILTER_FLAG_AUTH_CAPTURE) &&
                       (current_settings->smtp_auth_state == SMTP_AUTH_STATE_UNKNOWN) &&
                       (strlen_buf_trim >= STRLEN(SMTP_AUTH_PROMPT)) &&
                       (strncasecmp(start_buf_childinput, SMTP_AUTH_PROMPT, STRLEN(SMTP_AUTH_PROMPT)) == 0))
                {
                // Continue watching the AUTH conversation.
                output_writeln(current_settings, LOG_ACTION_CHILD_FROM, STDOUT_FD, start_buf_childinput, next_terminator - start_buf_childinput);
                }
              else if (((filter_return & FILTER_FLAG_AUTH_CAPTURE) == FILTER_FLAG_AUTH_CAPTURE) &&
                       (current_settings->smtp_auth_state == SMTP_AUTH_STATE_UNKNOWN) &&
                       (strlen_buf_trim >= STRLEN(SMTP_AUTH_SUCCESS)) &&
                       (strncasecmp(start_buf_childinput, SMTP_AUTH_SUCCESS, STRLEN(SMTP_AUTH_SUCCESS)) == 0))
                {
                tmp_strlen = strlen(current_settings->smtp_auth_response);
                memcpy(current_settings->smtp_auth_username, current_settings->smtp_auth_response, sizeof(char) * MINVAL(tmp_strlen, MAX_BUF));
                current_settings->smtp_auth_username[MINVAL(tmp_strlen, MAX_BUF)] = '\0';

                current_settings->smtp_auth_state = SMTP_AUTH_STATE_AUTHENTICATED;
                if (!current_settings->current_options->filter_action_locked)
                  {
                  current_settings->current_options->filter_action = FILTER_DECISION_DO_NOT_FILTER;
                  current_settings->current_options->rejection = NULL;
                  }

                filter_return ^= FILTER_FLAG_AUTH_CAPTURE;
                output_writeln(current_settings, LOG_ACTION_AUTH_SUCCESS, -1, NULL, -1);
                output_writeln(current_settings, LOG_ACTION_CHILD_FROM, STDOUT_FD, start_buf_childinput, next_terminator - start_buf_childinput);
                }
              else if (((filter_return & FILTER_FLAG_AUTH_CAPTURE) == FILTER_FLAG_AUTH_CAPTURE) &&
                       (current_settings->smtp_auth_state == SMTP_AUTH_STATE_UNKNOWN) &&
                       (strlen_buf_trim >= STRLEN(SMTP_AUTH_FAILURE)) &&
                       (strncasecmp(start_buf_childinput, SMTP_AUTH_FAILURE, STRLEN(SMTP_AUTH_FAILURE)) == 0))
                {
                current_settings->smtp_auth_state = SMTP_AUTH_STATE_NONE;
                filter_return ^= FILTER_FLAG_AUTH_CAPTURE;
                output_writeln(current_settings, LOG_ACTION_AUTH_FAILURE, -1, (current_settings->smtp_auth_response[0] != '\0') ? current_settings->smtp_auth_response : LOG_MISSING_DATA, -1);
                output_writeln(current_settings, LOG_ACTION_CHILD_FROM, STDOUT_FD, start_buf_childinput, next_terminator - start_buf_childinput);
                }
              else if (((filter_return & FILTER_MASK_TLS) == FILTER_FLAG_TLS_CAPTURE) &&
                       (strlen_buf_trim >= STRLEN(SMTP_TLS_SUCCESS)) &&
                       (strncasecmp(start_buf_childinput, SMTP_TLS_SUCCESS, STRLEN(SMTP_TLS_SUCCESS)) == 0))
                {
                output_writeln(current_settings, LOG_ACTION_CHILD_FROM, STDOUT_FD, start_buf_childinput, next_terminator - start_buf_childinput);
                current_settings->tls_state = TLS_STATE_ACTIVE_PASSTHROUGH;
                filter_return ^= FILTER_FLAG_TLS_CAPTURE;
                output_writeln(current_settings, LOG_ACTION_TLS_PASSTHROUGH_START, -1, NULL, -1);
                output_writeln(current_settings, LOG_ACTION_CHILD_FROM, STDOUT_FD, next_terminator, strlen_buf_childinput - (next_terminator - start_buf_childinput));
                next_terminator = NULL;
                start_buf_childinput = NULL;
                strlen_buf_childinput = 0;
                child_response_needed = 0;
                line_terminator = '\0';
                buf_childinput[strlen_buf_childinput] = '\0';

                SPAMDYKE_LOG_INFO(current_settings, "%s from: %s to: %s origin_ip: %s origin_rdns: %s auth: %s encryption: %s", SHORT_TLS_PASSTHROUGH, LOG_MISSING_DATA, LOG_MISSING_DATA, (current_settings->server_ip != NULL) ? current_settings->server_ip : LOG_MISSING_DATA, (current_settings->strlen_server_name > 0) ? current_settings->server_name : LOG_MISSING_DATA, (current_settings->smtp_auth_username[0] != '\0') ? current_settings->smtp_auth_username : LOG_MISSING_DATA, tls_state_desc(current_settings));
                }
              else
                {
                if ((filter_return & FILTER_FLAG_AUTH_CAPTURE) == FILTER_FLAG_AUTH_CAPTURE)
                  {
                  filter_return ^= FILTER_FLAG_AUTH_CAPTURE;
                  current_settings->smtp_auth_state = SMTP_AUTH_STATE_NONE;
                  }
                else if ((filter_return & FILTER_MASK_TLS) == FILTER_FLAG_TLS_CAPTURE)
                  filter_return ^= FILTER_FLAG_TLS_CAPTURE;

                output_writeln(current_settings, LOG_ACTION_CHILD_FROM, STDOUT_FD, start_buf_childinput, next_terminator - start_buf_childinput);
                }
              }
            else
              output_writeln(current_settings, LOG_ACTION_CHILD_FROM, STDOUT_FD, start_buf_childinput, next_terminator - start_buf_childinput);
            }
          else
            output_writeln(current_settings, LOG_ACTION_CHILD_FROM_DISCARDED, -1, start_buf_childinput, next_terminator - start_buf_childinput);

          start_buf_childinput = next_terminator;
          }

        /*
         * Move data and/or pointers to remove the buffered data that has
         * already been processed.
         */
        if ((line_terminator != '\0') &&
            (current_settings->tls_state != TLS_STATE_ACTIVE_PASSTHROUGH))
          {
          if (strlen_buf_childinput > 0)
            {
            if ((start_buf_childinput - buf_childinput) >= strlen_buf_childinput)
              {
              /* Used entire buffer, reset pointers */
              start_buf_childinput = NULL;
              strlen_buf_childinput = 0;
              }
            else if (strlen_buf_childinput >= MAX_NETWORK_BUF)
              {
              /* Buffer is full */
              if (start_buf_childinput > buf_childinput)
                {
                /* Unused space at the start, move data from the end to the start */
                strlen_buf_childinput -= start_buf_childinput - buf_childinput;
                memmove(buf_childinput, start_buf_childinput, strlen_buf_childinput);
                start_buf_childinput = buf_childinput;
                }
              else
                {
                /* Buffer is completely full, output data */
                output_writeln(current_settings, LOG_ACTION_CHILD_FROM, STDOUT_FD, start_buf_childinput, strlen_buf_childinput);
                start_buf_childinput = NULL;
                strlen_buf_childinput = 0;
                }
              }
            else if ((next_terminator == NULL) &&
                     (start_buf_childinput != buf_childinput))
              {
              /* Buffer is not full and no terminator found, move data */
              strlen_buf_childinput -= start_buf_childinput - buf_childinput;
              memmove(buf_childinput, start_buf_childinput, strlen_buf_childinput);
              start_buf_childinput = buf_childinput;
              }
            }
          }
        else if (strlen_buf_childinput > 0)
          {
          output_writeln(current_settings, LOG_ACTION_CHILD_FROM, STDOUT_FD, buf_childinput, strlen_buf_childinput);
          start_buf_childinput = NULL;
          strlen_buf_childinput = 0;
          }

        buf_childinput[strlen_buf_childinput] = '\0';
        }

      next_terminator = NULL;

      /*
       * Accept and buffer input from the remote server
       */
      if (FD_ISSET(STDIN_FD, &listen_set) ||
          NETWORK_CAN_READ(current_settings))
        {
        if ((read_result = NETWORK_READ(current_settings, STDIN_FD, buf_input + strlen_buf_input, MAX_NETWORK_BUF - strlen_buf_input)) > 0)
          {
          SPAMDYKE_LOG_EXCESSIVE(current_settings, LOG_DEBUGX_NETWORK_READ, read_result, STDIN_FD, strlen_buf_input + read_result, (start_buf_input != NULL) ? (start_buf_input - buf_input) : 0);
          strlen_buf_input += read_result;
          buf_input[strlen_buf_input] = '\0';
          usable_buf_input = 1;

          if (current_settings->tls_state != TLS_STATE_ACTIVE_PASSTHROUGH)
            {
            while ((strlen_buf_input > 0) &&
                   (buf_input[strlen_buf_input - 1] == '\0'))
              strlen_buf_input--;

            if ((strlen_buf_input > 0) &&
                (start_buf_input == NULL))
              start_buf_input = buf_input;
            }

          current_settings->command_start = time(NULL);
          }
        else if (!NETWORK_CAN_READ(current_settings))
          {
          SPAMDYKE_LOG_EXCESSIVE(current_settings, LOG_DEBUGX_NETWORK_FD_EOF, STDIN_FD, strlen_buf_input, (start_buf_input != NULL) ? (start_buf_input - buf_input) : 0);
          more_stdinput = 0;
          }
        }

      /*
       * Examine the remote server's input and pass it to smtp_filter() for
       * processing.  Depending on smtp_filter()'s return code, pass the
       * input to qmail or block it.
       */
      if ((strlen_buf_input > 0) &&
          !child_response_needed)
        {
        if ((line_terminator != '\0') &&
            (current_settings->tls_state != TLS_STATE_ACTIVE_PASSTHROUGH))
          {
          if (start_buf_input == NULL)
            start_buf_input = buf_input;

          if (((next_terminator = memchr(start_buf_input, line_terminator, strlen_buf_input - (start_buf_input - buf_input))) != NULL) ||
              (strlen_buf_input == MAX_NETWORK_BUF))
            {
            if (next_terminator != NULL)
              {
              next_terminator++;

              /*
               * This block allows spamdyke to "burst" message data to qmail
               * instead of sending it line-by-line.  For messages with large
               * attachments, this can be a big win (assuming the remote server
               * also sends the data in large chunks).
               */
              if ((current_settings->inside_data) &&
                  (strncmp(start_buf_input, SMTP_DATA_END, STRLEN(SMTP_DATA_END)) != 0))
                while ((tmp_terminator = memchr(next_terminator, line_terminator, strlen_buf_input - (next_terminator - buf_input))) != NULL)
                  {
                  if (strncmp(next_terminator, SMTP_DATA_END, STRLEN(SMTP_DATA_END)) != 0)
                    next_terminator = tmp_terminator + 1;
                  else
                    break;
                  }

              strlen_buf_trim = next_terminator - start_buf_input;
              while ((strlen_buf_trim > 0) &&
                     ((start_buf_input[strlen_buf_trim - 1] == '\r') ||
                      (start_buf_input[strlen_buf_trim - 1] == '\n')))
                strlen_buf_trim--;
              }
            else
              {
              next_terminator = buf_input + strlen_buf_input;
              strlen_buf_trim = strlen_buf_input;
              }

            output_writeln(current_settings, LOG_ACTION_REMOTE_FROM, -1, start_buf_input, next_terminator - start_buf_input);

            filter_return = smtp_filter(STDIN_FD, STDOUT_FD, start_buf_input, strlen_buf_trim, current_settings);
            switch (filter_return & FILTER_MASK_PASS)
              {
              case FILTER_FLAG_PASS:
                child_response_needed = (((filter_return & FILTER_MASK_CHILD_RESPONSE) == FILTER_FLAG_CHILD_RESPONSE_NEEDED) && !discard_childinput) ? 1 : 0;
                output_writeln(current_settings, LOG_ACTION_NONE, *outbound_fd, start_buf_input, next_terminator - start_buf_input);
                break;
              case FILTER_FLAG_QUIT:
                more_stdinput = 0;
                discard_childinput = 1;
                child_response_needed = 0;

                break;
              case FILTER_FLAG_INTERCEPT:
                child_response_needed = 0;

                if (((filter_return & FILTER_MASK_CHILD) == FILTER_FLAG_CHILD_QUIT) &&
                    (current_settings->current_options->rejection != NULL) &&
                    (*outbound_fd >= 0) &&
                    !discard_childinput)
                  {
                  SPAMDYKE_LOG_EXCESSIVE(current_settings, LOG_DEBUGX_CHILD_OUT_CLOSE, *outbound_fd);

                  close(*outbound_fd);
                  *outbound_fd = -1;

                  discard_childinput = 1;

                  if (current_settings->current_options->timeout_command == 0)
                    {
                    current_settings->current_options->timeout_command = TIMEOUT_IDLE_AFTER_QUIT_SECS;
                    SPAMDYKE_LOG_DEBUG(current_settings, LOG_DEBUG_IDLE_RESET, current_settings->current_options->timeout_command);
                    }
                  }
              }

            start_buf_input = next_terminator;
            }
          else
            {
            if (!more_stdinput)
              {
              if ((*outbound_fd) >= 0)
                {
                if ((strlen_buf_input - (start_buf_input - buf_input)) > 0)
                  output_writeln(current_settings, LOG_ACTION_REMOTE_FROM, *outbound_fd, start_buf_input, strlen_buf_input - (start_buf_input - buf_input));

                SPAMDYKE_LOG_EXCESSIVE(current_settings, LOG_DEBUGX_CHILD_OUT_CLOSE, *outbound_fd);

                close(*outbound_fd);
                *outbound_fd = -1;
                }

              strlen_buf_input = 0;
              buf_input[strlen_buf_input] = '\0';
              start_buf_input = NULL;
              }

            usable_buf_input = 0;
            }

          if (strlen_buf_input > 0)
            {
            if ((start_buf_input - buf_input) >= strlen_buf_input)
              {
              /* Used entire buffer, reset pointers */
              start_buf_input = NULL;
              strlen_buf_input = 0;
              }
            else if (strlen_buf_input >= MAX_NETWORK_BUF)
              {
              /* Buffer is full */
              if (start_buf_input > buf_input)
                {
                /* Unused space at the start, move data from the end to the start */
                strlen_buf_input -= start_buf_input - buf_input;
                memmove(buf_input, start_buf_input, strlen_buf_input);
                start_buf_input = buf_input;
                }
              else
                {
                /* Buffer is completely full, output data */
                output_writeln(current_settings, LOG_ACTION_REMOTE_FROM, *outbound_fd, start_buf_input, strlen_buf_input);
                start_buf_input = NULL;
                strlen_buf_input = 0;
                }
              }
            else if ((next_terminator == NULL) &&
                     (start_buf_input != buf_input))
              {
              /* Buffer is not full and no terminator found, move data */
              strlen_buf_input -= start_buf_input - buf_input;
              memmove(buf_input, start_buf_input, strlen_buf_input);
              start_buf_input = buf_input;
              }
            }
          }
        else
          {
          output_writeln(current_settings, LOG_ACTION_REMOTE_FROM, *outbound_fd, buf_input, strlen_buf_input);
          start_buf_input = NULL;
          strlen_buf_input = 0;
          }

        buf_input[strlen_buf_input] = '\0';
        }
      else if (!more_stdinput &&
               ((*outbound_fd) != -1))
        {
        SPAMDYKE_LOG_EXCESSIVE(current_settings, LOG_DEBUGX_CHILD_OUT_CLOSE, *outbound_fd);

        close(*outbound_fd);
        *outbound_fd = -1;

        discard_childinput = 1;
        }

      if (strlen_buf_input == 0)
        usable_buf_input = 0;
      }

    if (child_pid != -1)
      {
      if (waitpid(child_pid, &wait_status, WNOHANG) == child_pid)
        {
        if (WIFSTOPPED(wait_status))
          {
          if (!child_stopped)
            SPAMDYKE_LOG_EXCESSIVE(current_settings, LOG_DEBUGX_CHILD_EXIT_STOPPED, WSTOPSIG(wait_status));

          child_stopped = 1;
          }
        else
          {
          if (WIFEXITED(wait_status))
            SPAMDYKE_LOG_EXCESSIVE(current_settings, LOG_DEBUGX_CHILD_EXIT_NORMAL, WEXITSTATUS(wait_status));
          else if (WIFSIGNALED(wait_status) &&
                   WCOREDUMP(wait_status))
            SPAMDYKE_LOG_EXCESSIVE(current_settings, LOG_DEBUGX_CHILD_EXIT_SIGNAL_CORE, WTERMSIG(wait_status));
          else if (WIFSIGNALED(wait_status))
            SPAMDYKE_LOG_EXCESSIVE(current_settings, LOG_DEBUGX_CHILD_EXIT_SIGNAL, WTERMSIG(wait_status));

          child_pid = -1;
          }
        }
      else if (child_stopped)
        {
        SPAMDYKE_LOG_EXCESSIVE(current_settings, LOG_DEBUGX_CHILD_EXIT_STARTED, NULL);
        child_stopped = 0;
        }
      }
    }

  return;
  }

/*
 * Return value:
 *   FILTER_DECISION value
 */
int process_access(struct filter_settings *current_settings)
  {
  int return_value;
  int i;
  int j;
  int k;
  char command_text[MAX_BUF + 1];
  char *current_ptr;
  char *value_start_ptr;
  char *value_end_ptr;
  int len_envp;
  char tmp_delimiter;
  void *tmp_ptr;
  char **destination_envp;

  return_value = FILTER_DECISION_UNDECIDED;
  command_text[0] = '\0';
  current_settings->allow_relay = 0;

  if (current_settings->current_environment != NULL)
    {
    for (len_envp = 0; current_settings->current_environment[len_envp] != NULL; len_envp++);
    if ((destination_envp = malloc(sizeof(char *) * (len_envp + 1))) != NULL)
      {
      memcpy(destination_envp, current_settings->current_environment, sizeof(char *) * len_envp);
      destination_envp[len_envp] = NULL;

      for (j = 0; j < len_envp; j++)
        if (strncasecmp(current_settings->current_environment[j], ENVIRONMENT_ALLOW_RELAY, STRLEN(ENVIRONMENT_ALLOW_RELAY)) == 0)
          {
          SPAMDYKE_LOG_EXCESSIVE(current_settings, LOG_DEBUGX_ENVIRONMENT_RELAY_FOUND, current_settings->current_environment[j]);
          current_settings->allow_relay = 1;

          tmp_delimiter = current_settings->current_environment[j][STRLEN(ENVIRONMENT_ALLOW_RELAY) + 1];
          if (tmp_delimiter != '\0')
            {
            for (k = 0; (current_settings->current_environment[j][STRLEN(ENVIRONMENT_ALLOW_RELAY) + 1 + k] != '\0') && (current_settings->current_environment[j][STRLEN(ENVIRONMENT_ALLOW_RELAY) + 1 + k] != tmp_delimiter); k++)
              current_settings->additional_domain_text[k] = tolower((int)current_settings->current_environment[j][STRLEN(ENVIRONMENT_ALLOW_RELAY) + 1 + k]);

            current_settings->additional_domain_text[k] = '\0';
            }
          else
            current_settings->additional_domain_text[0] = '\0';

          break;
          }
      }
    else
      {
      SPAMDYKE_LOG_ERROR(current_settings, LOG_ERROR_MALLOC, (unsigned long)(sizeof(char *) * (len_envp + 1)));
      return_value = FILTER_DECISION_ERROR;
      }
    }
  else
    {
    destination_envp = NULL;
    len_envp = 0;
    }

  if (current_settings->current_options->access_list_file != NULL)
    for (i = 0; current_settings->current_options->access_list_file[i] != NULL; i++)
      if (search_tcprules_file(current_settings, command_text, MAX_BUF, current_settings->current_options->access_list_file[i], current_settings->server_ip, current_settings->server_name, current_settings->strlen_server_name) >= 0)
        {
        if ((command_text[0] == '\0') ||
            ((strncasecmp(command_text, TCPRULES_ALLOW, STRLEN(TCPRULES_ALLOW)) == 0) &&
             ((command_text[STRLEN(TCPRULES_ALLOW)] == TCPRULES_DELIMITER) ||
              (command_text[STRLEN(TCPRULES_ALLOW)] == '\0'))))
          {
          if ((strncasecmp(command_text, TCPRULES_ALLOW, STRLEN(TCPRULES_ALLOW)) == 0) &&
              (command_text[STRLEN(TCPRULES_ALLOW)] == TCPRULES_DELIMITER))
            {
            current_ptr = command_text + STRLEN(TCPRULES_ALLOW) + 1;
            while (((current_ptr - command_text) < MAX_BUF) &&
                   (current_ptr[0] != '\0'))
              {
              for (value_start_ptr = current_ptr; (((value_start_ptr - command_text) < (MAX_BUF - 3)) && (value_start_ptr[0] != '\0') && (value_start_ptr[0] != ENVIRONMENT_DELIMITER)); value_start_ptr++);
              if (value_start_ptr[0] == ENVIRONMENT_DELIMITER)
                {
                for (j = 0; j < len_envp; j++)
                  if (strncasecmp(destination_envp[j], current_ptr, value_start_ptr - current_ptr) == 0)
                    break;

                tmp_delimiter = value_start_ptr[1];
                value_start_ptr += 2;
                for (value_end_ptr = value_start_ptr; (((value_end_ptr - command_text) < (MAX_BUF - 1)) && (value_end_ptr[0] != '\0') && (value_end_ptr[0] != tmp_delimiter)); value_end_ptr++);
                  if (j == len_envp)
                    {
                    if ((tmp_ptr = realloc(destination_envp, sizeof(char *) * (len_envp + 2))) != NULL)
                      {
                      destination_envp = tmp_ptr;
                      destination_envp[len_envp] = NULL;
                      destination_envp[len_envp + 1] = NULL;
                      len_envp++;
                      }
                    else
                      {
                      SPAMDYKE_LOG_ERROR(current_settings, LOG_ERROR_MALLOC, (unsigned long)(sizeof(char *) * (len_envp + 2)));
                      return_value = FILTER_DECISION_ERROR;
                      break;
                      }
                    }

                if ((tmp_ptr = alloc_environment_variable(current_settings->original_environment, destination_envp[j], sizeof(char) * (value_end_ptr - current_ptr))) != NULL)
                  destination_envp[j] = tmp_ptr;
                else
                  {
                  SPAMDYKE_LOG_ERROR(current_settings, LOG_ERROR_MALLOC, (unsigned long)(sizeof(char) * ((value_end_ptr - value_start_ptr) + 1)));
                  return_value = FILTER_DECISION_ERROR;
                  break;
                  }

                if (strncasecmp(current_ptr, ENVIRONMENT_ALLOW_RELAY, STRLEN(ENVIRONMENT_ALLOW_RELAY)) == 0)
                  {
                  SPAMDYKE_LOG_EXCESSIVE(current_settings, LOG_DEBUGX_ENVIRONMENT_RELAY_ALLOWED, current_settings->current_options->access_list_file[i]);
                  current_settings->allow_relay = 1;

                  tmp_delimiter = current_ptr[STRLEN(ENVIRONMENT_ALLOW_RELAY) + 1];
                  if (tmp_delimiter != '\0')
                    {
                    for (k = 0; (current_ptr[STRLEN(ENVIRONMENT_ALLOW_RELAY) + 1 + k] != '\0') && (current_ptr[STRLEN(ENVIRONMENT_ALLOW_RELAY) + 1 + k] != tmp_delimiter); k++)
                      current_settings->additional_domain_text[k] = tolower((int)current_ptr[STRLEN(ENVIRONMENT_ALLOW_RELAY) + 1 + k]);

                    current_settings->additional_domain_text[k] = '\0';
                    }
                  else
                    current_settings->additional_domain_text[0] = '\0';
                  }

                SPAMDYKE_LOG_EXCESSIVE(current_settings, LOG_DEBUGX_ENVIRONMENT_ADD, current_settings->current_options->access_list_file[i], (value_start_ptr - current_ptr) - 1, current_ptr, value_end_ptr - value_start_ptr, value_start_ptr);
                memcpy(destination_envp[j], current_ptr, sizeof(char) * ((value_start_ptr - current_ptr) - 1));
                memcpy(destination_envp[j] + ((value_start_ptr - current_ptr) - 1), value_start_ptr, sizeof(char) * (value_end_ptr - value_start_ptr));
                destination_envp[j][(value_end_ptr - current_ptr) - 1] = '\0';
                current_ptr = value_end_ptr + 2;
                }
              else
                current_ptr = value_start_ptr;
              }
            }

          break;
          }
        else if ((strncasecmp(command_text, TCPRULES_DENY, STRLEN(TCPRULES_DENY)) == 0) &&
                 ((command_text[STRLEN(TCPRULES_DENY)] == TCPRULES_DELIMITER) ||
                  (command_text[STRLEN(TCPRULES_DENY)] == '\0')))
          {
          set_rejection(current_settings, REJECTION_ACCESS_DENIED, &current_settings->current_options->rejection, &current_settings->current_options->rejection_buf, current_settings->current_options->reject_message_buf, MAX_BUF);

          return_value = FILTER_DECISION_DO_FILTER;
          break;
          }
        }

  if (!current_settings->allow_relay &&
      (current_settings->current_options->local_domains_file != NULL) &&
      (current_settings->current_options->relay_level >= RELAY_LEVEL_NORMAL))
    {
    if ((tmp_ptr = realloc(destination_envp, sizeof(char *) * (len_envp + 2))) != NULL)
      {
      destination_envp = tmp_ptr;
      destination_envp[len_envp] = NULL;
      destination_envp[len_envp + 1] = NULL;
      len_envp++;

      if ((tmp_ptr = alloc_environment_variable(current_settings->original_environment, destination_envp[len_envp - 1], sizeof(char) * (STRLEN(ENVIRONMENT_ALLOW_RELAY) + 2))) != NULL)
        {
        SPAMDYKE_LOG_EXCESSIVE(current_settings, LOG_DEBUGX_ENVIRONMENT_RELAY_ADD, ENVIRONMENT_ALLOW_RELAY);

        destination_envp[len_envp - 1] = tmp_ptr;
        memcpy(destination_envp[len_envp - 1], ENVIRONMENT_ALLOW_RELAY, sizeof(char) * STRLEN(ENVIRONMENT_ALLOW_RELAY));
        destination_envp[len_envp - 1][STRLEN(ENVIRONMENT_ALLOW_RELAY)] = ENVIRONMENT_DELIMITER;
        destination_envp[len_envp - 1][STRLEN(ENVIRONMENT_ALLOW_RELAY) + 1] = '\0';
        }
      else
        {
        SPAMDYKE_LOG_ERROR(current_settings, LOG_ERROR_MALLOC, (unsigned long)(sizeof(char) * (STRLEN(ENVIRONMENT_ALLOW_RELAY) + 2)));
        return_value = FILTER_DECISION_ERROR;
        }
      }
    else
      {
      SPAMDYKE_LOG_ERROR(current_settings, LOG_ERROR_MALLOC, (unsigned long)(sizeof(char *) * (len_envp + 2)));
      return_value = FILTER_DECISION_ERROR;
      }
    }

  free_environment(current_settings->original_environment, &current_settings->current_environment, destination_envp);
  current_settings->current_environment = destination_envp;

  return(return_value);
  }

/*
 * Return value:
 *   0
 */
int do_spamdyke(struct filter_settings *current_settings, int argc, char *argv[])
  {
  static char *environment_remote_ip[] = ENVIRONMENT_REMOTE_IP;
  static int strlen_environment_remote_ip[] = STRLEN_ENVIRONMENT_REMOTE_IP;
  int i;
  int output_pipe[2];
  int input_pipe[2];
  int child_pid;
  int test_result;
  char new_filename[MAX_BUF + 1];
  int len_envp;
  char **destination_envp;
  int found_match;
  char *tmp_ptr;
  int environment_updated;
  char ip_octets[4][4];
  int ip_ints[4];
  int remote_ip_index;

  srandom(time(NULL) * getpid());
  signal(SIGPIPE, SIG_IGN);

  environment_updated = 0;

  if (current_settings->current_options->filter_action != FILTER_DECISION_ERROR)
    {
    print_current_environment(current_settings);
    print_configuration(current_settings);

    /* Find the remote server's IP and rDNS name. */
    for (i = 0; environment_remote_ip[i] != NULL; i++)
      if ((current_settings->server_ip = find_environment_variable(current_settings, current_settings->current_environment, environment_remote_ip[i], strlen_environment_remote_ip[i], &remote_ip_index)) != NULL)
        {
        SPAMDYKE_LOG_EXCESSIVE(current_settings, LOG_DEBUGX_REMOTE_IP_ENV, environment_remote_ip[i], current_settings->server_ip);
        break;
        }

    if ((current_settings->server_ip != NULL) &&
        (current_settings->server_ip[0] != '\0'))
      {
      if (!strncasecmp(current_settings->server_ip, LOCALHOST_NAME, STRLEN(LOCALHOST_NAME)))
        {
        SPAMDYKE_LOG_VERBOSE(current_settings, LOG_VERBOSE_REMOTEIP_LOCALHOST, current_settings->server_ip, LOCALHOST_IP);
        current_settings->server_ip = LOCALHOST_IP;
        current_settings->strlen_server_ip = STRLEN(LOCALHOST_IP);
        }
      else if ((sscanf(current_settings->server_ip, "%3[0-9].%3[0-9].%3[0-9].%3[0-9]", ip_octets[0], ip_octets[1], ip_octets[2], ip_octets[3]) != 4) ||
               (sscanf(ip_octets[0], "%d", &ip_ints[0]) != 1) ||
               (ip_ints[0] < 0) ||
               (ip_ints[0] > 255) ||
               (sscanf(ip_octets[1], "%d", &ip_ints[1]) != 1) ||
               (ip_ints[1] < 0) ||
               (ip_ints[1] > 255) ||
               (sscanf(ip_octets[2], "%d", &ip_ints[2]) != 1) ||
               (ip_ints[2] < 0) ||
               (ip_ints[2] > 255) ||
               (sscanf(ip_octets[3], "%d", &ip_ints[3]) != 1) ||
               (ip_ints[3] < 0) ||
               (ip_ints[3] > 255))
        {
        SPAMDYKE_LOG_VERBOSE(current_settings, LOG_VERBOSE_REMOTEIP_TEXT, current_settings->server_ip);

        if (nihdns_a(current_settings, current_settings->server_ip, ip_ints, NULL, 0))
          {
          current_settings->strlen_server_ip = snprintf(current_settings->tmp_server_ip, MAX_BUF, "%.*s%c%d.%d.%d.%d", strlen_environment_remote_ip[i], environment_remote_ip[i], ENVIRONMENT_DELIMITER, ip_ints[0], ip_ints[1], ip_ints[2], ip_ints[3]) - (strlen_environment_remote_ip[i] + 1);
          current_settings->server_ip = current_settings->tmp_server_ip + strlen_environment_remote_ip[i] + 1;
          SPAMDYKE_LOG_DEBUG(current_settings, LOG_DEBUG_REMOTEIP_DNS_FOUND, current_settings->server_ip);

          for (len_envp = 0; current_settings->current_environment[len_envp] != NULL; len_envp++);
          if ((destination_envp = malloc(sizeof(char *) * (len_envp + 1))) != NULL)
            {
            memcpy(destination_envp, current_settings->current_environment, sizeof(char *) * len_envp);
            destination_envp[len_envp] = NULL;

            if ((tmp_ptr = alloc_environment_variable(current_settings->original_environment, destination_envp[remote_ip_index], current_settings->strlen_server_ip + strlen_environment_remote_ip[i] + 2)) != NULL)
              {
              memcpy(tmp_ptr, current_settings->tmp_server_ip, current_settings->strlen_server_ip + strlen_environment_remote_ip[i] + 1);
              tmp_ptr[current_settings->strlen_server_ip + strlen_environment_remote_ip[i] + 1] = '\0';
              destination_envp[remote_ip_index] = tmp_ptr;

              free_environment(current_settings->original_environment, &current_settings->current_environment, destination_envp);
              current_settings->current_environment = destination_envp;
              environment_updated = 1;
              SPAMDYKE_LOG_DEBUG(current_settings, LOG_DEBUG_REMOTEIP_ENV_UPDATED, current_settings->tmp_server_ip);
              }
            else
              {
              SPAMDYKE_LOG_ERROR(current_settings, LOG_ERROR_MALLOC, sizeof(char) * (current_settings->strlen_server_ip + strlen_environment_remote_ip[i] + 2));
              current_settings->current_options->filter_action = FILTER_DECISION_ERROR;
              }
            }
          else
            {
            SPAMDYKE_LOG_ERROR(current_settings, LOG_ERROR_MALLOC, sizeof(char *) * (len_envp + 1));
            current_settings->current_options->filter_action = FILTER_DECISION_ERROR;
            }
          }
        else
          {
          current_settings->server_ip = NULL;
          SPAMDYKE_LOG_DEBUG(current_settings, LOG_DEBUG_REMOTEIP_DNS_NOT_FOUND, DEFAULT_REMOTE_IP);
          }
        }
      else
        current_settings->strlen_server_ip = strlen(current_settings->server_ip);
      }

    if ((current_settings->server_ip == NULL) ||
        (current_settings->server_ip[0] == '\0'))
      {
      SPAMDYKE_LOG_EXCESSIVE(current_settings, LOG_DEBUGX_REMOTE_IP_DEFAULT, DEFAULT_REMOTE_IP);
      current_settings->server_ip = DEFAULT_REMOTE_IP;
      current_settings->strlen_server_ip = STRLEN(DEFAULT_REMOTE_IP);
      }

    output_writeln(current_settings, LOG_ACTION_LOG_IP, -1, NULL, -1);

    nihdns_ptr(current_settings, current_settings->server_ip);
    output_writeln(current_settings, LOG_ACTION_LOG_RDNS, -1, NULL, -1);
    }

  if (current_settings->current_options->filter_action == FILTER_DECISION_CONFIG_TEST)
    {
    config_test(current_settings, argc, argv);
    exit(0);
    }
  else if ((current_settings->current_options->filter_action <= FILTER_DECISION_DO_FILTER) &&
           ((test_result = process_access(current_settings)) <= FILTER_DECISION_DO_FILTER))
    {
    if (current_settings->current_options->filter_action == FILTER_DECISION_UNDECIDED)
      current_settings->current_options->filter_action = test_result;

    if (current_settings->current_options->configuration_dir == NULL)
      {
      filter_level(current_settings, &current_settings->current_options->filter_action, &current_settings->current_options->filter_action_locked, &current_settings->current_options->rejection, &current_settings->current_options->rejection_buf, current_settings->current_options->reject_message_buf, MAX_BUF);
      filter_rdns_missing(current_settings, &current_settings->current_options->filter_action, &current_settings->current_options->filter_action_locked, &current_settings->current_options->rejection, &current_settings->current_options->rejection_buf, current_settings->current_options->reject_message_buf, MAX_BUF);
      filter_ip_in_rdns_cc(current_settings, &current_settings->current_options->filter_action, &current_settings->current_options->filter_action_locked, &current_settings->current_options->rejection, &current_settings->current_options->rejection_buf, current_settings->current_options->reject_message_buf, MAX_BUF);
      filter_rdns_whitelist(current_settings, &current_settings->current_options->filter_action, &current_settings->current_options->filter_action_locked, &current_settings->current_options->rejection);
      filter_rdns_whitelist_file(current_settings, &current_settings->current_options->filter_action, &current_settings->current_options->filter_action_locked, &current_settings->current_options->rejection);
      filter_rdns_whitelist_dir(current_settings, &current_settings->current_options->filter_action, &current_settings->current_options->filter_action_locked, &current_settings->current_options->rejection);
      filter_rdns_blacklist(current_settings, &current_settings->current_options->filter_action, &current_settings->current_options->filter_action_locked, &current_settings->current_options->rejection, &current_settings->current_options->rejection_buf, current_settings->current_options->reject_message_buf, MAX_BUF);
      filter_rdns_blacklist_file(current_settings, &current_settings->current_options->filter_action, &current_settings->current_options->filter_action_locked, &current_settings->current_options->rejection, &current_settings->current_options->rejection_buf, current_settings->current_options->reject_message_buf, MAX_BUF);
      filter_rdns_blacklist_dir(current_settings, &current_settings->current_options->filter_action, &current_settings->current_options->filter_action_locked, &current_settings->current_options->rejection, &current_settings->current_options->rejection_buf, current_settings->current_options->reject_message_buf, MAX_BUF);
      filter_ip_whitelist(current_settings, &current_settings->current_options->filter_action, &current_settings->current_options->filter_action_locked, &current_settings->current_options->rejection);
      filter_ip_blacklist(current_settings, &current_settings->current_options->filter_action, &current_settings->current_options->filter_action_locked, &current_settings->current_options->rejection, &current_settings->current_options->rejection_buf, current_settings->current_options->reject_message_buf, MAX_BUF);
      filter_ip_in_rdns_whitelist(current_settings, &current_settings->current_options->filter_action, &current_settings->current_options->filter_action_locked, &current_settings->current_options->rejection);
      filter_ip_in_rdns_blacklist(current_settings, &current_settings->current_options->filter_action, &current_settings->current_options->filter_action_locked, &current_settings->current_options->rejection, &current_settings->current_options->rejection_buf, current_settings->current_options->reject_message_buf, MAX_BUF);
      filter_rdns_resolve(current_settings, &current_settings->current_options->filter_action, &current_settings->current_options->filter_action_locked, &current_settings->current_options->rejection, &current_settings->current_options->rejection_buf, current_settings->current_options->reject_message_buf, MAX_BUF);
      filter_dns_rwl(current_settings, &current_settings->current_options->filter_action, &current_settings->current_options->filter_action_locked, &current_settings->current_options->rejection);
      filter_dns_rhswl(current_settings, &current_settings->current_options->filter_action, &current_settings->current_options->filter_action_locked, &current_settings->current_options->rejection);
      filter_dns_rbl(current_settings, &current_settings->current_options->filter_action, &current_settings->current_options->filter_action_locked, &current_settings->current_options->rejection, &current_settings->current_options->rejection_buf, current_settings->current_options->reject_message_buf, MAX_BUF);
      filter_dns_rhsbl(current_settings, &current_settings->current_options->filter_action, &current_settings->current_options->filter_action_locked, &current_settings->current_options->rejection, &current_settings->current_options->rejection_buf, current_settings->current_options->reject_message_buf, MAX_BUF);
      filter_earlytalker(current_settings, 1, &current_settings->current_options->filter_action, &current_settings->current_options->filter_action_locked, &current_settings->current_options->rejection, &current_settings->current_options->rejection_buf, current_settings->current_options->reject_message_buf, MAX_BUF);
      }
    else
      filter_earlytalker(current_settings, 1, NULL, &current_settings->current_options->filter_action_locked, NULL, NULL, NULL, 0);
    }

#ifdef HAVE_LIBSSL

  if ((current_settings->current_options->tls_certificate_file != NULL) &&
      !tls_init(current_settings))
    {
    current_settings->current_options->tls_certificate_file = NULL;
    current_settings->current_options->tls_privatekey_file = NULL;
    current_settings->current_options->tls_privatekey_password = NULL;
    }

#endif /* HAVE_LIBSSL */

  found_match = 0;
  if (current_settings->current_environment != NULL)
    {
    for (len_envp = 0; current_settings->current_environment[len_envp] != NULL; len_envp++)
      if (strncasecmp(current_settings->current_environment[len_envp], ENVIRONMENT_LOCAL_PORT_SMTP, STRLEN(ENVIRONMENT_LOCAL_PORT_SMTP)) == 0)
        {
        SPAMDYKE_LOG_EXCESSIVE(current_settings, LOG_DEBUGX_ENVIRONMENT_LOCAL_PORT_FOUND, current_settings->current_environment[len_envp]);
        found_match = 1;
        break;
        }
    }
  else
    len_envp = 0;

  if (!found_match)
    {
    if ((destination_envp = malloc(sizeof(char *) * (len_envp + 2))) != NULL)
      {
      memcpy(destination_envp, current_settings->current_environment, sizeof(char *) * len_envp);
      destination_envp[len_envp] = NULL;
      destination_envp[len_envp + 1] = NULL;

      for (i = 0; i < len_envp; i++)
        if (strncasecmp(current_settings->current_environment[i], ENVIRONMENT_LOCAL_PORT, STRLEN(ENVIRONMENT_LOCAL_PORT)) == 0)
          {
          SPAMDYKE_LOG_EXCESSIVE(current_settings, LOG_DEBUGX_ENVIRONMENT_LOCAL_PORT_FOUND, current_settings->current_environment[i]);
          break;
          }

      if ((tmp_ptr = alloc_environment_variable(current_settings->original_environment, destination_envp[i], sizeof(char) * (STRLEN(ENVIRONMENT_LOCAL_PORT_SMTP) + 1))) != NULL)
        {
        SPAMDYKE_LOG_EXCESSIVE(current_settings, LOG_DEBUGX_ENVIRONMENT_LOCAL_PORT_SET, ENVIRONMENT_LOCAL_PORT_SMTP);
        destination_envp[i] = tmp_ptr;
        memcpy(destination_envp[i], ENVIRONMENT_LOCAL_PORT_SMTP, sizeof(char) * STRLEN(ENVIRONMENT_LOCAL_PORT_SMTP));
        destination_envp[i][STRLEN(ENVIRONMENT_LOCAL_PORT_SMTP)] = '\0';
        environment_updated = 1;
        }
      else
        {
        SPAMDYKE_LOG_ERROR(current_settings, LOG_ERROR_MALLOC, sizeof(char) * (STRLEN(ENVIRONMENT_LOCAL_PORT_SMTP) + 1));
        current_settings->current_options->filter_action = FILTER_DECISION_ERROR;
        }

      free_environment(current_settings->original_environment, &current_settings->current_environment, destination_envp);
      current_settings->current_environment = destination_envp;
      }
    else
      {
      SPAMDYKE_LOG_ERROR(current_settings, LOG_ERROR_MALLOC, sizeof(char *) * (len_envp + 2));
      current_settings->current_options->filter_action = FILTER_DECISION_ERROR;
      }
    }

  if (current_settings->current_options->filter_action != FILTER_DECISION_ERROR)
    for (i = 0; current_settings->current_environment[i] != NULL; i++)
      if (strncasecmp(current_settings->current_environment[i], ENVIRONMENT_SMTPS, STRLEN(ENVIRONMENT_SMTPS)) == 0)
        {
        SPAMDYKE_LOG_EXCESSIVE(current_settings, LOG_DEBUGX_ENVIRONMENT_SMTPS_REMOVE, current_settings->current_environment[i]);
        free_environment_variable(current_settings->original_environment, &current_settings->current_environment, i);
        environment_updated = 1;
        break;
        }

  if (environment_updated)
    print_current_environment(current_settings);

  output_pipe[0] = -1;
  output_pipe[1] = -1;
  input_pipe[0] = -1;
  input_pipe[1] = -1;

  if (find_path(current_settings, current_settings->child_argv[0], current_settings->current_environment, new_filename, MAX_BUF))
    {
    if ((current_settings->current_options->filter_action == FILTER_DECISION_DO_FILTER) &&
        (current_settings->current_options->filter_grace <= FILTER_GRACE_NONE) &&
        (current_settings->current_options->rejection != NULL))
      {
      output_write_rejection(current_settings, current_settings->current_options->rejection, STDOUT_FD, REJECT_SEVERITY_HELO);

      middleman(&input_pipe[0], &output_pipe[1], '\n', current_settings, -1);
      }
    else
      {
      if (pipe(output_pipe) != -1)
        {
        if (pipe(input_pipe) != -1)
          {
          SPAMDYKE_LOG_EXCESSIVE(current_settings, LOG_DEBUGX_EXEC, new_filename);

          if ((child_pid = fork()) > 0)
            /* parent */
            {
            close(output_pipe[0]);
            close(input_pipe[1]);

            /*
             * Set input and output sockets to non-blocking
             * to prevent hangs inside OpenSSL.
             */
            if (fcntl(STDIN_FD, F_SETFL, fcntl(STDIN_FD, F_GETFL, 0) | O_NONBLOCK) == -1)
              SPAMDYKE_LOG_ERROR(current_settings, LOG_ERROR_NONBLOCK_INPUT "%s", strerror(errno));
            if (fcntl(STDOUT_FD, F_SETFL, fcntl(STDOUT_FD, F_GETFL, 0) | O_NONBLOCK) == -1)
              SPAMDYKE_LOG_ERROR(current_settings, LOG_ERROR_NONBLOCK_OUTPUT "%s", strerror(errno));

            middleman(&input_pipe[0], &output_pipe[1], '\n', current_settings, child_pid);

            if (output_pipe[1] != -1)
              close(output_pipe[1]);
            if (input_pipe[0] != -1)
              close(input_pipe[0]);

            waitpid(child_pid, NULL, WNOHANG);
            }
          else if (child_pid == 0)
            /* child */
            {
            close(output_pipe[1]);
            close(input_pipe[0]);

            if ((dup2(output_pipe[0], STDIN_FD) != -1) &&
                (dup2(input_pipe[1], STDOUT_FD) != -1))
              {
              signal(SIGPIPE, SIG_DFL);

              if (!execve(new_filename, current_settings->child_argv, current_settings->current_environment))
                {
                SPAMDYKE_LOG_ERROR(current_settings, LOG_ERROR_EXEC "%s: %s", current_settings->child_argv[0], strerror(errno));
                close(output_pipe[0]);
                close(input_pipe[1]);
                }
              }
            else
              {
              SPAMDYKE_LOG_ERROR(current_settings, LOG_ERROR_MOVE_DESCRIPTORS "%s", strerror(errno));

              close(output_pipe[0]);
              close(input_pipe[1]);

              current_settings->current_options->filter_action = FILTER_DECISION_FORK_ERROR;
              }
            }
          else
            {
            SPAMDYKE_LOG_ERROR(current_settings, LOG_ERROR_FORK "%s", strerror(errno));

            close(output_pipe[0]);
            close(output_pipe[1]);
            close(input_pipe[0]);
            close(input_pipe[1]);

            current_settings->current_options->filter_action = FILTER_DECISION_FORK_ERROR;
            }
          }
        else
          {
          SPAMDYKE_LOG_ERROR(current_settings, LOG_ERROR_PIPE "%s", strerror(errno));

          close(output_pipe[0]);
          close(output_pipe[1]);

          current_settings->current_options->filter_action = FILTER_DECISION_FORK_ERROR;
          }
        }
      else
        {
        SPAMDYKE_LOG_ERROR(current_settings, LOG_ERROR_PIPE "%s", strerror(errno));
        current_settings->current_options->filter_action = FILTER_DECISION_FORK_ERROR;
        }
      }

    if ((current_settings->current_options->filter_action == FILTER_DECISION_FORK_ERROR) &&
        !exec_path(current_settings, current_settings->child_argv[0], current_settings->child_argv, current_settings->current_environment))
      SPAMDYKE_LOG_ERROR(current_settings, LOG_ERROR_EXEC "%s: %s", current_settings->child_argv[0], strerror(errno));

#ifdef HAVE_LIBSSL
    if (tls_end(current_settings, STDIN_FD))
      output_writeln(current_settings, LOG_ACTION_TLS_END, -1, NULL, -1);
#endif /* HAVE_LIBSSL */

    }
  else
    SPAMDYKE_LOG_ERROR(current_settings, LOG_ERROR_EXEC_FILE "%s: %s", current_settings->child_argv[0], strerror(errno));

  nihdns_initialize(current_settings, 1);
  output_writeln(NULL, 0, -1, NULL, 0);
  SPAMDYKE_LOG_NONE(current_settings, NULL);

  return(0);
  }

int main(int argc, char *argv[], char *envp[])
  {
  return(prepare_settings(argc, argv, envp, &do_spamdyke));
  }
