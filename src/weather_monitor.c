/*
* Copyright 2015 Chad Voegele.
* See LICENSE file for license details.
*/

#include <glib.h>
#include <stdlib.h>
#include <curl/curl.h>
#include <string.h>

#include "weather_monitor.h"
#include "http_download.h"
#include "status_bar.h"
#include "configs.h"

struct monitor_fns weather_monitor_fns() {
  struct monitor_fns f;
  f.init = weather_init;
  f.sleep_time = weather_sleep_time;
  f.update_text = weather_update_text;
  f.free = weather_free;

  return f;
}

void* weather_init(GString* bar_text, GMutex* mutex, GKeyFile* configs) {
  struct weather_monitor* m = malloc(sizeof(struct weather_monitor));

  m->bar_text = bar_text;
  m->mutex = mutex;

  GError* error = NULL;
  char* weather_loc = g_key_file_get_string(
      configs, "configs", "weather_loc", &error);
  fail_on_error(error);

  m->request_str = g_string_new(NULL);
  g_string_printf(m->request_str,
      "http://w1.weather.gov/xml/current_obs/%s.xml", weather_loc);
  g_free(weather_loc);

  m->res = g_string_new(NULL);
  m->curl = curl_easy_init();
  m->icon = "^i(/usr/share/status_bar/temp.xbm)";

  m->err = malloc(strlen(m->icon) + 1);
  sprintf(m->err, "%s!", m->icon);

  return m;
}

gboolean weather_update_text(void* ptr) {
  struct weather_monitor* m;
  if ((m = (struct weather_monitor*)ptr) != NULL) {
    char* output;

    CURLcode code = download_data(m->curl, m->request_str->str, m->res);

    if (code == CURLE_OK && format_output(m->res, m->icon) != -1) {
      output = m->res->str;
    } else {
      output = m->err;
    }

    g_mutex_lock(m->mutex);
    m->bar_text = g_string_assign(m->bar_text, output);
    g_mutex_unlock(m->mutex);

    return TRUE;

  } else {
    fprintf(stderr, "Weather monitor not received in update.\n");
    exit(EXIT_FAILURE);
  }
}

int weather_sleep_time(void* ptr) {
  struct weather_monitor* m;
  if ((m = (struct weather_monitor*)ptr) != NULL) {
    return 3600;
  } else {
    fprintf(stderr, "Weather monitor not received in sleep_time.\n");
    exit(EXIT_FAILURE);
  }
}

void weather_free(void* ptr) {
  struct weather_monitor* m;
  if ((m = (struct weather_monitor*)ptr) != NULL) {
    free(m->err);
    g_string_free(m->res, TRUE);
    g_string_free(m->request_str, TRUE);
    curl_easy_cleanup(m->curl);

    free(m);

  } else {
    fprintf(stderr, "Weather monitor not received in close.\n");
    exit(EXIT_FAILURE);
  }
}

int format_output(GString* res, char* icon) {
  int code = -1;
  int weather_code = -1;
  int temp_code = -1;

  char** words = g_strsplit(res->str, "\n", -1);

  char* weather_str = NULL;
  char* temp_str = NULL;
  char** word = words;
  while (*word != 0) {
    if (strstr(*word, "</weather>") != NULL) {
      weather_str = malloc(strlen(*word)*sizeof(char));
      strcpy(weather_str, *word);
      g_strstrip(weather_str);
      weather_code = parse_xml_str(weather_str, "<weather>", "</weather>");
    }
    if (strstr(*word, "<temp_f>") != NULL) {
      temp_str = malloc(strlen(*word)*sizeof(char));
      strcpy(temp_str, *word);
      g_strstrip(temp_str);
      temp_code = parse_xml_str(temp_str, "<temp_f>", "</temp_f>");
    }
    word = word + 1;
  }

  if (temp_str != NULL && weather_str != NULL
      && temp_code == 0 && weather_code == 0) {
    res = g_string_truncate(res, 0);
    res = g_string_append(res, icon);
    res = g_string_append(res, temp_str);
    res = g_string_append(res, ", ");
    res = g_string_append(res, weather_str);
    code = 0;
  }

  free(weather_str);
  free(temp_str);
  g_strfreev(words);

  return code;
}

int parse_xml_str(char* str, const char* start_tag, const char* end_tag) {
  char* find, *end, *start;
  find = strstr(str, start_tag);
  end = strstr(str, end_tag);
  if (find == NULL || end == NULL) {
    return -1;
  }

  char* buffer = malloc(strlen(str)*sizeof(char));
  start = find + strlen(start_tag);
  strncpy(buffer, start, end-start);
  buffer[end-start] = '\0';
  strcpy(str, buffer);

  free(buffer);

  return 0;
}
