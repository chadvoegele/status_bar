/*
* Copyright 2015 Chad Voegele.
* See LICENSE file for license details.
*/

#pragma once

#include "status_bar.h"

struct thinkpad_battery_monitor {
  GString* str;
};

void* thinkpad_battery_monitor(struct monitor_refs*);
void thinkpad_battery_init(void*, void*);
const char* thinkpad_battery_update_text(void*);
int thinkpad_battery_sleep_time(void*);
void thinkpad_battery_close(void*);
