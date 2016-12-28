/*
* Copyright 2015-2017 Chad Voegele.
* See LICENSE file for license details.
*/

#include <glib.h>
#include <stdlib.h>
#include <alsa/asoundlib.h>

#include "volume_monitor.h"
#include "status_bar.h"
#include "base_monitor.h"

void* volume_init(GKeyFile* configs) {
  struct volume_monitor* m = malloc(sizeof(struct volume_monitor));

  m->base = base_monitor_init(volume_sleep_time, volume_update_text, volume_free);

  m->str = g_string_new(NULL);

  return m;
}

void cleanup_handle(snd_mixer_t* handle) {
  snd_mixer_detach(handle, "default");
  snd_mixer_close(handle);
}

int get_vol_mute(long* vol, int* mute) {
  snd_mixer_t* handle;

  int open_code = snd_mixer_open(&handle, 0);
  if (open_code != 0) return open_code;

  int attach_code = snd_mixer_attach(handle, "default");
  if (attach_code != 0) {
    snd_mixer_close(handle);
    return attach_code;
  }

  int register_code = snd_mixer_selem_register(handle, NULL, NULL);
  if (register_code != 0) {
    cleanup_handle(handle);
    return register_code;
  }

  int load_code = snd_mixer_load(handle);
  if (load_code != 0) {
    cleanup_handle(handle);
    return load_code;
  }

  snd_mixer_selem_id_t *sid;
  snd_mixer_selem_id_alloca(&sid);
  snd_mixer_selem_id_set_index(sid, 0);
  snd_mixer_selem_id_set_name(sid, "Master");
  snd_mixer_elem_t* elem = snd_mixer_find_selem(handle, sid);
  if (elem == 0) {
    cleanup_handle(handle);
    return 1;
  }

  int vol_code = snd_mixer_selem_get_playback_dB(elem, SND_MIXER_SCHN_UNKNOWN, vol);
  if (vol_code != 0) {
    cleanup_handle(handle);
    return vol_code;
  }

  int switch_code = snd_mixer_selem_get_playback_switch(elem, SND_MIXER_SCHN_UNKNOWN, mute);
  if (switch_code != 0) {
    cleanup_handle(handle);
    return switch_code;
  }

  /* snd_mixer_selem_id_free(sid); */
  snd_mixer_detach(handle, "default");
  snd_mixer_close(handle);
  return 0;
}

gboolean volume_update_text(void* ptr) {
  struct volume_monitor* m = (struct volume_monitor*)ptr;
  monitor_null_check(m, "volume_monitor", "update");

  long vol;
  int mute;
  int vol_mute_code = get_vol_mute(&vol, &mute);

  if (vol_mute_code != 0) {
    g_string_printf(m->str, "!");
  } else {
    if (mute == 0 || vol < -999999)  // muted
      g_string_printf(m->str, "");
    else
      g_string_printf(m->str, "%ld", vol/100);
  }

  g_mutex_lock(m->base->mutex);
  m->base->text = g_string_assign(m->base->text, m->str->str);
  g_mutex_unlock(m->base->mutex);

  return TRUE;
}

int volume_sleep_time(void* ptr) {
  return 1;
}

void volume_free(void* ptr) {
  struct volume_monitor* m = (struct volume_monitor*)ptr;
  monitor_null_check(m, "volume_monitor", "free");

  base_monitor_free(m->base);

  g_string_free(m->str, TRUE);

  free(m);
}
