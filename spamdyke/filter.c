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
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/stat.h>
#include <pwd.h>
#include <errno.h>

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
#include "dns.h"
#include "log.h"
#include "search_fs.h"
#include "filter.h"

static struct rejection_data *rejection = REJECTION_DATA;

void set_rejection(struct filter_settings *current_settings, int rejection_index, struct rejection_data **target_rejection, struct rejection_data *target_rejection_buf, char *target_message_buf, int size_target_message_buf)
  {
  if (target_rejection != NULL)
    {
    if ((current_settings->current_options->rejection_text[rejection_index] != NULL) &&
        (target_rejection_buf != NULL) &&
        (target_message_buf != NULL) &&
        (size_target_message_buf > 0))
      {
      memcpy(target_rejection_buf, &rejection[rejection_index], sizeof(struct rejection_data));
      target_rejection_buf->strlen_reject_message = MINVAL(strlen(current_settings->current_options->rejection_text[rejection_index]), size_target_message_buf);
      memcpy(target_message_buf, current_settings->current_options->rejection_text[rejection_index], sizeof(char) * target_rejection_buf->strlen_reject_message);
      target_message_buf[target_rejection_buf->strlen_reject_message] = '\0';
      target_rejection_buf->reject_message = target_message_buf;

      *target_rejection = target_rejection_buf;
      }
    else
      *target_rejection = &rejection[rejection_index];
    }

  return;
  }

/*
 * Return value:
 *   ERROR: 0
 *   NOT FOUND: 0
 *   SUCCESS: 1
 */
int search_ip(char *target_ip, int strlen_target_ip, char *target_name, int strlen_target_name)
  {
  int j;
  int k;

  j = 0;
  k = 0;
  while (k < strlen_target_name)
    if (j < strlen_target_ip)
      if ((target_ip[j] >= '0') &&
          (target_ip[j] <= '9'))
        {
        if (target_ip[j] == target_name[k])
          j++;
        else
          j = 0;

        k++;
        }
      else
        {
        while ((k < strlen_target_name) &&
               ((target_name[k] < '0') ||
                (target_name[k] > '9')))
          k++;

        j++;
        }
    else
      break;

  return(((j > 0) && (j == strlen_target_ip)) ? 1 : 0);
  }

/*
 * Return value:
 *   ERROR: -1
 *   NOT FOUND: 0
 *   SUCCESS: 1
 */
int is_ip_in_name(struct filter_settings *current_settings, char *target_ip, char *target_name)
  {
  int return_value;
  char ip_octets[4][4];
  int ip_ints[4];
  char tmp_ip[MAX_IP + 1];
  int tmp_strlen;
  struct in_addr tmp_addr;
  int strlen_target_ip;
  int strlen_target_name;

  return_value = -1;

  if ((target_ip != NULL) &&
      ((strlen_target_ip = strlen(target_ip)) > 0) &&
      (sscanf(target_ip, "%3[0-9].%3[0-9].%3[0-9].%3[0-9]", ip_octets[0], ip_octets[1], ip_octets[2], ip_octets[3]) == 4) &&
      (sscanf(ip_octets[0], "%d", &ip_ints[0]) == 1) &&
      (ip_ints[0] >= 0) &&
      (ip_ints[0] <= 255) &&
      (sscanf(ip_octets[1], "%d", &ip_ints[1]) == 1) &&
      (ip_ints[1] >= 0) &&
      (ip_ints[1] <= 255) &&
      (sscanf(ip_octets[2], "%d", &ip_ints[2]) == 1) &&
      (ip_ints[2] >= 0) &&
      (ip_ints[2] <= 255) &&
      (sscanf(ip_octets[3], "%d", &ip_ints[3]) == 1) &&
      (ip_ints[3] >= 0) &&
      (ip_ints[3] <= 255) &&
      (target_name != NULL) &&
      ((strlen_target_name = strlen(target_name)) > 0))
    {
    return_value = 0;

    SPAMDYKE_LOG_EXCESSIVE(current_settings, LOG_DEBUGX_IP_IN_RDNS, strlen_target_ip, target_ip, strlen_target_name, target_name);
    /* This block looks for the dotted-quad IP address in the rDNS name. */
    return_value = search_ip(target_ip, strlen_target_ip, target_name, strlen_target_name);

    /*
     * This block looks for the zero-padded dotted-quad IP address in the rDNS
     * name.  For example, if the IP is 11.22.33.44, this block looks for
     * 011.022.033.044
     */
    if (!return_value)
      {
      tmp_strlen = snprintf(tmp_ip, MAX_IP, "%.3d.%.3d.%.3d.%.3d", ip_ints[0], ip_ints[1], ip_ints[2], ip_ints[3]);
      SPAMDYKE_LOG_EXCESSIVE(current_settings, LOG_DEBUGX_IP_IN_RDNS, tmp_strlen, tmp_ip, strlen_target_name, target_name);
      return_value = search_ip(tmp_ip, tmp_strlen, target_name, strlen_target_name);
      }

    /*
     * This block looks for the partially zero-padded dotted-quad IP address in
     * the rDNS name.  For example, if the IP is 11.22.33.44, this block looks
     * for 11.022.033.044
     */
    if (!return_value)
      {
      tmp_strlen = snprintf(tmp_ip, MAX_IP, "%s.%.3d.%.3d.%.3d", ip_octets[0], ip_ints[1], ip_ints[2], ip_ints[3]);
      SPAMDYKE_LOG_EXCESSIVE(current_settings, LOG_DEBUGX_IP_IN_RDNS, tmp_strlen, tmp_ip, strlen_target_name, target_name);
      return_value = search_ip(tmp_ip, tmp_strlen, target_name, strlen_target_name);
      }

    /*
     * This block looks for the partially zero-padded dotted-quad IP address in
     * the rDNS name.  For example, if the IP is 11.22.33.44, this block looks
     * for 11.22.033.044
     */
    if (!return_value)
      {
      tmp_strlen = snprintf(tmp_ip, MAX_IP, "%s.%s.%.3d.%.3d", ip_octets[0], ip_octets[1], ip_ints[2], ip_ints[3]);
      SPAMDYKE_LOG_EXCESSIVE(current_settings, LOG_DEBUGX_IP_IN_RDNS, tmp_strlen, tmp_ip, strlen_target_name, target_name);
      return_value = search_ip(tmp_ip, tmp_strlen, target_name, strlen_target_name);
      }

    /*
     * This block looks for the partially zero-padded dotted-quad IP address in
     * the rDNS name.  For example, if the IP is 11.22.33.44, this block looks
     * for 11.22.33.044
     */
    if (!return_value)
      {
      tmp_strlen = snprintf(tmp_ip, MAX_IP, "%s.%s.%s.%.3d", ip_octets[0], ip_octets[1], ip_octets[2], ip_ints[3]);
      SPAMDYKE_LOG_EXCESSIVE(current_settings, LOG_DEBUGX_IP_IN_RDNS, tmp_strlen, tmp_ip, strlen_target_name, target_name);
      return_value = search_ip(tmp_ip, tmp_strlen, target_name, strlen_target_name);
      }

    /*
     * This block looks for the reversed dotted-quad IP address in the rDNS
     * name.  For example, if the IP is 11.22.33.44, this block looks for
     * 44.33.22.11.
     */
    if (!return_value)
      {
      tmp_strlen = snprintf(tmp_ip, MAX_IP, "%s.%s.%s.%s", ip_octets[3], ip_octets[2], ip_octets[1], ip_octets[0]);
      SPAMDYKE_LOG_EXCESSIVE(current_settings, LOG_DEBUGX_IP_IN_RDNS, tmp_strlen, tmp_ip, strlen_target_name, target_name);
      return_value = search_ip(tmp_ip, tmp_strlen, target_name, strlen_target_name);
      }

    /*
     * This block looks for the reversed zero-padded dotted-quad IP address in
     * the rDNS name.  For example, if the IP is 11.22.33.44, this block looks
     * for 044.033.022.011.
     */
    if (!return_value)
      {
      tmp_strlen = snprintf(tmp_ip, MAX_IP, "%.3d.%.3d.%.3d.%.3d", ip_ints[3], ip_ints[2], ip_ints[1], ip_ints[0]);
      SPAMDYKE_LOG_EXCESSIVE(current_settings, LOG_DEBUGX_IP_IN_RDNS, tmp_strlen, tmp_ip, strlen_target_name, target_name);
      return_value = search_ip(tmp_ip, tmp_strlen, target_name, strlen_target_name);
      }

    /*
     * This block looks for the reversed partially zero-padded dotted-quad IP
     * address in the rDNS name.  For example, if the IP is 11.22.33.44, this
     * block looks for 44.033.022.011.
     */
    if (!return_value)
      {
      tmp_strlen = snprintf(tmp_ip, MAX_IP, "%s.%.3d.%.3d.%.3d", ip_octets[3], ip_ints[2], ip_ints[1], ip_ints[0]);
      SPAMDYKE_LOG_EXCESSIVE(current_settings, LOG_DEBUGX_IP_IN_RDNS, tmp_strlen, tmp_ip, strlen_target_name, target_name);
      return_value = search_ip(tmp_ip, tmp_strlen, target_name, strlen_target_name);
      }

    /*
     * This block looks for the reversed partially zero-padded dotted-quad IP
     * address in the rDNS name.  For example, if the IP is 11.22.33.44, this
     * block looks for 44.33.022.011.
     */
    if (!return_value)
      {
      tmp_strlen = snprintf(tmp_ip, MAX_IP, "%s.%s.%.3d.%.3d", ip_octets[3], ip_octets[2], ip_ints[1], ip_ints[0]);
      SPAMDYKE_LOG_EXCESSIVE(current_settings, LOG_DEBUGX_IP_IN_RDNS, tmp_strlen, tmp_ip, strlen_target_name, target_name);
      return_value = search_ip(tmp_ip, tmp_strlen, target_name, strlen_target_name);
      }

    /*
     * This block looks for the reversed partially zero-padded dotted-quad IP
     * address in the rDNS name.  For example, if the IP is 11.22.33.44, this
     * block looks for 44.33.22.011.
     */
    if (!return_value)
      {
      tmp_strlen = snprintf(tmp_ip, MAX_IP, "%s.%s.%s.%.3d", ip_octets[3], ip_octets[2], ip_octets[1], ip_ints[0]);
      SPAMDYKE_LOG_EXCESSIVE(current_settings, LOG_DEBUGX_IP_IN_RDNS, tmp_strlen, tmp_ip, strlen_target_name, target_name);
      return_value = search_ip(tmp_ip, tmp_strlen, target_name, strlen_target_name);
      }

    /*
     * This block looks for the slightly out-of-order dotted-quad IP address in the rDNS
     * name.  For example, if the IP is 11.22.33.44, this block looks for
     * 44.11.22.33
     */
    if (!return_value)
      {
      tmp_strlen = snprintf(tmp_ip, MAX_IP, "%s.%s.%s.%s", ip_octets[3], ip_octets[0], ip_octets[1], ip_octets[2]);
      SPAMDYKE_LOG_EXCESSIVE(current_settings, LOG_DEBUGX_IP_IN_RDNS, tmp_strlen, tmp_ip, strlen_target_name, target_name);
      return_value = search_ip(tmp_ip, tmp_strlen, target_name, strlen_target_name);
      }

    /*
     * This block looks for the strangely out-of-order dotted-quad IP address in the rDNS
     * name.  For example, if the IP is 11.22.33.44, this block looks for
     * 33.22.11.44
     */
    if (!return_value)
      {
      tmp_strlen = snprintf(tmp_ip, MAX_IP, "%s.%s.%s.%s", ip_octets[2], ip_octets[1], ip_octets[0], ip_octets[3]);
      SPAMDYKE_LOG_EXCESSIVE(current_settings, LOG_DEBUGX_IP_IN_RDNS, tmp_strlen, tmp_ip, strlen_target_name, target_name);
      return_value = search_ip(tmp_ip, tmp_strlen, target_name, strlen_target_name);
      }

    /*
     * This block looks for the strangely out-of-order dotted-quad IP address in the rDNS
     * name.  For example, if the IP is 11.22.33.44, this block looks for
     * 44.33.1122
     */
    if (!return_value)
      {
      tmp_strlen = snprintf(tmp_ip, MAX_IP, "%s.%s.%s%s", ip_octets[3], ip_octets[2], ip_octets[0], ip_octets[1]);
      SPAMDYKE_LOG_EXCESSIVE(current_settings, LOG_DEBUGX_IP_IN_RDNS, tmp_strlen, tmp_ip, strlen_target_name, target_name);
      return_value = search_ip(tmp_ip, tmp_strlen, target_name, strlen_target_name);
      }

    /* This block looks for the strangely out-of-order dotted-quad IP address in the rDNS
     * name.  For example, if the IP is 11.22.33.44, this block looks for
     * 3344.11.22
     */
    if (!return_value)
      {
      tmp_strlen = snprintf(tmp_ip, MAX_IP, "%s%s.%s.%s", ip_octets[2], ip_octets[3], ip_octets[0], ip_octets[1]);
      SPAMDYKE_LOG_EXCESSIVE(current_settings, LOG_DEBUGX_IP_IN_RDNS, tmp_strlen, tmp_ip, strlen_target_name, target_name);
      return_value = search_ip(tmp_ip, tmp_strlen, target_name, strlen_target_name);
      }

    /*
     * This block looks for the dotted-quad IP address with the last two octets encoded as a long integer in the rDNS
     * name.  For example, if the IP is 11.22.33.44, this block looks for
     * 11.22.8492
     */
    if (!return_value)
      {
      tmp_strlen = snprintf(tmp_ip, MAX_IP, "%s.%s.%d", ip_octets[0], ip_octets[1], (ip_ints[2] * 256) + ip_ints[3]);
      SPAMDYKE_LOG_EXCESSIVE(current_settings, LOG_DEBUGX_IP_IN_RDNS, tmp_strlen, tmp_ip, strlen_target_name, target_name);
      return_value = search_ip(tmp_ip, tmp_strlen, target_name, strlen_target_name);
      }

    /*
     * This block looks for the undotted-quad IP address in the rDNS name.
     * For example, if the IP is 11.22.33.44, this block looks for
     * 11223344.
     */
    if (!return_value)
      {
      tmp_strlen = snprintf(tmp_ip, MAX_IP, "%s%s%s%s", ip_octets[0], ip_octets[1], ip_octets[2], ip_octets[3]);

      SPAMDYKE_LOG_EXCESSIVE(current_settings, LOG_DEBUGX_IP_IN_RDNS, tmp_strlen, tmp_ip, strlen_target_name, target_name);
      if (strstr(target_name, tmp_ip) != NULL)
        return_value = 1;
      }

    /*
     * This block looks for the partially dotted-quad IP address in the rDNS
     * name.  For example, if the IP is 11.22.33.44, this block looks for
     * 11.22.3344
     */
    if (!return_value)
      {
      tmp_strlen = snprintf(tmp_ip, MAX_IP, "%s.%s.%s%s", ip_octets[0], ip_octets[1], ip_octets[2], ip_octets[3]);
      SPAMDYKE_LOG_EXCESSIVE(current_settings, LOG_DEBUGX_IP_IN_RDNS, tmp_strlen, tmp_ip, strlen_target_name, target_name);
      return_value = search_ip(tmp_ip, tmp_strlen, target_name, strlen_target_name);
      }

    /*
     * This block looks for the partially dotted-quad IP address in the rDNS
     * name.  For example, if the IP is 11.22.33.44, this block looks for
     * 11.223344
     */
    if (!return_value)
      {
      tmp_strlen = snprintf(tmp_ip, MAX_IP, "%s.%s%s%s", ip_octets[0], ip_octets[1], ip_octets[2], ip_octets[3]);
      SPAMDYKE_LOG_EXCESSIVE(current_settings, LOG_DEBUGX_IP_IN_RDNS, tmp_strlen, tmp_ip, strlen_target_name, target_name);
      return_value = search_ip(tmp_ip, tmp_strlen, target_name, strlen_target_name);
      }

    /*
     * This block looks for the undotted-, zero-padded-quad IP address in the
     * rDNS name.  For example, if the IP is 11.22.33.44, this block looks for
     * 011022033044
     */
    if (!return_value)
      {
      tmp_strlen = snprintf(tmp_ip, MAX_IP, "%.3d%.3d%.3d%.3d", ip_ints[0], ip_ints[1], ip_ints[2], ip_ints[3]);

      SPAMDYKE_LOG_EXCESSIVE(current_settings, LOG_DEBUGX_IP_IN_RDNS, tmp_strlen, tmp_ip, strlen_target_name, target_name);
      if (strstr(target_name, tmp_ip) != NULL)
        return_value = 1;
      }

    /*
     * This block looks for the undotted-, partially zero-padded-quad IP address in the
     * rDNS name.  For example, if the IP is 11.22.33.44, this block looks for
     * 11022033044
     */
    if (!return_value)
      {
      tmp_strlen = snprintf(tmp_ip, MAX_IP, "%s%.3d%.3d%.3d", ip_octets[0], ip_ints[1], ip_ints[2], ip_ints[3]);

      SPAMDYKE_LOG_EXCESSIVE(current_settings, LOG_DEBUGX_IP_IN_RDNS, tmp_strlen, tmp_ip, strlen_target_name, target_name);
      if (strstr(target_name, tmp_ip) != NULL)
        return_value = 1;
      }

    /*
     * This block looks for the undotted-, partially zero-padded-quad IP address in the
     * rDNS name.  For example, if the IP is 11.22.33.44, this block looks for
     * 1122033044
     */
    if (!return_value)
      {
      tmp_strlen = snprintf(tmp_ip, MAX_IP, "%s%s%.3d%.3d", ip_octets[0], ip_octets[1], ip_ints[2], ip_ints[3]);

      SPAMDYKE_LOG_EXCESSIVE(current_settings, LOG_DEBUGX_IP_IN_RDNS, tmp_strlen, tmp_ip, strlen_target_name, target_name);
      if (strstr(target_name, tmp_ip) != NULL)
        return_value = 1;
      }

    /*
     * This block looks for the undotted-, partially zero-padded-quad IP address in the
     * rDNS name.  For example, if the IP is 11.22.33.44, this block looks for
     * 112233044
     */
    if (!return_value)
      {
      tmp_strlen = snprintf(tmp_ip, MAX_IP, "%s%s%s%.3d", ip_octets[0], ip_octets[1], ip_octets[2], ip_ints[3]);

      SPAMDYKE_LOG_EXCESSIVE(current_settings, LOG_DEBUGX_IP_IN_RDNS, tmp_strlen, tmp_ip, strlen_target_name, target_name);
      if (strstr(target_name, tmp_ip) != NULL)
        return_value = 1;
      }

    /*
     * This block looks for the undotted-quad reversed IP address in the rDNS name.
     * For example, if the IP is 11.22.33.44, this block looks for
     * 44332211.
     */
    if (!return_value)
      {
      tmp_strlen = snprintf(tmp_ip, MAX_IP, "%s%s%s%s", ip_octets[3], ip_octets[2], ip_octets[1], ip_octets[0]);

      SPAMDYKE_LOG_EXCESSIVE(current_settings, LOG_DEBUGX_IP_IN_RDNS, tmp_strlen, tmp_ip, strlen_target_name, target_name);
      if (strstr(target_name, tmp_ip) != NULL)
        return_value = 1;
      }

    /*
     * This block looks for the undotted-, zero-padded-quad reversed IP address in the
     * rDNS name.  For example, if the IP is 11.22.33.44, this block looks for
     * 044033022011
     */
    if (!return_value)
      {
      tmp_strlen = snprintf(tmp_ip, MAX_IP, "%.3d%.3d%.3d%.3d", ip_ints[3], ip_ints[2], ip_ints[1], ip_ints[0]);

      SPAMDYKE_LOG_EXCESSIVE(current_settings, LOG_DEBUGX_IP_IN_RDNS, tmp_strlen, tmp_ip, strlen_target_name, target_name);
      if (strstr(target_name, tmp_ip) != NULL)
        return_value = 1;
      }

    /*
     * This block looks for the IP address as an unsigned long integer.
     * For example, if the IP is 85.135.72.234, this block looks for
     * 3930621781.
     */
    if (!return_value &&
        inet_aton(target_ip, &tmp_addr))
      {
      tmp_strlen = snprintf(tmp_ip, MAX_IP, "%lu", (long unsigned int)tmp_addr.s_addr);

      SPAMDYKE_LOG_EXCESSIVE(current_settings, LOG_DEBUGX_IP_IN_RDNS, tmp_strlen, tmp_ip, strlen_target_name, target_name);
      if (strstr(target_name, tmp_ip) != NULL)
        return_value = 1;
      }

    /*
     * This block looks for the IP address as a sequence of hex bytes.
     * For example, if the IP is 85.135.72.234, this block looks for
     * 5080d7e3.
     */
    if (!return_value)
      {
      tmp_strlen = snprintf(tmp_ip, MAX_IP, "%.2x%.2x%.2x%.2x", ip_ints[0], ip_ints[1], ip_ints[2], ip_ints[3]);

      SPAMDYKE_LOG_EXCESSIVE(current_settings, LOG_DEBUGX_IP_IN_RDNS, tmp_strlen, tmp_ip, strlen_target_name, target_name);
      if (strstr(target_name, tmp_ip) != NULL)
        return_value = 1;
      }
    }

  return(return_value);
  }

