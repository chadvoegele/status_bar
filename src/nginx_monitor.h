/*
* Copyright 2015-2021 Chad Voegele.
* See LICENSE file for license details.
*/

#pragma once

#include <curl/curl.h>

#include "status_bar.h"
#include "base_monitor.h"

struct nginx_monitor {
  struct base_monitor* base;

  GString* icon;
  GString* request_str;

  char* err;
  GString* res;
  struct http_data* http_data;
};

void* nginx_init(GArray*);
gboolean nginx_update_text(void*);
int nginx_sleep_time(void*);
void nginx_free(void*);

int format_nginx_status(GString*, GString*);
