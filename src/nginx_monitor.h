/*
* Copyright 2015 Chad Voegele.
* See LICENSE file for license details.
*/

#pragma once

#include <curl/curl.h>

#include "status_bar.h"

struct nginx_monitor {
  GString* bar_text;
  GMutex* mutex;

  GString* request_str;
  char* err;
  char* icon;
  GString* res;
  CURL* curl;
};

struct monitor_fns nginx_monitor_fns();
void* nginx_init(GString*, GMutex*, GKeyFile*);
gboolean nginx_update_text(void*);
int nginx_sleep_time(void*);
void nginx_free(void*);

int format_nginx_status(GString*, char*);
