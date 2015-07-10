/*
* Copyright 2015 Chad Voegele.
* See LICENSE file for license details.
*/

#pragma once

#include "status_bar.h"

struct thinkpad_fan_monitor {
  GString* str;
};

void* thinkpad_fan_monitor(struct monitor_refs*);
void thinkpad_fan_init(void*, void*);
const char* thinkpad_fan_update_text(void*);
int thinkpad_fan_sleep_time(void*);
void thinkpad_fan_close(void*);
