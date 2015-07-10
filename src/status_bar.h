/*
* Copyright 2015 Chad Voegele.
* See LICENSE file for license details.
*/

#pragma once

#include <glib.h>
#include <stdio.h>

#define MAX_MONITORS 20
#define MAX_TEXT_LENGTH 100

// https://airtower.wordpress.com/2010/06/16/catch-sigterm-exit-gracefully/
extern volatile sig_atomic_t is_running_global;

struct monitor_refs {
  GKeyFile* configs;
  GMutex mutex;
  char text[MAX_TEXT_LENGTH];
  void* (*monitor)(struct monitor_refs*);
};

struct status_bar {
  int n_monitors;
  FILE* dzen_pipe;
  GKeyFile* configs;
  struct monitor_refs monitors[MAX_MONITORS];
};

int status_bar();
void init_status_bar(struct status_bar*);
void run_status_bar(struct status_bar*);
void close_status_bar(struct status_bar*);
void init_signal_handler();
void term(int);
void* thread_fun(void*);
