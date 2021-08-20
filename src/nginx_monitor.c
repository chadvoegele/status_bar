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

void* nginx_init(GArray* arguments) {
  monitor_arg_check("nginx", arguments, "(icon, status_uri)");

  struct nginx_monitor* m = malloc(sizeof(struct nginx_monitor));

  m->base = base_monitor_init(nginx_sleep_time, nginx_update_text, nginx_free);

  char* icon = g_array_index(arguments, GString*, 0)->str;
  m->icon = g_string_new(icon);

  char* status_uri = g_array_index(arguments, GString*, 1)->str;
  m->request_str = g_string_new(status_uri);

  m->res = g_string_new(NULL);
  m->http_data = http_init();

  m->err = malloc((strlen(m->icon->str) + 2)*sizeof(char));
  sprintf(m->err, "%s!", m->icon->str);

  g_string_printf(m->base->text, "%s", m->icon->str);

  return m;
}

void nginx_result_callback(CURLcode code, void* userdata) {
  struct nginx_monitor* m = (struct nginx_monitor*)userdata;

  if (code != CURLE_OK) {
      m->base->text = g_string_assign(m->base->text, m->err);
  }
}

size_t nginx_http_callback(char* ptr, size_t size, size_t nmemb, void* userdata) {
  struct nginx_monitor* m = (struct nginx_monitor*)userdata;

  g_string_set_size(m->res, 0);
  m->res = g_string_append_len(m->res, ptr, size*nmemb);

  char* output;
  if (format_nginx_status(m->res, m->icon) != -1) {
    output = m->res->str;
    m->base->text = g_string_assign(m->base->text, output);
  } else {
    output = m->err;
  }

  return size*nmemb;
}

gboolean nginx_update_text(void* ptr) {
  struct nginx_monitor* m = (struct nginx_monitor*)ptr;
  monitor_null_check(m, "nginx_monitor", "update");

  download_data(m->http_data, m->request_str->str, nginx_http_callback, m, nginx_result_callback);

  return TRUE;
}

int nginx_sleep_time(void* ptr) {
  return 5;
}

void nginx_free(void* ptr) {
  struct nginx_monitor* m = (struct nginx_monitor*)ptr;
  monitor_null_check(m, "nginx_monitor", "free");

  g_string_free(m->icon, TRUE);
  g_string_free(m->request_str, TRUE);

  free(m->err);
  g_string_free(m->res, TRUE);
  http_free(m->http_data);

  base_monitor_free(m->base);

  free(m);
}

int format_nginx_status(GString* res, GString* icon) {
  int code = -1;

  int active;
  GString* output = g_string_new(icon->str);

  char** words = g_strsplit(res->str, "\n", -1);

  char** word = words;
  while (*word != 0) {
    g_strstrip(*word);

    if (strstr(*word, "Active connections: ") != NULL) {
      int nread = sscanf(*word, "Active connections: %d", &active);
      if (nread == 1) {
        g_string_append_printf(output, "%d", active);
        code = 0;
      }
    }

    word = word + 1;
  }

  g_string_assign(res, output->str);
  g_string_free(output, TRUE);
  g_strfreev(words);
  return code;
}
