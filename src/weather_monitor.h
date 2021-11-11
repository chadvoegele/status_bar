/*
* Copyright 2015-2021 Chad Voegele.
* See LICENSE file for license details.
*/

#pragma once

#include <glib.h>

#include "status_bar.h"
#include "base_monitor.h"
#include "http_download.h"

struct weather_monitor {
  struct base_monitor* base;

  GString* icon;
  GString* sun_icon;
  GString* cloudy_icon;
  GString* rain_icon;
  GString* storm_icon;
  GString* snow_icon;

  GString* res;
  char* err;
  GString* request_str;
  struct http_data* http_data;
};

void* weather_init(GArray*);
gboolean weather_update_text(void*);
int weather_sleep_time(void*);
void weather_free(void*);

char* convert_weather_text_to_icon(struct weather_monitor*, char*);
int format_output(struct weather_monitor*, GString*);
int parse_xml_str(char*, const char*, const char*);
char* strlower(char*);
