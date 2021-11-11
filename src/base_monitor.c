/*
* Copyright 2015-2021 Chad Voegele.
* See LICENSE file for license details.
*/

#include <stdlib.h>
#include "base_monitor.h"

struct base_monitor* base_monitor_init(int (*sleep_time)(void*), gboolean (*update_text)(void*), void (*free)(void*)) {
  struct base_monitor* m = malloc(sizeof(struct base_monitor));

  m->sleep_time = sleep_time;
  m->update_text = update_text;
  m->free = free;
  m->text = g_string_new(NULL);

  return m;
}

void base_monitor_free(struct base_monitor* m) {
  g_string_free(m->text, TRUE);
  free(m);
}
