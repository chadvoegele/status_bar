/*
* Copyright 2015-2017 Chad Voegele.
* See LICENSE file for license details.
*/

#pragma once

#include "status_bar.h"
#include "base_monitor.h"

struct net_monitor {
  struct base_monitor* base;

  GArray* rx;
  GArray* tx;
  int last_rx;
  int last_tx;
  GString* str;
};

void* net_init(GKeyFile*);
gboolean net_update_text(void*);
int net_sleep_time(void*);
void net_free(void*);

void find_interfaces(GArray*, GArray*);
int total_bytes(GArray*);
