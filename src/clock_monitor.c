/*
* Copyright 2015 Chad Voegele.
* See LICENSE file for license details.
*/

#include <glib.h>
#include <stdlib.h>
#include <time.h>

#include "clock_monitor.h"
#include "status_bar.h"

#define MAX_TEXT_LENGTH 100

struct monitor_fns clock_monitor_fns() {
  struct monitor_fns f;
  f.init = clock_init;
  f.sleep_time = clock_sleep_time;
  f.update_text = clock_update_text;
  f.free = clock_free;

  return f;
}

void* clock_init(GString* bar_text, GMutex* mutex, GKeyFile* configs) {
  struct clock_monitor* m = malloc(sizeof(struct clock_monitor));

  m->bar_text = bar_text;
  m->mutex = mutex;

  m->colon_on = TRUE;
  m->str = malloc(MAX_TEXT_LENGTH*sizeof(char));

  return m;
}

gboolean clock_update_text(void* ptr) {
  struct clock_monitor* m;
  if ((m = (struct clock_monitor*)ptr) == NULL) {
    fprintf(stderr, "clock monitor not received in update.\n");
    exit(EXIT_FAILURE);
  }

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

  g_mutex_lock(m->mutex);
  m->bar_text = g_string_assign(m->bar_text, m->str);
  g_mutex_unlock(m->mutex);

  return TRUE;
}

int clock_sleep_time(void* ptr) {
  struct clock_monitor* m;
  if ((m = (struct clock_monitor*)ptr) == NULL) {
    fprintf(stderr, "clock monitor not received in sleep_time.\n");
    exit(EXIT_FAILURE);
  }

  return 5;
}

void clock_free(void* ptr) {
  struct clock_monitor* m;
  if ((m = (struct clock_monitor*)ptr) == NULL) {
    fprintf(stderr, "clock monitor not received in close.\n");
    exit(EXIT_FAILURE);
  }

  free(m->str);
  free(m);
}
