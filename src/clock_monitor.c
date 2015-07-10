/*
* Copyright 2015 Chad Voegele.
* See LICENSE file for license details.
*/

#include <glib.h>
#include <stdlib.h>
#include <time.h>

#include "monitor_utils.h"
#include "clock_monitor.h"
#include "status_bar.h"

void* clock_monitor(struct monitor_refs* mr) {
  struct clock_monitor m;
  void* ptr;
  ptr = monitor_loop(mr, &m, clock_init, clock_update_text,
      clock_sleep_time, clock_close);
  return ptr;
}

void clock_init(void* ptr1, void* ptr2) {
  struct clock_monitor* m;
  if ((m = (struct clock_monitor*)ptr2) != NULL) {
    m->colon_on = TRUE;
    m->str = malloc(MAX_TEXT_LENGTH*sizeof(char));

  } else {
    fprintf(stderr, "clock monitor not received in init.\n");
    exit(EXIT_FAILURE);
  }
}

const char* clock_update_text(void* ptr) {
  struct clock_monitor* m;
  if ((m = (struct clock_monitor*)ptr) != NULL) {
    time_t rawtime;
    struct tm timeinfo;
    time(&rawtime);
    localtime_r(&rawtime, &timeinfo);

    char* format_str;
    if (m->colon_on) {
      format_str = "%B%e,%l:%M";
    } else {
      format_str = "%B%e,%l %M";
    }
    m->colon_on = !m->colon_on;
    strftime(m->str, MAX_TEXT_LENGTH, format_str, &timeinfo);

    return m->str;

  } else {
    fprintf(stderr, "clock monitor not received in update.\n");
    exit(EXIT_FAILURE);
  }
}

int clock_sleep_time(void* ptr) {
  struct clock_monitor* m;
  if ((m = (struct clock_monitor*)ptr) != NULL) {
    return 5;
  } else {
    fprintf(stderr, "clock monitor not received in sleep_time.\n");
    exit(EXIT_FAILURE);
  }
}

void clock_close(void* ptr) {
  struct clock_monitor* m;
  if ((m = (struct clock_monitor*)ptr) != NULL) {
    free(m->str);

  } else {
    fprintf(stderr, "clock monitor not received in close.\n");
    exit(EXIT_FAILURE);
  }
}
