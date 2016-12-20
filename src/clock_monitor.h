/*
* Copyright 2015-2017 Chad Voegele.
* See LICENSE file for license details.
*/

#pragma once

#include <glib.h>

#include "status_bar.h"

struct clock_monitor {
  GString* bar_text;
  GMutex* mutex;

  gboolean colon_on;
  char* str;
};

struct monitor_fns clock_monitor_fns();
void* clock_init(GString*, GMutex*, GKeyFile*);
gboolean clock_update_text(void*);
int clock_sleep_time(void*);
void clock_free(void*);
