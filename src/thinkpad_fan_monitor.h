/*
* Copyright 2015 Chad Voegele.
* See LICENSE file for license details.
*/

#pragma once

#include "status_bar.h"

struct thinkpad_fan_monitor {
  GString* bar_text;
  GMutex* mutex;

  GString* str;
};

struct monitor_fns thinkpad_fan_monitor_fns();
void* thinkpad_fan_init(GString*, GMutex*, GKeyFile*);
gboolean thinkpad_fan_update_text(void*);
int thinkpad_fan_sleep_time(void*);
void thinkpad_fan_free(void*);
