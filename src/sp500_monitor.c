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

  m->base->text = g_string_assign(m->base->text, output);

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
  char* apikey = getenv("STATUS_BAR_SP500_APIKEY");
  if (apikey == NULL) {
    fprintf(stderr, "No apikey found in environment variable STATUS_BAR_SP500_APIKEY\n");
    return -1;
  }

  const char* url = "https://www.alphavantage.co/query?function=GLOBAL_QUOTE&symbol=SPY&apikey=%s";
  char* quote_request = malloc(sizeof(char*)*(strlen(url)+strlen(apikey)));
  int quote_request_ret = sprintf(quote_request, url, apikey);
  if (quote_request_ret < 0) {
    return -1;
  }

  GString* quote = g_string_new(NULL);
  CURLcode quote_code = download_data(curl, quote_request, quote);

  if (quote_code != CURLE_OK) {
    return -1;
  }

  float price;
  float change_percent;

  char* price_key = "\"05. price\":";
  char* price_start = strstr(quote->str, price_key);
  if (price_start == NULL) {
    return -1;
  }

  int nread_price = sscanf(price_start + strlen(price_key), " \"%f\"", &price);
  if (nread_price == 0) {
    return -1;
  }

  char* change_key = "\"10. change percent\":";
  char* change_start = strstr(quote->str, change_key);
  if (change_start == NULL) {
    return -1;
  }

  int nread_change = sscanf(change_start + strlen(change_key), " \"%f%%\"", &change_percent);
  if (nread_change == 0) {
    return -1;
  }

  g_string_printf(res, "%s%.2f (%.2f%%)", icon->str, price, change_percent);

  free(quote_request);
  g_string_free(quote, TRUE);

  return 0;
}
