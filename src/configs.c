/*
* Copyright 2015-2021 Chad Voegele.
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
#include "volume_monitor.h"
#include "nginx_monitor.h"
#include "cpu_usage_monitor.h"
#include "users_monitor.h"
#include "text_monitor.h"
#include "sys_file_monitor.h"

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

void parse_monitor_config(char* str, GString* monitor, GArray* arguments) {
  GScanner* scanner = g_scanner_new(NULL);
  g_scanner_input_text(scanner, str, strlen(str));
  while (g_scanner_get_next_token(scanner) != G_TOKEN_EOF) {
    GTokenType type = g_scanner_cur_token(scanner);
    if (type == G_TOKEN_IDENTIFIER) {
      g_string_assign(monitor, g_scanner_cur_value(scanner).v_identifier);
    }
    if (type == G_TOKEN_STRING) {
      GString* arg = g_string_new_len(NULL, 100);
      g_string_assign(arg, g_scanner_cur_value(scanner).v_string);
      g_array_append_val(arguments, arg);
    }
  }
  g_scanner_destroy(scanner);
}

void init_monitors(GKeyFile* configs, gsize* length, void*** monitors) {
  char** monitor_configs;
  GError* error = NULL;
  monitor_configs = g_key_file_get_string_list(configs, "configs", "monitors", length, &error);
  fail_on_error(error);

  *monitors = malloc((*length)*sizeof(void*));

  GString* monitor = g_string_new_len(NULL, 100);
  GArray* arguments = g_array_new(FALSE, FALSE, sizeof(GString*));

  for (int i = 0; i < *length; i++) {
    parse_monitor_config(monitor_configs[i], monitor, arguments);

    void* m = convert_string_to_monitor(monitor->str, arguments);
    (*monitors)[i] = m;

    for (size_t i = 0; i < arguments->len; i++) {
      g_string_free(g_array_index(arguments, GString*, i), TRUE);
    }
    g_array_set_size(arguments, 0);
  }

  g_string_free(monitor, TRUE);
  g_array_free(arguments, TRUE);

  g_strfreev(monitor_configs);
}

void* convert_string_to_monitor(char* str, GArray* arguments) {
  if (strcmp("net", str) == 0) {
    return net_init(arguments);
  } else if (strcmp("clock", str) == 0) {
    return clock_init(arguments);
  } else if (strcmp("dropbox", str) == 0) {
    return dropbox_init(arguments);
  } else if (strcmp("memory", str) == 0) {
    return memory_init(arguments);
  } else if (strcmp("sp500", str) == 0) {
    return sp500_init(arguments);
  } else if (strcmp("battery", str) == 0) {
    return battery_init(arguments);
  } else if (strcmp("thinkpad_fan", str) == 0) {
    return thinkpad_fan_init(arguments);
  } else if (strcmp("thinkpad_temp", str) == 0) {
    return thinkpad_temp_init(arguments);
  } else if (strcmp("weather", str) == 0) {
    return weather_init(arguments);
  } else if (strcmp("volume", str) == 0) {
    return volume_init(arguments);
  } else if (strcmp("nginx", str) == 0) {
    return nginx_init(arguments);
  } else if (strcmp("cpu_usage", str) == 0) {
    return cpu_usage_init(arguments);
  } else if (strcmp("users", str) == 0) {
    return users_init(arguments);
  } else if (strcmp("text", str) == 0) {
    return text_init(arguments);
  } else if (strcmp("sys_file", str) == 0) {
    return sys_file_init(arguments);
  } else {
    fprintf(stderr, "Monitor %s not found.\n", str);
    exit(EXIT_FAILURE);
  }
}
