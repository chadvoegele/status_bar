/*
* Copyright 2015-2021 Chad Voegele.
* See LICENSE file for license details.
*/

#pragma once

#include <glib.h>

#include "status_bar.h"
#include "base_monitor.h"

struct clock_monitor {
  struct base_monitor* base;

  gboolean colon_on;
  char* str;
};

void* clock_init(GArray*);
gboolean clock_update_text(void*);
int clock_sleep_time(void*);
void clock_free(void*);