/*
 * Return value:
 *   ERROR: -1
 *   NO DNSRBL MATCH FOUND: 0
 *   DNSRBL MATCH FOUND: 1
 */
int check_dnsrbl(struct filter_settings *current_settings, char *target_ip_address, char **target_dnsrbl_array, char *target_message_format, char *target_message_buf, int size_target_message_buf, int *return_dnsrbl_index)
  {
  int return_value;
  int i;
  int num_dnsrbl;
  int name_len;
  char **rbl_name_array;
  char ip_octets[4][4];

  return_value = 0;

  if ((target_ip_address != NULL) &&
      (target_dnsrbl_array != NULL) &&
      (sscanf(target_ip_address, "%3[0-9].%3[0-9].%3[0-9].%3[0-9]", ip_octets[0], ip_octets[1], ip_octets[2], ip_octets[3]) == 4))
    {
    for (num_dnsrbl = 0; target_dnsrbl_array[num_dnsrbl] != NULL; num_dnsrbl++);
    if ((rbl_name_array = (char **)malloc(sizeof(char *) * (num_dnsrbl + 1))) != NULL)
      {
      for (i = 0; i <= num_dnsrbl; i++)
        rbl_name_array[i] = NULL;

      for (i = 0; i < num_dnsrbl; i++)
        {
        name_len = strlen(target_dnsrbl_array[i]) + 16;
        if (((rbl_name_array[i] = (char *)malloc(sizeof(char) * (name_len + 1))) == NULL) ||
            (snprintf(rbl_name_array[i], name_len + 1, "%s.%s.%s.%s.%s", ip_octets[3], ip_octets[2], ip_octets[1], ip_octets[0], target_dnsrbl_array[i]) <= 0))
          {
          SPAMDYKE_LOG_ERROR(current_settings, LOG_ERROR_MALLOC, sizeof(char) * (name_len + 1));
          return_value = -1;
          break;
          }
        }

      if ((return_value == 0) &&
          (nihdns_rbl(current_settings, rbl_name_array, target_message_format, target_message_buf, size_target_message_buf, target_dnsrbl_array, return_dnsrbl_index, NULL) > 0))
        return_value = 1;

      for (i = 0; i < num_dnsrbl; i++)
        if (rbl_name_array[i] != NULL)
          free(rbl_name_array[i]);

      free(rbl_name_array);
      }
    else
      {
      SPAMDYKE_LOG_ERROR(current_settings, LOG_ERROR_MALLOC, sizeof(char *) * (num_dnsrbl + 1));
      return_value = -1;
      }
    }

  return(return_value);
  }

/*
 * Return value:
 *   ERROR: -1
 *   NO RHSBL MATCH FOUND: 0
 *   RHSBL MATCH FOUND: 1
 */
int check_rhsbl(struct filter_settings *current_settings, char *target_right_side, char **target_rhsbl_array, char *target_message_format, char *target_message_buf, int size_target_message_buf, int *return_rhsbl_index)
  {
  int return_value;
  int i;
  int j;
  int num_rhsbl;
  int num_domains;
  char **rbl_name_array;
  char *tmp_ptr;
  int strlen_right_side;
  int strlen_rbl_name;
  char *domain_start[2];
  int tmp_rhsbl_index;

  return_value = 0;

  if ((target_right_side != NULL) &&
      (target_rhsbl_array != NULL))
    {
    for (num_rhsbl = 0; target_rhsbl_array[num_rhsbl] != NULL; num_rhsbl++);

    domain_start[0] = NULL;
    domain_start[1] = NULL;

    strlen_right_side = strlen(target_right_side);
    for (tmp_ptr = (target_right_side + strlen_right_side - 1); (tmp_ptr != NULL) && (tmp_ptr > target_right_side); tmp_ptr--)
      if (tmp_ptr[0] == '.')
        {
        if (domain_start[0] == NULL)
          domain_start[0] = tmp_ptr + 1;
        else if (domain_start[1] == NULL)
          {
          domain_start[1] = tmp_ptr + 1;
          break;
          }
        }

    if ((tmp_ptr == target_right_side) &&
        (tmp_ptr != domain_start[0]))
      domain_start[1] = target_right_side;

    if (domain_start[1] != NULL)
      {
      num_domains = 0;

      tmp_ptr = target_right_side;
      while ((tmp_ptr != NULL) &&
             (tmp_ptr <= domain_start[1]))
        {
        num_domains++;
        if ((tmp_ptr = strchr(tmp_ptr, '.')) != NULL)
          tmp_ptr++;
        }

      if ((rbl_name_array = (char **)malloc(sizeof(char *) * ((num_domains * num_rhsbl) + 1))) != NULL)
        {
        for (i = 0; i <= (num_domains * num_rhsbl); i++)
          rbl_name_array[i] = NULL;

        i = 0;

        tmp_ptr = target_right_side;
        while ((tmp_ptr != NULL) &&
               (tmp_ptr <= domain_start[1]))
          {
          for (j = 0; j < num_rhsbl; j++)
            {
            strlen_rbl_name = strlen(tmp_ptr) + strlen(target_rhsbl_array[j]) + 1;
            if (((rbl_name_array[(i * num_rhsbl) + j] = (char *)malloc(sizeof(char) * (strlen_rbl_name + 1))) == NULL) ||
                (snprintf(rbl_name_array[(i * num_rhsbl) + j], strlen_rbl_name + 1, "%s.%s", tmp_ptr, target_rhsbl_array[j]) <= 0))
              {
              SPAMDYKE_LOG_ERROR(current_settings, LOG_ERROR_MALLOC, sizeof(char) * (strlen_rbl_name + 1));
              return_value = -1;
              break;
              }
            }

          if ((tmp_ptr = strchr(tmp_ptr, '.')) != NULL)
            tmp_ptr++;

          i++;
          }

        if ((return_value == 0) &&
            (nihdns_rbl(current_settings, rbl_name_array, target_message_format, target_message_buf, size_target_message_buf, target_rhsbl_array, &tmp_rhsbl_index, NULL) > 0))
          {
          if (return_rhsbl_index != NULL)
            *return_rhsbl_index = tmp_rhsbl_index % num_rhsbl;

          return_value = 1;
          }

        for (i = 0; i <= (num_domains * num_rhsbl); i++)
          if (rbl_name_array[i] != NULL)
            free(rbl_name_array[i]);

        free(rbl_name_array);
        }
      else
        {
        SPAMDYKE_LOG_ERROR(current_settings, LOG_ERROR_MALLOC, (sizeof(char *) * num_domains * num_rhsbl) + 1);
        return_value = -1;
        }
      }
    }

  return(return_value);
  }

/*
 * EXPECTS:
 *   target_domain_name must point to a location within target_name or be NULL.
 *
 * RETURN VALUE:
 *   0: target_name does not contain the IP address and target_keyword
 *   1: target_name does contain the IP address and target_keyword
 */
int examine_ip_in_rdns_keyword_entry(struct filter_settings *current_settings, char *target_keyword, int strlen_target_keyword, char *target_ip, char *target_name, int strlen_target_name, char *target_domain_name, int *return_ip_in_name)
  {
  int return_value;
  int j;
  char keyword[MAX_FILE_BUF + 1];
  char *tld_start;
  int strlen_subkeyword;
  char *name_ptr;
  int subkeyword_found;
  int at_least_one_not_found;
  int ip_in_name;

  return_value = 0;
  ip_in_name = (return_ip_in_name != NULL) ? *return_ip_in_name : -1;

  /* Check for IP and keywords in name */
  if ((target_keyword != NULL) &&
      (strlen_target_keyword > 0) &&
      (target_ip != NULL) &&
      (target_name != NULL) &&
      (strlen_target_name > 0))
    {
    if (ip_in_name == -1)
      ip_in_name = is_ip_in_name(current_settings, target_ip, target_name);

    if (ip_in_name)
      {
      /* It's important to know where the domain name begins, since disqualifying mail.dhcp.com isn't fair. */
      if ((target_domain_name == NULL) ||
          (target_domain_name < target_name) ||
          (target_domain_name > (target_name + strlen_target_name)))
        {
        tld_start = NULL;
        for (j = (strlen_target_name - 1); j >= 0; j--)
          if (target_name[j] == '.')
            {
            for (j--; j >= 0; j--)
              if (target_name[j] == '.')
                {
                tld_start = target_name + j + 1;
                break;
                }

            break;
            }
        }
      else
        tld_start = target_domain_name;

      if (tld_start != NULL)
        {
        for (j = 0; j < strlen_target_keyword; j++)
          keyword[j] = (target_keyword[j] == ' ') ? '\0' : target_keyword[j];
        keyword[j] = '\0';

        j = 0;
        at_least_one_not_found = 0;
        while (j < strlen_target_keyword)
          {
          strlen_subkeyword = strlen(keyword + j);

          subkeyword_found = 0;
          name_ptr = target_name;
          while (((name_ptr - target_name) < strlen_target_name) &&
                 ((name_ptr = strstr(name_ptr, keyword + j)) != NULL) &&
                 ((keyword[j] == '.') ||
                  (name_ptr < tld_start)))
            {
            /*
             * The preceding and following characters around the match must be
             * non-alphanumeric so we don't reject 10.0.0.1.notdialup.example.com.
             * This means 10.0.0.1.dialup3.example.com will get through but the
             * alternative is worse.
             *
             * In pseudo-code: 
             *   (((keyword is at start of name) ||
             *     (first character in keyword is not a number or letter) ||
             *     (character in name before keyword is not a number or letter)) &&
             *    ((keyword is at end of name) ||
             *     (last character in keyword is not a number or letter) ||
             *     (character in name after keyword is not a number or letter)))
             */
            if (((name_ptr == target_name) ||
                 !isalnum((int)(keyword + j)[0]) ||
                 !isalnum((int)(name_ptr - 1)[0])) &&
                ((name_ptr == ((target_name + strlen_target_name) - strlen_subkeyword)) ||
                 !isalnum((int)(keyword + j + strlen_subkeyword - 1)[0]) ||
                 !isalnum((int)(name_ptr + strlen_subkeyword)[0])))
              {
              subkeyword_found = 1;
              break;
              }

            name_ptr += strlen_subkeyword;
            }

          if (!subkeyword_found)
            {
            at_least_one_not_found = 1;
            break;
            }

          j += strlen_subkeyword + 1;
          while ((j < strlen_target_keyword) &&
                 (keyword[j] == '\0'))
            j++;
          }

        if (!at_least_one_not_found)
          return_value = 1;
        }
      }

    if (return_ip_in_name != NULL)
      *return_ip_in_name = ip_in_name;
    }

  return(return_value);
  }

/*
 * RETURN VALUE:
 *   0: target_name does not contain the IP address and a keyword
 *   1: target_name does contain the IP address and a keyword
 */
