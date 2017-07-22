/*
* Copyright 2015-2017 Chad Voegele.
* See LICENSE file for license details.
*/

#pragma once

#include "status_bar.h"

void* nct6775_temp_init(GArray*);

int convert_nct6775_temp(int);
