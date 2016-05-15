/*
* Copyright 2015 Chad Voegele.
* See LICENSE file for license details.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <glib.h>

#include "status_bar.h"
#include "thinkpad_fan_monitor.h"

struct monitor_fns thinkpad_fan_monitor_fns() {
  struct monitor_fns f;
  f.init = thinkpad_fan_init;
  f.sleep_time = thinkpad_fan_sleep_time;
  f.update_text = thinkpad_fan_update_text;
  f.free = thinkpad_fan_free;

  return f;
}

void* thinkpad_fan_init(GString* bar_text, GMutex* mutex, GKeyFile* configs) {
  struct thinkpad_fan_monitor* m = malloc(sizeof(struct thinkpad_fan_monitor));

  m->bar_text = bar_text;
  m->mutex = mutex;

  m->str = g_string_new(NULL);

  return m;
}

gboolean thinkpad_fan_update_text(void* ptr) {
  struct thinkpad_fan_monitor* m;
  if ((m = (struct thinkpad_fan_monitor*)ptr) != NULL) {
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
      g_string_printf(m->str, "%d", speed);
    }
    else {
      g_string_printf(m->str, "!");
    }

    if (fan_file != NULL)
      fclose(fan_file);

    g_mutex_lock(m->mutex);
    m->bar_text = g_string_assign(m->bar_text, m->str->str);
    g_mutex_unlock(m->mutex);

    return TRUE;

  } else {
    fprintf(stderr, "thinkpad_fan monitor not received in update.\n");
    exit(EXIT_FAILURE);
  }
}

int thinkpad_fan_sleep_time(void* ptr) {
  struct thinkpad_fan_monitor* m;
  if ((m = (struct thinkpad_fan_monitor*)ptr) != NULL) {
    return 5;
  } else {
    fprintf(stderr, "thinkpad_fan monitor not received in sleep_time.\n");
    exit(EXIT_FAILURE);
  }
}

void thinkpad_fan_free(void* ptr) {
  struct thinkpad_fan_monitor* m;
  if ((m = (struct thinkpad_fan_monitor*)ptr) != NULL) {
    g_string_free(m->str, TRUE);
    free(m);

  } else {
    fprintf(stderr, "thinkpad_fan monitor not received in close.\n");
    exit(EXIT_FAILURE);
  }
}