int check_ip_in_rdns_keyword(struct filter_settings *current_settings, char **target_keyword_file, char *target_ip, char *target_name, int strlen_target_name, char *verbose_match_format, int *return_ip_in_name)
  {
  int return_value;
  int i;
  int j;
  FILE *tmp_file;
  char orig_keyword[MAX_FILE_BUF + 1];
  int line_num;
  char *tld_start;
  int strlen_keyword;
  int ip_in_name;

  return_value = 0;
  ip_in_name = (return_ip_in_name != NULL) ? *return_ip_in_name : -1;

  /* Check for IP and keywords in name */
  if ((target_keyword_file != NULL) &&
      (target_ip != NULL) &&
      (target_name != NULL) &&
      (strlen_target_name > 0))
    {
    if (ip_in_name == -1)
      ip_in_name = is_ip_in_name(current_settings, target_ip, target_name);

    if (ip_in_name)
      {
      for (i = 0; (target_keyword_file[i] != NULL) && !return_value; i++)
        if ((tmp_file = fopen(target_keyword_file[i], "r")) != NULL)
          {
          /* It's important to know where the domain name begins, since disqualifying mail.dhcp.com isn't fair. */
          tld_start = NULL;
          for (j = (strlen_target_name - 1); j >= 0; j--)
            if (target_name[j] == '.')
              {
              for (j--; j >= 0; j--)
                if (target_name[j] == '.')
                  {
                  tld_start = target_name + j + 1;
                  break;
                  }

              break;
              }

          if (tld_start != NULL)
            {
            line_num = 0;

            while (!feof(tmp_file) &&
                   (line_num < MAX_FILE_LINES))
              {
              if ((fscanf(tmp_file, "%" STRINGIFY(MAX_FILE_BUF) "[^\r\n]", orig_keyword) == 1) &&
                  (orig_keyword[0] != COMMENT_DELIMITER) &&
                  ((strlen_keyword = strlen(orig_keyword)) > 0) &&
                  examine_ip_in_rdns_keyword_entry(current_settings, orig_keyword, strlen_keyword, target_ip, target_name, strlen_target_name, tld_start, return_ip_in_name))
                {
                if (verbose_match_format != NULL)
                  SPAMDYKE_LOG_VERBOSE(current_settings, verbose_match_format, target_ip, target_name, orig_keyword, target_keyword_file[i], line_num + 1);

                return_value = 1;
                break;
                }

              fscanf(tmp_file, "%*1[\r\n]");
              line_num++;
              }

            if (line_num == MAX_FILE_LINES)
              SPAMDYKE_LOG_VERBOSE(current_settings, LOG_VERBOSE_FILE_TOO_LONG "%s", MAX_FILE_LINES, target_keyword_file[i]);

            fclose(tmp_file);
            }
          }
        else
          SPAMDYKE_LOG_ERROR(current_settings, LOG_ERROR_OPEN_KEYWORDS "%s: %s", target_keyword_file[i], strerror(errno));
      }

    if (return_ip_in_name != NULL)
      *return_ip_in_name = ip_in_name;
    }

  return(return_value);
  }

/*
 * Return value:
 *   FILTER_DECISION value
 */
int filter_rdns_missing(struct filter_settings *current_settings, int *target_action, int *return_action_locked, struct rejection_data **target_rejection, struct rejection_data *target_rejection_buf, char *target_message_buf, int size_target_message_buf)
  {
  int return_value;

  return_value = FILTER_DECISION_UNDECIDED;

  /* Check for rDNS name */
  if (((target_action == NULL) ||
       ((*target_action) < FILTER_DECISION_DO_FILTER)) &&
      ((return_action_locked == NULL) ||
       !(*return_action_locked)) &&
      current_settings->current_options->check_rdns_exist)
    {
    SPAMDYKE_LOG_DEBUG(current_settings, LOG_DEBUG_FILTER_RDNS_MISSING, (current_settings->strlen_server_name > 0) ? current_settings->server_name : LOG_MISSING_DATA);

    if (current_settings->strlen_server_name == 0)
      {
      if (target_action != NULL)
        *target_action = FILTER_DECISION_DO_FILTER;
      set_rejection(current_settings, REJECTION_RDNS_MISSING, target_rejection, target_rejection_buf, target_message_buf, size_target_message_buf);

      SPAMDYKE_LOG_VERBOSE(current_settings, LOG_VERBOSE_FILTER_RDNS_MISSING, current_settings->server_ip);
      return_value = FILTER_DECISION_DO_FILTER;
      }
    }

  return(return_value);
  }

int filter_ip_in_rdns_cc(struct filter_settings *current_settings, int *target_action, int *return_action_locked, struct rejection_data **target_rejection, struct rejection_data *target_rejection_buf, char *target_message_buf, int size_target_message_buf)
  {
  int return_value;
  int i;

  return_value = FILTER_DECISION_UNDECIDED;

  /* Check for IP and CC in name */
  if (((target_action == NULL) ||
       ((*target_action) < FILTER_DECISION_DO_FILTER)) &&
      ((return_action_locked == NULL) ||
       !(*return_action_locked)) &&
      current_settings->current_options->check_ip_in_rdns_cc &&
      (current_settings->strlen_server_name > 0))
    {
    SPAMDYKE_LOG_DEBUG(current_settings, LOG_DEBUG_FILTER_IP_IN_RDNS_CC, current_settings->server_name);

    if (current_settings->ip_in_server_name == -1)
      current_settings->ip_in_server_name = is_ip_in_name(current_settings, current_settings->server_ip, current_settings->server_name);

    if (current_settings->ip_in_server_name &&
        (current_settings->strlen_server_name > 0))
      for (i = (current_settings->strlen_server_name - 1); i >= 0; i--)
        if (current_settings->server_name[i] == '.')
          {
          if ((current_settings->strlen_server_name - i) == 3)
            {
            if (target_action != NULL)
              *target_action = FILTER_DECISION_DO_FILTER;
            set_rejection(current_settings, REJECTION_IP_IN_NAME_CC, target_rejection, target_rejection_buf, target_message_buf, size_target_message_buf);

            SPAMDYKE_LOG_VERBOSE(current_settings, LOG_VERBOSE_FILTER_IP_IN_RDNS_CC, current_settings->server_ip, current_settings->server_name);
            return_value = FILTER_DECISION_DO_FILTER;
            }

          break;
          }
    }

  return(return_value);
  }

int filter_rdns_whitelist(struct filter_settings *current_settings, int *target_action, int *return_action_locked, struct rejection_data **target_rejection)
  {
  int return_value;
  int i;

  return_value = FILTER_DECISION_UNDECIDED;

  /* Check rDNS whitelist */
  if (((target_action == NULL) ||
       ((*target_action) < FILTER_DECISION_DO_NOT_FILTER)) &&
      ((return_action_locked == NULL) ||
       !(*return_action_locked)) &&
      (current_settings->current_options->whitelist_rdns != NULL) &&
      (current_settings->strlen_server_name > 0))
    {
    SPAMDYKE_LOG_DEBUG(current_settings, LOG_DEBUG_FILTER_RDNS_WHITELIST, current_settings->server_name);

    for (i = 0; current_settings->current_options->whitelist_rdns[i] != NULL; i++)
      if (examine_entry(current_settings->server_name, current_settings->strlen_server_name, current_settings->current_options->whitelist_rdns[i], strlen(current_settings->current_options->whitelist_rdns[i]), '.', ".", '\0', NULL))
        {
        if (target_action != NULL)
          *target_action = FILTER_DECISION_DO_NOT_FILTER;
        if (target_rejection != NULL)
          *target_rejection = NULL;

        SPAMDYKE_LOG_VERBOSE(current_settings, LOG_VERBOSE_FILTER_RDNS_WHITELIST, current_settings->server_ip, current_settings->server_name, current_settings->current_options->whitelist_rdns[i]);
        return_value = FILTER_DECISION_DO_NOT_FILTER;
        break;
        }
    }

  return(return_value);
  }

int filter_rdns_whitelist_file(struct filter_settings *current_settings, int *target_action, int *return_action_locked, struct rejection_data **target_rejection)
  {
  int return_value;
  int i;
  int search_return;

  return_value = FILTER_DECISION_UNDECIDED;

  /* Check rDNS whitelist */
  if (((target_action == NULL) ||
       ((*target_action) < FILTER_DECISION_DO_NOT_FILTER)) &&
      ((return_action_locked == NULL) ||
       !(*return_action_locked)) &&
      (current_settings->current_options->whitelist_rdns_file != NULL) &&
      (current_settings->strlen_server_name > 0))
    {
    SPAMDYKE_LOG_DEBUG(current_settings, LOG_DEBUG_FILTER_RDNS_WHITELIST_FILE, current_settings->server_name);

    for (i = 0; current_settings->current_options->whitelist_rdns_file[i] != NULL; i++)
      if ((search_return = search_file(current_settings, current_settings->current_options->whitelist_rdns_file[i], current_settings->server_name, current_settings->strlen_server_name, '.', ".", '\0', NULL)) > 0)
        {
        if (target_action != NULL)
          *target_action = FILTER_DECISION_DO_NOT_FILTER;
        if (target_rejection != NULL)
          *target_rejection = NULL;

        SPAMDYKE_LOG_VERBOSE(current_settings, LOG_VERBOSE_FILTER_RDNS_WHITELIST_FILE, current_settings->server_ip, current_settings->server_name, current_settings->current_options->whitelist_rdns_file[i], search_return);
        return_value = FILTER_DECISION_DO_NOT_FILTER;
        break;
        }
    }

  return(return_value);
  }

int filter_rdns_whitelist_dir(struct filter_settings *current_settings, int *target_action, int *return_action_locked, struct rejection_data **target_rejection)
  {
  int return_value;
  int i;
  char *search_return;

  return_value = FILTER_DECISION_UNDECIDED;

  /* Check rDNS whitelist */
  if (((target_action == NULL) ||
       ((*target_action) < FILTER_DECISION_DO_NOT_FILTER)) &&
      ((return_action_locked == NULL) ||
       !(*return_action_locked)) &&
      (current_settings->current_options->whitelist_rdns_dir != NULL) &&
      (current_settings->strlen_server_name > 0))
    {
    SPAMDYKE_LOG_DEBUG(current_settings, LOG_DEBUG_FILTER_RDNS_WHITELIST_DIR, current_settings->server_name);

    for (i = 0; current_settings->current_options->whitelist_rdns_dir[i] != NULL; i++)
      if ((search_return = search_domain_directory(current_settings, current_settings->current_options->whitelist_rdns_dir[i], current_settings->server_name, current_settings->strlen_server_name)) != NULL)
        {
        if (target_action != NULL)
          *target_action = FILTER_DECISION_DO_NOT_FILTER;
        if (target_rejection != NULL)
          *target_rejection = NULL;

        SPAMDYKE_LOG_VERBOSE(current_settings, LOG_VERBOSE_FILTER_RDNS_WHITELIST_DIR, current_settings->server_ip, current_settings->server_name, current_settings->current_options->whitelist_rdns_dir[i], search_return);
        return_value = FILTER_DECISION_DO_NOT_FILTER;
        break;
        }
    }

  return(return_value);
  }

int filter_rdns_blacklist(struct filter_settings *current_settings, int *target_action, int *return_action_locked, struct rejection_data **target_rejection, struct rejection_data *target_rejection_buf, char *target_message_buf, int size_target_message_buf)
  {
  int return_value;
  int i;

  return_value = FILTER_DECISION_UNDECIDED;

  /* Check rDNS blacklist */
  if (((target_action == NULL) ||
       ((*target_action) < FILTER_DECISION_DO_FILTER)) &&
      ((return_action_locked == NULL) ||
       !(*return_action_locked)) &&
      (current_settings->current_options->blacklist_rdns != NULL) &&
      (current_settings->strlen_server_name > 0))
    {
    SPAMDYKE_LOG_DEBUG(current_settings, LOG_DEBUG_FILTER_RDNS_BLACKLIST, current_settings->server_name);

    for (i = 0; current_settings->current_options->blacklist_rdns[i] != NULL; i++)
      if (examine_entry(current_settings->server_name, current_settings->strlen_server_name, current_settings->current_options->blacklist_rdns[i], strlen(current_settings->current_options->blacklist_rdns[i]), '.', ".", '\0', NULL))
        {
        if (target_action != NULL)
          *target_action = FILTER_DECISION_DO_FILTER;
        set_rejection(current_settings, REJECTION_BLACKLIST_NAME, target_rejection, target_rejection_buf, target_message_buf, size_target_message_buf);

        SPAMDYKE_LOG_VERBOSE(current_settings, LOG_VERBOSE_FILTER_RDNS_BLACKLIST, current_settings->server_ip, current_settings->server_name, current_settings->current_options->blacklist_rdns[i]);
        return_value = FILTER_DECISION_DO_FILTER;
        break;
        }
    }

  return(return_value);
  }

int filter_rdns_blacklist_file(struct filter_settings *current_settings, int *target_action, int *return_action_locked, struct rejection_data **target_rejection, struct rejection_data *target_rejection_buf, char *target_message_buf, int size_target_message_buf)
  {
  int return_value;
  int i;
  int search_return;

  return_value = FILTER_DECISION_UNDECIDED;

  /* Check rDNS blacklist */
  if (((target_action == NULL) ||
       ((*target_action) < FILTER_DECISION_DO_FILTER)) &&
      ((return_action_locked == NULL) ||
       !(*return_action_locked)) &&
      (current_settings->current_options->blacklist_rdns_file != NULL) &&
      (current_settings->strlen_server_name > 0))
    {
    SPAMDYKE_LOG_DEBUG(current_settings, LOG_DEBUG_FILTER_RDNS_BLACKLIST_FILE, current_settings->server_name);

    for (i = 0; current_settings->current_options->blacklist_rdns_file[i] != NULL; i++)
      if ((search_return = search_file(current_settings, current_settings->current_options->blacklist_rdns_file[i], current_settings->server_name, current_settings->strlen_server_name, '.', ".", '\0', NULL)) > 0)
        {
        if (target_action != NULL)
          *target_action = FILTER_DECISION_DO_FILTER;
        set_rejection(current_settings, REJECTION_BLACKLIST_NAME, target_rejection, target_rejection_buf, target_message_buf, size_target_message_buf);

        SPAMDYKE_LOG_VERBOSE(current_settings, LOG_VERBOSE_FILTER_RDNS_BLACKLIST_FILE, current_settings->server_ip, current_settings->server_name, current_settings->current_options->blacklist_rdns_file[i], search_return);
        return_value = FILTER_DECISION_DO_FILTER;
        break;
        }
    }

  return(return_value);
  }

int filter_rdns_blacklist_dir(struct filter_settings *current_settings, int *target_action, int *return_action_locked, struct rejection_data **target_rejection, struct rejection_data *target_rejection_buf, char *target_message_buf, int size_target_message_buf)
  {
  int return_value;
  int i;
  char *search_return;

  return_value = FILTER_DECISION_UNDECIDED;

  /* Check rDNS blacklist */
  if (((target_action == NULL) ||
       ((*target_action) < FILTER_DECISION_DO_FILTER)) &&
      ((return_action_locked == NULL) ||
       !(*return_action_locked)) &&
      (current_settings->current_options->blacklist_rdns_dir != NULL) &&
      (current_settings->strlen_server_name > 0))
    {
    SPAMDYKE_LOG_DEBUG(current_settings, LOG_DEBUG_FILTER_RDNS_BLACKLIST_DIR, current_settings->server_name);

    for (i = 0; current_settings->current_options->blacklist_rdns_dir[i] != NULL; i++)
      if ((search_return = search_domain_directory(current_settings, current_settings->current_options->blacklist_rdns_dir[i], current_settings->server_name, current_settings->strlen_server_name)) != NULL)
        {
        if (target_action != NULL)
          *target_action = FILTER_DECISION_DO_FILTER;
        set_rejection(current_settings, REJECTION_BLACKLIST_NAME, target_rejection, target_rejection_buf, target_message_buf, size_target_message_buf);

        SPAMDYKE_LOG_VERBOSE(current_settings, LOG_VERBOSE_FILTER_RDNS_BLACKLIST_DIR, current_settings->server_ip, current_settings->server_name, current_settings->current_options->blacklist_rdns_dir[i], search_return);
        return_value = FILTER_DECISION_DO_FILTER;
        break;
        }
    }

  return(return_value);
  }

