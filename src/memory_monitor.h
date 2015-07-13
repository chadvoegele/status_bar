/*
* Copyright 2015 Chad Voegele.
* See LICENSE file for license details.
*/

#pragma once

#include <glib.h>

#include "status_bar.h"

struct memory_monitor {
  GString* bar_text;
  GMutex* mutex;

  GString* str;
};

struct monitor_fns memory_monitor_fns();
void* memory_init(GString*, GMutex*, GKeyFile*);
gboolean memory_update_text(void*);
int memory_sleep_time(void*);
void memory_free(void*);
