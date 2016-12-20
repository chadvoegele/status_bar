/*
* Copyright 2015-2017 Chad Voegele.
* See LICENSE file for license details.
*/

#pragma once

#include "status_bar.h"

struct monitor_fns it87_temp_monitor_fns();
void* it87_temp_init(GString*, GMutex*, GKeyFile*);

int convert_it87_temp(int);
