/*
  addsecs -- a simple program for printing the date as of X seconds ago
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
#include <time.h>

int main(int argc, char *argv[])
  {
  time_t now;
  struct tm *then;
  int secsago;

  if ((argc == 2) &&
      (sscanf(argv[1], "%d", &secsago) == 1))
    {
    now = time(NULL) + secsago;
    then = localtime(&now);

    printf("%d%.2d%.2d%.2d%.2d.%.2d\n", then->tm_year + 1900, then->tm_mon + 1, then->tm_mday, then->tm_hour, then->tm_min, then->tm_sec);
    }
  else
    printf("USAGE: addsecs SECS_AGO\n");

  return(0);
  }
