/*
* Copyright 2015-2017 Chad Voegele.
* See LICENSE file for license details.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <glib.h>
#include <limits.h>

#include "status_bar.h"
#include "base_monitor.h"
#include "thinkpad_temp_monitor.h"

void* thinkpad_temp_init(GArray* arguments) {
  struct thinkpad_temp_monitor* m = malloc(sizeof(struct thinkpad_temp_monitor));

  m->base = base_monitor_init(thinkpad_temp_sleep_time, thinkpad_temp_update_text, thinkpad_temp_free);

  m->str = g_string_new(NULL);

  return m;
}

gboolean thinkpad_temp_update_text(void* ptr) {
  struct thinkpad_temp_monitor* m = (struct thinkpad_temp_monitor*)ptr;
  monitor_null_check(m, "thinkpad_temp_monitor", "update");

  FILE* temp_file;
  char* temp_file_path = "/proc/acpi/ibm/thermal";

  int n_read;
  int temps[7];
  temp_file = fopen(temp_file_path, "r");
  if (temp_file == NULL) {
    fprintf(stderr, "Can't open temp file %s!\n", temp_file_path);
    n_read = 0;
  } else {
    n_read = fscanf(temp_file, "%*s %d %d %*d %d %d %*d %d %*d %d %d",
        &temps[0],
        &temps[1],
        &temps[2],
        &temps[3],
        &temps[4],
        &temps[5],
        &temps[6]);
  }

  if (n_read == 7) {
    int min_temp = INT_MAX;
    int max_temp = INT_MIN;

    for (int i = 0; i < 7; i++) {
      int temp = temps[i];
      min_temp = temp < min_temp ? temp : min_temp;
      max_temp = temp > max_temp ? temp : max_temp;
    }

    g_string_printf(m->str, "%d%d",
        min_temp, max_temp);

  } else {
    g_string_printf(m->str, "!");
  }

  if (temp_file != NULL)
    fclose(temp_file);

  g_mutex_lock(m->base->mutex);
  m->base->text = g_string_assign(m->base->text, m->str->str);
  g_mutex_unlock(m->base->mutex);

  return TRUE;
}

int thinkpad_temp_sleep_time(void* ptr) {
  return 5;
}

void thinkpad_temp_free(void* ptr) {
  struct thinkpad_temp_monitor* m = (struct thinkpad_temp_monitor*)ptr;
  monitor_null_check(m, "thinkpad_temp_monitor", "free");

  base_monitor_free(m->base);

  g_string_free(m->str, TRUE);
  free(m);
}
