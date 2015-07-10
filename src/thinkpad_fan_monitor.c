/*
* Copyright 2015 Chad Voegele.
* See LICENSE file for license details.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <glib.h>

#include "status_bar.h"
#include "monitor_utils.h"
#include "thinkpad_fan_monitor.h"

void* thinkpad_fan_monitor(struct monitor_refs* mr) {
  struct thinkpad_fan_monitor m;
  void* ptr;
  ptr = monitor_loop(mr, &m, thinkpad_fan_init, thinkpad_fan_update_text,
      thinkpad_fan_sleep_time, thinkpad_fan_close);
  return ptr;
}

void thinkpad_fan_init(void* ptr1, void* ptr2) {
  struct thinkpad_fan_monitor* m;
  if ((m = (struct thinkpad_fan_monitor*)ptr2) != NULL) {
    m->str = g_string_new(NULL);

  } else {
    fprintf(stderr, "thinkpad_fan monitor not received in init.\n");
    exit(EXIT_FAILURE);
  }
}

const char* thinkpad_fan_update_text(void* ptr) {
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
      g_string_printf(m->str, "^i(/usr/share/status_bar/fan.xbm)%d", speed);
    }
    else {
      g_string_printf(m->str, "^i(/usr/share/status_bar/fan.xbm)!");
    }

    fclose(fan_file);
    return m->str->str;

  } else {
    fprintf(stderr, "thinkpad_fan monitor not received in update.\n");
    exit(EXIT_FAILURE);
  }
}

int thinkpad_fan_sleep_time(void* ptr) {
  struct thinkpad_fan_monitor* m;
  if ((m = (struct thinkpad_fan_monitor*)ptr) != NULL) {
    return 1;
  } else {
    fprintf(stderr, "thinkpad_fan monitor not received in sleep_time.\n");
    exit(EXIT_FAILURE);
  }
}

void thinkpad_fan_close(void* ptr) {
  struct thinkpad_fan_monitor* m;
  if ((m = (struct thinkpad_fan_monitor*)ptr) != NULL) {
    g_string_free(m->str, TRUE);

  } else {
    fprintf(stderr, "thinkpad_fan monitor not received in close.\n");
    exit(EXIT_FAILURE);
  }
}
