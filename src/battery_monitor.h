/*
* Copyright 2015-2017 Chad Voegele.
* See LICENSE file for license details.
*/

#pragma once

#include <glib.h>
#include "base_monitor.h"

struct battery_monitor {
  struct base_monitor* base;

  GString* alert_fgcolor;
  GString* alert_bgcolor;

  GString* str;
  GString* battery_full_path_str;
  GString* battery_now_path_str;
};

void* battery_init(GKeyFile*);
gboolean battery_update_text(void*);
int battery_sleep_time(void*);
void battery_free(void*);
