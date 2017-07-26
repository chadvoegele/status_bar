/*
* Copyright 2015-2017 Chad Voegele.
* See LICENSE file for license details.
*/

#pragma once

#include "status_bar.h"
#include "base_monitor.h"

typedef unsigned long long bytes_t;

struct net_monitor {
  struct base_monitor* base;

  GString* tx_icon;
  GString* rx_icon;

  GArray* rx;
  GArray* tx;
  bytes_t last_rx;
  bytes_t last_tx;
  GString* str;
};

void* net_init(GArray*);
gboolean net_update_text(void*);
int net_sleep_time(void*);
void net_free(void*);

void find_interfaces(GArray*, GArray*);
bytes_t total_bytes(GArray*);
void bytes_to_human_readable(bytes_t, char*, bytes_t*);
