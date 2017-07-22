/*
* Copyright 2015-2017 Chad Voegele.
* See LICENSE file for license details.
*/

#include <glib.h>
#include <stdlib.h>
#include <string.h>
#include <utmp.h>

#include "status_bar.h"
#include "users_monitor.h"

typedef struct utmp utmp_t;

void* users_init(GArray* arguments) {
  struct users_monitor* m = malloc(sizeof(struct users_monitor));

  m->base = base_monitor_init(users_sleep_time, users_update_text, users_free);

  m->str = g_string_new(NULL);

  return m;
}

gboolean users_update_text(void* ptr) {
  struct users_monitor* m = (struct users_monitor*)ptr;
  monitor_null_check(m, "users_monitor", "update");

  utmp_t* utent;
  int nusers = 0;

  setutent();
  while ((utent = getutent())) {
    if (utent->ut_type == USER_PROCESS) {
      nusers = nusers + 1;
    }
  }
  endutent();

  g_string_printf(m->str, "%d", nusers);

  g_mutex_lock(m->base->mutex);
  m->base->text = g_string_assign(m->base->text, m->str->str);
  g_mutex_unlock(m->base->mutex);

  return TRUE;
}

int users_sleep_time(void* ptr) {
  return 2;
}

void users_free(void* ptr) {
  struct users_monitor* m = (struct users_monitor*)ptr;
  monitor_null_check(m, "users_monitor", "free");

  g_string_free(m->str, TRUE);

  base_monitor_free(m->base);

  free(m);
}
