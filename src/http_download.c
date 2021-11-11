/*
* Copyright 2015-2021 Chad Voegele.
* See LICENSE file for license details.
*/

#include <glib.h>
#include <glib-unix.h>
#include <curl/curl.h>

#include "http_download.h"

struct closure {
  void (*function)(CURLcode, void*);
  void* data;
};

struct socket_info {
  GIOChannel* channel;
  guint watch;
};

void clean_curl(struct http_data* http_data) {
  CURLMsg *message;
  int pending;

  while ((message = curl_multi_info_read(http_data->curl, &pending))) {
    switch (message->msg) {
    case CURLMSG_LAST:
    case CURLMSG_NONE:
      break;
    case CURLMSG_DONE:
      CURL* easy_handle = message->easy_handle;

      struct closure* closure;
      curl_easy_getinfo(easy_handle, CURLINFO_PRIVATE, &closure);
      closure->function((message->data).result, closure->data);
      g_free(closure);

      curl_multi_remove_handle(http_data->curl, easy_handle);
      curl_easy_cleanup(easy_handle);
      break;
    }
  }
}

gboolean on_timeout(void* userp) {
  struct http_data* http_data = (struct http_data*)userp;

  int running;
  curl_multi_socket_action(http_data->curl, CURL_SOCKET_TIMEOUT, 0, &running);
  return FALSE;
}

gboolean handle_timeout(CURLM *curl, long timeout_ms, void *userp) {
  struct http_data* http_data = (struct http_data*)userp;

  if (http_data->id > 0) {
    g_source_remove(http_data->id);
    http_data->id = 0;
    clean_curl(http_data);
  }

  if (timeout_ms == -1) {
    return 0;
  }

  http_data->id = g_timeout_add(timeout_ms, on_timeout, http_data);
  return 0;
}

gboolean event_cb(GIOChannel* ch, GIOCondition condition, gpointer data)
{
  struct http_data* http_data = (struct http_data*)data;

  int fd = g_io_channel_unix_get_fd(ch);
  int action =
    ((condition & G_IO_IN) ? CURL_CSELECT_IN : 0) |
    ((condition & G_IO_OUT) ? CURL_CSELECT_OUT : 0);

  int still_running;
  curl_multi_socket_action(http_data->curl, fd, action, &still_running);

  clean_curl(http_data);

  if (still_running > 0) {
    return TRUE;
  }

  if (http_data->id > 0) {
    g_source_remove(http_data->id);
  }

  return FALSE;
}

void remsock(struct socket_info* s)
{
  if (!s) {
    return;
  }

  if (s->watch > 0) {
    g_io_channel_unref(s->channel);
    g_source_remove(s->watch);
  }

  g_free(s);
}

void setsock(struct socket_info* socket_info, curl_socket_t s, int action, struct http_data* http_data)
{
  GIOCondition kind =
    ((action & CURL_POLL_IN) ? G_IO_IN : 0) |
    ((action & CURL_POLL_OUT) ? G_IO_OUT : 0);

  if (socket_info->watch > 0) {
    g_source_remove(socket_info->watch);
  }
  socket_info->watch = g_io_add_watch(socket_info->channel, kind, event_cb, http_data);
}

static void addsock(curl_socket_t s, int action, struct http_data* http_data)
{
  struct socket_info* socket_info = g_new(struct socket_info, 1);

  socket_info->channel = g_io_channel_unix_new(s);
  socket_info->watch = 0;
  setsock(socket_info, s, action, http_data);
  curl_multi_assign(http_data->curl, s, socket_info);
}

int handle_socket(CURL *easy, curl_socket_t s, int what, void *userp, void *socketp) {
  struct http_data* http_data = (struct http_data*)userp;
  struct socket_info *socket_info = (struct socket_info*) socketp;

  if(what == CURL_POLL_REMOVE) {
    remsock(socket_info);
    return 0;
  }

  if(!socket_info) {
    addsock(s, what, http_data);
  }
  else {
    setsock(socket_info, s, what, http_data);
  }

  return 0;
}

gpointer http_init() {
  struct http_data* http_data = g_new(struct http_data, 1);
  http_data->id = 0;
  http_data->curl = curl_multi_init();

  curl_multi_setopt(http_data->curl, CURLMOPT_SOCKETFUNCTION, handle_socket);
  curl_multi_setopt(http_data->curl, CURLMOPT_SOCKETDATA, http_data);
  curl_multi_setopt(http_data->curl, CURLMOPT_TIMERFUNCTION, handle_timeout);
  curl_multi_setopt(http_data->curl, CURLMOPT_TIMERDATA, http_data);

  return http_data;
}

void http_free(struct http_data* http_data) {
  curl_multi_cleanup(http_data->curl);
  g_free(http_data);
}

CURLcode download_data(struct http_data* http_data, char* request_str, void* callback, void* userdata, void* result_callback) {
  struct closure* closure = g_new(struct closure, 1);
  closure->function = result_callback;
  closure->data = userdata;

  CURL* easy_curl = curl_easy_init();
  curl_easy_setopt(easy_curl, CURLOPT_USERAGENT, "c");
  curl_easy_setopt(easy_curl, CURLOPT_URL, request_str);
  curl_easy_setopt(easy_curl, CURLOPT_TIMEOUT, 10L);
  curl_easy_setopt(easy_curl, CURLOPT_WRITEFUNCTION, callback);
  curl_easy_setopt(easy_curl, CURLOPT_WRITEDATA, userdata);
  curl_easy_setopt(easy_curl, CURLOPT_PRIVATE, closure);
  curl_multi_add_handle(http_data->curl, easy_curl);
  return 1;
}
