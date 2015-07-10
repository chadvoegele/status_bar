/*
* Copyright 2015 Chad Voegele.
* See LICENSE file for license details.
*/

#pragma once

#include <curl/curl.h>

#include "status_bar.h"

struct sp500_monitor {
  GString* request_str;
  char* err;
  char* icon;
  GString* res;
  CURL* curl;
};

void* sp500_monitor(struct monitor_refs*);
void sp500_init(void*, void*);
const char* sp500_update_text(void*);
int sp500_sleep_time(void*);
void sp500_close(void*);

int format_price(GString*, char*);
