/*
* Copyright 2015-2017 Chad Voegele.
* See LICENSE file for license details.
*/

#pragma once

#include <glib.h>

#include "status_bar.h"
#include "base_monitor.h"

struct text_monitor {
  struct base_monitor* base;

  GString* str;
};

void* text_init(GArray*);
gboolean text_update_text(void*);
int text_sleep_time(void*);
void text_free(void*);
