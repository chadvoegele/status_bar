/*
* Copyright 2015-2017 Chad Voegele.
* See LICENSE file for license details.
*/

#include <glib.h>
#include <stdlib.h>
#include <string.h>
#include <curl/curl.h>

#include "status_bar.h"
#include "sp500_monitor.h"
#include "http_download.h"

void* sp500_init(GArray* arguments) {
  struct sp500_monitor* m = malloc(sizeof(struct sp500_monitor));

  m->base = base_monitor_init(sp500_sleep_time, sp500_update_text, sp500_free);

  m->request_str = g_string_new(NULL);
  g_string_printf(m->request_str,
      "http://download.finance.yahoo.com/d/quotes.csv?s=%%5EGSPC&f=l1c");

  m->res = g_string_new(NULL);
  m->curl = curl_easy_init();

  m->err = malloc(2*sizeof(char));
  sprintf(m->err, "!");

  return m;
}

gboolean sp500_update_text(void* ptr) {
  struct sp500_monitor* m = (struct sp500_monitor*)ptr;
  monitor_null_check(m, "sp500_monitor", "update");

  CURLcode code = download_data(m->curl, m->request_str->str, m->res);
  char* output;

  if (code == CURLE_OK && format_price(m->res) != -1) {
    output = m->res->str;
  } else {
    output = m->err;
  }

  g_mutex_lock(m->base->mutex);
  m->base->text = g_string_assign(m->base->text, output);
  g_mutex_unlock(m->base->mutex);

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

  base_monitor_free(m->base);

  free(m);
}

int format_price(GString* res) {
  int code = -1;

  char* buf1 = malloc(strlen(res->str)*sizeof(char));
  char* buf2 = malloc(strlen(res->str)*sizeof(char));

  int nread = sscanf(res->str, "%[0-9.],%*s - %[+-0-9.]", buf1, buf2);

  if (nread == 2) {
    g_string_printf(res, "%s (%s%%)", buf1, buf2);
    code = 0;
  }

  free(buf1);
  free(buf2);

  return code;
}
