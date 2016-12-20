/*
* Copyright 2015-2017 Chad Voegele.
* See LICENSE file for license details.
*/

#pragma once

#include "status_bar.h"

struct sys_file_monitor {
  GString* bar_text;
  GMutex* mutex;

  gunichar icon;
  GArray* temp_filenames;
  int(*convert)(int);
  GString* str;
};

gboolean sys_file_update_text(void*);
int sys_file_sleep_time(void*);
void sys_file_free(void*);

void* sys_file_init_config(gunichar, GArray*, int(*convert)(int), GString*,
    GMutex*, GKeyFile*);
void append_filename(GArray*, char*);
