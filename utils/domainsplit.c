/*
  domainsplit -- a utility to find the base domain name of an FQDN
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
#include "config.h"

#define BUF_SIZE 1024

int main(int argc, char *argv[])
  {
  int i;
  int j;
  int strlen_buf;
  int dot;
  int tmp_int;
  unsigned char tmp_buf[BUF_SIZE];

  if ((argc == 2) &&
      (strcmp(argv[1], "-h") == 0))
    printf(
      PACKAGE_NAME " " PACKAGE_VERSION " (C)2010 Sam Clippinger, " PACKAGE_BUGREPORT "\n"
      "http://www.spamdyke.org/\n"
      "\n"
      "USAGE: domainsplit [FQDN [FQDN] ...]\n"
      "       echo FQDN | domainsplit\n"
      "\n"
      "domainsplit takes the given FQDN and returns the base domain name. For FQDNs\n"
      "with three (or more) characters in their TLD, domainsplit returns the top two\n"
      "domain parts. For FQDNs with two character country codes, domainsplit returns\n"
      "the top three domain parts. For example:\n"
      "  example -> example\n"
      "  example.com -> example.com\n"
      "  foo.example.com -> example.com\n"
      "  foo.bar.example.com -> example.com\n"
      "  example.us -> example.us\n"
      "  example.com.us -> example.com.us\n"
      "  foo.example.com.us -> example.com.us\n"
      "  foo.bar.example.com.us -> example.com.us\n"
      "\n"
      "domainsplit will accept input on its command line or through stdin and will\n"
      "process as many FQDNs as it is given.\n"
      );
  else if (argc >= 2)
    {
    for (j = 1; j < argc; j++)
      {
      strlen_buf = strlen(argv[j]);
      for (i = 0; i < strlen_buf; i++)
        argv[j][i] = tolower((int)argv[j][i]);

      dot = 0;
      for (i = strlen_buf - 1; i >= 0; i--)
        if ((argv[j][i] == '.') &&
            ((dot > 0) ||
             ((strlen_buf - i) > 3)) &&
            (++dot >= 2))
          break;

      printf("%s\n", (dot >= 2) ? (argv[j] + i + 1) : argv[j]);
      }
    }
  else
    {
    strlen_buf = 0;
    while (!feof(stdin))
      if ((((tmp_int = getc(stdin)) == EOF) ||
           ((tmp_buf[strlen_buf] = (unsigned char)tmp_int) && 0) ||
           isspace(tmp_buf[strlen_buf]) ||
           (strlen_buf >= BUF_SIZE)) &&
          (strlen_buf > 0))
        {
        tmp_buf[strlen_buf] = '\0';
        for (i = 0; i < strlen_buf; i++)
          tmp_buf[i] = tolower(tmp_buf[i]);

        dot = 0;
        for (i = strlen_buf - 1; i >= 0; i--)
          if ((tmp_buf[i] == '.') &&
              ((dot > 0) ||
               ((strlen_buf - i) > 3)) &&
              (++dot >= 2))
            break;

        printf("%s\n", (dot >= 2) ? (tmp_buf + i + 1) : tmp_buf);
        strlen_buf = 0;
        }
      else
        strlen_buf++;
    }

  return(0);
  }
