/*
* Copyright 2015-2017 Chad Voegele.
* See LICENSE file for license details.
*/

#include <glib.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "memory_monitor.h"
#include "status_bar.h"
#include "base_monitor.h"

#define MAX_MEMINFO_LENGTH 100

// memory_init(icon)
void* memory_init(GArray* arguments) {
  struct memory_monitor* m = malloc(sizeof(struct memory_monitor));

  m->base = base_monitor_init(memory_sleep_time, memory_update_text, memory_free);

  char* icon = g_array_index(arguments, GString*, 0)->str;
  m->icon = g_string_new(icon);

  m->str = g_string_new(NULL);

  return m;
}

gboolean memory_update_text(void* ptr) {
  struct memory_monitor* m = (struct memory_monitor*)ptr;
  monitor_null_check(m, "memory_monitor", "update");

  char* mem_file_path = "/proc/meminfo";
  FILE* mem_file = fopen(mem_file_path, "r");
  if (mem_file == NULL) {
    fprintf(stderr, "Can't open mem file %s!\n", mem_file_path);
  }

  int available = 0;
  int total = -1;
  char buf[MAX_MEMINFO_LENGTH];
  if (mem_file != NULL) {
    while (fgets(buf, MAX_MEMINFO_LENGTH, mem_file) != NULL) {
      if (strncmp(buf, "MemTotal:", 9) == 0) {
        sscanf(buf, "%*s%d", &total);
      } else if (strncmp(buf, "MemAvailable:", 13) == 0) {
        sscanf(buf, "%*s%d", &available);
      }
    }
  }
  int usage_pct = 100 * (1 - (1.0 * available) / (1.0 * total));

  if (total != -1) {
    g_string_printf(m->str, "%s%d%%", m->icon->str, usage_pct);
  } else {
    g_string_printf(m->str, "%s!", m->icon->str);
  }

  if (mem_file != NULL)
    fclose(mem_file);

  g_mutex_lock(m->base->mutex);
  m->base->text = g_string_assign(m->base->text, m->str->str);
  g_mutex_unlock(m->base->mutex);

  return TRUE;
}

int memory_sleep_time(void* ptr) {
  return 30;
}

void memory_free(void* ptr) {
  struct memory_monitor* m = (struct memory_monitor*)ptr;
  monitor_null_check(m, "memory_monitor", "free");

  base_monitor_free(m->base);

  g_string_free(m->icon, TRUE);
  g_string_free(m->str, TRUE);
  free(m);
}
