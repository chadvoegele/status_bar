/*
* Copyright 2015 Chad Voegele.
* See LICENSE file for license details.
*/

#pragma once

#include <glib.h>

void fail_on_error(GError*);
void load_configs(GKeyFile*);
void init_monitors(GKeyFile*, struct monitor_refs*, int*);
void build_dzen_str(GKeyFile*, GString*);
void* (*convert_string_to_monitor(char*))(struct monitor_refs*);
