/*
* Copyright 2015-2017 Chad Voegele.
* See LICENSE file for license details.
*/

#pragma once

#include "status_bar.h"

struct battery_monitor {
  GString* bar_text;
  GMutex* mutex;
  GString* alert_fgcolor;
  GString* alert_bgcolor;

  GString* str;
  GString* battery_full_path_str;
  GString* battery_now_path_str;
};

struct monitor_fns battery_monitor_fns();
void* battery_init(GString*, GMutex*, GKeyFile*);
gboolean battery_update_text(void*);
int battery_sleep_time(void*);
void battery_free(void*);
