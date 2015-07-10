/*
* Copyright 2015 Chad Voegele.
* See LICENSE file for license details.
*/

#pragma once

#include <glib.h>
#include <curl/curl.h>

#include "status_bar.h"

struct weather_monitor {
  GString* request_str;
  char* err;
  char* icon;
  GString* res;
  CURL* curl;
};

void* weather_monitor(struct monitor_refs*);
void weather_init(void*, void*);
const char* weather_update_text(void*);
int weather_sleep_time(void*);
void weather_close(void*);

int format_output(GString*, char*);
int parse_xml_str(char*, const char*, const char*);
