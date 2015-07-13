/*
* Copyright 2015 Chad Voegele.
* See LICENSE file for license details.
*/

#pragma once

#include "status_bar.h"

struct thinkpad_temp_monitor {
  GString* bar_text;
  GMutex* mutex;

  GString* str;
};

struct monitor_fns thinkpad_temp_monitor_fns();
void* thinkpad_temp_init(GString*, GMutex*, GKeyFile*);
gboolean thinkpad_temp_update_text(void*);
int thinkpad_temp_sleep_time(void*);
void thinkpad_temp_free(void*);
