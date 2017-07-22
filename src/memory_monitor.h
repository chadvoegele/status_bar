/*
* Copyright 2015-2017 Chad Voegele.
* See LICENSE file for license details.
*/

#pragma once

#include <glib.h>

#include "status_bar.h"
#include "base_monitor.h"

struct memory_monitor {
  struct base_monitor* base;

  GString* icon;
  GString* str;
};

void* memory_init(GArray*);
gboolean memory_update_text(void*);
int memory_sleep_time(void*);
void memory_free(void*);