int filter_ip_whitelist(struct filter_settings *current_settings, int *target_action, int *return_action_locked, struct rejection_data **target_rejection)
  {
  int return_value;
  int i;
  int search_return;

  return_value = FILTER_DECISION_UNDECIDED;

  /* Check IP whitelist */
  if (((target_action == NULL) ||
       ((*target_action) < FILTER_DECISION_DO_NOT_FILTER)) &&
      ((return_action_locked == NULL) ||
       !(*return_action_locked)) &&
      ((current_settings->current_options->whitelist_ip != NULL) ||
       (current_settings->current_options->whitelist_ip_file != NULL)))
    {
    SPAMDYKE_LOG_DEBUG(current_settings, LOG_DEBUG_FILTER_IP_WHITELIST, current_settings->server_ip);

    if (current_settings->current_options->whitelist_ip != NULL)
      for (i = 0; current_settings->current_options->whitelist_ip[i] != NULL; i++)
        if (examine_tcprules_entry(current_settings, NULL, 0, current_settings->current_options->whitelist_ip[i], strlen(current_settings->current_options->whitelist_ip[i]), current_settings->server_ip, current_settings->server_name, current_settings->strlen_server_name))
          {
          if (target_action != NULL)
            *target_action = FILTER_DECISION_DO_NOT_FILTER;
          if (target_rejection != NULL)
            *target_rejection = NULL;

          SPAMDYKE_LOG_VERBOSE(current_settings, LOG_VERBOSE_FILTER_IP_WHITELIST, current_settings->server_ip, current_settings->current_options->whitelist_ip[i]);
          return_value = FILTER_DECISION_DO_NOT_FILTER;
          break;
          }

    if ((return_value == FILTER_DECISION_UNDECIDED) &&
        (current_settings->current_options->whitelist_ip_file != NULL))
      for (i = 0; current_settings->current_options->whitelist_ip_file[i] != NULL; i++)
        if ((search_return = search_tcprules_file(current_settings, NULL, 0, current_settings->current_options->whitelist_ip_file[i], current_settings->server_ip, current_settings->server_name, current_settings->strlen_server_name)) > 0)
          {
          if (target_action != NULL)
            *target_action = FILTER_DECISION_DO_NOT_FILTER;
          if (target_rejection != NULL)
            *target_rejection = NULL;

          SPAMDYKE_LOG_VERBOSE(current_settings, LOG_VERBOSE_FILTER_IP_WHITELIST_FILE, current_settings->server_ip, current_settings->current_options->whitelist_ip_file[i], search_return);
          return_value = FILTER_DECISION_DO_NOT_FILTER;
          break;
          }
    }

  return(return_value);
  }

int filter_ip_blacklist(struct filter_settings *current_settings, int *target_action, int *return_action_locked, struct rejection_data **target_rejection, struct rejection_data *target_rejection_buf, char *target_message_buf, int size_target_message_buf)
  {
  int return_value;
  int i;
  int search_return;

  return_value = FILTER_DECISION_UNDECIDED;

  if (((target_action == NULL) ||
       ((*target_action) < FILTER_DECISION_DO_FILTER)) &&
      ((return_action_locked == NULL) ||
       !(*return_action_locked)) &&
      ((current_settings->current_options->blacklist_ip != NULL) ||
       (current_settings->current_options->blacklist_ip_file != NULL)))
    {
    SPAMDYKE_LOG_DEBUG(current_settings, LOG_DEBUG_FILTER_IP_BLACKLIST, current_settings->server_ip);

    if (current_settings->current_options->blacklist_ip != NULL)
      for (i = 0; current_settings->current_options->blacklist_ip[i] != NULL; i++)
        if (examine_tcprules_entry(current_settings, NULL, 0, current_settings->current_options->blacklist_ip[i], strlen(current_settings->current_options->blacklist_ip[i]), current_settings->server_ip, current_settings->server_name, current_settings->strlen_server_name))
          {
          if (target_action != NULL)
            *target_action = FILTER_DECISION_DO_FILTER;
          set_rejection(current_settings, REJECTION_BLACKLIST_IP, target_rejection, target_rejection_buf, target_message_buf, size_target_message_buf);

          SPAMDYKE_LOG_VERBOSE(current_settings, LOG_VERBOSE_FILTER_IP_BLACKLIST, current_settings->server_ip, current_settings->current_options->blacklist_ip[i]);
          return_value = FILTER_DECISION_DO_FILTER;
          break;
          }

    if ((return_value == FILTER_DECISION_UNDECIDED) &&
        (current_settings->current_options->blacklist_ip_file != NULL))
      for (i = 0; current_settings->current_options->blacklist_ip_file[i] != NULL; i++)
        if ((search_return = search_tcprules_file(current_settings, NULL, 0, current_settings->current_options->blacklist_ip_file[i], current_settings->server_ip, current_settings->server_name, current_settings->strlen_server_name)) > 0)
          {
          if (target_action != NULL)
            *target_action = FILTER_DECISION_DO_FILTER;
          set_rejection(current_settings, REJECTION_BLACKLIST_IP, target_rejection, target_rejection_buf, target_message_buf, size_target_message_buf);

          SPAMDYKE_LOG_VERBOSE(current_settings, LOG_VERBOSE_FILTER_IP_BLACKLIST_FILE, current_settings->server_ip, current_settings->current_options->blacklist_ip_file[i], search_return);
          return_value = FILTER_DECISION_DO_FILTER;
          break;
          }
    }

  return(return_value);
  }

int filter_ip_in_rdns_blacklist(struct filter_settings *current_settings, int *target_action, int *return_action_locked, struct rejection_data **target_rejection, struct rejection_data *target_rejection_buf, char *target_message_buf, int size_target_message_buf)
  {
  int return_value;
  int i;

  return_value = FILTER_DECISION_UNDECIDED;

  /* Check for IP and keywords in name */
  if (((target_action == NULL) ||
       ((*target_action) < FILTER_DECISION_DO_FILTER)) &&
      ((return_action_locked == NULL) ||
       !(*return_action_locked)) &&
      ((current_settings->current_options->blacklist_rdns_keyword != NULL) ||
       (current_settings->current_options->blacklist_rdns_keyword_file != NULL)) &&
      (current_settings->strlen_server_name > 0))
    {
    SPAMDYKE_LOG_DEBUG(current_settings, LOG_DEBUG_FILTER_IP_IN_RDNS_BLACKLIST, current_settings->server_ip, current_settings->server_name);

    if (current_settings->current_options->blacklist_rdns_keyword != NULL)
      for (i = 0; current_settings->current_options->blacklist_rdns_keyword[i] != NULL; i++)
        if (examine_ip_in_rdns_keyword_entry(current_settings, current_settings->current_options->blacklist_rdns_keyword[i], strlen(current_settings->current_options->blacklist_rdns_keyword[i]), current_settings->server_ip, current_settings->server_name, current_settings->strlen_server_name, NULL, &current_settings->ip_in_server_name))
          {
          if (target_action != NULL)
            *target_action = FILTER_DECISION_DO_FILTER;
          set_rejection(current_settings, REJECTION_IP_IN_NAME, target_rejection, target_rejection_buf, target_message_buf, size_target_message_buf);

          SPAMDYKE_LOG_VERBOSE(current_settings, LOG_VERBOSE_FILTER_IP_IN_RDNS_BLACKLIST, current_settings->server_ip, current_settings->server_name, current_settings->current_options->blacklist_rdns_keyword[i]);
          return_value = FILTER_DECISION_DO_FILTER;
          break;
          }

    if ((return_value == FILTER_DECISION_UNDECIDED) &&
        (current_settings->current_options->blacklist_rdns_keyword_file != NULL) &&
        check_ip_in_rdns_keyword(current_settings, current_settings->current_options->blacklist_rdns_keyword_file, current_settings->server_ip, current_settings->server_name, current_settings->strlen_server_name, LOG_VERBOSE_FILTER_IP_IN_RDNS_BLACKLIST_FILE, &current_settings->ip_in_server_name))
      {
      if (target_action != NULL)
        *target_action = FILTER_DECISION_DO_FILTER;
      set_rejection(current_settings, REJECTION_IP_IN_NAME, target_rejection, target_rejection_buf, target_message_buf, size_target_message_buf);

      return_value = FILTER_DECISION_DO_FILTER;
      }
    }

  return(return_value);
  }

int filter_ip_in_rdns_whitelist(struct filter_settings *current_settings, int *target_action, int *return_action_locked, struct rejection_data **target_rejection)
  {
  int return_value;
  int i;

  return_value = FILTER_DECISION_UNDECIDED;

  /* Check for IP and keywords in name */
  if (((target_action == NULL) ||
       ((*target_action) < FILTER_DECISION_DO_NOT_FILTER)) &&
      ((return_action_locked == NULL) ||
       !(*return_action_locked)) &&
      ((current_settings->current_options->whitelist_rdns_keyword != NULL) ||
       (current_settings->current_options->whitelist_rdns_keyword_file != NULL)) &&
      (current_settings->strlen_server_name > 0))
    {
    SPAMDYKE_LOG_DEBUG(current_settings, LOG_DEBUG_FILTER_IP_IN_RDNS_WHITELIST, current_settings->server_ip, current_settings->server_name);

    if (current_settings->current_options->whitelist_rdns_keyword != NULL)
      for (i = 0; current_settings->current_options->whitelist_rdns_keyword[i] != NULL; i++)
        if (examine_ip_in_rdns_keyword_entry(current_settings, current_settings->current_options->whitelist_rdns_keyword[i], strlen(current_settings->current_options->whitelist_rdns_keyword[i]), current_settings->server_ip, current_settings->server_name, current_settings->strlen_server_name, NULL, &current_settings->ip_in_server_name))
          {
          if (target_action != NULL)
            *target_action = FILTER_DECISION_DO_NOT_FILTER;
          if (target_rejection != NULL)
            *target_rejection = NULL;

          SPAMDYKE_LOG_VERBOSE(current_settings, LOG_VERBOSE_FILTER_IP_IN_RDNS_WHITELIST, current_settings->server_ip, current_settings->server_name, current_settings->current_options->whitelist_rdns_keyword[i]);
          return_value = FILTER_DECISION_DO_NOT_FILTER;
          break;
          }

    if ((return_value == FILTER_DECISION_UNDECIDED) &&
        (current_settings->current_options->whitelist_rdns_keyword_file != NULL) &&
        check_ip_in_rdns_keyword(current_settings, current_settings->current_options->whitelist_rdns_keyword_file, current_settings->server_ip, current_settings->server_name, current_settings->strlen_server_name, LOG_VERBOSE_FILTER_IP_IN_RDNS_WHITELIST_FILE, &current_settings->ip_in_server_name))
      {
      if (target_action != NULL)
        *target_action = FILTER_DECISION_DO_NOT_FILTER;
      if (target_rejection != NULL)
        *target_rejection = NULL;

      return_value = FILTER_DECISION_DO_NOT_FILTER;
      }
    }

  return(return_value);
  }

int filter_rdns_resolve(struct filter_settings *current_settings, int *target_action, int *return_action_locked, struct rejection_data **target_rejection, struct rejection_data *target_rejection_buf, char *target_message_buf, int size_target_message_buf)
  {
  int return_value;

  return_value = FILTER_DECISION_UNDECIDED;

  /* Check for rDNS resolution */
  if (((target_action == NULL) ||
       ((*target_action) < FILTER_DECISION_DO_FILTER)) &&
      ((return_action_locked == NULL) ||
       !(*return_action_locked)) &&
      current_settings->current_options->check_rdns_resolve &&
      (current_settings->strlen_server_name > 0))
    {
    SPAMDYKE_LOG_DEBUG(current_settings, LOG_DEBUG_FILTER_RDNS_RESOLVE, current_settings->server_name);

    if (((strncasecmp(current_settings->server_name, LOCALHOST_NAME, STRLEN(LOCALHOST_NAME)) != 0) &&
         !nihdns_a(current_settings, current_settings->server_name, NULL, NULL, 0)) ||
        ((strncasecmp(current_settings->server_name, LOCALHOST_NAME, STRLEN(LOCALHOST_NAME)) == 0) &&
         (strncasecmp(current_settings->server_ip, LOCALHOST_IP, STRLEN(LOCALHOST_IP)) != 0)))
      {
      if (target_action != NULL)
        *target_action = FILTER_DECISION_DO_FILTER;
      set_rejection(current_settings, REJECTION_RDNS_RESOLVE, target_rejection, target_rejection_buf, target_message_buf, size_target_message_buf);

      SPAMDYKE_LOG_VERBOSE(current_settings, LOG_VERBOSE_FILTER_RDNS_RESOLVE, current_settings->server_ip, current_settings->server_name);
      return_value = FILTER_DECISION_DO_FILTER;
      }
    }

  return(return_value);
  }

int filter_dns_rwl(struct filter_settings *current_settings, int *target_action, int *return_action_locked, struct rejection_data **target_rejection)
  {
  int return_value;
  int i;
  int num_fqdn;
  int num_names;
  int rwl_index;
  char **name_array;

  return_value = FILTER_DECISION_UNDECIDED;
  num_fqdn = 0;
  num_names = 0;
  rwl_index = -1;
  name_array = NULL;

  /* Check DNS RWL */
  if (((target_action == NULL) ||
       ((*target_action) < FILTER_DECISION_DO_NOT_FILTER)) &&
      ((return_action_locked == NULL) ||
       !(*return_action_locked)) &&
      ((current_settings->current_options->dnsrwl_fqdn != NULL) ||
       (current_settings->current_options->dnsrwl_fqdn_file != NULL)))
    {
    SPAMDYKE_LOG_DEBUG(current_settings, LOG_DEBUG_FILTER_DNS_RWL, current_settings->server_ip);

    if (current_settings->current_options->dnsrwl_fqdn_file != NULL)
      {
      if (current_settings->current_options->dnsrwl_fqdn != NULL)
        {
        for (num_fqdn = 0; current_settings->current_options->dnsrwl_fqdn[num_fqdn] != NULL; num_fqdn++);
        if (num_fqdn > 0)
          {
          if ((name_array = (char **)malloc(sizeof(char *) * (num_fqdn + 1))) != NULL)
            {
            memcpy(name_array, current_settings->current_options->dnsrwl_fqdn, sizeof(char *) * (num_fqdn + 1));
            num_names = num_fqdn;
            }
          else
            {
            SPAMDYKE_LOG_ERROR(current_settings, LOG_ERROR_MALLOC, sizeof(char *) * (num_fqdn + 1));
            return_value = FILTER_DECISION_ERROR;
            }
          }
        }

      for (i = 0; current_settings->current_options->dnsrwl_fqdn_file[i] != NULL; i++)
        if (read_file(current_settings, current_settings->current_options->dnsrwl_fqdn_file[i], &name_array, num_names, 1, -1) != -1)
          for (; name_array[num_names] != NULL; num_names++);
        else
          {
          return_value = FILTER_DECISION_ERROR;
          break;
          }
      }
    else
      name_array = current_settings->current_options->dnsrwl_fqdn;

    if ((return_value == FILTER_DECISION_UNDECIDED) &&
        (strcmp(current_settings->server_ip, LOCALHOST_IP) != 0) &&
        (check_dnsrbl(current_settings, current_settings->server_ip, name_array, NULL, NULL, 0, &rwl_index) == 1))
      {
      if (target_action != NULL)
        *target_action = FILTER_DECISION_DO_NOT_FILTER;
      if (target_rejection != NULL)
        *target_rejection = NULL;

      SPAMDYKE_LOG_VERBOSE(current_settings, LOG_VERBOSE_FILTER_DNS_RWL, current_settings->server_ip, name_array[rwl_index]);
      return_value = FILTER_DECISION_DO_NOT_FILTER;
      }

    if ((name_array != NULL) &&
        (name_array != current_settings->current_options->dnsrwl_fqdn))
      {
      for (i = num_fqdn; name_array[i] != NULL; i++)
        free(name_array[i]);

      free(name_array);
      }
    }

  return(return_value);
  }

