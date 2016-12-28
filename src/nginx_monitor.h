/*
* Copyright 2015-2017 Chad Voegele.
* See LICENSE file for license details.
*/

#pragma once

#include <curl/curl.h>

#include "status_bar.h"
#include "base_monitor.h"

struct nginx_monitor {
  struct base_monitor* base;

  GString* request_str;
  char* err;
  char* icon;
  GString* res;
  CURL* curl;
};

void* nginx_init(GKeyFile*);
gboolean nginx_update_text(void*);
int nginx_sleep_time(void*);
void nginx_free(void*);

int format_nginx_status(GString*, char*);
