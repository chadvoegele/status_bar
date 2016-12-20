/*
* Copyright 2015 Chad Voegele.
* See LICENSE file for license details.
*/

#include <glib.h>
#include <stdlib.h>
#include <string.h>
#include <curl/curl.h>

#include "status_bar.h"
#include "sp500_monitor.h"
#include "http_download.h"

struct monitor_fns sp500_monitor_fns() {
  struct monitor_fns f;
  f.init = sp500_init;
  f.sleep_time = sp500_sleep_time;
  f.update_text = sp500_update_text;
  f.free = sp500_free;

  return f;
}

void* sp500_init(GString* bar_text, GMutex* mutex, GKeyFile* configs) {
  struct sp500_monitor* m = malloc(sizeof(struct sp500_monitor));

  m->bar_text = bar_text;
  m->mutex = mutex;

  m->request_str = g_string_new(NULL);
  g_string_printf(m->request_str,
      "http://download.finance.yahoo.com/d/quotes.csv?s=%%5EGSPC&f=l1c");

  m->res = g_string_new(NULL);
  m->curl = curl_easy_init();
  m->icon = "";

  m->err = malloc((strlen(m->icon) + 2)*sizeof(char));
  sprintf(m->err, "%s!", m->icon);

  return m;
}

gboolean sp500_update_text(void* ptr) {
  struct sp500_monitor* m = (struct sp500_monitor*)ptr;
  monitor_null_check(m, "sp500_monitor", "update");

  CURLcode code = download_data(m->curl, m->request_str->str, m->res);
  char* output;

  if (code == CURLE_OK && format_price(m->res, m->icon) != -1) {
    output = m->res->str;
  } else {
    output = m->err;
  }

  g_mutex_lock(m->mutex);
  m->bar_text = g_string_assign(m->bar_text, output);
  g_mutex_unlock(m->mutex);

  return TRUE;
}

int sp500_sleep_time(void* ptr) {
  return 60;
}

void sp500_free(void* ptr) {
  struct sp500_monitor* m = (struct sp500_monitor*)ptr;
  monitor_null_check(m, "sp500_monitor", "free");

  free(m->err);
  g_string_free(m->res, TRUE);
  g_string_free(m->request_str, TRUE);
  curl_easy_cleanup(m->curl);
  free(m);
}

int format_price(GString* res, char* icon) {
  int code = -1;

  char* buf1 = malloc(strlen(res->str)*sizeof(char));
  char* buf2 = malloc(strlen(res->str)*sizeof(char));

  int nread = sscanf(res->str, "%[0-9.],%*s - %[+-0-9.]", buf1, buf2);

  if (nread == 2) {
    g_string_printf(res, "%s%s (%s%%)", icon, buf1, buf2);
    code = 0;
  }

  free(buf1);
  free(buf2);

  return code;
}
