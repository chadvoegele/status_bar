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
#include "thinkpad_fan_monitor.h"

void* thinkpad_fan_init(GArray* arguments) {
  monitor_arg_check("thinkpad_fan", arguments, "(icon)");

  struct thinkpad_fan_monitor* m = malloc(sizeof(struct thinkpad_fan_monitor));

  m->base = base_monitor_init(thinkpad_fan_sleep_time, thinkpad_fan_update_text, thinkpad_fan_free);

  char* icon = g_array_index(arguments, GString*, 0)->str;
  m->icon = g_string_new(icon);

  m->str = g_string_new(NULL);

  return m;
}

gboolean thinkpad_fan_update_text(void* ptr) {
  struct thinkpad_fan_monitor* m = (struct thinkpad_fan_monitor*)ptr;
  monitor_null_check(m, "thinkpad_fan_monitor", "update");

  int n_read = 0;

  FILE* fan_file;
  char* fan_file_path = "/proc/acpi/ibm/fan";

  fan_file = fopen(fan_file_path, "r");
  if (fan_file == NULL) {
    fprintf(stderr, "Can't open fan file %s!\n", fan_file_path);
  }

  int speed;
  if (fan_file != NULL)
    n_read = fscanf(fan_file, "%*s %*s %*s %d", &speed);

  if (n_read == 1) {
    g_string_printf(m->str, "%s%d", m->icon->str, speed);
  }
  else {
    g_string_printf(m->str, "%s!", m->icon->str);
  }

  if (fan_file != NULL)
    fclose(fan_file);

  m->base->text = g_string_assign(m->base->text, m->str->str);

  return TRUE;
}

int thinkpad_fan_sleep_time(void* ptr) {
  return 5;
}

void thinkpad_fan_free(void* ptr) {
  struct thinkpad_fan_monitor* m = (struct thinkpad_fan_monitor*)ptr;
  monitor_null_check(m, "thinkpad_fan_monitor", "free");

  g_string_free(m->icon, TRUE);

  base_monitor_free(m->base);

  g_string_free(m->str, TRUE);
  free(m);
}
