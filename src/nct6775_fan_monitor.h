/*
* Copyright 2015-2017 Chad Voegele.
* See LICENSE file for license details.
*/

#pragma once

#include "status_bar.h"

struct monitor_fns nct6775_fan_monitor_fns();
void* nct6775_fan_init(GString*, GMutex*, GKeyFile*);

int convert_nct6775_fan(int);