int filter_dns_rhswl(struct filter_settings *current_settings, int *target_action, int *return_action_locked, struct rejection_data **target_rejection)
  {
  int return_value;
  int i;
  int num_fqdn;
  int num_names;
  int rhswl_index;
  char **name_array;

  return_value = FILTER_DECISION_UNDECIDED;
  num_fqdn = 0;
  num_names = 0;
  rhswl_index = -1;
  name_array = NULL;

  /* Check RHSWL */
  if (((target_action == NULL) ||
       ((*target_action) < FILTER_DECISION_DO_NOT_FILTER)) &&
      ((return_action_locked == NULL) ||
       !(*return_action_locked)) &&
      ((current_settings->current_options->rhswl_fqdn != NULL) ||
       (current_settings->current_options->rhswl_fqdn_file != NULL)) &&
      (strcmp(current_settings->server_ip, LOCALHOST_IP) != 0) &&
      (current_settings->strlen_server_name > 0))
    {
    SPAMDYKE_LOG_DEBUG(current_settings, LOG_DEBUG_FILTER_DNS_RHSWL, current_settings->server_name);

    if (current_settings->current_options->rhswl_fqdn_file != NULL)
      {
      if (current_settings->current_options->rhswl_fqdn != NULL)
        {
        for (num_fqdn = 0; current_settings->current_options->rhswl_fqdn[num_fqdn] != NULL; num_fqdn++);
        if (num_fqdn > 0)
          {
          if ((name_array = (char **)malloc(sizeof(char *) * (num_fqdn + 1))) != NULL)
            {
            memcpy(name_array, current_settings->current_options->rhswl_fqdn, sizeof(char *) * (num_fqdn + 1));
            num_names = num_fqdn;
            }
          else
            {
            SPAMDYKE_LOG_ERROR(current_settings, LOG_ERROR_MALLOC, sizeof(char *) * (num_fqdn + 1));
            return_value = FILTER_DECISION_ERROR;
            }
          }
        }

      for (i = 0; current_settings->current_options->rhswl_fqdn_file[i] != NULL; i++)
        if (read_file(current_settings, current_settings->current_options->rhswl_fqdn_file[i], &name_array, num_names, 1, -1) != -1)
          for (; name_array[num_names] != NULL; num_names++);
        else
          {
          return_value = FILTER_DECISION_ERROR;
          break;
          }
      }
    else
      name_array = current_settings->current_options->rhswl_fqdn;

    if ((return_value == FILTER_DECISION_UNDECIDED) &&
        (check_rhsbl(current_settings, current_settings->server_name, name_array, NULL, NULL, 0, &rhswl_index) == 1))
      {
      if (target_action != NULL)
        *target_action = FILTER_DECISION_DO_NOT_FILTER;
      if (target_rejection != NULL)
        *target_rejection = NULL;

      SPAMDYKE_LOG_VERBOSE(current_settings, LOG_VERBOSE_FILTER_DNS_RHSWL, current_settings->server_name, name_array[rhswl_index]);
      return_value = FILTER_DECISION_DO_NOT_FILTER;
      }

    if ((name_array != NULL) &&
        (name_array != current_settings->current_options->rhswl_fqdn))
      {
      for (i = num_fqdn; name_array[i] != NULL; i++)
        free(name_array[i]);

      free(name_array);
      }
    }

  return(return_value);
  }

int filter_dns_rbl(struct filter_settings *current_settings, int *target_action, int *return_action_locked, struct rejection_data **target_rejection, struct rejection_data *target_rejection_buf, char *target_message_buf, int size_target_message_buf)
  {
  int return_value;
  int i;
  int num_fqdn;
  int num_names;
  int rbl_index;
  char **name_array;

  return_value = FILTER_DECISION_UNDECIDED;
  num_fqdn = 0;
  num_names = 0;
  rbl_index = -1;
  name_array = NULL;

  /* Check DNS RBL */
  if (((target_action == NULL) ||
       ((*target_action) < FILTER_DECISION_DO_FILTER)) &&
      ((return_action_locked == NULL) ||
       !(*return_action_locked)) &&
      ((current_settings->current_options->dnsrbl_fqdn != NULL) ||
       (current_settings->current_options->dnsrbl_fqdn_file != NULL)))
    {
    SPAMDYKE_LOG_DEBUG(current_settings, LOG_DEBUG_FILTER_DNS_RBL, current_settings->server_ip);

    if (current_settings->current_options->dnsrbl_fqdn_file != NULL)
      {
      if (current_settings->current_options->dnsrbl_fqdn != NULL)
        {
        for (num_fqdn = 0; current_settings->current_options->dnsrbl_fqdn[num_fqdn] != NULL; num_fqdn++);
        if (num_fqdn > 0)
          {
          if ((name_array = (char **)malloc(sizeof(char *) * (num_fqdn + 1))) != NULL)
            {
            memcpy(name_array, current_settings->current_options->dnsrbl_fqdn, sizeof(char *) * (num_fqdn + 1));
            num_names = num_fqdn;
            }
          else
            {
            SPAMDYKE_LOG_ERROR(current_settings, LOG_ERROR_MALLOC, sizeof(char *) * (num_fqdn + 1));
            return_value = FILTER_DECISION_ERROR;
            }
          }
        }

      for (i = 0; current_settings->current_options->dnsrbl_fqdn_file[i] != NULL; i++)
        if (read_file(current_settings, current_settings->current_options->dnsrbl_fqdn_file[i], &name_array, num_names, 1, -1) != -1)
          for (; name_array[num_names] != NULL; num_names++);
        else
          {
          return_value = FILTER_DECISION_ERROR;
          break;
          }
      }
    else
      name_array = current_settings->current_options->dnsrbl_fqdn;

    if ((return_value == FILTER_DECISION_UNDECIDED) &&
        (strcmp(current_settings->server_ip, LOCALHOST_IP) != 0) &&
        (check_dnsrbl(current_settings, current_settings->server_ip, name_array, (current_settings->current_options->rejection_text[REJECTION_RBL] != NULL) ? current_settings->current_options->rejection_text[REJECTION_RBL] : rejection[REJECTION_RBL].reject_message, target_message_buf, size_target_message_buf, &rbl_index) == 1))
      {
      if (target_action != NULL)
        *target_action = FILTER_DECISION_DO_FILTER;
      if (target_rejection != NULL)
        {
        memcpy(target_rejection_buf, &rejection[REJECTION_RBL], sizeof(struct rejection_data));
        target_rejection_buf->strlen_reject_message = strlen(target_message_buf);
        target_rejection_buf->reject_message = target_message_buf;
        *target_rejection = target_rejection_buf;
        }

      SPAMDYKE_LOG_VERBOSE(current_settings, LOG_VERBOSE_FILTER_DNS_RBL, current_settings->server_ip, name_array[rbl_index]);
      return_value = FILTER_DECISION_DO_FILTER;
      }

    if ((name_array != NULL) &&
        (name_array != current_settings->current_options->dnsrbl_fqdn))
      {
      for (i = num_fqdn; name_array[i] != NULL; i++)
        free(name_array[i]);

      free(name_array);
      }
    }

  return(return_value);
  }

int filter_dns_rhsbl(struct filter_settings *current_settings, int *target_action, int *return_action_locked, struct rejection_data **target_rejection, struct rejection_data *target_rejection_buf, char *target_message_buf, int size_target_message_buf)
  {
  int return_value;
  int i;
  int num_fqdn;
  int num_names;
  int rhsbl_index;
  char **name_array;

  return_value = FILTER_DECISION_UNDECIDED;
  num_fqdn = 0;
  num_names = 0;
  rhsbl_index = -1;
  name_array = NULL;

  /* Check RHSBL */
  if (((target_action == NULL) ||
       ((*target_action) < FILTER_DECISION_DO_FILTER)) &&
      ((return_action_locked == NULL) ||
       !(*return_action_locked)) &&
      ((current_settings->current_options->rhsbl_fqdn != NULL) ||
       (current_settings->current_options->rhsbl_fqdn_file != NULL)) &&
      (strcmp(current_settings->server_ip, LOCALHOST_IP) != 0) &&
      (current_settings->strlen_server_name > 0))
    {
    SPAMDYKE_LOG_DEBUG(current_settings, LOG_DEBUG_FILTER_DNS_RHSBL, current_settings->server_name);

    if (current_settings->current_options->rhsbl_fqdn_file != NULL)
      {
      if (current_settings->current_options->rhsbl_fqdn != NULL)
        {
        for (num_fqdn = 0; current_settings->current_options->rhsbl_fqdn[num_fqdn] != NULL; num_fqdn++);
        if (num_fqdn > 0)
          {
          if ((name_array = (char **)malloc(sizeof(char *) * (num_fqdn + 1))) != NULL)
            {
            memcpy(name_array, current_settings->current_options->rhsbl_fqdn, sizeof(char *) * (num_fqdn + 1));
            num_names = num_fqdn;
            }
          else
            {
            SPAMDYKE_LOG_ERROR(current_settings, LOG_ERROR_MALLOC, sizeof(char *) * (num_fqdn + 1));
            return_value = FILTER_DECISION_ERROR;
            }
          }
        }

      for (i = 0; current_settings->current_options->rhsbl_fqdn_file[i] != NULL; i++)
        if (read_file(current_settings, current_settings->current_options->rhsbl_fqdn_file[i], &name_array, num_names, 1, -1) != -1)
          for (; name_array[num_names] != NULL; num_names++);
        else
          {
          return_value = FILTER_DECISION_ERROR;
          break;
          }
      }
    else
      name_array = current_settings->current_options->rhsbl_fqdn;

    if ((return_value == FILTER_DECISION_UNDECIDED) &&
        (check_rhsbl(current_settings, current_settings->server_name, name_array, (current_settings->current_options->rejection_text[REJECTION_RHSBL] != NULL) ? current_settings->current_options->rejection_text[REJECTION_RHSBL] : rejection[REJECTION_RHSBL].reject_message, target_message_buf, size_target_message_buf, &rhsbl_index) == 1))
      {
      if (target_action != NULL)
        *target_action = FILTER_DECISION_DO_FILTER;
      if (target_rejection != NULL)
        {
        memcpy(target_rejection_buf, &rejection[REJECTION_RHSBL], sizeof(struct rejection_data));
        target_rejection_buf->strlen_reject_message = strlen(target_message_buf);
        target_rejection_buf->reject_message = target_message_buf;
        *target_rejection = target_rejection_buf;
        }

      SPAMDYKE_LOG_VERBOSE(current_settings, LOG_VERBOSE_FILTER_DNS_RHSBL, current_settings->server_name, name_array[rhsbl_index]);
      return_value = FILTER_DECISION_DO_FILTER;
      }

    if ((name_array != NULL) &&
        (name_array != current_settings->current_options->rhsbl_fqdn))
      {
      for (i = num_fqdn; name_array[i] != NULL; i++)
        free(name_array[i]);

      free(name_array);
      }
    }

  return(return_value);
  }

int filter_earlytalker(struct filter_settings *current_settings, int initial_connection, int *target_action, int *return_action_locked, struct rejection_data **target_rejection, struct rejection_data *target_rejection_buf, char *target_message_buf, int size_target_message_buf)
  {
  static int return_value = FILTER_DECISION_UNDECIDED;
  static int last_earlytalker = 0;
  struct timeval tmp_time;
  fd_set listen_set;

  /* Check for earlytalkers */
  if (((target_action == NULL) ||
       ((*target_action) < FILTER_DECISION_DO_FILTER)) &&
      ((return_action_locked == NULL) ||
       !(*return_action_locked)) &&
      (current_settings->current_options->check_earlytalker > 0))
    {
    SPAMDYKE_LOG_DEBUG(current_settings, LOG_DEBUG_FILTER_EARLYTALKER, current_settings->current_options->check_earlytalker);

    if ((return_value == FILTER_DECISION_UNDECIDED) &&
        (current_settings->current_options->check_earlytalker != last_earlytalker))
      {
      if (initial_connection)
        {
        tmp_time.tv_sec = current_settings->current_options->check_earlytalker;
        tmp_time.tv_usec = 0;

        FD_ZERO(&listen_set);
        FD_SET(STDIN_FD, &listen_set);
        if (select(STDIN_FD + 1, &listen_set, NULL, NULL, &tmp_time) == 1)
          return_value = FILTER_DECISION_DO_FILTER;
        }
      else
        SPAMDYKE_LOG_ERROR(current_settings, LOG_ERROR_LATE_EARLYTALKER);

      last_earlytalker = current_settings->current_options->check_earlytalker;
      }

    if (return_value == FILTER_DECISION_DO_FILTER)
      {
      if (target_action != NULL)
        *target_action = FILTER_DECISION_DO_FILTER;
      set_rejection(current_settings, REJECTION_EARLYTALKER, target_rejection, target_rejection_buf, target_message_buf, size_target_message_buf);

      SPAMDYKE_LOG_VERBOSE(current_settings, LOG_VERBOSE_FILTER_EARLYTALKER, current_settings->current_options->check_earlytalker);
      return_value = FILTER_DECISION_DO_FILTER;
      }
    }

  return(return_value);
  }

int filter_sender_whitelist(struct filter_settings *current_settings, int *target_action, int *return_action_locked, struct rejection_data **target_rejection)
  {
  int return_value;
  int i;
  int search_return;
  char tmp_sender_address[MAX_ADDRESS + 1];
  int strlen_sender_address;

  return_value = FILTER_DECISION_UNDECIDED;

  if (((target_action == NULL) ||
       ((*target_action) < FILTER_DECISION_DO_NOT_FILTER)) &&
      ((return_action_locked == NULL) ||
       !(*return_action_locked)) &&
      ((current_settings->current_options->whitelist_sender != NULL) ||
       (current_settings->current_options->whitelist_sender_file != NULL)) &&
      (reassemble_address(current_settings->sender_username, current_settings->sender_domain, NULL, tmp_sender_address, MAX_ADDRESS, &strlen_sender_address) != NULL) &&
      (strlen_sender_address > 0))
    {
    SPAMDYKE_LOG_DEBUG(current_settings, LOG_DEBUG_FILTER_SENDER_WHITELIST, tmp_sender_address);

    if (current_settings->current_options->whitelist_sender != NULL)
      for (i = 0; current_settings->current_options->whitelist_sender[i] != NULL; i++)
        if (examine_entry(tmp_sender_address, strlen_sender_address, current_settings->current_options->whitelist_sender[i], strlen(current_settings->current_options->whitelist_sender[i]), '@', "@.", '@', "@"))
          {
          if (target_action != NULL)
            *target_action = FILTER_DECISION_DO_NOT_FILTER;
          if (target_rejection != NULL)
            *target_rejection = NULL;

          SPAMDYKE_LOG_VERBOSE(current_settings, LOG_VERBOSE_FILTER_SENDER_WHITELIST, tmp_sender_address, current_settings->current_options->whitelist_sender[i]);
          return_value = FILTER_DECISION_DO_NOT_FILTER;
          break;
          }

    if ((return_value == FILTER_DECISION_UNDECIDED) &&
        (current_settings->current_options->whitelist_sender_file != NULL))
      for (i = 0; current_settings->current_options->whitelist_sender_file[i] != NULL; i++)
        if ((search_return = search_file(current_settings, current_settings->current_options->whitelist_sender_file[i], tmp_sender_address, strlen_sender_address, '@', "@.", '@', "@")) > 0)
          {
          if (target_action != NULL)
            *target_action = FILTER_DECISION_DO_NOT_FILTER;
          if (target_rejection != NULL)
            *target_rejection = NULL;

          SPAMDYKE_LOG_VERBOSE(current_settings, LOG_VERBOSE_FILTER_SENDER_WHITELIST_FILE, tmp_sender_address, current_settings->current_options->whitelist_sender_file[i], search_return);
          return_value = FILTER_DECISION_DO_NOT_FILTER;
          break;
          }
    }

  return(return_value);
  }

