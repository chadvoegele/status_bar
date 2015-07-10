/*
* Copyright 2015 Chad Voegele.
* See LICENSE file for license details.
*/

#pragma once

#include <glib.h>

#include "status_bar.h"

volatile sig_atomic_t sleep_check(int sleep, volatile sig_atomic_t* is_running);
void* monitor_loop(struct monitor_refs*, void*,
    void(*init)(void*, void*), const char* (*update)(void*),
    int(*get_sleep_time)(void*), void(*close)(void*));
