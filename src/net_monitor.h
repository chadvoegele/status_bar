/*
* Copyright 2015 Chad Voegele.
* See LICENSE file for license details.
*/

#pragma once

#include "status_bar.h"

struct net_monitor {
  GArray* rx;
  GArray* tx;
  int last_rx;
  int last_tx;
  GString* str;
};

void* net_monitor(struct monitor_refs*);
void net_init(void*, void*);
const char* net_update_text(void*);
int net_sleep_time(void*);
void net_close(void*);

void find_interfaces(GArray*, GArray*);
int total_bytes(GArray*);
