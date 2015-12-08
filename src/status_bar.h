/*
* Copyright 2015 Chad Voegele.
* See LICENSE file for license details.
*/

#pragma once

#include <glib.h>
#include <stdio.h>

// https://airtower.wordpress.com/2010/06/16/catch-sigterm-exit-gracefully/
extern volatile sig_atomic_t is_running_global;

struct monitor_fns {
  void* (*init)(GString*, GMutex*, GKeyFile*);
  int (*sleep_time)(void*);
  gboolean (*update_text)(void*);
  void (*free)(void*);
};

struct monitor_refs {
  struct monitor_fns fns;
  GString* text;
  GMutex mutex;
  void* monitor;
};

struct status_bar {
  FILE* dzen_pipe;
  GKeyFile* configs;
  int n_monitors;
  struct monitor_refs* monitors;
  GMainLoop* loop;
};

int status_bar();
void init_status_bar(struct status_bar*);
void run_status_bar(struct status_bar*);
gboolean update_status_bar(void*);
void close_status_bar(struct status_bar*);
