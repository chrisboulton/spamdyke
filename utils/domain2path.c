/*
  domain2path -- a utility for translating domain names to file paths.
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

int main(int argc, char *argv[])
  {
  int i;
  int j;
  int strlen_argv;
  char *domain_delimiter[3];
  int directory_only;

  directory_only = 0;

  if ((argc == 1) ||
      (strcmp(argv[1], "-h") == 0))
    printf(
      PACKAGE_NAME " " PACKAGE_VERSION " (C)2010 Sam Clippinger, " PACKAGE_BUGREPORT "\n"
      "http://www.spamdyke.org/\n"
      "\n"
      "USAGE: domain2path [ -d ] DOMAIN_NAME\n"
      "\n"
      "domain2path takes an FQDN, breaks it apart at the dots and reverses it using\n"
      "the following patterns:\n"
      "        e-five.d-four.c-three.b-two.a-one becomes\n"
      "                a-one/b/b-two/c-three/e-five.d-four.c-three.b-two.a-one\n"
      "        d-four.c-three.b-two.a-one becomes\n"
      "                a-one/b/b-two/c-three/d-four.c-three.b-two.a-one\n"
      "        c-three.b-two.a-one becomes\n"
      "                a-one/b/b-two/c-three.b-two.a-one\n"
      "        b-two.a-one becomes\n"
      "                a-one/b/b-two/b-two.a-one\n"
      "        a-one becomes\n"
      "                a-one/a/a-one\n"
      "In essence, the last three sections of the name are reversed and become \n"
      "directory names.  If there are more than three sections, no further directories\n"
      "are created.  Also, the first letter of the next-to-last section is used as\n"
      "a directory name.  The FQDN is always used as the filename.\n"
      "\n"
      "The -d flag makes domain2path only return the directory portion of the path,\n"
      "without the filename.  This is useful in scripts that must create the directory\n"
      "before creating the file.\n"
      );
  else
    {
    j = 1;

    if (strcmp(argv[1], "-d") == 0)
      {
      directory_only = 1;
      j = 2;
      }

    strlen_argv = strlen(argv[j]);
    for (i = 0; i < strlen_argv; i++)
      argv[j][i] = tolower((int)argv[j][i]);

    domain_delimiter[0] = NULL;
    domain_delimiter[1] = NULL;
    domain_delimiter[2] = NULL;

    for (i = strlen_argv - 1; i >= 0; i--)
      if (argv[j][i] == '.')
        {
        if (domain_delimiter[0] == NULL)
          domain_delimiter[0] = argv[j] + i + 1;
        else if (domain_delimiter[1] == NULL)
          domain_delimiter[1] = argv[j] + i + 1;
        else if (domain_delimiter[2] == NULL)
          {
          domain_delimiter[2] = argv[j] + i + 1;
          break;
          }
        }

    if (domain_delimiter[2] != NULL)
      if (directory_only)
        printf("%.*s/%c/%.*s/%.*s", (int)(strlen_argv - (domain_delimiter[0] - argv[j])), domain_delimiter[0], domain_delimiter[1][0], (int)((domain_delimiter[0] - domain_delimiter[1]) - 1), domain_delimiter[1], (int)((domain_delimiter[1] - domain_delimiter[2]) - 1), domain_delimiter[2]);
      else
        printf("%.*s/%c/%.*s/%.*s/%.*s", (int)(strlen_argv - (domain_delimiter[0] - argv[j])), domain_delimiter[0], domain_delimiter[1][0], (int)((domain_delimiter[0] - domain_delimiter[1]) - 1), domain_delimiter[1], (int)((domain_delimiter[1] - domain_delimiter[2]) - 1), domain_delimiter[2], strlen_argv, argv[j]);
    else if (domain_delimiter[1] != NULL)
      if (directory_only)
        printf("%.*s/%c/%.*s", (int)(strlen_argv - (domain_delimiter[0] - argv[j])), domain_delimiter[0], domain_delimiter[1][0], (int)((domain_delimiter[0] - domain_delimiter[1]) - 1), domain_delimiter[1]);
      else
        printf("%.*s/%c/%.*s/%.*s", (int)(strlen_argv - (domain_delimiter[0] - argv[j])), domain_delimiter[0], domain_delimiter[1][0], (int)((domain_delimiter[0] - domain_delimiter[1]) - 1), domain_delimiter[1], strlen_argv, argv[j]);
    else if (domain_delimiter[0] != NULL)
      if (directory_only)
        printf("%.*s/%c/%.*s", (int)(strlen_argv - (domain_delimiter[0] - argv[j])), domain_delimiter[0], argv[j][0], (int)((domain_delimiter[0] - argv[j]) - 1), argv[j]);
      else
        printf("%.*s/%c/%.*s/%.*s", (int)(strlen_argv - (domain_delimiter[0] - argv[j])), domain_delimiter[0], argv[j][0], (int)((domain_delimiter[0] - argv[j]) - 1), argv[j], strlen_argv, argv[j]);
    else
      if (directory_only)
        printf("%.*s/%c", strlen_argv, argv[j], argv[j][0]);
      else
        printf("%.*s/%c/%.*s", strlen_argv, argv[j], argv[j][0], strlen_argv, argv[j]);
    }

  return(0);
  }
