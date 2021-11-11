/*
* Copyright 2015-2021 Chad Voegele.
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
  monitor_arg_check("user", arguments, "(icon)");

  struct users_monitor* m = malloc(sizeof(struct users_monitor));

  m->base = base_monitor_init(users_sleep_time, users_update_text, users_free);

  char* icon = g_array_index(arguments, GString*, 0)->str;
  m->icon = g_string_new(icon);

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

  g_string_printf(m->str, "%s%d", m->icon->str, nusers);

  m->base->text = g_string_assign(m->base->text, m->str->str);

  return TRUE;
}

int users_sleep_time(void* ptr) {
  return 2;
}

void users_free(void* ptr) {
  struct users_monitor* m = (struct users_monitor*)ptr;
  monitor_null_check(m, "users_monitor", "free");

  g_string_free(m->str, TRUE);

  g_string_free(m->icon, TRUE);

  base_monitor_free(m->base);

  free(m);
}
