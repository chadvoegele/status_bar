/*
* Copyright 2015-2017 Chad Voegele.
* See LICENSE file for license details.
*/

#pragma once

#include <curl/curl.h>

#include "status_bar.h"

struct sp500_monitor {
  GString* bar_text;
  GMutex* mutex;

  GString* request_str;
  char* err;
  char* icon;
  GString* res;
  CURL* curl;
};

struct monitor_fns sp500_monitor_fns();
void* sp500_init(GString*, GMutex*, GKeyFile*);
gboolean sp500_update_text(void*);
int sp500_sleep_time(void*);
void sp500_free(void*);

int format_price(GString*, char*);
