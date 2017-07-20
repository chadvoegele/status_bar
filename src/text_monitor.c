/*
* Copyright 2015-2017 Chad Voegele.
* See LICENSE file for license details.
*/

#include <glib.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "text_monitor.h"
#include "status_bar.h"
#include "base_monitor.h"

void* text_init(GKeyFile* configs) {
  struct text_monitor* m = malloc(sizeof(struct text_monitor));

  m->base = base_monitor_init(text_sleep_time, text_update_text, text_free);

  m->str = g_string_new(NULL);

  return m;
}

gboolean text_update_text(void* ptr) {
  struct text_monitor* m = (struct text_monitor*)ptr;
  monitor_null_check(m, "text_monitor", "update");

  g_string_printf(m->str, "%%{r}");

  g_mutex_lock(m->base->mutex);
  m->base->text = g_string_assign(m->base->text, m->str->str);
  g_mutex_unlock(m->base->mutex);

  return TRUE;
}

int text_sleep_time(void* ptr) {
  return 36000;
}

void text_free(void* ptr) {
  struct text_monitor* m = (struct text_monitor*)ptr;
  monitor_null_check(m, "text_monitor", "free");

  base_monitor_free(m->base);

  g_string_free(m->str, TRUE);
  free(m);
}
