/*
* Copyright 2015 Chad Voegele.
* See LICENSE file for license details.
*/

#include <glib.h>
#include <stdlib.h>
#include <string.h>
#include <curl/curl.h>

#include "status_bar.h"
#include "monitor_utils.h"
#include "sp500_monitor.h"
#include "http_download.h"

void* sp500_monitor(struct monitor_refs* mr) {
  struct sp500_monitor m;
  void* ptr;
  ptr = monitor_loop(mr, &m, sp500_init, sp500_update_text,
      sp500_sleep_time, sp500_close);
  return ptr;
}

void sp500_init(void* ptr1, void* ptr2) {
  struct sp500_monitor* m;
  struct monitor_refs* mr;
  if ((m = (struct sp500_monitor*)ptr2) != NULL
      && (mr = (struct monitor_refs*)ptr1) != NULL) {
    m->request_str = g_string_new(NULL);
    g_string_printf(m->request_str,
        "http://download.finance.yahoo.com/d/quotes.csv?s=%%5EGSPC&f=l1c");

    m->res = g_string_new(NULL);
    m->curl = curl_easy_init();
    m->icon = "^i(/usr/share/status_bar/load.xbm)";

    m->err = malloc(strlen(m->icon) + 1);
    sprintf(m->err, "%s!", m->icon);

  } else {
    fprintf(stderr, "SP500 monitor not received in init or monitor_refs not recevied in init.\n");
    exit(EXIT_FAILURE);
  }
}

const char* sp500_update_text(void* ptr) {
  struct sp500_monitor* m;
  if ((m = (struct sp500_monitor*)ptr) != NULL) {
    CURLcode code = download_data(m->curl, m->request_str->str, m->res);
    char* output;

    if (code == CURLE_OK && format_price(m->res, m->icon) != -1) {
      output = m->res->str;
    } else {
      output = m->err;
    }

    return output;

  } else {
    fprintf(stderr, "sp500 monitor not received in update_text.\n");
    exit(EXIT_FAILURE);
  }
}

int sp500_sleep_time(void* ptr) {
  struct sp500_monitor* m;
  if ((m = (struct sp500_monitor*)ptr) != NULL) {
    return 60;
  } else {
    fprintf(stderr, "sp500 monitor not received in sleep_time.\n");
    exit(EXIT_FAILURE);
  }
}

void sp500_close(void* ptr) {
  struct sp500_monitor* m;
  if ((m = (struct sp500_monitor*)ptr) != NULL) {
    free(m->err);
    g_string_free(m->res, TRUE);
    g_string_free(m->request_str, TRUE);
    curl_easy_cleanup(m->curl);

  } else {
    fprintf(stderr, "sp500 monitor not received in close.\n");
    exit(EXIT_FAILURE);
  }
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
