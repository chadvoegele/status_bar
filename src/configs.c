/*
* Copyright 2015 Chad Voegele.
* See LICENSE file for license details.
*/

#include <stdlib.h>
#include <glib.h>
#include <string.h>

#include "status_bar.h"
#include "configs.h"

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

void build_dzen_str(GKeyFile* configs, GString* str) {
  GError* error = NULL;
  char* fg_color = g_key_file_get_string(configs, "configs", "fgcolor", &error);
  fail_on_error(error);

  error = NULL;
  char* bg_color = g_key_file_get_string(configs, "configs", "bgcolor", &error);
  fail_on_error(error);

  error = NULL;
  char* width = g_key_file_get_string(configs, "configs", "width", &error);
  fail_on_error(error);

  error = NULL;
  char* height = g_key_file_get_string(configs, "configs", "height", &error);
  fail_on_error(error);

  error = NULL;
  char* font_size = g_key_file_get_string(configs, "configs", "font_size", &error);
  fail_on_error(error);

  error = NULL;
  char* icon_font_size = g_key_file_get_string(configs, "configs", "icon_font_size", &error);
  fail_on_error(error);

  g_string_printf(str, "lemonbar -F \\%s -B \\%s -g \"%sx%s+0+0\" -f \"-*-terminus-medium-*-*-*-%s-*-*-*-*-*-*-*\" -f \"-*-ionicons-medium-*-*-*-%s-*-*-*-*-*-*-*\"",
      fg_color,
      bg_color,
      width,
      height,
      font_size,
      icon_font_size);

  g_free(fg_color);
  g_free(bg_color);
  g_free(width);
  g_free(height);
  g_free(font_size);
  g_free(icon_font_size);
}

void init_monitors(GKeyFile* configs, GArray** fns) {
  char** monitor_configs;
  gsize lengths;
  GError* error = NULL;
  monitor_configs = g_key_file_get_string_list(configs, "configs", "monitors", &lengths, &error);
  fail_on_error(error);

  *fns = g_array_sized_new(FALSE, FALSE, sizeof(struct monitor_fns), lengths);

  int i = 0;
  for (i = 0; i < lengths; i++) {
    struct monitor_fns fn = convert_string_to_monitor_fns(monitor_configs[i]);
    g_array_append_val(*fns, fn);
  }

  g_strfreev(monitor_configs);
}

struct monitor_fns convert_string_to_monitor_fns(char* str) {
  if (strcmp("net", str) == 0) {
    return net_monitor_fns();
  } else if (strcmp("clock", str) == 0) {
    return clock_monitor_fns();
  } else if (strcmp("dropbox", str) == 0) {
    return dropbox_monitor_fns();
  } else if (strcmp("memory", str) == 0) {
    return memory_monitor_fns();
  } else if (strcmp("sp500", str) == 0) {
    return sp500_monitor_fns();
  } else if (strcmp("battery", str) == 0) {
    return battery_monitor_fns();
  } else if (strcmp("thinkpad_fan", str) == 0) {
    return thinkpad_fan_monitor_fns();
  } else if (strcmp("thinkpad_temp", str) == 0) {
    return thinkpad_temp_monitor_fns();
  } else if (strcmp("weather", str) == 0) {
    return weather_monitor_fns();
  } else if (strcmp("core_temp", str) == 0) {
    return core_temp_monitor_fns();
  } else if (strcmp("it87_temp", str) == 0) {
    return it87_temp_monitor_fns();
  } else if (strcmp("it87_fan", str) == 0) {
    return it87_fan_monitor_fns();
  } else if (strcmp("nct6775_temp", str) == 0) {
    return nct6775_temp_monitor_fns();
  } else if (strcmp("nct6775_fan", str) == 0) {
    return nct6775_fan_monitor_fns();
  } else if (strcmp("volume", str) == 0) {
    return volume_monitor_fns();
  } else if (strcmp("nginx", str) == 0) {
    return nginx_monitor_fns();
  } else {
    fprintf(stderr, "Monitor %s not found.\n", str);
    exit(EXIT_FAILURE);
  }
}
