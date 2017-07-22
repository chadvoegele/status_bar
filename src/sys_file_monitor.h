/*
* Copyright 2015-2017 Chad Voegele.
* See LICENSE file for license details.
*/

#pragma once

#include "status_bar.h"
#include "base_monitor.h"

struct sys_file_monitor {
  struct base_monitor* base;

  GArray* temp_filenames;
  int(*convert)(int);
  GString* str;
};

gboolean sys_file_update_text(void*);
int sys_file_sleep_time(void*);
void sys_file_free(void*);

void* sys_file_init_config(GArray*, int(*convert)(int), GArray*);
void append_filename(GArray*, char*);
