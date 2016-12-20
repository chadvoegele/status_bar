/*
* Copyright 2015-2017 Chad Voegele.
* See LICENSE file for license details.
*/

#pragma once

#include "status_bar.h"

struct monitor_fns core_temp_monitor_fns();
void* core_temp_init(GString*, GMutex*, GKeyFile*);

int convert_core_temp(int);
