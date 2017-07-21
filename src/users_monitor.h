/*
* Copyright 2015-2017 Chad Voegele.
* See LICENSE file for license details.
*/

#pragma once

#include "status_bar.h"
#include "base_monitor.h"

struct users_monitor {
  struct base_monitor* base;

  GString* str;
};

void* users_init(GKeyFile*);
gboolean users_update_text(void*);
int users_sleep_time(void*);
void users_free(void*);