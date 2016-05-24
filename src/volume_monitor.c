/*
* Copyright 2015 Chad Voegele.
* See LICENSE file for license details.
*/

#include <glib.h>
#include <stdlib.h>
#include <alsa/asoundlib.h>

#include "volume_monitor.h"
#include "status_bar.h"

struct monitor_fns volume_monitor_fns() {
  struct monitor_fns f;
  f.init = volume_init;
  f.sleep_time = volume_sleep_time;
  f.update_text = volume_update_text;
  f.free = volume_free;

  return f;
}

void* volume_init(GString* bar_text, GMutex* mutex, GKeyFile* configs) {
  struct volume_monitor* m = malloc(sizeof(struct volume_monitor));

  m->bar_text = bar_text;
  m->mutex = mutex;

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
  struct volume_monitor* m;
  if ((m = (struct volume_monitor*)ptr) != NULL) {

    long vol;
    int mute;
    int vol_mute_code = get_vol_mute(&vol, &mute);

    if (vol_mute_code != 0) {
      g_string_printf(m->str, "!");
    } else {
      if (mute == 0 || vol < -999999)  // muted
        g_string_printf(m->str, "M");
      else
        g_string_printf(m->str, "%ld", vol/100);
    }

    g_mutex_lock(m->mutex);
    m->bar_text = g_string_assign(m->bar_text, m->str->str);
    g_mutex_unlock(m->mutex);

    return TRUE;

  } else {
    fprintf(stderr, "volume monitor not received in update.\n");
    exit(EXIT_FAILURE);
  }
}

int volume_sleep_time(void* ptr) {
  struct volume_monitor* m;
  if ((m = (struct volume_monitor*)ptr) != NULL) {
    return 1;
  } else {
    fprintf(stderr, "volume monitor not received in sleep_time.\n");
    exit(EXIT_FAILURE);
  }
}

void volume_free(void* ptr) {
  struct volume_monitor* m;
  if ((m = (struct volume_monitor*)ptr) != NULL) {
    g_string_free(m->str, TRUE);

    free(m);

  } else {
    fprintf(stderr, "volume monitor not received in close.\n");
    exit(EXIT_FAILURE);
  }
}
