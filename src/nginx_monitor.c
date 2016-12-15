/*
* Copyright 2015 Chad Voegele.
* See LICENSE file for license details.
*/

#include <glib.h>
#include <stdlib.h>
#include <string.h>
#include <curl/curl.h>

#include "status_bar.h"
#include "nginx_monitor.h"
#include "http_download.h"
#include "configs.h"

struct monitor_fns nginx_monitor_fns() {
  struct monitor_fns f;
  f.init = nginx_init;
  f.sleep_time = nginx_sleep_time;
  f.update_text = nginx_update_text;
  f.free = nginx_free;

  return f;
}

void* nginx_init(GString* bar_text, GMutex* mutex, GKeyFile* configs) {
  struct nginx_monitor* m = malloc(sizeof(struct nginx_monitor));

  m->bar_text = bar_text;
  m->mutex = mutex;

  GError* error = NULL;
  char* status_uri = g_key_file_get_string(
      configs, "configs", "nginx_status_uri", &error);
  fail_on_error(error);

  m->request_str = g_string_new(NULL);
  g_string_printf(m->request_str, "%s", status_uri);
  g_free(status_uri);

  m->res = g_string_new(NULL);
  m->curl = curl_easy_init();
  m->icon = "";

  m->err = malloc((strlen(m->icon) + 2)*sizeof(char));
  sprintf(m->err, "%s!", m->icon);

  return m;
}

gboolean nginx_update_text(void* ptr) {
  struct nginx_monitor* m;
  if ((m = (struct nginx_monitor*)ptr) == NULL) {
    fprintf(stderr, "nginx monitor not received in update_text.\n");
    exit(EXIT_FAILURE);
  }

  CURLcode code = download_data(m->curl, m->request_str->str, m->res);
  char* output;

  if (code == CURLE_OK && format_nginx_status(m->res, m->icon) != -1) {
    output = m->res->str;
  } else {
    output = m->err;
  }

  g_mutex_lock(m->mutex);
  m->bar_text = g_string_assign(m->bar_text, output);
  g_mutex_unlock(m->mutex);

  return TRUE;
}

int nginx_sleep_time(void* ptr) {
  struct nginx_monitor* m;
  if ((m = (struct nginx_monitor*)ptr) == NULL) {
    fprintf(stderr, "nginx monitor not received in sleep_time.\n");
    exit(EXIT_FAILURE);
  }

  return 5;
}

void nginx_free(void* ptr) {
  struct nginx_monitor* m;
  if ((m = (struct nginx_monitor*)ptr) == NULL) {
    fprintf(stderr, "nginx monitor not received in close.\n");
    exit(EXIT_FAILURE);
  }

  free(m->err);
  g_string_free(m->res, TRUE);
  g_string_free(m->request_str, TRUE);
  curl_easy_cleanup(m->curl);
  free(m);
}

int format_nginx_status(GString* res, char* icon) {
  int code = 0;

  int active, reading, writing, waiting;
  GString* output = g_string_new(icon);

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
    } else if (strstr(*word, "Reading: ") != NULL) {
      int nread = sscanf(*word, "Reading: %d Writing: %d Waiting: %d",
          &reading, &writing, &waiting);
      if (nread == 3) {
        g_string_append_printf(output, " %d %d %d", reading, writing, waiting);
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
