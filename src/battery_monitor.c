/*
* Copyright 2015-2021 Chad Voegele.
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

void* battery_init(GArray* arguments) {
  monitor_arg_check("battery", arguments, "(battery_full_icon, battery_empty_icon, battery_charging_icon, alert_fgcolor, alert_bgcolor, battery_full_path, battery_now_path, battery_status_path)");

  struct battery_monitor* m = malloc(sizeof(struct battery_monitor));

  m->base = base_monitor_init(battery_sleep_time, battery_update_text, battery_free);

  char* battery_full_icon = g_array_index(arguments, GString*, 0)->str;
  m->battery_full_icon = g_string_new(battery_full_icon);

  char* battery_empty_icon = g_array_index(arguments, GString*, 1)->str;
  m->battery_empty_icon = g_string_new(battery_empty_icon);

  char* battery_charging_icon = g_array_index(arguments, GString*, 2)->str;
  m->battery_charging_icon = g_string_new(battery_charging_icon);

  char* alert_fgcolor = g_array_index(arguments, GString*, 3)->str;
  m->alert_fgcolor = g_string_new(alert_fgcolor);

  char* alert_bgcolor = g_array_index(arguments, GString*, 4)->str;
  m->alert_bgcolor = g_string_new(alert_bgcolor);

  char* battery_full_path = g_array_index(arguments, GString*, 5)->str;
  m->battery_full_path_str = g_string_new(battery_full_path);

  char* battery_now_path = g_array_index(arguments, GString*, 6)->str;
  m->battery_now_path_str = g_string_new(battery_now_path);

  char* battery_status_path = g_array_index(arguments, GString*, 7)->str;
  m->battery_status_path_str = g_string_new(battery_status_path);

  m->str = g_string_new(NULL);

  m->last_now = -1;

  return m;
}

gboolean battery_update_text(void* ptr) {
  struct battery_monitor* m = (struct battery_monitor*)ptr;
  monitor_null_check(m, "battery_monitor", "update");

  int full, now;
  int n_full = 0;
  int n_now = 0;
  int is_charging = 0;

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

  FILE* battery_status_file;
  char* battery_status_file_path = m->battery_status_path_str->str;
  battery_status_file = fopen(battery_status_file_path, "r");
  if (battery_status_file == NULL) {
    fprintf(stderr, "Can't open battery file %s!\n", battery_status_file_path);
  }

  if (battery_status_file != NULL) {
    char status_file_contents[25];
    fscanf(battery_status_file, "%s", status_file_contents);
    if (strcmp("Charging", status_file_contents) == 0) {
      is_charging = 1;
    }
  }

  if (n_full == 1 && n_now == 1) {
    int battpct = (int)(100.0*now/full);
    if (is_charging == 1) {
      g_string_printf(m->str, "%s%d%%", m->battery_charging_icon->str, battpct);
    } else if (battpct <= 10) {
      g_string_printf(m->str, "<span background=\"%s\" foreground=\"%s\">%s%d%%</span>",
          m->alert_bgcolor->str, m->alert_fgcolor->str, m->battery_empty_icon->str, battpct);
    } else {
      g_string_printf(m->str, "%s%d%%", m->battery_full_icon->str, battpct);
    }

    if (m->last_now != -1) {
      int rate = (now - m->last_now)/1000;
      g_string_append_printf(m->str, " %+dmA", rate);
    } else {
      g_string_append_printf(m->str, "  mA");
    }

    m->last_now = now;
  } else {
    g_string_printf(m->str, "%s!", m->battery_full_icon->str);
  }


  if (battery_now_file != NULL)
    fclose(battery_now_file);

  if (battery_full_file != NULL)
    fclose(battery_full_file);

  if (battery_status_file != NULL)
    fclose(battery_status_file);

  m->base->text = g_string_assign(m->base->text, m->str->str);

  return TRUE;
}

int battery_sleep_time(void* ptr) {
  return 10;
}

void battery_free(void* ptr) {
  struct battery_monitor* m = (struct battery_monitor*)ptr;
  monitor_null_check(m, "battery_monitor", "free");

  g_string_free(m->str, TRUE);
  g_string_free(m->battery_full_icon, TRUE);
  g_string_free(m->battery_empty_icon, TRUE);
  g_string_free(m->battery_charging_icon, TRUE);
  g_string_free(m->alert_fgcolor, TRUE);
  g_string_free(m->alert_bgcolor, TRUE);
  g_string_free(m->battery_full_path_str, TRUE);
  g_string_free(m->battery_now_path_str, TRUE);
  g_string_free(m->battery_status_path_str, TRUE);

  base_monitor_free(m->base);

  free(m);
}
