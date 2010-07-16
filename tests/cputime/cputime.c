/*
  cputime -- a program for tracking and printing the CPU usage of a process
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
#include <sys/times.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <errno.h>

#ifdef __linux__
#undef __USE_XOPEN2K
#endif

#include <time.h>

#ifndef CLK_TCK
#ifdef __APPLE__
#include <machine/limits.h>
#endif
#endif

int main(int argc, char *argv[])
  {
  int return_value;
  int wait_status;
  struct tms tmp_tms;

  return_value = -1;

  if (argc >= 2)
    {
    if (fork() == 0)
      {
      if (execvp(argv[1], argv + 1) == -1)
        fprintf(stderr, "ERROR: %s", strerror(errno));

      exit(0);
      }
    else
      {
      wait(&wait_status);
      return_value = WEXITSTATUS(wait_status);
      }

    times(&tmp_tms);
    fprintf(stderr, "\n\ncpu: %lf\nsys: %lf\n", (double)tmp_tms.tms_cutime / CLK_TCK, (double)tmp_tms.tms_cstime / CLK_TCK);
    }
  else
    fprintf(stderr, "USAGE: cputime PATH\n");

  return(return_value);
  }
