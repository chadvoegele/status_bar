/*
* Copyright 2015 Chad Voegele.
* See LICENSE file for license details.
*/

#pragma once

#include "status_bar.h"

struct thinkpad_temp_monitor {
  GString* str;
};

void* thinkpad_temp_monitor(struct monitor_refs*);
void thinkpad_temp_init(void*, void*);
const char* thinkpad_temp_update_text(void*);
int thinkpad_temp_sleep_time(void*);
void thinkpad_temp_close(void*);
