/*
* Copyright 2015-2021 Chad Voegele.
* See LICENSE file for license details.
*/

#include <glib.h>
#include <stdlib.h>
#include <curl/curl.h>
#include <string.h>

#include "weather_monitor.h"
#include "http_download.h"
#include "status_bar.h"
#include "base_monitor.h"
#include "configs.h"

void* weather_init(GArray* arguments) {
  monitor_arg_check("weather", arguments, "(icon, location_key, sun_icon, cloudy_icon, rain_icon, storm_icon, snow_icon)");

  struct weather_monitor* m = malloc(sizeof(struct weather_monitor));

  m->base = base_monitor_init(weather_sleep_time, weather_update_text, weather_free);

  char* icon = g_array_index(arguments, GString*, 0)->str;
  m->icon = g_string_new(icon);

  char* weather_loc = g_array_index(arguments, GString*, 1)->str;

  m->request_str = g_string_new(NULL);
  g_string_printf(m->request_str, "https://w1.weather.gov/xml/current_obs/%s.xml", weather_loc);

  m->sun_icon = g_string_new(g_array_index(arguments, GString*, 2)->str);
  m->cloudy_icon = g_string_new(g_array_index(arguments, GString*, 3)->str);
  m->rain_icon = g_string_new(g_array_index(arguments, GString*, 4)->str);
  m->storm_icon = g_string_new(g_array_index(arguments, GString*, 5)->str);
  m->snow_icon = g_string_new(g_array_index(arguments, GString*, 6)->str);

  m->res = g_string_new(NULL);
  m->http_data = http_init();

  m->err = malloc((strlen(m->icon->str) + 2)*sizeof(char));
  sprintf(m->err, "%s!", m->icon->str);

  g_string_printf(m->base->text, "%s", m->icon->str);

  return m;
}

void weather_result_callback(CURLcode code, void* userdata) {
  struct weather_monitor* m = (struct weather_monitor*)userdata;

  if (code != CURLE_OK) {
      m->base->text = g_string_assign(m->base->text, m->err);
  }
}

size_t weather_http_callback(char* ptr, size_t size, size_t nmemb, void* userdata) {
  struct weather_monitor* m = (struct weather_monitor*)userdata;

  g_string_set_size(m->res, 0);
  m->res = g_string_append_len(m->res, ptr, size*nmemb);

  char* output;
  if (format_output(m, m->res) != -1) {
    output = m->res->str;
  } else {
    output = m->err;
  }

  m->base->text = g_string_assign(m->base->text, output);
  return size*nmemb;
}

gboolean weather_update_text(void* ptr) {
  struct weather_monitor* m = (struct weather_monitor*)ptr;
  monitor_null_check(m, "weather_monitor", "update");

  download_data(m->http_data, m->request_str->str, NULL, weather_http_callback, m, weather_result_callback);

  return TRUE;
}

int weather_sleep_time(void* ptr) {
  return 3600;
}

void weather_free(void* ptr) {
  struct weather_monitor* m = (struct weather_monitor*)ptr;
  monitor_null_check(m, "weather_monitor", "free");

  free(m->err);
  g_string_free(m->res, TRUE);
  g_string_free(m->request_str, TRUE);
  http_free(m->http_data);

  g_string_free(m->icon, TRUE);
  g_string_free(m->sun_icon, TRUE);
  g_string_free(m->cloudy_icon, TRUE);
  g_string_free(m->rain_icon, TRUE);
  g_string_free(m->storm_icon, TRUE);
  g_string_free(m->snow_icon, TRUE);

  base_monitor_free(m->base);

  free(m);
}

int format_output(struct weather_monitor* m, GString* res) {
  int return_code = -1;
  int weather_code = -1;
  int temp_code = -1;

  char** words = g_strsplit(res->str, "\n", -1);

  char* weather_str = NULL;
  char* temp_str = NULL;
  char** word = words;
  while (*word != 0) {
    if (strstr(*word, "</weather>") != NULL) {
      weather_str = malloc((1+strlen(*word))*sizeof(char));
      strcpy(weather_str, *word);
      g_strstrip(weather_str);
      weather_code = parse_xml_str(weather_str, "<weather>", "</weather>");
    }
    if (strstr(*word, "<temp_f>") != NULL) {
      temp_str = malloc((1+strlen(*word))*sizeof(char));
      strcpy(temp_str, *word);
      g_strstrip(temp_str);
      temp_code = parse_xml_str(temp_str, "<temp_f>", "</temp_f>");
    }
    word = word + 1;
  }

  if (temp_str != NULL && weather_str != NULL
      && temp_code == 0 && weather_code == 0) {
    res = g_string_truncate(res, 0);
    res = g_string_append(res, m->icon->str);
    res = g_string_append(res, temp_str);
    res = g_string_append(res, " ");
    res = g_string_append(res, convert_weather_text_to_icon(m, weather_str));
    return_code = 0;
  }

  free(weather_str);
  free(temp_str);
  g_strfreev(words);

  return return_code;
}

char* convert_weather_text_to_icon(struct weather_monitor* m, char* weather_text) {
  char* lower_weather_text = strlower(weather_text);
  if (strstr(lower_weather_text, "storm")) {
    return m->storm_icon->str;
  }

  if (strstr(lower_weather_text, "snow")) {
    return m->snow_icon->str;
  }

  if (strstr(lower_weather_text, "rain")) {
    return m->rain_icon->str;
  }

  if (strstr(lower_weather_text, "cloudy")) {
    return m->cloudy_icon->str;
  }

  if (strstr(lower_weather_text, "sun") || strstr(lower_weather_text, "fair")) {
    return m->sun_icon->str;
  }

  fprintf(stderr, "Unknown weather: %s\n", lower_weather_text);
  return "?";
}

char* strlower(char* str) {
  char* start = str;
  while (*str != '\0') {
    *str = tolower(*str);
    str++;
  }
  return start;
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
