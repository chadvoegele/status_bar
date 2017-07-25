/*
* Copyright 2015-2017 Chad Voegele.
* See LICENSE file for license details.
*/

#pragma once

#include <glib.h>
#include <stdio.h>

// https://airtower.wordpress.com/2010/06/16/catch-sigterm-exit-gracefully/
extern volatile sig_atomic_t is_running_global;

struct status_bar {
  FILE* display_pipe;
  GKeyFile* configs;
  gsize n_monitors;
  void** monitors;
  GMainLoop* loop;
};

int status_bar(int argc, char** argv);
void init_status_bar(struct status_bar*, int argc, char** argv);
void run_status_bar(struct status_bar*);
gboolean update_status_bar(void*);
void close_status_bar(struct status_bar*);

void monitor_null_check(void* ptr, const char* monitor_name, const char* monitor_fn);
void monitor_arg_check(const char* monitor_name, GArray* actual, const char* expected);
