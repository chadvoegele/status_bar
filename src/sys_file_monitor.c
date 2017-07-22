/** Copyright 2015-2017 Chad Voegele.
* See LICENSE file for license details.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <glib.h>
#include <limits.h>

#include "status_bar.h"
#include "base_monitor.h"
#include "sys_file_monitor.h"

void* sys_file_init_config(gunichar icon, GArray* temp_filenames,
    int(*convert)(int), GArray* arguments) {
  struct sys_file_monitor* m = malloc(sizeof(struct sys_file_monitor));

  m->base = base_monitor_init(sys_file_sleep_time, sys_file_update_text, sys_file_free);

  m->icon = icon;
  m->temp_filenames = temp_filenames;
  m->convert = convert;
  m->str = g_string_new(NULL);

  return m;
}

void append_filename(GArray* filenames, char* filename) {
  GString* filename_str = g_string_new(filename);
  g_array_append_val(filenames, filename_str);
}

gboolean sys_file_update_text(void* ptr) {
  struct sys_file_monitor* m = (struct sys_file_monitor*)ptr;
  monitor_null_check(m, "sys_file_monitor", "update");

  int n_temps = m->temp_filenames->len;

  int min_temp = INT_MAX;
  int max_temp = INT_MIN;
  int n_read = 0;
  for (int i_file = 0; i_file < n_temps; i_file++) {
    char* temp_file_path = g_array_index(m->temp_filenames, GString*, i_file)->str;
    FILE* temp_file = fopen(temp_file_path, "r");
    if (temp_file == NULL) {
      fprintf(stderr, "Can't open temp file %s!\n", temp_file_path);
      continue;
    }

    int temp;
    if (fscanf(temp_file, "%d", &temp) == 1) {
      min_temp = temp < min_temp ? temp : min_temp;
      max_temp = temp > max_temp ? temp : max_temp;
      n_read++;
    }

    fclose(temp_file);
  }

  m->str = g_string_set_size(m->str, 0);
  g_string_append_unichar(m->str, m->icon);
  if (n_read == n_temps) {
    if (n_temps == 1) {
      g_string_append_printf(m->str, "%d", m->convert(min_temp));
    } else {
      g_string_append_printf(m->str, "%d%d", m->convert(min_temp), m->convert(max_temp));
    }

  } else {
    g_string_append_printf(m->str, "X!");
  }

  g_mutex_lock(m->base->mutex);
  m->base->text = g_string_assign(m->base->text, m->str->str);
  g_mutex_unlock(m->base->mutex);

  return TRUE;
}

int sys_file_sleep_time(void* ptr) {
  return 5;
}

void sys_file_free(void* ptr) {
  struct sys_file_monitor* m = (struct sys_file_monitor*)ptr;
  monitor_null_check(m, "sys_file_monitor", "free");

  for (int i = 0; i < m->temp_filenames->len; i++) {
    g_string_free(g_array_index(m->temp_filenames, GString*, i), TRUE);
  }
  g_array_free(m->temp_filenames, TRUE);

  g_string_free(m->str, TRUE);

  base_monitor_free(m->base);

  free(m);
}
