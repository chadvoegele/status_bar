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
#include "users_monitor.h"
#include "text_monitor.h"

error_t parse_opt(int key, char *arg, struct argp_state *state) {
  char** config_file_ptr = (char**)state->input;
  switch (key) {
    case 'c': *config_file_ptr = arg; break;
    default: return ARGP_ERR_UNKNOWN;
  }
  return 0;
}

void parse_arguments(int argc, char** argv, char** config_file) {
  static char doc[] = "";
  static char args_doc[] = "";
  struct argp_option options[] = {
    {"config",  'c', "status_bar.conf",   0,  "location of config file" },
    {0},
  };
  struct argp argp = { options, parse_opt, args_doc, doc };
  argp_parse(&argp, argc, argv, 0, 0, config_file);
}

void load_configs(GKeyFile* configs, int argc, char** argv) {
  GString* config_path = g_string_new(NULL);

  char* config_file_arg = NULL;
  char* path = NULL;
  char* home = NULL;
  parse_arguments(argc, argv, &config_file_arg);
  if (config_file_arg != NULL) {
    g_string_printf(config_path, "%s", config_file_arg);
  } else if ((path = getenv("XDG_CONFIG_HOME")) != NULL) {
    g_string_printf(config_path, "%s/status_bar.conf", path);
  } else if ((home = getenv("HOME")) != NULL) {
    g_string_printf(config_path, "%s/.config/status_bar.conf", home);
  } else {
    fprintf(stderr, "Either --config, XDG_CONFIG_HOME, or HOME must defined for config file.\n");
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
  char* pipeto = g_key_file_get_string(configs, "configs", "pipeto", &error);
  fail_on_error(error);

  str = g_string_truncate(str, 0);
  g_string_append_printf(str, "%s", pipeto);

  g_free(pipeto);
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
  } else if (strcmp("users", str) == 0) {
    return users_init(configs);
  } else if (strcmp("text", str) == 0) {
    return text_init(configs);
  } else {
    fprintf(stderr, "Monitor %s not found.\n", str);
    exit(EXIT_FAILURE);
  }
}
