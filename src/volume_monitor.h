/*
* Copyright 2015-2021 Chad Voegele.
* See LICENSE file for license details.
*/

#pragma once

#include <glib.h>
#include <alsa/asoundlib.h>

#include "status_bar.h"
#include "base_monitor.h"

struct volume_monitor {
  struct base_monitor* base;

  GString* loud_icon;
  GString* mute_icon;

  GString* str;
};

void* volume_init(GArray*);
gboolean volume_update_text(void*);
int volume_sleep_time(void*);
void volume_free(void*);
void cleanup_handle(snd_mixer_t*);
int get_vol_mute(long*, int*);
