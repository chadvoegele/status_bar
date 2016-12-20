/*
* Copyright 2015-2017 Chad Voegele.
* See LICENSE file for license details.
*/

#pragma once

#include <glib.h>

void fail_on_error(GError*);
void load_configs(GKeyFile*);
void init_monitors(GKeyFile*, GArray**);
void build_display_cmd_str(GKeyFile*, GString*);
struct monitor_fns convert_string_to_monitor_fns(char*);
