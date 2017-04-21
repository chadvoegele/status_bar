/*
* Copyright 2015-2017 Chad Voegele.
* See LICENSE file for license details.
*/

#include <stdlib.h>
#include <glib.h>
#include <string.h>

#include "configs.h"
#include "base_monitor.h"

#include "clock_monitor.h"
#include "thinkpad_temp_monitor.h"
#include "thinkpad_fan_monitor.h"
#include "battery_monitor.h"
#include "memory_monitor.h"
#include "net_monitor.h"
#include "dropbox_monitor.h"
#include "sp500_monitor.h"
#include "weather_monitor.h"
#include "core_temp_monitor.h"
#include "it87_temp_monitor.h"
#include "it87_fan_monitor.h"
#include "nct6775_temp_monitor.h"
#include "nct6775_fan_monitor.h"
#include "volume_monitor.h"
#include "nginx_monitor.h"
#include "cpu_usage_monitor.h"

void load_configs(GKeyFile* configs) {
  GString* config_path = g_string_new(NULL);
  char* path, *home;

  if ((path = getenv("XDG_CONFIG_HOME")) != NULL) {
    g_string_printf(config_path, "%s/status_bar.conf", path);
  } else if ((home = getenv("HOME")) != NULL) {
    g_string_printf(config_path, "%s/.config/status_bar.conf", home);
  } else {
    fprintf(stderr, "Either XDG_CONFIG_HOME or HOME must defined for config file.\n");
    exit(EXIT_FAILURE);
  }

  GError* error = NULL;
  if (!g_key_file_load_from_file(configs, config_path->str, G_KEY_FILE_NONE, &error)) {
    fprintf(stderr, "%s: %s\n", error->message, config_path->str);
    g_error_free(error);
  }

  g_string_free(config_path, TRUE);
}

void fail_on_error(GError* error) {
  if (error != NULL) {
    fprintf(stderr, "%s\n", error->message);
    g_error_free(error);
    exit(EXIT_FAILURE);
  }
}

void build_display_cmd_str(GKeyFile* configs, GString* str) {
  GError* error = NULL;
  char* fg_color = g_key_file_get_string(configs, "configs", "fgcolor", &error);
  fail_on_error(error);

  error = NULL;
  char* bg_color = g_key_file_get_string(configs, "configs", "bgcolor", &error);
  fail_on_error(error);

  error = NULL;
  char* width = g_key_file_get_string(configs, "configs", "width", &error);

  error = NULL;
  char* height = g_key_file_get_string(configs, "configs", "height", &error);

  error = NULL;
  gsize font_length;
  char** fonts = g_key_file_get_string_list(configs, "configs", "fonts", &font_length, &error);
  fail_on_error(error);

  str = g_string_truncate(str, 0);
  g_string_append_printf(str, "lemonbar");
  g_string_append_printf(str, " -F \\%s", fg_color);
  g_string_append_printf(str, " -B \\%s", bg_color);
  g_string_append_printf(str, " -g \"");
  if (width) {
    g_string_append_printf(str, "%s", width);
  }
  g_string_append_printf(str, "x");
  if (height) {
    g_string_append_printf(str, "%s", height);
  }
  g_string_append_printf(str, "+0+0\"");
  for (gsize i = 0; i < font_length; i++) {
    g_string_append_printf(str, " -f \"%s\"", fonts[i]);
  }

  g_free(fg_color);
  g_free(bg_color);
  g_free(width);
  g_free(height);
  g_strfreev(fonts);
}

void init_monitors(GKeyFile* configs, gsize* length, void*** monitors) {
  char** monitor_configs;
  GError* error = NULL;
  monitor_configs = g_key_file_get_string_list(configs, "configs", "monitors", length, &error);
  fail_on_error(error);

  *monitors = malloc((*length)*sizeof(void*));

  for (int i = 0; i < *length; i++) {
    void* m = convert_string_to_monitor(monitor_configs[i], configs);
    (*monitors)[i] = m;
  }

  g_strfreev(monitor_configs);
}

void* convert_string_to_monitor(char* str, GKeyFile* configs) {
  if (strcmp("net", str) == 0) {
    return net_init(configs);
  } else if (strcmp("clock", str) == 0) {
    return clock_init(configs);
  } else if (strcmp("dropbox", str) == 0) {
    return dropbox_init(configs);
  } else if (strcmp("memory", str) == 0) {
    return memory_init(configs);
  } else if (strcmp("sp500", str) == 0) {
    return sp500_init(configs);
  } else if (strcmp("battery", str) == 0) {
    return battery_init(configs);
  } else if (strcmp("thinkpad_fan", str) == 0) {
    return thinkpad_fan_init(configs);
  } else if (strcmp("thinkpad_temp", str) == 0) {
    return thinkpad_temp_init(configs);
  } else if (strcmp("weather", str) == 0) {
    return weather_init(configs);
  } else if (strcmp("core_temp", str) == 0) {
    return core_temp_init(configs);
  } else if (strcmp("it87_temp", str) == 0) {
    return it87_temp_init(configs);
  } else if (strcmp("it87_fan", str) == 0) {
    return it87_fan_init(configs);
  } else if (strcmp("nct6775_temp", str) == 0) {
    return nct6775_temp_init(configs);
  } else if (strcmp("nct6775_fan", str) == 0) {
    return nct6775_fan_init(configs);
  } else if (strcmp("volume", str) == 0) {
    return volume_init(configs);
  } else if (strcmp("nginx", str) == 0) {
    return nginx_init(configs);
  } else if (strcmp("cpu_usage", str) == 0) {
    return cpu_usage_init(configs);
  } else {
    fprintf(stderr, "Monitor %s not found.\n", str);
    exit(EXIT_FAILURE);
  }
}
