/*
* Copyright 2015-2017 Chad Voegele.
* See LICENSE file for license details.
*/

#pragma once

#include "base_monitor.h"

struct cpu_usage_monitor {
  struct base_monitor* base;

  GString* icon;
  GString* str;
  unsigned int last_total;
  unsigned int last_idle;
};

void* cpu_usage_init(GArray*);
gboolean cpu_usage_update_text(void*);
int cpu_usage_sleep_time(void*);
void cpu_usage_free(void*);

unsigned int cpu_usage_calc_total(unsigned int* times);
unsigned int cpu_usage_calc_idle(unsigned int* times);
