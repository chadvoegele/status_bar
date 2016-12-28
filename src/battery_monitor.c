/*
* Copyright 2015-2017 Chad Voegele.
* See LICENSE file for license details.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <glib.h>

#include "status_bar.h"
#include "base_monitor.h"
#include "configs.h"
#include "battery_monitor.h"

void* battery_init(GKeyFile* configs) {
  struct battery_monitor* m = malloc(sizeof(struct battery_monitor));

  m->base = base_monitor_init(battery_sleep_time, battery_update_text, battery_free);

  GError* error = NULL;
  char* alert_fgcolor = g_key_file_get_string(
      configs, "configs", "alert_fgcolor", &error);
  fail_on_error(error);
  m->alert_fgcolor = g_string_new(alert_fgcolor);
  g_free(alert_fgcolor);

  error = NULL;
  char* alert_bgcolor = g_key_file_get_string(
      configs, "configs", "alert_bgcolor", &error);
  fail_on_error(error);
  m->alert_bgcolor = g_string_new(alert_bgcolor);
  g_free(alert_bgcolor);

  m->str = g_string_new(NULL);

  error = NULL;
  char* battery_full_path = g_key_file_get_string(
      configs, "configs", "battery_full_path", &error);
  fail_on_error(error);

  m->battery_full_path_str = g_string_new(NULL);
  g_string_printf(m->battery_full_path_str, "%s", battery_full_path);
  g_free(battery_full_path);

  error = NULL;
  char* battery_now_path = g_key_file_get_string(
      configs, "configs", "battery_now_path", &error);
  fail_on_error(error);

  m->battery_now_path_str = g_string_new(NULL);
  g_string_printf(m->battery_now_path_str, "%s", battery_now_path);
  g_free(battery_now_path);

  return m;
}

gboolean battery_update_text(void* ptr) {
  struct battery_monitor* m = (struct battery_monitor*)ptr;
  monitor_null_check(m, "battery_monitor", "update");

  int full, now;
  int n_full = 0;
  int n_now = 0;

  FILE* battery_full_file;
  char* battery_full_file_path = m->battery_full_path_str->str;
  battery_full_file = fopen(battery_full_file_path, "r");
  if (battery_full_file == NULL) {
    fprintf(stderr, "Can't open battery file %s\n", battery_full_file_path);
  }

  FILE* battery_now_file;
  char* battery_now_file_path = m->battery_now_path_str->str;
  battery_now_file = fopen(battery_now_file_path, "r");
  if (battery_now_file == NULL) {
    fprintf(stderr, "Can't open battery file %s!\n", battery_now_file_path);
  }

  if (battery_full_file != NULL && battery_now_file != NULL) {
    n_full = fscanf(battery_full_file, "%d", &full);
    n_now = fscanf(battery_now_file, "%d", &now);
  }

  if (n_full == 1 && n_now == 1) {
    int battpct = (int)(100.0*now/full);
    if (battpct <= 10) {
      g_string_printf(m->str, "%%{B%s}%%{F%s}%d%%%%{B-}%%{F-}",
          m->alert_bgcolor->str, m->alert_fgcolor->str, battpct);
    } else {
      g_string_printf(m->str, "%d%%", battpct);
    }
  } else {
    g_string_printf(m->str, "!");
  }

  if (battery_now_file != NULL)
    fclose(battery_now_file);

  if (battery_full_file != NULL)
    fclose(battery_full_file);

  g_mutex_lock(m->base->mutex);
  m->base->text = g_string_assign(m->base->text, m->str->str);
  g_mutex_unlock(m->base->mutex);

  return TRUE;
}

int battery_sleep_time(void* ptr) {
  return 300;
}

void battery_free(void* ptr) {
  struct battery_monitor* m = (struct battery_monitor*)ptr;
  monitor_null_check(m, "battery_monitor", "free");

  g_string_free(m->str, TRUE);
  g_string_free(m->alert_fgcolor, TRUE);
  g_string_free(m->alert_bgcolor, TRUE);

  base_monitor_free(m->base);

  free(m);
}
