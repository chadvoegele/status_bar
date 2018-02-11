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
  monitor_arg_check("sp500", arguments, "(icon)");

  struct sp500_monitor* m = malloc(sizeof(struct sp500_monitor));

  m->base = base_monitor_init(sp500_sleep_time, sp500_update_text, sp500_free);

  char* icon = g_array_index(arguments, GString*, 0)->str;
  m->icon = g_string_new(icon);

  m->res = g_string_new(NULL);
  m->curl = curl_easy_init();

  m->err = malloc((strlen(m->icon->str) + 2)*sizeof(char));
  sprintf(m->err, "%s!", m->icon->str);

  return m;
}

gboolean sp500_update_text(void* ptr) {
  struct sp500_monitor* m = (struct sp500_monitor*)ptr;
  monitor_null_check(m, "sp500_monitor", "update");

  char* output;
  if (!format_price(m->curl, m->res, m->icon)) {
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
  return 300;
}

void sp500_free(void* ptr) {
  struct sp500_monitor* m = (struct sp500_monitor*)ptr;
  monitor_null_check(m, "sp500_monitor", "free");

  g_string_free(m->icon, TRUE);

  free(m->err);
  g_string_free(m->res, TRUE);
  curl_easy_cleanup(m->curl);

  base_monitor_free(m->base);

  free(m);
}

int format_price(CURL* curl, GString* res, GString* icon) {
  GString* quote = g_string_new(NULL);
  char* quote_request = "https://api.iextrading.com/1.0/stock/spy/quote?displayPercent=true";
  CURLcode quote_code = download_data(curl, quote_request, quote);

  if (quote_code != CURLE_OK) {
    return -1;
  }

  float close;
  float change_percent;

  char* close_key = "\"close\":";
  char* close_start = strstr(quote->str, close_key);
  if (close_start == NULL) {
    return -1;
  }

  int nread_close = sscanf(close_start + strlen(close_key), "%f", &close);
  if (nread_close == 0) {
    return -1;
  }

  char* change_key = "\"changePercent\":";
  char* change_start = strstr(quote->str, change_key);
  if (change_start == NULL) {
    return -1;
  }

  int nread_change = sscanf(change_start + strlen(change_key), "%f", &change_percent);
  if (nread_change == 0) {
    return -1;
  }

  g_string_printf(res, "%s%.2f (%.2f%%)", icon->str, close, change_percent);

  g_string_free(quote, TRUE);

  return 0;
}
