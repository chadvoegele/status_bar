/*
* Copyright 2015-2017 Chad Voegele.
* See LICENSE file for license details.
*/

#pragma once

#include <glib.h>
#include <curl/curl.h>

#include "status_bar.h"
#include "base_monitor.h"

struct weather_monitor {
  struct base_monitor* base;

  GString* icon;

  GString* res;
  char* err;
  GString* request_str;
  CURL* curl;
};

void* weather_init(GArray*);
gboolean weather_update_text(void*);
int weather_sleep_time(void*);
void weather_free(void*);

int format_output(GString*, GString*);
int parse_xml_str(char*, const char*, const char*);
