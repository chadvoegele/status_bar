/*
* Copyright 2015 Chad Voegele.
* See LICENSE file for license details.
*/

#pragma once

#include <glib.h>

#include "status_bar.h"

struct memory_monitor {
  GString* str;
};

void* memory_monitor(struct monitor_refs*);
void memory_init(void*, void*);
const char* memory_update_text(void*);
int memory_sleep_time(void*);
void memory_close(void*);
