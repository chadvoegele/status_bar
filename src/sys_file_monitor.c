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

void* sys_file_init(GArray* arguments) {
  monitor_arg_check("sys_file", arguments, "(icon, multiplier, varargs)");

  struct sys_file_monitor* m = malloc(sizeof(struct sys_file_monitor));

  m->base = base_monitor_init(sys_file_sleep_time, sys_file_update_text, sys_file_free);

  char* icon = g_array_index(arguments, GString*, 0)->str;
  m->icon = g_string_new(icon);

  char* multiplier = g_array_index(arguments, GString*, 1)->str;
  m->multiplier = atof(multiplier);

  GArray* filenames = g_array_new(FALSE, FALSE, sizeof(GString*));
  for (int i = 2; i < arguments->len; i++) {
    GString* str = g_string_new(g_array_index(arguments, GString*, i)->str);
    g_array_append_val(filenames, str);
  }
  m->filenames = filenames;
  m->str = g_string_new(NULL);

  return m;
}

gboolean sys_file_update_text(void* ptr) {
  struct sys_file_monitor* m = (struct sys_file_monitor*)ptr;
  monitor_null_check(m, "sys_file_monitor", "update");

  int n_files = m->filenames->len;

  int min_val = INT_MAX;
  int max_val = INT_MIN;
  int n_read = 0;
  for (int i_file = 0; i_file < n_files; i_file++) {
    char* file_path = g_array_index(m->filenames, GString*, i_file)->str;
    FILE* file = fopen(file_path, "r");
    if (file == NULL) {
      fprintf(stderr, "Can't open temp file %s!\n", file_path);
      continue;
    }

    int val;
    if (fscanf(file, "%d", &val) == 1) {
      min_val = val < min_val ? val : min_val;
      max_val = val > max_val ? val : max_val;
      n_read++;
    }

    fclose(file);
  }

  m->str = g_string_set_size(m->str, 0);
  if (n_read == n_files) {
    if (n_files == 1) {
      g_string_append_printf(m->str, "%s%d", m->icon->str, (int)(m->multiplier*min_val));
    } else {
      g_string_append_printf(m->str, "%s%d%d", m->icon->str, (int)(m->multiplier*min_val), (int)(m->multiplier*max_val));
    }

  } else {
    g_string_append_printf(m->str, "%s!", m->icon->str);
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

  g_string_free(m->icon, TRUE);
  for (int i = 0; i < m->filenames->len; i++) {
    g_string_free(g_array_index(m->filenames, GString*, i), TRUE);
  }
  g_array_free(m->filenames, TRUE);

  g_string_free(m->str, TRUE);

  base_monitor_free(m->base);

  free(m);
}
