/*
* Copyright 2015-2017 Chad Voegele.
* See LICENSE file for license details.
*/

#pragma once

#include "status_bar.h"

struct cpu_usage_monitor {
  GString* bar_text;
  GMutex* mutex;

  GString* str;
  unsigned int last_total;
  unsigned int last_idle;
};

struct monitor_fns cpu_usage_monitor_fns();
void* cpu_usage_init(GString*, GMutex*, GKeyFile*);
gboolean cpu_usage_update_text(void*);
int cpu_usage_sleep_time(void*);
void cpu_usage_free(void*);

unsigned int cpu_usage_calc_total(unsigned int* times);
unsigned int cpu_usage_calc_idle(unsigned int* times);
