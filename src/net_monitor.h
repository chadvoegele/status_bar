/*
* Copyright 2015-2017 Chad Voegele.
* See LICENSE file for license details.
*/

#pragma once

#include "status_bar.h"

struct net_monitor {
  GString* bar_text;
  GMutex* mutex;

  GArray* rx;
  GArray* tx;
  int last_rx;
  int last_tx;
  GString* str;
};

struct monitor_fns net_monitor_fns();
void* net_init(GString*, GMutex*, GKeyFile*);
gboolean net_update_text(void*);
int net_sleep_time(void*);
void net_free(void*);

void find_interfaces(GArray*, GArray*);
int total_bytes(GArray*);
