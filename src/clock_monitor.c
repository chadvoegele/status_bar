/*
* Copyright 2015-2017 Chad Voegele.
* See LICENSE file for license details.
*/

#include <glib.h>
#include <stdlib.h>
#include <time.h>

#include "clock_monitor.h"
#include "status_bar.h"
#include "base_monitor.h"

#define MAX_TEXT_LENGTH 100

void* clock_init(GArray* arguments) {
  monitor_arg_check("clock", arguments, "()");

  struct clock_monitor* m = malloc(sizeof(struct clock_monitor));

  m->base = base_monitor_init(clock_sleep_time, clock_update_text, clock_free);

  m->colon_on = TRUE;
  m->str = malloc(MAX_TEXT_LENGTH*sizeof(char));

  return m;
}

gboolean clock_update_text(void* ptr) {
  struct clock_monitor* m = (struct clock_monitor*)ptr;
  monitor_null_check(m, "clock_monitor", "update");

  time_t rawtime;
  struct tm timeinfo;
  time(&rawtime);
  localtime_r(&rawtime, &timeinfo);

  char* format_str;
  if (m->colon_on) {
    format_str = "%B%e,%l:%M";
  } else {
    format_str = "%B%e,%l %M";
  }
  m->colon_on = !m->colon_on;
  strftime(m->str, MAX_TEXT_LENGTH, format_str, &timeinfo);

  g_mutex_lock(m->base->mutex);
  m->base->text = g_string_assign(m->base->text, m->str);
  g_mutex_unlock(m->base->mutex);

  return TRUE;
}

int clock_sleep_time(void* ptr) {
  return 5;
}

void clock_free(void* ptr) {
  struct clock_monitor* m = (struct clock_monitor*)ptr;
  monitor_null_check(m, "clock_monitor", "free");

  free(m->str);

  base_monitor_free(m->base);

  free(m);
}
