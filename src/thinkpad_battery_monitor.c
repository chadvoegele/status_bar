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
#include "thinkpad_battery_monitor.h"

struct monitor_fns thinkpad_battery_monitor_fns() {
  struct monitor_fns f;
  f.init = thinkpad_battery_init;
  f.sleep_time = thinkpad_battery_sleep_time;
  f.update_text = thinkpad_battery_update_text;
  f.free = thinkpad_battery_free;

  return f;
}

void* thinkpad_battery_init(GString* bar_text, GMutex* mutex, GKeyFile* configs) {
  struct thinkpad_battery_monitor* m = malloc(sizeof(struct thinkpad_battery_monitor));

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

  return m;
}

gboolean thinkpad_battery_update_text(void* ptr) {
  struct thinkpad_battery_monitor* m;
  if ((m = (struct thinkpad_battery_monitor*)ptr) != NULL) {
    int full, now;
    int n_full = 0;
    int n_now = 0;

    FILE* battery_full_file;
    char* battery_full_file_path = "/sys/class/power_supply/BAT0/energy_full";
    battery_full_file = fopen(battery_full_file_path, "r");
    if (battery_full_file == NULL) {
      fprintf(stderr, "Can't open battery file %s\n", battery_full_file_path);
    }

    FILE* battery_now_file;
    char* battery_now_file_path = "/sys/class/power_supply/BAT0/energy_now";
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

    fclose(battery_now_file);
    fclose(battery_full_file);

    g_mutex_lock(m->mutex);
    m->bar_text = g_string_assign(m->bar_text, m->str->str);
    g_mutex_unlock(m->mutex);

    return TRUE;

  } else {
    fprintf(stderr, "thinkpad_battery monitor not received in update.\n");
    exit(EXIT_FAILURE);
  }
}

int thinkpad_battery_sleep_time(void* ptr) {
  struct thinkpad_battery_monitor* m;
  if ((m = (struct thinkpad_battery_monitor*)ptr) != NULL) {
    return 300;
  } else {
    fprintf(stderr, "thinkpad_battery monitor not received in sleep_time.\n");
    exit(EXIT_FAILURE);
  }
}

void thinkpad_battery_free(void* ptr) {
  struct thinkpad_battery_monitor* m;
  if ((m = (struct thinkpad_battery_monitor*)ptr) != NULL) {
    g_string_free(m->str, TRUE);
    g_string_free(m->alert_fgcolor, TRUE);
    free(m);

  } else {
    fprintf(stderr, "thinkpad_battery monitor not received in close.\n");
    exit(EXIT_FAILURE);
  }
}