int filter_sender_rhswl(struct filter_settings *current_settings, int *target_action, int *return_action_locked, struct rejection_data **target_rejection)
  {
  int return_value;
  int i;
  int num_fqdn;
  int num_names;
  int rhswl_index;
  char **name_array;

  return_value = FILTER_DECISION_UNDECIDED;
  num_fqdn = 0;
  num_names = 0;
  rhswl_index = -1;
  name_array = NULL;

  if (((target_action == NULL) ||
       ((*target_action) < FILTER_DECISION_DO_NOT_FILTER)) &&
      ((return_action_locked == NULL) ||
       !(*return_action_locked)) &&
      ((current_settings->current_options->rhswl_fqdn != NULL) ||
       (current_settings->current_options->rhswl_fqdn_file != NULL)) &&
      (current_settings->sender_domain != NULL))
    {
    SPAMDYKE_LOG_DEBUG(current_settings, LOG_DEBUG_FILTER_SENDER_RHSWL, current_settings->sender_domain);

    if (current_settings->current_options->rhswl_fqdn_file != NULL)
      {
      if (current_settings->current_options->rhswl_fqdn != NULL)
        {
        for (num_fqdn = 0; current_settings->current_options->rhswl_fqdn[num_fqdn] != NULL; num_fqdn++);
        if (num_fqdn > 0)
          {
          if ((name_array = (char **)malloc(sizeof(char *) * (num_fqdn + 1))) != NULL)
            {
            memcpy(name_array, current_settings->current_options->rhswl_fqdn, sizeof(char *) * (num_fqdn + 1));
            num_names = num_fqdn;
            }
          else
            {
            SPAMDYKE_LOG_ERROR(current_settings, LOG_ERROR_MALLOC, sizeof(char *) * (num_fqdn + 1));
            return_value = FILTER_DECISION_ERROR;
            }
          }
        }

      for (i = 0; current_settings->current_options->rhswl_fqdn_file[i] != NULL; i++)
        if (read_file(current_settings, current_settings->current_options->rhswl_fqdn_file[i], &name_array, num_names, 1, -1) != -1)
          for (; name_array[num_names] != NULL; num_names++);
        else
          {
          return_value = FILTER_DECISION_ERROR;
          break;
          }
      }
    else
      name_array = current_settings->current_options->rhswl_fqdn;

    if ((return_value == FILTER_DECISION_UNDECIDED) &&
        (check_rhsbl(current_settings, current_settings->sender_domain, name_array, NULL, NULL, 0, &rhswl_index) == 1))
      {
      if (target_action != NULL)
        *target_action = FILTER_DECISION_DO_NOT_FILTER;
      if (target_rejection != NULL)
        *target_rejection = NULL;

      SPAMDYKE_LOG_VERBOSE(current_settings, LOG_VERBOSE_FILTER_SENDER_RHSWL, current_settings->sender_domain, name_array[rhswl_index]);
      return_value = FILTER_DECISION_DO_NOT_FILTER;
      }

    if ((name_array != NULL) &&
        (name_array != current_settings->current_options->rhswl_fqdn))
      {
      for (i = num_fqdn; name_array[i] != NULL; i++)
        free(name_array[i]);

      free(name_array);
      }
    }

  return(return_value);
  }

int filter_sender_blacklist(struct filter_settings *current_settings, int *target_action, int *return_action_locked, struct rejection_data **target_rejection, struct rejection_data *target_rejection_buf, char *target_message_buf, int size_target_message_buf)
  {
  int return_value;
  int i;
  int search_return;
  char tmp_sender_address[MAX_ADDRESS + 1];
  int strlen_sender_address;

  return_value = FILTER_DECISION_UNDECIDED;

  if (((target_action == NULL) ||
       ((*target_action) < FILTER_DECISION_DO_FILTER)) &&
      ((return_action_locked == NULL) ||
       !(*return_action_locked)) &&
      ((current_settings->current_options->blacklist_sender != NULL) ||
       (current_settings->current_options->blacklist_sender_file != NULL)) &&
      (reassemble_address(current_settings->sender_username, current_settings->sender_domain, NULL, tmp_sender_address, MAX_ADDRESS, &strlen_sender_address) != NULL) &&
      (strlen_sender_address > 0))
    {
    SPAMDYKE_LOG_DEBUG(current_settings, LOG_DEBUG_FILTER_SENDER_BLACKLIST, tmp_sender_address);

    if (current_settings->current_options->blacklist_sender != NULL)
      for (i = 0; current_settings->current_options->blacklist_sender[i] != NULL; i++)
        if (examine_entry(tmp_sender_address, strlen_sender_address, current_settings->current_options->blacklist_sender[i], strlen(current_settings->current_options->blacklist_sender[i]), '@', "@.", '@', "@."))
          {
          if (target_action != NULL)
            *target_action = FILTER_DECISION_DO_FILTER;
          set_rejection(current_settings, REJECTION_SENDER_BLACKLISTED, target_rejection, target_rejection_buf, target_message_buf, size_target_message_buf);

          SPAMDYKE_LOG_VERBOSE(current_settings, LOG_VERBOSE_FILTER_SENDER_BLACKLIST, tmp_sender_address, current_settings->current_options->blacklist_sender[i]);
          return_value = FILTER_DECISION_DO_FILTER;
          break;
          }

    if ((return_value == FILTER_DECISION_UNDECIDED) &&
        (current_settings->current_options->blacklist_sender_file != NULL))
      for (i = 0; current_settings->current_options->blacklist_sender_file[i] != NULL; i++)
        if ((search_return = search_file(current_settings, current_settings->current_options->blacklist_sender_file[i], tmp_sender_address, strlen_sender_address, '@', "@.", '@', "@")) > 0)
          {
          if (target_action != NULL)
            *target_action = FILTER_DECISION_DO_FILTER;
          set_rejection(current_settings, REJECTION_SENDER_BLACKLISTED, target_rejection, target_rejection_buf, target_message_buf, size_target_message_buf);

          SPAMDYKE_LOG_VERBOSE(current_settings, LOG_VERBOSE_FILTER_SENDER_BLACKLIST_FILE, tmp_sender_address, current_settings->current_options->blacklist_sender_file[i], search_return);
          return_value = FILTER_DECISION_DO_FILTER;
          break;
          }
    }

  return(return_value);
  }

int filter_sender_rhsbl(struct filter_settings *current_settings, int *target_action, int *return_action_locked, struct rejection_data **target_rejection, struct rejection_data *target_rejection_buf, char *target_message_buf, int size_target_message_buf)
  {
  int return_value;
  int i;
  int num_fqdn;
  int num_names;
  int rhsbl_index;
  char **name_array;

  return_value = FILTER_DECISION_UNDECIDED;
  num_fqdn = 0;
  num_names = 0;
  rhsbl_index = -1;
  name_array = NULL;

  /* Check RHSBL */
  if (((target_action == NULL) ||
       ((*target_action) < FILTER_DECISION_DO_FILTER)) &&
      ((return_action_locked == NULL) ||
       !(*return_action_locked)) &&
      ((current_settings->current_options->rhsbl_fqdn != NULL) ||
       (current_settings->current_options->rhsbl_fqdn_file != NULL)) &&
      (current_settings->sender_domain != NULL))
    {
    SPAMDYKE_LOG_DEBUG(current_settings, LOG_DEBUG_FILTER_SENDER_RHSBL, current_settings->sender_domain);

    if (current_settings->current_options->rhsbl_fqdn_file != NULL)
      {
      if (current_settings->current_options->rhsbl_fqdn != NULL)
        {
        for (num_fqdn = 0; current_settings->current_options->rhsbl_fqdn[num_fqdn] != NULL; num_fqdn++);
        if (num_fqdn > 0)
          {
          if ((name_array = (char **)malloc(sizeof(char *) * (num_fqdn + 1))) != NULL)
            {
            memcpy(name_array, current_settings->current_options->rhsbl_fqdn, sizeof(char *) * (num_fqdn + 1));
            num_names = num_fqdn;
            }
          else
            {
            SPAMDYKE_LOG_ERROR(current_settings, LOG_ERROR_MALLOC, sizeof(char *) * (num_fqdn + 1));
            return_value = FILTER_DECISION_ERROR;
            }
          }
        }

      for (i = 0; current_settings->current_options->rhsbl_fqdn_file[i] != NULL; i++)
        if (read_file(current_settings, current_settings->current_options->rhsbl_fqdn_file[i], &name_array, num_names, 1, -1) != -1)
          for (; name_array[num_names] != NULL; num_names++);
        else
          {
          return_value = FILTER_DECISION_ERROR;
          break;
          }
      }
    else
      name_array = current_settings->current_options->rhsbl_fqdn;

    if ((return_value == FILTER_DECISION_UNDECIDED) &&
        (check_rhsbl(current_settings, current_settings->sender_domain, name_array, (current_settings->current_options->rejection_text[REJECTION_RHSBL] != NULL) ? current_settings->current_options->rejection_text[REJECTION_RHSBL] : rejection[REJECTION_RHSBL].reject_message, target_message_buf, size_target_message_buf, &rhsbl_index) == 1))
      {
      if (target_action != NULL)
        *target_action = FILTER_DECISION_DO_FILTER;
      if (target_rejection != NULL)
        {
        memcpy(target_rejection_buf, &rejection[REJECTION_RHSBL], sizeof(struct rejection_data));
        target_rejection_buf->strlen_reject_message = strlen(target_message_buf);
        target_rejection_buf->reject_message = target_message_buf;
        *target_rejection = target_rejection_buf;
        }

      SPAMDYKE_LOG_VERBOSE(current_settings, LOG_VERBOSE_FILTER_SENDER_RHSBL, current_settings->sender_domain, name_array[rhsbl_index]);
      return_value = FILTER_DECISION_DO_FILTER;
      }

    if ((name_array != NULL) &&
        (name_array != current_settings->current_options->rhsbl_fqdn))
      {
      for (i = num_fqdn; name_array[i] != NULL; i++)
        free(name_array[i]);

      free(name_array);
      }
    }

  return(return_value);
  }

int filter_level(struct filter_settings *current_settings, int *target_action, int *return_action_locked, struct rejection_data **target_rejection, struct rejection_data *target_rejection_buf, char *target_message_buf, int size_target_message_buf)
  {
  int return_value;

  return_value = FILTER_DECISION_UNDECIDED;

  switch (current_settings->current_options->filter_level)
    {
    case FILTER_LEVEL_NORMAL:
      break;
    case FILTER_LEVEL_ALLOW_ALL:
      if ((target_action == NULL) ||
          ((*target_action) < FILTER_DECISION_DO_NOT_FILTER))
        {
        if (target_action != NULL)
          *target_action = FILTER_DECISION_DO_NOT_FILTER;
        if (target_rejection != NULL)
          *target_rejection = NULL;

        SPAMDYKE_LOG_VERBOSE(current_settings, LOG_VERBOSE_FILTER_ALLOW_ALL);
        return_value = FILTER_DECISION_DO_NOT_FILTER;
        }

      break;
    case FILTER_LEVEL_REQUIRE_AUTH:
      if ((target_action == NULL) ||
          ((*target_action) < FILTER_DECISION_DO_FILTER))
        {
        SPAMDYKE_LOG_DEBUG(current_settings, LOG_DEBUG_FILTER_SMTP_AUTH, (current_settings->smtp_auth_state == SMTP_AUTH_STATE_AUTHENTICATED) ? "true" : "false");

        if (current_settings->smtp_auth_state != SMTP_AUTH_STATE_AUTHENTICATED)
          {
          if (target_action != NULL)
            *target_action = FILTER_DECISION_DO_FILTER;
          set_rejection(current_settings, REJECTION_AUTH_REQUIRED, target_rejection, target_rejection_buf, target_message_buf, size_target_message_buf);

          SPAMDYKE_LOG_VERBOSE(current_settings, LOG_VERBOSE_FILTER_SMTP_AUTH);
          return_value = FILTER_DECISION_DO_FILTER;
          }
        }

      break;
    case FILTER_LEVEL_REJECT_ALL:
      if ((target_action == NULL) ||
          ((*target_action) < FILTER_DECISION_DO_FILTER))
        {
        if (target_action != NULL)
          *target_action = FILTER_DECISION_DO_FILTER;
        set_rejection(current_settings, REJECTION_UNCONDITIONAL, target_rejection, target_rejection_buf, target_message_buf, size_target_message_buf);

        SPAMDYKE_LOG_VERBOSE(current_settings, LOG_VERBOSE_FILTER_REJECT_ALL);
        return_value = FILTER_DECISION_DO_FILTER;

        if (return_action_locked != NULL)
          *return_action_locked = 1;
        }

      break;
    }

  return(return_value);
  }

int filter_sender_no_mx(struct filter_settings *current_settings, int *target_action, int *return_action_locked, struct rejection_data **target_rejection, struct rejection_data *target_rejection_buf, char *target_message_buf, int size_target_message_buf)
  {
  int return_value;

  return_value = FILTER_DECISION_UNDECIDED;

  if (((target_action == NULL) ||
       ((*target_action) < FILTER_DECISION_DO_FILTER)) &&
      ((return_action_locked == NULL) ||
       !(*return_action_locked)) &&
      !current_settings->local_sender &&
      current_settings->current_options->check_sender_mx &&
      (current_settings->sender_domain != NULL) &&
      (current_settings->sender_domain[0] != '\0'))
    {
    SPAMDYKE_LOG_DEBUG(current_settings, LOG_DEBUG_FILTER_SENDER_MX, current_settings->sender_domain);

    if (!nihdns_mx(current_settings, current_settings->sender_domain, NULL))
      {
      if (target_action != NULL)
        *target_action = FILTER_DECISION_DO_FILTER;
      set_rejection(current_settings, REJECTION_SENDER_NO_MX, target_rejection, target_rejection_buf, target_message_buf, size_target_message_buf);

      SPAMDYKE_LOG_VERBOSE(current_settings, LOG_VERBOSE_FILTER_SENDER_MX, current_settings->sender_domain);
      return_value = FILTER_DECISION_DO_FILTER;
      }
    }

  return(return_value);
  }

int filter_recipient_whitelist(struct filter_settings *current_settings, int *target_action, int *return_action_locked, struct rejection_data **target_rejection)
  {
  int return_value;
  int i;
  int search_return;
  char tmp_recipient_address[MAX_ADDRESS + 1];
  int strlen_recipient_address;

  return_value = FILTER_DECISION_UNDECIDED;

  if (((target_action == NULL) ||
       ((*target_action) < FILTER_DECISION_TRANSIENT_DO_NOT_FILTER)) &&
      ((return_action_locked == NULL) ||
       !(*return_action_locked)) &&
      ((current_settings->current_options->whitelist_recipient != NULL) ||
       (current_settings->current_options->whitelist_recipient_file != NULL)) &&
      (reassemble_address(current_settings->recipient_username, current_settings->recipient_domain, NULL, tmp_recipient_address, MAX_ADDRESS, &strlen_recipient_address) != NULL) &&
      (strlen_recipient_address > 0))
    {
    SPAMDYKE_LOG_DEBUG(current_settings, LOG_DEBUG_FILTER_RECIPIENT_WHITELIST, tmp_recipient_address);

    if (current_settings->current_options->whitelist_recipient != NULL)
      for (i = 0; current_settings->current_options->whitelist_recipient[i] != NULL; i++)
        if (examine_entry(tmp_recipient_address, strlen_recipient_address, current_settings->current_options->whitelist_recipient[i], strlen(current_settings->current_options->whitelist_recipient[i]), '@', "@.", '@', "@"))
          {
          if (target_action != NULL)
            *target_action = FILTER_DECISION_TRANSIENT_DO_NOT_FILTER;
          if (target_rejection != NULL)
            *target_rejection = NULL;

          SPAMDYKE_LOG_VERBOSE(current_settings, LOG_VERBOSE_FILTER_RECIPIENT_WHITELIST, tmp_recipient_address, current_settings->current_options->whitelist_recipient[i]);
          return_value = FILTER_DECISION_TRANSIENT_DO_NOT_FILTER;
          break;
          }

    if ((return_value == FILTER_DECISION_UNDECIDED) &&
        (current_settings->current_options->whitelist_recipient_file != NULL))
      for (i = 0; current_settings->current_options->whitelist_recipient_file[i] != NULL; i++)
        if ((search_return = search_file(current_settings, current_settings->current_options->whitelist_recipient_file[i], tmp_recipient_address, strlen_recipient_address, '@', "@.", '@', "@")) > 0)
          {
          if (target_action != NULL)
            *target_action = FILTER_DECISION_TRANSIENT_DO_NOT_FILTER;
          if (target_rejection != NULL)
            *target_rejection = NULL;

          SPAMDYKE_LOG_VERBOSE(current_settings, LOG_VERBOSE_FILTER_RECIPIENT_WHITELIST_FILE, tmp_recipient_address, current_settings->current_options->whitelist_recipient_file[i], search_return);
          return_value = FILTER_DECISION_TRANSIENT_DO_NOT_FILTER;
          break;
          }
    }

  return(return_value);
  }

