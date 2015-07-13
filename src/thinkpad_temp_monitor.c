/*
* Copyright 2015 Chad Voegele.
* See LICENSE file for license details.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <glib.h>

#include "status_bar.h"
#include "thinkpad_temp_monitor.h"

struct monitor_fns thinkpad_temp_monitor_fns() {
  struct monitor_fns f;
  f.init = thinkpad_temp_init;
  f.sleep_time = thinkpad_temp_sleep_time;
  f.update_text = thinkpad_temp_update_text;
  f.free = thinkpad_temp_free;

  return f;
}

void* thinkpad_temp_init(GString* bar_text, GMutex* mutex, GKeyFile* configs) {
  struct thinkpad_temp_monitor* m = malloc(sizeof(struct thinkpad_temp_monitor));

  m->bar_text = bar_text;
  m->mutex = mutex;

  m->str = g_string_new(NULL);

  return m;
}

gboolean thinkpad_temp_update_text(void* ptr) {
  struct thinkpad_temp_monitor* m;
  if ((m = (struct thinkpad_temp_monitor*)ptr) != NULL) {
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
      g_string_printf(m->str, "^i(/usr/share/status_bar/cpu.xbm)%d %d %d %d %d %d %d",
          temps[0],
          temps[1],
          temps[2],
          temps[3],
          temps[4],
          temps[5],
          temps[6]);

    } else {
      g_string_printf(m->str, "^i(/usr/share/status_bar/cpu.xbm)!");
    }
    fclose(temp_file);

    g_mutex_lock(m->mutex);
    m->bar_text = g_string_assign(m->bar_text, m->str->str);
    g_mutex_unlock(m->mutex);

    return TRUE;

  } else {
    fprintf(stderr, "thinkpad_temp monitor not received in update.\n");
    exit(EXIT_FAILURE);
  }
}

int thinkpad_temp_sleep_time(void* ptr) {
  struct thinkpad_temp_monitor* m;
  if ((m = (struct thinkpad_temp_monitor*)ptr) != NULL) {
    return 5;
  } else {
    fprintf(stderr, "thinkpad_temp monitor not received in sleep_time.\n");
    exit(EXIT_FAILURE);
  }
}

void thinkpad_temp_free(void* ptr) {
  struct thinkpad_temp_monitor* m;
  if ((m = (struct thinkpad_temp_monitor*)ptr) != NULL) {
    g_string_free(m->str, TRUE);
    free(m);

  } else {
    fprintf(stderr, "thinkpad_temp monitor not received in close.\n");
    exit(EXIT_FAILURE);
  }
}
