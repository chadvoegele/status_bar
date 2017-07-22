/*
* Copyright 2015-2017 Chad Voegele.
* See LICENSE file for license details.
*/

#pragma once

#include "status_bar.h"
#include "base_monitor.h"

struct thinkpad_fan_monitor {
  struct base_monitor* base;

  GString* icon;

  GString* str;
};

void* thinkpad_fan_init(GArray*);
gboolean thinkpad_fan_update_text(void*);
int thinkpad_fan_sleep_time(void*);
void thinkpad_fan_free(void*);
