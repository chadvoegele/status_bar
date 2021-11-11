/*
* Copyright 2015-2021 Chad Voegele.
* See LICENSE file for license details.
*/

#pragma once

#include "status_bar.h"
#include "base_monitor.h"

struct thinkpad_temp_monitor {
  struct base_monitor* base;

  GString* icon;

  GString* str;
};

void* thinkpad_temp_init(GArray*);
gboolean thinkpad_temp_update_text(void*);
int thinkpad_temp_sleep_time(void*);
void thinkpad_temp_free(void*);