int filter_recipient_local(struct filter_settings *current_settings, int *target_action, int *return_action_locked, struct rejection_data **target_rejection, struct rejection_data *target_rejection_buf, char *target_message_buf, int size_target_message_buf)
  {
  int return_value;
  char tmp_address[MAX_ADDRESS + 1];

  return_value = FILTER_DECISION_UNDECIDED;

  SPAMDYKE_LOG_DEBUG(current_settings, LOG_DEBUG_FILTER_RECIPIENT_LOCAL, reassemble_address(current_settings->recipient_username, current_settings->recipient_domain, NULL, tmp_address, MAX_ADDRESS, NULL));

  if (((target_action == NULL) ||
       ((*target_action) < FILTER_DECISION_TRANSIENT_DO_FILTER) ||
       ((*target_rejection) == NULL)) &&
      ((return_action_locked == NULL) ||
       !(*return_action_locked)) &&
      (current_settings->recipient_domain[0] == '\0'))
    {
    if (target_action != NULL)
      *target_action = FILTER_DECISION_TRANSIENT_DO_FILTER;
    set_rejection(current_settings, REJECTION_RCPT_TO_LOCAL, target_rejection, target_rejection_buf, target_message_buf, size_target_message_buf);

    SPAMDYKE_LOG_VERBOSE(current_settings, LOG_VERBOSE_FILTER_RECIPIENT_LOCAL, reassemble_address(current_settings->recipient_username, current_settings->recipient_domain, NULL, tmp_address, MAX_ADDRESS, NULL));
    return_value = FILTER_DECISION_TRANSIENT_DO_FILTER;
    }

  return(return_value);
  }

int filter_recipient_relay(struct filter_settings *current_settings, int *target_action, int *return_action_locked, struct rejection_data **target_rejection, struct rejection_data *target_rejection_buf, char *target_message_buf, int size_target_message_buf)
  {
  int return_value;
  int prevent_relay;
  char tmp_address[MAX_ADDRESS + 1];

  return_value = FILTER_DECISION_UNDECIDED;
  prevent_relay = 0;
  tmp_address[0] = '\0';

  SPAMDYKE_LOG_DEBUG(current_settings, LOG_DEBUG_FILTER_RELAY, current_settings->current_options->relay_level, reassemble_address(current_settings->recipient_username, current_settings->recipient_domain, NULL, tmp_address, MAX_ADDRESS, NULL), current_settings->server_ip, (current_settings->strlen_server_name > 0) ? current_settings->server_name : LOG_MISSING_DATA, current_settings->local_recipient ? "true" : "false", current_settings->allow_relay ? "true" : "false");

  if (((*target_rejection) == NULL) &&
      ((return_action_locked == NULL) ||
       !(*return_action_locked)))
    {
    if (current_settings->current_options->relay_level == RELAY_LEVEL_NO_RELAY)
      {
      if (!current_settings->local_recipient)
        {
        prevent_relay = 1;

        if (return_action_locked != NULL)
          *return_action_locked = 1;
        }
      }
    else if ((target_action == NULL) ||
             ((*target_action) < FILTER_DECISION_TRANSIENT_DO_FILTER))
      switch (current_settings->current_options->relay_level)
        {
        case RELAY_LEVEL_NO_CHECK:
          break;
        case RELAY_LEVEL_NORMAL:
          if (((target_action == NULL) ||
               ((*target_action) < FILTER_DECISION_TRANSIENT_DO_FILTER)) &&
              !current_settings->local_recipient &&
              !current_settings->allow_relay)
            prevent_relay = 1;

          break;
        case RELAY_LEVEL_ALLOW_ALL:
          break;
        }
    }

  if (prevent_relay)
    {
    if (target_action != NULL)
      *target_action = FILTER_DECISION_TRANSIENT_DO_FILTER;
    set_rejection(current_settings, REJECTION_RELAYING_DENIED, target_rejection, target_rejection_buf, target_message_buf, size_target_message_buf);

    SPAMDYKE_LOG_VERBOSE(current_settings, LOG_VERBOSE_FILTER_RELAY);
    return_value = FILTER_DECISION_TRANSIENT_DO_FILTER;
    }

  return(return_value);
  }

int filter_recipient_max(struct filter_settings *current_settings, int *target_action, int *return_action_locked, struct rejection_data **target_rejection, struct rejection_data *target_rejection_buf, char *target_message_buf, int size_target_message_buf)
  {
  int return_value;

  return_value = FILTER_DECISION_UNDECIDED;

  if (((target_action == NULL) ||
       ((*target_action) < FILTER_DECISION_TRANSIENT_DO_FILTER) ||
       ((*target_rejection) == NULL)) &&
      ((return_action_locked == NULL) ||
       !(*return_action_locked)) &&
      (current_settings->current_options->max_rcpt_to > 0))
    {
    SPAMDYKE_LOG_DEBUG(current_settings, LOG_DEBUG_FILTER_RECIPIENT_MAX, current_settings->current_options->max_rcpt_to, current_settings->num_rcpt_to);

    if (current_settings->num_rcpt_to >= current_settings->current_options->max_rcpt_to)
      {
      if (target_action != NULL)
        *target_action = FILTER_DECISION_TRANSIENT_DO_FILTER;
      set_rejection(current_settings, REJECTION_RCPT_TO, target_rejection, target_rejection_buf, target_message_buf, size_target_message_buf);

      SPAMDYKE_LOG_VERBOSE(current_settings, LOG_VERBOSE_FILTER_RECIPIENT_MAX, current_settings->current_options->max_rcpt_to);
      return_value = FILTER_DECISION_TRANSIENT_DO_FILTER;
      }
    }

  return(return_value);
  }

int filter_recipient_blacklist(struct filter_settings *current_settings, int *target_action, int *return_action_locked, struct rejection_data **target_rejection, struct rejection_data *target_rejection_buf, char *target_message_buf, int size_target_message_buf)
  {
  int return_value;
  int i;
  int search_return;
  char tmp_address[MAX_ADDRESS + 1];
  int strlen_recipient_address;

  return_value = FILTER_DECISION_UNDECIDED;

  if (((target_action == NULL) ||
       ((*target_action) < FILTER_DECISION_TRANSIENT_DO_FILTER) ||
       ((*target_rejection) == NULL)) &&
      ((return_action_locked == NULL) ||
       !(*return_action_locked)) &&
      ((current_settings->current_options->blacklist_recipient != NULL) ||
       (current_settings->current_options->blacklist_recipient_file != NULL)) &&
      (reassemble_address(current_settings->recipient_username, current_settings->recipient_domain, NULL, tmp_address, MAX_ADDRESS, &strlen_recipient_address) != NULL) &&
      (strlen_recipient_address > 0))
    {
    SPAMDYKE_LOG_DEBUG(current_settings, LOG_DEBUG_FILTER_RECIPIENT_BLACKLIST, tmp_address);

    if (current_settings->current_options->blacklist_recipient != NULL)
      for (i = 0; current_settings->current_options->blacklist_recipient[i] != NULL; i++)
        if (examine_entry(tmp_address, strlen_recipient_address, current_settings->current_options->blacklist_recipient[i], strlen(current_settings->current_options->blacklist_recipient[i]), '@', "@.", '@', "@"))
          {
          if (target_action != NULL)
            *target_action = FILTER_DECISION_TRANSIENT_DO_FILTER;
          set_rejection(current_settings, REJECTION_RECIPIENT_BLACKLISTED, target_rejection, target_rejection_buf, target_message_buf, size_target_message_buf);

          SPAMDYKE_LOG_VERBOSE(current_settings, LOG_VERBOSE_FILTER_RECIPIENT_BLACKLIST, tmp_address, current_settings->current_options->blacklist_recipient[i]);
          return_value = FILTER_DECISION_TRANSIENT_DO_FILTER;
          break;
          }

    if ((return_value == FILTER_DECISION_UNDECIDED) &&
        (current_settings->current_options->blacklist_recipient_file != NULL))
      for (i = 0; current_settings->current_options->blacklist_recipient_file[i] != NULL; i++)
        if ((search_return = search_file(current_settings, current_settings->current_options->blacklist_recipient_file[i], tmp_address, strlen_recipient_address, '@', "@.", '@', "@")) > 0)
          {
          if (target_action != NULL)
            *target_action = FILTER_DECISION_TRANSIENT_DO_FILTER;
          set_rejection(current_settings, REJECTION_RECIPIENT_BLACKLISTED, target_rejection, target_rejection_buf, target_message_buf, size_target_message_buf);

          SPAMDYKE_LOG_VERBOSE(current_settings, LOG_VERBOSE_FILTER_RECIPIENT_BLACKLIST_FILE, tmp_address, current_settings->current_options->blacklist_recipient_file[i], search_return);
          return_value = FILTER_DECISION_TRANSIENT_DO_FILTER;
          break;
          }
    }

  return(return_value);
  }

