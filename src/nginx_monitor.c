/*
* Copyright 2015-2017 Chad Voegele.
* See LICENSE file for license details.
*/

#include <glib.h>
#include <stdlib.h>
#include <string.h>
#include <curl/curl.h>

#include "status_bar.h"
#include "base_monitor.h"
#include "nginx_monitor.h"
#include "http_download.h"
#include "configs.h"

// nginx_init(status_uri)
void* nginx_init(GArray* arguments) {
  struct nginx_monitor* m = malloc(sizeof(struct nginx_monitor));

  m->base = base_monitor_init(nginx_sleep_time, nginx_update_text, nginx_free);

  char* status_uri = g_array_index(arguments, GString*, 0)->str;
  m->request_str = g_string_new(status_uri);

  m->res = g_string_new(NULL);
  m->curl = curl_easy_init();

  m->err = malloc(2*sizeof(char));
  sprintf(m->err, "!");

  return m;
}

gboolean nginx_update_text(void* ptr) {
  struct nginx_monitor* m = (struct nginx_monitor*)ptr;
  monitor_null_check(m, "nginx_monitor", "update");

  CURLcode code = download_data(m->curl, m->request_str->str, m->res);
  char* output;

  if (code == CURLE_OK && format_nginx_status(m->res) != -1) {
    output = m->res->str;
  } else {
    output = m->err;
  }

  g_mutex_lock(m->base->mutex);
  m->base->text = g_string_assign(m->base->text, output);
  g_mutex_unlock(m->base->mutex);

  return TRUE;
}

int nginx_sleep_time(void* ptr) {
  return 5;
}

void nginx_free(void* ptr) {
  struct nginx_monitor* m = (struct nginx_monitor*)ptr;
  monitor_null_check(m, "nginx_monitor", "free");

  free(m->err);
  g_string_free(m->res, TRUE);
  g_string_free(m->request_str, TRUE);
  curl_easy_cleanup(m->curl);

  base_monitor_free(m->base);

  free(m);
}

int format_nginx_status(GString* res) {
  int code = 0;

  int active;
  GString* output = g_string_new(NULL);

  char** words = g_strsplit(res->str, "\n", -1);

  char** word = words;
  while (*word != 0) {
    g_strstrip(*word);

    if (strstr(*word, "Active connections: ") != NULL) {
      int nread = sscanf(*word, "Active connections: %d", &active);
      if (nread == 1) {
        g_string_append_printf(output, "%d", active);
      } else {
        code = -1;
      }
    }

    word = word + 1;
  }

  g_string_assign(res, output->str);
  g_string_free(output, TRUE);
  g_strfreev(words);
  return code;
}
