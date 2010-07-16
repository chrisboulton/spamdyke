/*
  smtpauth_plain - a program for generated encoded usernames and passwords
  for the PLAIN algorithm of the SMTP AUTH protocol.
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
#include "base64.h"

#define MAX_BUF         4095

int main(int argc, char *argv[])
  {
  char source[MAX_BUF + 1];
  int strlen_source;
  unsigned char destination[MAX_BUF + 1];

  if (argc == 3)
    {
    strlen_source = snprintf(source, MAX_BUF, "%c%s%c%s", '\0', argv[1], '\0', argv[2]);
    base64_encode(destination, MAX_BUF, (unsigned char *)source, strlen_source);
    printf("C: AUTH PLAIN\nS: 334\nC: %s\n", destination);
    }
  else
    printf("USAGE: smtpauth_plain username password\n");

  return(0);
  }
