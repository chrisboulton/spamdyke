/*
  smtpauth_crammd5 - a program for generated encoded usernames and passwords
  for the CRAM-MD5 algorithm of the SMTP AUTH protocol.
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
#include <stdint.h>
#include "base64.h"
#include "md5.h"

#define MAX_BUF         4095
#define IPAD_BYTE       0x36
#define OPAD_BYTE       0x5C

int main(int argc, char *argv[])
  {
  unsigned char challenge[MAX_BUF + 1];
  int strlen_challenge;
  unsigned char ipad[MAX_BUF + 1];
  unsigned char opad[MAX_BUF + 1];
  unsigned char result[16];
  unsigned char prepend[MAX_BUF + 1];
  int strlen_prepend;
  unsigned char secret[64];
  unsigned char final[MAX_BUF + 1];
  int i;

  if (argc == 4)
    {
    for (i = 0; i < 64; i++)
      {
      ipad[i] = IPAD_BYTE;
      opad[i] = OPAD_BYTE;
      }

    if (strlen(argv[2]) > 64)
      {
      md5(secret, (unsigned char *)argv[2], strlen(argv[2]));
      for (i = 16; i < 64; i++)
        secret[i] = '\0';
      }
    else
      {
      strncpy((char *)secret, argv[2], strlen(argv[2]));
      for (i = strlen(argv[2]); i < 64; i++)
        secret[i] = '\0';
      }

    for (i = 0; i < 64; i++)
      {
      ipad[i] ^= secret[i];
      opad[i] ^= secret[i];
      }

    strlen_challenge = base64_decode(challenge, MAX_BUF, (unsigned char *)argv[3], strlen(argv[3]));

    for (i = 0; i < strlen_challenge; i++)
      ipad[i + 64] = challenge[i];

    md5(opad + 64, ipad, strlen_challenge + 64);
    md5(result, opad, 80);

    strlen_prepend = snprintf((char *)prepend, MAX_BUF, "%s ", argv[1]);
    for (i = 0; i < 16; i++)
      snprintf((char *)(prepend + strlen_prepend + (i * 2)), MAX_BUF, "%.2x", result[i]);
    strlen_prepend += 32;

    base64_encode(final, MAX_BUF, prepend, strlen_prepend);

    printf("C: AUTH CRAM-MD5\nS: 334 %s\nC: %s\n", argv[3], final);
    }
  else
    printf("USAGE: smtpauth_crammd5 username password encoded_challenge\n");

  return(0);
  }