int filter_recipient_graylist(struct filter_settings *current_settings, int *target_action, int *return_action_locked, struct rejection_data **target_rejection, struct rejection_data *target_rejection_buf, char *target_message_buf, int size_target_message_buf)
  {
  int return_value;
  int i;
  int continue_processing;
  int found_match;
  int graylist_index;
  int search_return;
  struct stat tmp_stat;
  FILE *tmp_file;
  char graylist_path[MAX_PATH + 1];
  char log_entry[MAX_BUF + 1];
  int strlen_log_entry;
  char tmp_path[MAX_PATH + 1];
  char tmp_address[MAX_ADDRESS + 1];
  char tmp_sender_address[MAX_ADDRESS + 1];
  char tmp_recipient_address[MAX_ADDRESS + 1];
  char canonicalized_sender_username[MAX_ADDRESS + 1];
  char canonicalized_sender_domain[MAX_ADDRESS + 1];
  char canonicalized_recipient_username[MAX_ADDRESS + 1];
  char canonicalized_recipient_domain[MAX_ADDRESS + 1];

  return_value = FILTER_DECISION_UNDECIDED;

  if (((target_action == NULL) ||
       ((*target_action) < FILTER_DECISION_TRANSIENT_DO_FILTER) ||
       ((*target_rejection) == NULL)) &&
      ((return_action_locked == NULL) ||
       !(*return_action_locked)) &&
      (current_settings->current_options->graylist_level != GRAYLIST_LEVEL_NONE) &&
      (current_settings->current_options->graylist_dir != NULL) &&
      (reassemble_address(current_settings->sender_username, current_settings->sender_domain, NULL, tmp_sender_address, MAX_ADDRESS, NULL) != NULL) &&
      (reassemble_address(current_settings->recipient_username, current_settings->recipient_domain, NULL, tmp_recipient_address, MAX_ADDRESS, NULL) != NULL) &&
      (tmp_recipient_address[0] != '\0'))
    {
    canonicalize_path(canonicalized_sender_username, MAX_ADDRESS, current_settings->sender_username, strlen(current_settings->sender_username));
    canonicalize_path(canonicalized_sender_domain, MAX_ADDRESS, current_settings->sender_domain, strlen(current_settings->sender_domain));
    canonicalize_path(canonicalized_recipient_username, MAX_ADDRESS, current_settings->recipient_username, strlen(current_settings->recipient_username));
    canonicalize_path(canonicalized_recipient_domain, MAX_ADDRESS, current_settings->recipient_domain, strlen(current_settings->recipient_domain));

    SPAMDYKE_LOG_DEBUG(current_settings, LOG_DEBUG_FILTER_GRAYLIST, (tmp_recipient_address[0] != '\0') ? tmp_recipient_address : LOG_MISSING_DATA, (tmp_sender_address[0] != '\0') ? tmp_sender_address : LOG_MISSING_DATA);

    found_match = 0;
    graylist_index = -1;

    if (current_settings->current_options->graylist_exception_ip != NULL)
      for (i = 0; current_settings->current_options->graylist_exception_ip[i] != NULL; i++)
        if (examine_tcprules_entry(current_settings, NULL, 0, current_settings->current_options->graylist_exception_ip[i], strlen(current_settings->current_options->graylist_exception_ip[i]), current_settings->server_ip, current_settings->server_name, current_settings->strlen_server_name))
          {
          found_match = 1;
          break;
          }

    if (!found_match &&
        (current_settings->current_options->graylist_exception_ip_file != NULL))
      for (i = 0; current_settings->current_options->graylist_exception_ip_file[i] != NULL; i++)
        if (search_tcprules_file(current_settings, NULL, 0, current_settings->current_options->graylist_exception_ip_file[i], current_settings->server_ip, current_settings->server_name, current_settings->strlen_server_name) > 0)
          {
          found_match = 1;
          break;
          }

    if (!found_match &&
        (current_settings->current_options->graylist_exception_rdns != NULL))
      for (i = 0; current_settings->current_options->graylist_exception_rdns[i] != NULL; i++)
        if (examine_entry(current_settings->server_name, current_settings->strlen_server_name, current_settings->current_options->graylist_exception_rdns[i], strlen(current_settings->current_options->graylist_exception_rdns[i]), '.', ".", '\0', NULL))
          {
          found_match = 1;
          break;
          }

    if (!found_match &&
        (current_settings->current_options->graylist_exception_rdns_file != NULL))
      for (i = 0; current_settings->current_options->graylist_exception_rdns_file[i] != NULL; i++)
        if (search_file(current_settings, current_settings->current_options->graylist_exception_rdns_file[i], current_settings->server_name, current_settings->strlen_server_name, '.', ".", '\0', NULL) > 0)
          {
          found_match = 1;
          break;
          }

    if (!found_match &&
        (current_settings->current_options->graylist_exception_rdns_dir != NULL))
      for (i = 0; current_settings->current_options->graylist_exception_rdns_dir[i] != NULL; i++)
        if (search_domain_directory(current_settings, current_settings->current_options->graylist_exception_rdns_dir[i], current_settings->server_name, current_settings->strlen_server_name) != NULL)
          {
          found_match = 1;
          break;
          }

    continue_processing = 0;

    /* Figure out what kind of graylisting is wanted and if it's needed for this connection. */
    if ((((current_settings->current_options->graylist_level & GRAYLIST_LEVEL_MASK_BEHAVIOR) == GRAYLIST_LEVEL_FLAG_ONLY) &&
         found_match) ||
        (((current_settings->current_options->graylist_level & GRAYLIST_LEVEL_MASK_BEHAVIOR) == GRAYLIST_LEVEL_FLAG_ALWAYS) &&
         !found_match))
      {
      for (graylist_index = 0; current_settings->current_options->graylist_dir[graylist_index] != NULL; graylist_index++)
        if ((snprintf(graylist_path, MAX_PATH, "%s" DIR_DELIMITER_STR "%s", current_settings->current_options->graylist_dir[graylist_index], canonicalized_recipient_domain) < MAX_PATH) &&
            (stat(graylist_path, &tmp_stat) == 0) &&
            S_ISDIR(tmp_stat.st_mode))
          {
          SPAMDYKE_LOG_EXCESSIVE(current_settings, LOG_DEBUGX_GRAYLIST_DOMAIN_FOUND, graylist_path);
          continue_processing = 1;
          break;
          }

      if (!continue_processing &&
          ((current_settings->current_options->graylist_level & GRAYLIST_LEVEL_MASK_CREATION) == GRAYLIST_LEVEL_FLAG_CREATE) &&
          current_settings->local_recipient)
        {
        for (graylist_index = 0; current_settings->current_options->graylist_dir[graylist_index] != NULL; graylist_index++)
          if ((snprintf(graylist_path, MAX_PATH, "%s" DIR_DELIMITER_STR "%s", current_settings->current_options->graylist_dir[graylist_index], canonicalized_recipient_domain) < MAX_PATH) &&
              (mkdir(graylist_path, MKDIR_MODE) == 0))
            {
            SPAMDYKE_LOG_EXCESSIVE(current_settings, LOG_DEBUGX_GRAYLIST_DOMAIN_CREATE, graylist_path);
            continue_processing = 1;
            break;
            }
          else
            SPAMDYKE_LOG_ERROR(current_settings, LOG_ERROR_MKDIR "%s: %s", graylist_path, strerror(errno));
        }
      }

    /* Construct the path to the recipient's graylist folder and check if it exists */
    if (continue_processing &&
        (snprintf(graylist_path, MAX_PATH, "%s" DIR_DELIMITER_STR "%s" DIR_DELIMITER_STR "%s", current_settings->current_options->graylist_dir[graylist_index], canonicalized_recipient_domain, canonicalized_recipient_username) < MAX_PATH) &&
        (stat(graylist_path, &tmp_stat) != 0))
      {
      if (errno == ENOENT)
        {
        if (mkdir(graylist_path, MKDIR_MODE) == 0)
          SPAMDYKE_LOG_EXCESSIVE(current_settings, LOG_DEBUGX_GRAYLIST_RECIPIENT_CREATE, graylist_path);
        else
          {
          continue_processing = 0;
          SPAMDYKE_LOG_ERROR(current_settings, LOG_ERROR_MKDIR "%s: %s", graylist_path, strerror(errno));
          }
        }
      else
        {
        continue_processing = 0;
        SPAMDYKE_LOG_ERROR(current_settings, LOG_ERROR_STAT "%s: %s", graylist_path, strerror(errno));
        }
      }

    /*
     * Special case of moving pre-4.0 files to the 4.0 structure: if the sender
     * is empty, the existing file is named "_none" and the new file should be
     * named "_none/_none".
     */
    if (continue_processing &&
        (current_settings->sender_username[0] == '\0') &&
        (current_settings->sender_domain[0] == '\0') &&
        (snprintf(graylist_path, MAX_PATH, "%s" DIR_DELIMITER_STR "%s" DIR_DELIMITER_STR "%s" DIR_DELIMITER_STR SENDER_DOMAIN_NONE_TEMP, current_settings->current_options->graylist_dir[graylist_index], canonicalized_recipient_domain, canonicalized_recipient_username) < MAX_PATH) &&
        (snprintf(tmp_path, MAX_PATH, "%s" DIR_DELIMITER_STR "%s" DIR_DELIMITER_STR "%s" DIR_DELIMITER_STR SENDER_DOMAIN_NONE, current_settings->current_options->graylist_dir[graylist_index], canonicalized_recipient_domain, canonicalized_recipient_username) < MAX_PATH) &&
        (stat(tmp_path, &tmp_stat) == 0) &&
        S_ISREG(tmp_stat.st_mode))
      {
      if (rename(tmp_path, graylist_path) == 0)
        {
        SPAMDYKE_LOG_EXCESSIVE(current_settings, LOG_DEBUGX_GRAYLIST_MOVE, tmp_path, graylist_path);

        if (mkdir(tmp_path, MKDIR_MODE) == 0)
          {
          SPAMDYKE_LOG_EXCESSIVE(current_settings, LOG_DEBUGX_GRAYLIST_SENDER_CREATE, tmp_path);

          if ((snprintf(tmp_path, MAX_PATH, "%s" DIR_DELIMITER_STR "%s" DIR_DELIMITER_STR "%s" DIR_DELIMITER_STR SENDER_DOMAIN_NONE DIR_DELIMITER_STR SENDER_ADDRESS_NONE, current_settings->current_options->graylist_dir[graylist_index], canonicalized_recipient_domain, canonicalized_recipient_username) < MAX_PATH) &&
              (rename(graylist_path, tmp_path) == 0))
            SPAMDYKE_LOG_EXCESSIVE(current_settings, LOG_DEBUGX_GRAYLIST_MOVE, graylist_path, tmp_path);
          else
            {
            SPAMDYKE_LOG_ERROR(current_settings, LOG_ERROR_MOVE "%s to %s: %s", graylist_path, tmp_path, strerror(errno));

            if (unlink(graylist_path) != 0)
              {
              continue_processing = 0;
              SPAMDYKE_LOG_ERROR(current_settings, LOG_ERROR_UNLINK "%s: %s", graylist_path, strerror(errno));
              }
            }
          }
        else
          {
          continue_processing = 0;
          SPAMDYKE_LOG_ERROR(current_settings, LOG_ERROR_MKDIR "%s: %s", graylist_path, strerror(errno));
          }
        }
      else
        {
        SPAMDYKE_LOG_ERROR(current_settings, LOG_ERROR_MOVE "%s to %s: %s", tmp_path, graylist_path, strerror(errno));

        if (unlink(tmp_path) != 0)
          {
          continue_processing = 0;
          SPAMDYKE_LOG_ERROR(current_settings, LOG_ERROR_UNLINK "%s: %s", tmp_path, strerror(errno));
          }
        }
      }
    /* Construct the path to the sender's domain's graylist folder and check if it exists */
    else if (continue_processing &&
             (snprintf(graylist_path, MAX_PATH, "%s" DIR_DELIMITER_STR "%s" DIR_DELIMITER_STR "%s" DIR_DELIMITER_STR "%s", current_settings->current_options->graylist_dir[graylist_index], canonicalized_recipient_domain, canonicalized_recipient_username, (canonicalized_sender_domain[0] != '\0') ? canonicalized_sender_domain : SENDER_DOMAIN_NONE) < MAX_PATH) &&
             (stat(graylist_path, &tmp_stat) != 0))
      {
      if (errno == ENOENT)
        {
        if (mkdir(graylist_path, MKDIR_MODE) == 0)
          SPAMDYKE_LOG_EXCESSIVE(current_settings, LOG_DEBUGX_GRAYLIST_SENDER_CREATE, graylist_path);
        else
          {
          continue_processing = 0;
          SPAMDYKE_LOG_ERROR(current_settings, LOG_ERROR_MKDIR "%s: %s", graylist_path, strerror(errno));
          }
        }
      else
        {
        continue_processing = 0;
        SPAMDYKE_LOG_ERROR(current_settings, LOG_ERROR_STAT "%s: %s", graylist_path, strerror(errno));
        }
      }

    /* Construct the path to the sender->user file using the pre-4.0 logic and move it to the 4.0 path if it exists */
    if (continue_processing &&
        (current_settings->sender_username[0] != '\0') &&
        (current_settings->sender_domain[0] != '\0') &&
        (snprintf(graylist_path, MAX_PATH, "%s" DIR_DELIMITER_STR "%s" DIR_DELIMITER_STR "%s" DIR_DELIMITER_STR "%s" DIR_DELIMITER_STR "%s", current_settings->current_options->graylist_dir[graylist_index], canonicalized_recipient_domain, canonicalized_recipient_username, (canonicalized_sender_domain[0] != '\0') ? canonicalized_sender_domain : SENDER_DOMAIN_NONE, canonicalized_sender_username) < MAX_PATH) &&
        (snprintf(tmp_path, MAX_PATH, "%s" DIR_DELIMITER_STR "%s" DIR_DELIMITER_STR "%s" DIR_DELIMITER_STR "%s", current_settings->current_options->graylist_dir[graylist_index], canonicalized_recipient_domain, canonicalized_recipient_username, canonicalize_path(tmp_address, MAX_ADDRESS, tmp_sender_address, -1)) < MAX_PATH) &&
        (stat(tmp_path, &tmp_stat) == 0) &&
        S_ISREG(tmp_stat.st_mode))
      {
      if (rename(tmp_path, graylist_path) == 0)
        SPAMDYKE_LOG_EXCESSIVE(current_settings, LOG_DEBUGX_GRAYLIST_MOVE, tmp_path, graylist_path);
      else
        {
        SPAMDYKE_LOG_ERROR(current_settings, LOG_ERROR_MOVE "%s to %s: %s", tmp_path, graylist_path, strerror(errno));

        if (unlink(tmp_path) != 0)
          {
          continue_processing = 0;
          SPAMDYKE_LOG_ERROR(current_settings, LOG_ERROR_UNLINK "%s: %s", tmp_path, strerror(errno));
          }
        }
      }

    /* Construct the path to the sender->user file and check if it exists */
    if (continue_processing &&
        (snprintf(graylist_path, MAX_PATH, "%s" DIR_DELIMITER_STR "%s" DIR_DELIMITER_STR "%s" DIR_DELIMITER_STR "%s" DIR_DELIMITER_STR "%s", current_settings->current_options->graylist_dir[graylist_index], canonicalized_recipient_domain, canonicalized_recipient_username, (canonicalized_sender_domain[0] != '\0') ? canonicalized_sender_domain : SENDER_DOMAIN_NONE, (canonicalized_sender_username[0] != '\0') ? canonicalized_sender_username : SENDER_ADDRESS_NONE) < MAX_PATH) &&
        (stat(graylist_path, &tmp_stat) == 0))
      /* Check that the file is a "regular" file and the age matches the command line options */
      if ((S_ISREG(tmp_stat.st_mode)) &&
          (((current_settings->current_options->graylist_min_secs > 0) &&
            (((tmp_stat.st_mtime + current_settings->current_options->graylist_min_secs) <= time(NULL)) ||
             (tmp_stat.st_size > 0))) ||
           (current_settings->current_options->graylist_min_secs == 0)) &&
          (((current_settings->current_options->graylist_max_secs > 0) &&
            ((tmp_stat.st_mtime + current_settings->current_options->graylist_max_secs) >= time(NULL))) ||
           (current_settings->current_options->graylist_max_secs == 0)))
        {
        /* User was graylisted but is now allowed.  Log the name of their sending host to the file. */
        if (current_settings->strlen_server_name > 0)
          strlen_log_entry = snprintf(log_entry, MAX_BUF, "%s %s", current_settings->server_ip, current_settings->server_name);
        else
          strlen_log_entry = snprintf(log_entry, MAX_BUF, "%s", current_settings->server_ip);

        search_return = search_file(current_settings, graylist_path, log_entry, strlen_log_entry, '\0', NULL, '\0', NULL);
        if ((tmp_file = fopen(graylist_path, "a")) != NULL)
          {
          chmod(graylist_path, CHMOD_MODE);

          if ((search_return <= 0) &&
              (fprintf(tmp_file, "%s\n", log_entry) == -1))
            SPAMDYKE_LOG_ERROR(current_settings, LOG_ERROR_FPRINTF_BYTES "%s", strlen_log_entry, graylist_path, strerror(errno));

          fclose(tmp_file);
          }
        else
          SPAMDYKE_LOG_ERROR(current_settings, LOG_ERROR_GRAYLIST_FILE "%s: %s", graylist_path, strerror(errno));
        }
      else
        {
        /* The file's age is outside the bounds of the command line options. */
        SPAMDYKE_LOG_VERBOSE(current_settings, LOG_VERBOSE_FILTER_GRAYLIST, tmp_sender_address, tmp_recipient_address, graylist_path);
        return_value = FILTER_DECISION_TRANSIENT_DO_FILTER;

        /* The file was too old -- truncate it. */
        if ((current_settings->current_options->graylist_max_secs > 0) &&
            ((tmp_stat.st_mtime + current_settings->current_options->graylist_max_secs) <= time(NULL)))
          {
          if ((tmp_file = fopen(graylist_path, "w")) != NULL)
            {
            chmod(graylist_path, CHMOD_MODE);

            fclose(tmp_file);
            }
          else
            {
            return_value = FILTER_DECISION_UNDECIDED;
            SPAMDYKE_LOG_ERROR(current_settings, LOG_ERROR_GRAYLIST_FILE "%s: %s", graylist_path, strerror(errno));
            }
          }

        if (return_value == FILTER_DECISION_TRANSIENT_DO_FILTER)
          {
          if (target_action != NULL)
            *target_action = FILTER_DECISION_TRANSIENT_DO_FILTER;
          set_rejection(current_settings, REJECTION_GRAYLISTED, target_rejection, target_rejection_buf, target_message_buf, size_target_message_buf);
          }
        }
    /* Sender->user graylist file does not exist -- create it. */
    else if (continue_processing &&
             ((tmp_file = fopen(graylist_path, "w")) != NULL))
      {
      chmod(graylist_path, CHMOD_MODE);

      fclose(tmp_file);

      if (target_action != NULL)
        *target_action = FILTER_DECISION_TRANSIENT_DO_FILTER;
      set_rejection(current_settings, REJECTION_GRAYLISTED, target_rejection, target_rejection_buf, target_message_buf, size_target_message_buf);

      SPAMDYKE_LOG_VERBOSE(current_settings, LOG_VERBOSE_FILTER_GRAYLIST, tmp_sender_address, tmp_recipient_address, graylist_path);
      return_value = FILTER_DECISION_TRANSIENT_DO_FILTER;
      }
    else if (continue_processing)
      SPAMDYKE_LOG_ERROR(current_settings, LOG_ERROR_GRAYLIST_FILE "%s: %s", graylist_path, strerror(errno));
    }

  return(return_value);
  }

/*
 * Return value:
 *   FILTER_DECISION value
 */
int filter_identical_from_to(struct filter_settings *current_settings, int *target_action, int *return_action_locked, struct rejection_data **target_rejection, struct rejection_data *target_rejection_buf, char *target_message_buf, int size_target_message_buf)
  {
  int return_value;
  char tmp_sender_address[MAX_ADDRESS + 1];
  char tmp_recipient_address[MAX_ADDRESS + 1];
  int strlen_sender_address;
  int strlen_recipient_address;

  return_value = FILTER_DECISION_UNDECIDED;

  /* Check if the sender and recipient addresses are the same. */
  if (((target_action == NULL) ||
       ((*target_action) < FILTER_DECISION_DO_FILTER)) &&
      ((return_action_locked == NULL) ||
       !(*return_action_locked)) &&
      current_settings->current_options->check_identical_from_to &&
      (reassemble_address(current_settings->sender_username, current_settings->sender_domain, NULL, tmp_sender_address, MAX_ADDRESS, &strlen_sender_address) != NULL) &&
      (strlen_sender_address > 0) &&
      (reassemble_address(current_settings->recipient_username, current_settings->recipient_domain, NULL, tmp_recipient_address, MAX_ADDRESS, &strlen_recipient_address) != NULL) &&
      (strlen_recipient_address > 0))
    {
    SPAMDYKE_LOG_DEBUG(current_settings, LOG_DEBUG_FILTER_IDENTICAL_FROM_TO, tmp_sender_address, tmp_recipient_address);

    if (strncmp(tmp_sender_address, tmp_recipient_address, MAXVAL(strlen_sender_address, strlen_recipient_address)) == 0)
      {
      if (target_action != NULL)
        *target_action = FILTER_DECISION_TRANSIENT_DO_FILTER;
      set_rejection(current_settings, REJECTION_IDENTICAL_FROM_TO, target_rejection, target_rejection_buf, target_message_buf, size_target_message_buf);

      SPAMDYKE_LOG_VERBOSE(current_settings, LOG_VERBOSE_FILTER_IDENTICAL_FROM_TO, tmp_sender_address, tmp_recipient_address);
      return_value = FILTER_DECISION_TRANSIENT_DO_FILTER;
      }
    }

  return(return_value);
  }

/*
int filter_template(struct filter_settings *current_settings, int *target_action, int *return_action_locked, struct rejection_data **target_rejection)
  {
  int return_value;

  return_value = FILTER_DECISION_UNDECIDED;

  return(return_value);
  }
*/
