/*
* Copyright 2015 Chad Voegele.
* See LICENSE file for license details.
*/

#include <glib.h>
#include <stdlib.h>
#include <sys/unistd.h>
#include <string.h>

#include "status_bar.h"
#include "monitor_utils.h"

#define CHECK_FREQ 30

volatile sig_atomic_t sleep_check(int sleep_time, volatile sig_atomic_t* is_running) {
  if (sleep_time / CHECK_FREQ <= 0) {
    sleep(sleep_time);
  } else {
    int i = 0;
    while (i < sleep_time / CHECK_FREQ && *is_running) {
      sleep(CHECK_FREQ);
      i = i + 1;
    }
  }
  return *is_running;
}

void* monitor_loop(struct monitor_refs* mr, void* monitor,
    void(*init)(void*, void*), const char* (*update)(void*),
    int(*get_sleep_time)(void*), void(*close)(void*)) {
  if (update == NULL) {
    fprintf(stderr, "update must not be null.\n");
    exit(EXIT_FAILURE);
  }

  if (get_sleep_time == NULL) {
    fprintf(stderr, "get_sleep_time must not be null.\n");
    exit(EXIT_FAILURE);
  }

  if (init != NULL) init(mr, monitor);
  const char* text = update(monitor);
  strcpy(mr->text, text);
  g_mutex_unlock(&mr->mutex);
  int sleep_time = get_sleep_time(monitor);
  while (sleep_check(sleep_time, &is_running_global)) {
    text = update(monitor);
    g_mutex_lock(&mr->mutex);
    strcpy(mr->text, text);
    g_mutex_unlock(&mr->mutex);
  }
  if (close != NULL) close(monitor);
  return NULL;
}
