/*
* Copyright 2015 Chad Voegele.
* See LICENSE file for license details.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <glib.h>

#include "monitor_utils.h"
#include "status_bar.h"
#include "thinkpad_battery_monitor.h"

void* thinkpad_battery_monitor(struct monitor_refs* mr) {
  struct thinkpad_battery_monitor m;
  void* ptr;
  ptr = monitor_loop(mr, &m, thinkpad_battery_init, thinkpad_battery_update_text,
      thinkpad_battery_sleep_time, thinkpad_battery_close);
  return ptr;
}

void thinkpad_battery_init(void* ptr1, void* ptr2) {
  struct thinkpad_battery_monitor* m;
  if ((m = (struct thinkpad_battery_monitor*)ptr2) != NULL) {
    m->str = g_string_new(NULL);

  } else {
    fprintf(stderr, "thinkpad_battery monitor not received in init.\n");
    exit(EXIT_FAILURE);
  }
}

const char* thinkpad_battery_update_text(void* ptr) {
  struct thinkpad_battery_monitor* m;
  if ((m = (struct thinkpad_battery_monitor*)ptr) != NULL) {
    int full, now;
    int n_full = 0;
    int n_now = 0;

    FILE* battery_full_file;
    char* battery_full_file_path = "/sys/class/power_supply/BAT0/energy_full";
    battery_full_file = fopen(battery_full_file_path, "r");
    if (battery_full_file == NULL) {
      fprintf(stderr, "Can't open battery file %s\n", battery_full_file_path);
    }

    FILE* battery_now_file;
    char* battery_now_file_path = "/sys/class/power_supply/BAT0/energy_now";
    battery_now_file = fopen(battery_now_file_path, "r");
    if (battery_now_file == NULL) {
      fprintf(stderr, "Can't open battery file %s!\n", battery_now_file_path);
    }

    if (battery_full_file != NULL && battery_now_file != NULL) {
      n_full = fscanf(battery_full_file, "%d", &full);
      n_now = fscanf(battery_now_file, "%d", &now);
    }

    if (n_full == 1 && n_now == 1) {
      g_string_printf(m->str, "^i(/usr/share/status_bar/power-bat.xbm)%d%%",
          (int)(100.0*now/full));
    } else {
      g_string_printf(m->str, "^i(/usr/share/status_bar/power-bat.xbm)!");
    }

    fclose(battery_now_file);
    fclose(battery_full_file);
    return m->str->str;

  } else {
    fprintf(stderr, "thinkpad_battery monitor not received in update.\n");
    exit(EXIT_FAILURE);
  }
}

int thinkpad_battery_sleep_time(void* ptr) {
  struct thinkpad_battery_monitor* m;
  if ((m = (struct thinkpad_battery_monitor*)ptr) != NULL) {
    return 1;
  } else {
    fprintf(stderr, "thinkpad_battery monitor not received in sleep_time.\n");
    exit(EXIT_FAILURE);
  }
}

void thinkpad_battery_close(void* ptr) {
  struct thinkpad_battery_monitor* m;
  if ((m = (struct thinkpad_battery_monitor*)ptr) != NULL) {
    g_string_free(m->str, TRUE);

  } else {
    fprintf(stderr, "thinkpad_battery monitor not received in close.\n");
    exit(EXIT_FAILURE);
  }
}
