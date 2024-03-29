/*
* Copyright 2015-2021 Chad Voegele.
* See LICENSE file for license details.
*/

#pragma once

#include <glib.h>
#include <argp.h>

void parse_arguments(int argc, char** argv, char** config_file);
error_t parse_opt(int key, char *arg, struct argp_state *state);
void fail_on_error(GError*);
void load_configs(GKeyFile*, int argc, char** argv);
void init_monitors(GKeyFile*, gsize*, void***);
void build_display_cmd_str(GKeyFile*, GString*);
void parse_monitor_config(char* str, GString* monitor, GArray* arguments);
void* convert_string_to_monitor(char*, GArray*);
