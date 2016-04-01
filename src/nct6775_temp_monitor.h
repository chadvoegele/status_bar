/*
* Copyright 2015 Chad Voegele.
* See LICENSE file for license details.
*/

#pragma once

#include "status_bar.h"

struct monitor_fns nct6775_temp_monitor_fns();
void* nct6775_temp_init(GString*, GMutex*, GKeyFile*);

int convert_nct6775_temp(int);
