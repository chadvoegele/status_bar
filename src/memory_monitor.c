/*
* Copyright 2015 Chad Voegele.
* See LICENSE file for license details.
*/

#include <glib.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "memory_monitor.h"
#include "monitor_utils.h"
#include "status_bar.h"

#define MAX_MEMINFO_LENGTH 100

void* memory_monitor(struct monitor_refs* mr) {
  struct memory_monitor m;
  void* ptr;
  ptr = monitor_loop(mr, &m, memory_init, memory_update_text,
      memory_sleep_time, memory_close);
  return ptr;
}

void memory_init(void* ptr1, void* ptr2) {
  struct memory_monitor* m;
  if ((m = (struct memory_monitor*)ptr2) != NULL) {
    m->str = g_string_new(NULL);

  } else {
    fprintf(stderr, "memory monitor not received in init.\n");
    exit(EXIT_FAILURE);
  }
}

const char* memory_update_text(void* ptr) {
  struct memory_monitor* m;
  if ((m = (struct memory_monitor*)ptr) != NULL) {
    char* mem_file_path = "/proc/meminfo";
    FILE* mem_file = fopen(mem_file_path, "r");
    if (mem_file == NULL) {
      fprintf(stderr, "Can't open mem file %s!\n", mem_file_path);
    }

    int active = 0;
    int total = -1;
    char buf[MAX_MEMINFO_LENGTH];
    if (mem_file != NULL) {
      while (fgets(buf, MAX_MEMINFO_LENGTH, mem_file) != NULL) {
        if (strncmp(buf, "Active:", 7) == 0) {
          sscanf(buf, "%*s%d", &active);
        } else if (strncmp(buf, "MemTotal:", 9) == 0) {
          sscanf(buf, "%*s%d", &total);
        }
      }
    }
    int usage_pct = 100.0 * active / total;

    if (total != -1) {
      g_string_printf(m->str, "^i(/usr/share/status_bar/mem.xbm)%d%%", usage_pct);
    } else {
      g_string_printf(m->str, "^i(/usr/share/status_bar/mem.xbm)!");
    }

    fclose(mem_file);
    return m->str->str;

  } else {
    fprintf(stderr, "memory monitor not received in update.\n");
    exit(EXIT_FAILURE);
  }
}

int memory_sleep_time(void* ptr) {
  struct memory_monitor* m;
  if ((m = (struct memory_monitor*)ptr) != NULL) {
    return 1;
  } else {
    fprintf(stderr, "memory monitor not received in sleep_time.\n");
    exit(EXIT_FAILURE);
  }
}

void memory_close(void* ptr) {
  struct memory_monitor* m;
  if ((m = (struct memory_monitor*)ptr) != NULL) {
    g_string_free(m->str, TRUE);

  } else {
    fprintf(stderr, "memory monitor not received in close.\n");
    exit(EXIT_FAILURE);
  }
}
