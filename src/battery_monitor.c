/*
* Copyright 2015 Chad Voegele.
* See LICENSE file for license details.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <glib.h>

#include "status_bar.h"
#include "configs.h"
#include "battery_monitor.h"

struct monitor_fns battery_monitor_fns() {
  struct monitor_fns f;
  f.init = battery_init;
  f.sleep_time = battery_sleep_time;
  f.update_text = battery_update_text;
  f.free = battery_free;

  return f;
}

void* battery_init(GString* bar_text, GMutex* mutex, GKeyFile* configs) {
  struct battery_monitor* m = malloc(sizeof(struct battery_monitor));

  m->bar_text = bar_text;
  m->mutex = mutex;

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
  struct battery_monitor* m;
  if ((m = (struct battery_monitor*)ptr) != NULL) {
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

    g_mutex_lock(m->mutex);
    m->bar_text = g_string_assign(m->bar_text, m->str->str);
    g_mutex_unlock(m->mutex);

    return TRUE;

  } else {
    fprintf(stderr, "battery monitor not received in update.\n");
    exit(EXIT_FAILURE);
  }
}

int battery_sleep_time(void* ptr) {
  struct battery_monitor* m;
  if ((m = (struct battery_monitor*)ptr) != NULL) {
    return 300;
  } else {
    fprintf(stderr, "battery monitor not received in sleep_time.\n");
    exit(EXIT_FAILURE);
  }
}

void battery_free(void* ptr) {
  struct battery_monitor* m;
  if ((m = (struct battery_monitor*)ptr) != NULL) {
    g_string_free(m->str, TRUE);
    g_string_free(m->alert_fgcolor, TRUE);
    g_string_free(m->alert_bgcolor, TRUE);
    free(m);

  } else {
    fprintf(stderr, "battery monitor not received in close.\n");
    exit(EXIT_FAILURE);
  }
}
