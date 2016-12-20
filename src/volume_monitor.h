/*
* Copyright 2015-2017 Chad Voegele.
* See LICENSE file for license details.
*/

#pragma once

#include <glib.h>
#include <alsa/asoundlib.h>

#include "status_bar.h"

struct volume_monitor {
  GString* bar_text;
  GMutex* mutex;

  GString* str;
};

struct monitor_fns volume_monitor_fns();
void* volume_init(GString*, GMutex*, GKeyFile*);
gboolean volume_update_text(void*);
int volume_sleep_time(void*);
void volume_free(void*);
void cleanup_handle(snd_mixer_t*);
int get_vol_mute(long*, int*);
