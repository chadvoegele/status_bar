/*
* Copyright 2015 Chad Voegele.
* See LICENSE file for license details.
*/

#pragma once

#include "status_bar.h"

struct thinkpad_battery_monitor {
  GString* bar_text;
  GMutex* mutex;
  GString* alert_fgcolor;
  GString* alert_bgcolor;

  GString* str;
};

struct monitor_fns thinkpad_battery_monitor_fns();
void* thinkpad_battery_init(GString*, GMutex*, GKeyFile*);
gboolean thinkpad_battery_update_text(void*);
int thinkpad_battery_sleep_time(void*);
void thinkpad_battery_free(void*);
