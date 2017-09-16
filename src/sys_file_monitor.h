/*
* Copyright 2015-2017 Chad Voegele.
* See LICENSE file for license details.
*/

#pragma once

#include "status_bar.h"
#include "base_monitor.h"

struct sys_file_monitor {
  struct base_monitor* base;

  GString* icon;
  GArray* filenames;
  float multiplier;
  GString* str;
};

void* sys_file_init(GArray*);
gboolean sys_file_update_text(void*);
int sys_file_sleep_time(void*);
void sys_file_free(void*);
