/*
* Copyright 2015-2021 Chad Voegele.
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

  m->url = build_url();
  m->http_data = http_init();

  m->err = malloc((strlen(m->icon->str) + 2)*sizeof(char));
  sprintf(m->err, "%s!", m->icon->str);

  g_string_printf(m->base->text, "%s", m->icon->str);

  return m;
}

void sp500_result_callback(CURLcode code, void* userdata) {
  struct sp500_monitor* m = (struct sp500_monitor*)userdata;

  if (code != CURLE_OK) {
      m->base->text = g_string_assign(m->base->text, m->err);
  }
}

size_t sp500_http_callback(char* ptr, size_t size, size_t nmemb, void* userdata) {
  struct sp500_monitor* m = (struct sp500_monitor*)userdata;

  char* output;
  GString* res = g_string_new(NULL);
  if (!format_price(res, ptr, m->icon)) {
    output = res->str;
  } else {
    output = m->err;
  }

  m->base->text = g_string_assign(m->base->text, output);
  g_string_free(res, TRUE);
  return size*nmemb;
}

gboolean sp500_update_text(void* ptr) {
  struct sp500_monitor* m = (struct sp500_monitor*)ptr;
  monitor_null_check(m, "sp500_monitor", "update");

  if (m->url == NULL) {
    m->base->text = g_string_assign(m->base->text, m->err);
    return TRUE;
  }

  download_data(m->http_data, m->url->str, NULL, sp500_http_callback, m, sp500_result_callback);

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
  if (m->url != NULL) {
    g_string_free(m->url, TRUE);
  }
  http_free(m->http_data);

  base_monitor_free(m->base);

  free(m);
}

GString* build_url() {
  char* apikey = getenv("STATUS_BAR_SP500_APIKEY");
  if (apikey == NULL) {
    fprintf(stderr, "No apikey found in environment variable STATUS_BAR_SP500_APIKEY\n");
    return NULL;
  }

  GString* url = g_string_new(NULL);
  const char* url_format = "https://www.alphavantage.co/query?function=GLOBAL_QUOTE&symbol=SPY&apikey=%s";
  g_string_printf(url, url_format, apikey);
  return url;
}

int format_price(GString* res, char* quote, GString* icon) {
  float price;
  float change_percent;

  char* price_key = "\"05. price\":";
  char* price_start = strstr(quote, price_key);
  if (price_start == NULL) {
    return -1;
  }

  int nread_price = sscanf(price_start + strlen(price_key), " \"%f\"", &price);
  if (nread_price == 0) {
    return -1;
  }

  char* change_key = "\"10. change percent\":";
  char* change_start = strstr(quote, change_key);
  if (change_start == NULL) {
    return -1;
  }

  int nread_change = sscanf(change_start + strlen(change_key), " \"%f%%\"", &change_percent);
  if (nread_change == 0) {
    return -1;
  }

  g_string_printf(res, "%s%.2f (%.2f%%)", icon->str, price, change_percent);

  return 0;
}
