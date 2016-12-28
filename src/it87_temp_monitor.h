/*
* Copyright 2015-2017 Chad Voegele.
* See LICENSE file for license details.
*/

#pragma once

#include "status_bar.h"

void* it87_temp_init(GKeyFile*);

int convert_it87_temp(int);
