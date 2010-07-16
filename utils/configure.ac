# Process this file with autoconf to produce a configure script.
################################################################################
# spamdyke -- a filter for stopping spam at connection time.
# Copyright (C) 2010 Sam Clippinger (samc (at) silence (dot) org)
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License version 2 as
# published by the Free Software Foundation.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

AC_PREREQ(2.59)
AC_INIT([[spamdyke]], [[4.1.0]], [[samc (at) silence (dot) org]])
AC_CONFIG_SRCDIR([dnsa.c])
AC_CONFIG_HEADER([config.h])

# Checks for programs.
AC_PROG_CC([[gcc]])

AC_CHECK_HEADERS([stdint.h])
AC_CHECK_HEADERS([sys/inttypes.h])
AC_HEADER_TIME

AC_ARG_WITH([debug],
            [AS_HELP_STRING([--with-debug], [include debugging symbols (for gdb) @<:@default=no@:>@]) ],
            [ AS_IF([ test "x$with_debug" != "xno" ],
                    [ CFLAGS=-g ],
                    [ AC_SUBST([STRIP_CMD], ["strip spamdyke"])
                      CFLAGS=-O2
                    ])
            ],
            [ AC_SUBST([STRIP_CMD], ["strip spamdyke"])
              CFLAGS=-O2
            ])

# Checks for libraries.
AC_CHECK_LIB([resolv], [res_search], [], [ AC_CHECK_LIB([resolv], [__res_search]) ])
AC_CHECK_LIB([socket], [bind], [], [ AC_CHECK_LIB([socket], [__bind]) ])
AC_CHECK_LIB([nsl], [inet_aton], [], [ AC_CHECK_LIB([nsl], [__inet_aton]) ])

AC_MSG_CHECKING([whether nameserver constants are defined in arpa/nameser.h])
AC_TRY_COMPILE( AC_INCLUDES_DEFAULT
               [ #include <arpa/nameser.h> ],
               [ int i = PACKETSZ; ],
               [ AC_MSG_RESULT([yes]) ],
               [ AC_MSG_RESULT([no])
                 AC_MSG_CHECKING([whether nameserver constants are defined in arpa/nameser_compat.h])
                 AC_TRY_COMPILE( AC_INCLUDES_DEFAULT
                                [ #include <arpa/nameser.h>
                                  #include <arpa/nameser_compat.h> ],
                                [ int i = PACKETSZ; ],
                                [ AC_MSG_RESULT([yes])
                                  AC_DEFINE([HAVE_NAMESER_COMPAT], [1]) ],
                                [ AC_MSG_RESULT([no])
                                  AC_MSG_FAILURE([Unable to compile without nameserver constants.])
                                ])
               ])

AC_CONFIG_FILES([Makefile])
AC_OUTPUT
