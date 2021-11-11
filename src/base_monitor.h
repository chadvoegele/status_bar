/*
* Copyright 2015-2021 Chad Voegele.
* See LICENSE file for license details.
*/

#pragma once

#include <glib.h>

struct base_monitor_base {
  struct base_monitor* base;
};

struct base_monitor {
  int (*sleep_time)(void*);
  gboolean (*update_text)(void*);
  void (*free)(void*);

  GString* text;
};

struct base_monitor* base_monitor_init(int (*sleep_time)(void*), gboolean (*update_text)(void*), void (*free)(void*));
void base_monitor_free(struct base_monitor*);
