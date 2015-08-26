/*
* Copyright 2015 Chad Voegele.
* See LICENSE file for license details.
*/

#pragma once

#include "status_bar.h"

struct monitor_fns it87_fan_monitor_fns();
void* it87_fan_init(GString*, GMutex*, GKeyFile*);

int convert_it87_fan(int);
