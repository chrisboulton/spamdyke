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
#ifndef DNS_H
#define DNS_H

#include "spamdyke.h"

#define NIHDNS_CLASS_INTERNET           1

int nihdns_initialize(struct filter_settings *current_settings, int close_socket);
int nihdns_rbl(struct filter_settings *current_settings, char **target_name_array, char *target_message_format, char *target_message_buf, int size_target_message_buf, char **target_rbl, int *return_target_name_index, struct previous_action *history);
int nihdns_ptr(struct filter_settings *current_settings, char *target_ip);
int nihdns_mx(struct filter_settings *current_settings, char *target_name, struct previous_action *history);
int nihdns_a(struct filter_settings *current_settings, char *target_name, int *return_octets, struct previous_action *history, int disqualify_localhost);

#endif /* DNS_H */
