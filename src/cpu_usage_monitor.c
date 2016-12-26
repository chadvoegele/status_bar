/*
* Copyright 2015-2017 Chad Voegele.
* See LICENSE file for license details.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <glib.h>

#include "status_bar.h"
#include "cpu_usage_monitor.h"

#define N_TIMES 10

struct monitor_fns cpu_usage_monitor_fns() {
  struct monitor_fns f;
  f.init = cpu_usage_init;
  f.sleep_time = cpu_usage_sleep_time;
  f.update_text = cpu_usage_update_text;
  f.free = cpu_usage_free;

  return f;
}

void* cpu_usage_init(GString* bar_text, GMutex* mutex, GKeyFile* configs) {
  struct cpu_usage_monitor* m = malloc(sizeof(struct cpu_usage_monitor));

  m->bar_text = bar_text;
  m->mutex = mutex;

  m->str = g_string_new(NULL);
  m->last_total = 1U;
  m->last_idle = 1U;

  return m;
}

gboolean cpu_usage_update_text(void* ptr) {
  struct cpu_usage_monitor* m = (struct cpu_usage_monitor*)ptr;
  monitor_null_check(m, "cpu_usage_monitor", "update");

  char* time_file_path = "/proc/stat";

  int n_read;
  unsigned int times[N_TIMES];
  double usage_pct;

  FILE* time_file = fopen(time_file_path, "r");
  if (time_file == NULL) {
    fprintf(stderr, "Can't open file %s!\n", time_file_path);
    n_read = 0;
    usage_pct = 0;
    m->last_total = 1U;
    m->last_idle = 1U;

  } else {
    n_read = fscanf(time_file, "%*s %d %d %d %d %d %d %d %d %d %d",
        &times[0],
        &times[1],
        &times[2],
        &times[3],
        &times[4],
        &times[5],
        &times[6],
        &times[7],
        &times[8],
        &times[9]);
    unsigned int total = cpu_usage_calc_total(times);
    unsigned int idle = cpu_usage_calc_idle(times);
    unsigned int diff_total = total - m->last_total;
    unsigned int diff_idle = idle - m->last_idle;
    double idle_pct = diff_idle / (double)diff_total;
    usage_pct = 1 - idle_pct;
    m->last_total = total;
    m->last_idle = idle;
  }

  gunichar icon;
  sscanf("U+62529", "U+%06"G_GINT32_FORMAT"X", &icon);
  m->str = g_string_set_size(m->str, 0);
  g_string_append_unichar(m->str, icon);

  if (n_read == N_TIMES) {
    g_string_append_printf(m->str, "%d%%", (int)(100*usage_pct));

  } else {
    g_string_append_printf(m->str, "!");
  }

  if (time_file != NULL) {
    fclose(time_file);
  }

  g_mutex_lock(m->mutex);
  m->bar_text = g_string_assign(m->bar_text, m->str->str);
  g_mutex_unlock(m->mutex);

  return TRUE;
}

int cpu_usage_sleep_time(void* ptr) {
  return 1;
}

void cpu_usage_free(void* ptr) {
  struct cpu_usage_monitor* m = (struct cpu_usage_monitor*)ptr;
  monitor_null_check(m, "cpu_usage_monitor", "free");

  g_string_free(m->str, TRUE);
  free(m);
}

unsigned int cpu_usage_calc_total(unsigned int* times) {
  unsigned int total = 0U;
  for (size_t i = 0; i < N_TIMES; i++) {
    total = total + times[i];
  }
  return total;
}

unsigned int cpu_usage_calc_idle(unsigned int* times) {
  unsigned int idle = times[3] + times[4];
  return idle;
}
