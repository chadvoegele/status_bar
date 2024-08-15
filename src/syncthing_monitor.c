/*
* Copyright 2015-2021 Chad Voegele.
* See LICENSE file for license details.
*/

#include <glib.h>
#include <stdlib.h>
#include <string.h>
#include <curl/curl.h>
#include <jq.h>
#include <jv.h>

#include "status_bar.h"
#include "syncthing_monitor.h"
#include "http_download.h"

enum SyncthingState {
  SYNCED,
  SYNCING,
  SCANNING
};

void* syncthing_init(GArray* arguments) {
  monitor_arg_check("syncthing", arguments, "(sync_down_icon, sync_up_icon, synced_icon, sync_error_icon)");

  struct syncthing_monitor* m = malloc(sizeof(struct syncthing_monitor));

  m->base = base_monitor_init(syncthing_sleep_time, syncthing_update_text, syncthing_free);
  m->http_data = http_init();

  m->sync_down_icon = g_string_new(g_array_index(arguments, GString*, 0)->str);
  m->sync_up_icon = g_string_new(g_array_index(arguments, GString*, 1)->str);
  m->synced_icon = g_string_new(g_array_index(arguments, GString*, 2)->str);
  m->sync_error_icon = g_string_new(g_array_index(arguments, GString*, 3)->str);

  m->api_key_header = g_string_new(NULL);
  m->headers = NULL;
  set_headers(m);

  m->url = g_string_new(NULL);
  m->response_buffer = g_string_new(NULL);
  m->last_id = -1;
  m->output = g_string_new(NULL);

  return m;
}

void set_headers(struct syncthing_monitor* m) {
  char* apikey = getenv("STATUS_BAR_SYNCTHING_APIKEY");
  if (apikey == NULL) {
    fprintf(stderr, "No apikey found in environment variable STATUS_BAR_SYNCTHING_APIKEY\n");
    return;
  }

  m->api_key_header = g_string_assign(m->api_key_header, "X-API-Key: ");
  m->api_key_header = g_string_append(m->api_key_header, apikey);
  m->headers = curl_slist_append(m->headers, m->api_key_header->str);
}

void syncthing_result_callback(CURLcode code, void* userdata) {
  struct syncthing_monitor* m = (struct syncthing_monitor*)userdata;

  g_string_set_size(m->output, 0);

  if (code != CURLE_OK) {
      fprintf(stderr, "Invalid syncthing response code: %d\n", code);
      m->base->text = g_string_assign(m->output, m->sync_error_icon->str);
      return;
  }

  jv response = jv_parse_sized(m->response_buffer->str, m->response_buffer->len);
  g_string_set_size(m->response_buffer, 0);

  if (!jv_is_valid(response)) {
    fprintf(stderr, "Invalid syncthing response: %*s\n", (int)m->response_buffer->len, m->response_buffer->str);
    g_string_assign(m->output, m->sync_error_icon->str);
    jv_free(response);
    return;
  }

  enum SyncthingState bar_state = SYNCED;

  gboolean has_completion = FALSE;
  float completion = 999.0;

  jv_array_foreach(response, i, vi) {
    jv vi_type = jv_object_get(jv_copy(vi), jv_string("type"));
    if (strcmp("StateChanged", jv_string_value(vi_type)) == 0) {
      jv vi_data = jv_object_get(jv_copy(vi), jv_string("data"));
      jv vi_to = jv_object_get(jv_copy(vi_data), jv_string("to"));
      const char* state = jv_string_value(vi_to);

      if (strcmp("syncing", state) == 0) {
        bar_state = SYNCING;
      } else if (strcmp("scanning", state) == 0) {
        bar_state = SYNCED;
      }

      jv_free(vi_to);
      jv_free(vi_data);
    } else if (strcmp("FolderCompletion", jv_string_value(vi_type)) == 0) {
      jv vi_data = jv_object_get(jv_copy(vi), jv_string("data"));
      jv vi_completion = jv_object_get(jv_copy(vi_data), jv_string("completion"));

      float this_completion = (float)jv_number_value(vi_completion);
      has_completion = TRUE;
      if (this_completion < completion) {
        completion = this_completion;
      }

      jv_free(vi_completion);
      jv_free(vi_data);
    }

    m->last_id = (int)jv_number_value(jv_object_get(jv_copy(vi), jv_string("id")));
    jv_free(vi_type);
    jv_free(vi);
  }

  if (bar_state == SCANNING || has_completion) {
    g_string_assign(m->output, m->sync_up_icon->str);
  } else if (bar_state == SYNCING) {
    g_string_assign(m->output, m->sync_down_icon->str);
  } else if (bar_state == SYNCED) {
    g_string_assign(m->output, m->synced_icon->str);
  }

  if (has_completion) {
    g_string_append_printf(m->output, "%4.2f%%", completion);
  }

  m->base->text = g_string_assign(m->base->text, m->output->str);

  jv_free(response);
}

size_t syncthing_http_callback(char* ptr, size_t size, size_t nmemb, void* userdata) {
  struct syncthing_monitor* m = (struct syncthing_monitor*)userdata;
  g_string_append_len(m->response_buffer, ptr, nmemb);
  return nmemb;
}

gboolean syncthing_update_text(void* ptr) {
  struct syncthing_monitor* m = (struct syncthing_monitor*)ptr;
  monitor_null_check(m, "syncthing_monitor", "update");

  if (m->headers == NULL) {
    fprintf(stderr, "No headers set!");
    m->base->text = g_string_assign(m->base->text, m->sync_error_icon->str);
    return TRUE;
  }

  const char* URL = "http://localhost:8384/rest/events?timeout=0&events=StateChanged,FolderCompletion";
  m->url = g_string_assign(m->url, URL);
  if (m->last_id != -1) {
    g_string_append_printf(m->url, "&since=%d", m->last_id);
  }
  download_data(m->http_data, m->url->str, m->headers, syncthing_http_callback, m, syncthing_result_callback);

  return TRUE;
}

int syncthing_sleep_time(void* ptr) {
  return 5;
}

void syncthing_free(void* ptr) {
  struct syncthing_monitor* m = (struct syncthing_monitor*)ptr;
  monitor_null_check(m, "syncthing_monitor", "free");
  g_string_free(m->url, TRUE);
  g_string_free(m->output, TRUE);
  g_string_free(m->api_key_header, TRUE);
  g_string_free(m->response_buffer, TRUE);

  g_string_free(m->sync_down_icon, TRUE);
  g_string_free(m->sync_up_icon, TRUE);
  g_string_free(m->synced_icon, TRUE);
  g_string_free(m->sync_error_icon, TRUE);

  curl_slist_free_all(m->headers);
  http_free(m->http_data);
  base_monitor_free(m->base);
  free(m);
}
