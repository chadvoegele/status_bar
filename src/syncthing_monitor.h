/*
* Copyright 2015-2021 Chad Voegele.
* See LICENSE file for license details.
*/

#pragma once

#include <curl/curl.h>

#include "status_bar.h"
#include "base_monitor.h"
#include "http_download.h"

struct syncthing_monitor {
  struct base_monitor* base;

  struct http_data* http_data;
  struct curl_slist* headers;

  GString* sync_down_icon;
  GString* sync_up_icon;
  GString* synced_icon;
  GString* sync_error_icon;

  GString* api_key_header;
  int last_id;
  GString* url;
  double completion;

  GString* response_buffer;
};

void* syncthing_init(GArray*);
gboolean syncthing_update_text(void*);
int syncthing_sleep_time(void*);
void syncthing_free(void*);

void set_headers(struct syncthing_monitor*);
