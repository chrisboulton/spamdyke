/*
  exitvalue - a program for generating exit codes.
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

int main(int argc, char *argv[])
  {
  int return_value;
  int tmp_int;

  return_value = 0;

  if ((argc == 2) &&
      (sscanf(argv[1], "%d", &tmp_int) == 1) &&
      (tmp_int >= 0) &&
      (tmp_int <= 255))
    return_value = tmp_int;
  else
    printf("USAGE: exitvalue code\n");

  return(return_value);
  }
