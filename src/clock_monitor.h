/*
* Copyright 2015 Chad Voegele.
* See LICENSE file for license details.
*/

#pragma once

#include <glib.h>

#include "status_bar.h"

struct clock_monitor {
  gboolean colon_on;
  char* str;
};

void* clock_monitor(struct monitor_refs*);
void clock_init(void*, void*);
const char* clock_update_text(void*);
int clock_sleep_time(void*);
void clock_close(void*);
