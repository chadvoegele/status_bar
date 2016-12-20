/*
* Copyright 2015-2017 Chad Voegele.
* See LICENSE file for license details.
*/

#pragma once

#include <glib.h>
#include <curl/curl.h>

#include "status_bar.h"

struct weather_monitor {
  GString* bar_text;
  GMutex* mutex;

  GString* res;
  char* err;
  GString* request_str;
  char* icon;
  CURL* curl;
};

struct monitor_fns weather_monitor_fns();
void* weather_init(GString*, GMutex*, GKeyFile*);
gboolean weather_update_text(void*);
int weather_sleep_time(void*);
void weather_free(void*);

int format_output(GString*, char*);
int parse_xml_str(char*, const char*, const char*);
