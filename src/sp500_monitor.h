/*
* Copyright 2015-2021 Chad Voegele.
* See LICENSE file for license details.
*/

#pragma once

#include <curl/curl.h>

#include "status_bar.h"
#include "base_monitor.h"

struct sp500_monitor {
  struct base_monitor* base;

  GString* icon;

  char* err;
  GString* url;
  struct http_data* http_data;
};

GString* build_url();
void* sp500_init(GArray*);
gboolean sp500_update_text(void*);
int sp500_sleep_time(void*);
void sp500_free(void*);

int format_price(GString*, char*, GString*);
