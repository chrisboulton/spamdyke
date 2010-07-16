/*
  timefilter -- a utility for printing log messages from a given time range
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
#include <time.h>
#include <string.h>
#include <ctype.h>
#include "config.h"

char *month_list[] = { "jan", "feb", "mar", "apr", "may", "jun", "jul", "aug", "sep", "oct", "nov", "dec" };
time_t now;
struct tm *today;

unsigned long long get_tai(char *target_string)
  {
  unsigned long long secs;
  unsigned long long nanosecs;
  unsigned long long u;
  char month[3];
  int i;
  struct tm *tmp_tm;

  secs = 0;
  nanosecs = 0;

  if (target_string[0] == '@')
    {
    for (target_string++; target_string[0] != '\0'; target_string++)
      {
      u = target_string[0] - '0';
      if (u >= 10)
        {
        u = target_string[0] - 'a';
        if (u >= 6)
          break;
        u += 10;
        }

      secs <<= 4;
      secs += nanosecs >> 28;
      nanosecs &= 0xfffffff;
      nanosecs <<= 4;
      nanosecs += u;
      }

    secs -= 4611686018427387914ULL;
    }
  else
    {
    tmp_tm = localtime(&now);

    tmp_tm->tm_year = 0;

    if ((sscanf(target_string, "%c%c%c %d %d:%d:%d", &month[0], &month[1], &month[2], &tmp_tm->tm_mday, &tmp_tm->tm_hour, &tmp_tm->tm_min, &tmp_tm->tm_sec) == 7) ||
        (sscanf(target_string, "%*[^[]\[%d/%c%c%c/%d:%d:%d:%d", &tmp_tm->tm_mday, &month[0], &month[1], &month[2], &tmp_tm->tm_year, &tmp_tm->tm_hour, &tmp_tm->tm_min, &tmp_tm->tm_sec) == 8))
      {
      month[0] = tolower((int)month[0]);
      month[1] = tolower((int)month[1]);
      month[2] = tolower((int)month[2]);

      for (i = 0; i < 12; i++)
        if (strncmp(month, month_list[i], 3) == 0)
          {
          tmp_tm->tm_mon = i;
          break;
          }

      if (tmp_tm->tm_year == 0)
        {
        if ((tmp_tm->tm_mon > today->tm_mon) ||
            ((tmp_tm->tm_mon == today->tm_mon) &&
             (tmp_tm->tm_mday > today->tm_mday)) ||
            ((tmp_tm->tm_mon == today->tm_mon) &&
             (tmp_tm->tm_mday == today->tm_mday) &&
             (tmp_tm->tm_hour > today->tm_hour)) ||
            ((tmp_tm->tm_mon == today->tm_mon) &&
             (tmp_tm->tm_mday == today->tm_mday) &&
             (tmp_tm->tm_hour == today->tm_hour) &&
             (tmp_tm->tm_min > today->tm_min)) ||
            ((tmp_tm->tm_mon == today->tm_mon) &&
             (tmp_tm->tm_mday == today->tm_mday) &&
             (tmp_tm->tm_hour == today->tm_hour) &&
             (tmp_tm->tm_min == today->tm_min) &&
             (tmp_tm->tm_sec > today->tm_sec)))
          tmp_tm->tm_year = today->tm_year - 1;
        else
          tmp_tm->tm_year = today->tm_year;
        }
      else
        tmp_tm->tm_year -= 1900;

      tmp_tm->tm_isdst = -1;

      secs = mktime(tmp_tm);
      }
    }

  return(secs);
  }

void usage()
  {
  printf(
    PACKAGE_NAME " " PACKAGE_VERSION " (C)2010 Sam Clippinger, " PACKAGE_BUGREPORT "\n"
    "http://www.spamdyke.org/\n"
    "\n"
    "USAGE: timefilter START_SECS_AGO END_SECS_AGO\n"
    "\n"
    "Accepts input from stdin and prints only those lines that start with timestamps\n"
    "within the given range.  The range start is calculated by subtracting\n"
    "START_SECS_AGO from the current system time.  The range end is calculated by\n"
    "subtracting END_SECS_AGO from the current system time.  Obviously, if\n"
    "END_SECS_AGO is greater than START_SECS_AGO, nothing will be printed.\n"
    "\n"
    "TAI64 timestamps, syslog-style timestamps and Apache-style timestamps are\n"
    "accepted.  TAI64 timestamps generally resemble this example:\n"
    "\t@4000000048ffdbee2618dc6c\n"
    "syslog-style timestamps generally resemble this example:\n"
    "\tOct 22 21:17:11\n"
    "Apache-style timestamps generally resemble this example:\n"
    "\t0.0.0.0 - - [22/Oct/2008:21:03:50\n"
    "timefilter only understands the abbreviations of English months, however, so\n"
    "it may not work properly in other locales.\n"
    "\n"
    "Prefixing START_SECS_AGO and/or END_SECS_AGO with '+' will force timefilter to\n"
    "adjust for daylight savings time if the log is kept in standard time (e.g.\n"
    "Apache logs).\n"
    );

  return;
  }

int main(int argc, char *argv[])
  {
  char tmp_buf[65536];
  unsigned long long start_time;
  unsigned long long end_time;
  unsigned long long tmp_time;

  if (argc == 3)
    {
    now = time(NULL);
    today = localtime(&now);

    start_time = now - atoi(argv[1]);
    end_time = now - atoi(argv[2]);
    if (today->tm_isdst &&
        ((argv[1][0] == '+') ||
         (argv[2][0] == '+')))
      {
      now += 3600;
      today = localtime(&now);
      start_time += 3600;
      end_time += 3600;
      }

    while (scanf("%65535[^\r\n]%*[\r\n]", tmp_buf) == 1)
      if (((tmp_time = get_tai(tmp_buf)) >= start_time) &&
          (tmp_time <= end_time))
        printf("%s\n", tmp_buf);
    }
  else
    usage();

  return(0);
  }
