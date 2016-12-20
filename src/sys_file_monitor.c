/** Copyright 2015-2017 Chad Voegele.
* See LICENSE file for license details.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <glib.h>

#include "status_bar.h"
#include "sys_file_monitor.h"

void* sys_file_init_config(gunichar icon, GArray* temp_filenames,
    int(*convert)(int),
    GString* bar_text, GMutex* mutex, GKeyFile* configs) {
  struct sys_file_monitor* m = malloc(sizeof(struct sys_file_monitor));

  m->bar_text = bar_text;
  m->mutex = mutex;

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

  g_string_truncate(m->str, 0);
  int n_read = 0;
  for (int i_file = 0; i_file < n_temps; i_file++) {
    char* temp_file_path = g_array_index(m->temp_filenames, GString*, i_file)->str;
    FILE* temp_file;
    int temp;
    temp_file = fopen(temp_file_path, "r");
    if (temp_file == NULL) {
      fprintf(stderr, "Can't open temp file %s!\n", temp_file_path);
    } else {
      fscanf(temp_file, "%d", &temp);
      n_read++;
      g_string_append_printf(m->str, " %d", m->convert(temp));
      fclose(temp_file);
    }
  }

  if (n_read == n_temps) {
    g_string_prepend_unichar(m->str, m->icon);

  } else {
    g_string_printf(m->str, "U+%04"G_GINT32_FORMAT"X!", m->icon);
  }

  g_mutex_lock(m->mutex);
  m->bar_text = g_string_assign(m->bar_text, m->str->str);
  g_mutex_unlock(m->mutex);

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
  free(m);
}
