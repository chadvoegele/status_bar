/*
* Copyright 2015-2017 Chad Voegele.
* See LICENSE file for license details.
*/

#include <stdio.h>
#include <stdlib.h>
#include <glib.h>
#include <glib-unix.h>
#include <sys/unistd.h>
#include <string.h>
#include <limits.h>

#include "status_bar.h"
#include "base_monitor.h"
#include "signal_handler.h"

#include "configs.h"

int main(int argc, char** argv) {
  return status_bar(argc, argv);
}

int status_bar(int argc, char** argv) {
  struct status_bar status_bar;
  init_status_bar(&status_bar, argc, argv);
  run_status_bar(&status_bar);
  close_status_bar(&status_bar);
  return EXIT_SUCCESS;
}

void init_status_bar(struct status_bar* status_bar, int argc, char** argv) {
  status_bar->configs = g_key_file_new();
  load_configs(status_bar->configs, argc, argv);

  GString* display_cmd_str = g_string_new(NULL);
  build_display_cmd_str(status_bar->configs, display_cmd_str);
  status_bar->display_pipe = popen(display_cmd_str->str, "w");
  g_string_free(display_cmd_str, TRUE);

  status_bar->loop = g_main_loop_new(g_main_context_default(), FALSE);

  g_unix_signal_add(SIGINT, quit_loop, status_bar->loop);
  g_unix_signal_add(SIGTERM, quit_loop, status_bar->loop);

  init_monitors(status_bar->configs, &status_bar->n_monitors, &status_bar->monitors);

  int min_update = INT_MAX;
  for (int i = 0; i < status_bar->n_monitors; i++) {
    void* m = status_bar->monitors[i];
    struct base_monitor* bm = ((struct base_monitor_base*)m)->base;
    int seconds = bm->sleep_time(m);
    g_timeout_add_seconds(seconds, bm->update_text, m);

    if (min_update > seconds)
      min_update = seconds;
  }

  g_timeout_add_seconds(min_update, update_status_bar, status_bar);
}

void run_status_bar(struct status_bar* status_bar) {
  for (int i = 0; i < status_bar->n_monitors; i++) {
    void* m = status_bar->monitors[i];
    struct base_monitor* bm = ((struct base_monitor_base*)m)->base;
    bm->update_text(m);
  }
  update_status_bar(status_bar);

  g_main_loop_run(status_bar->loop);
}

gboolean update_status_bar(void* ptr) {
  struct status_bar* status_bar;
  if ((status_bar = (struct status_bar*)ptr) == NULL) {
    fprintf(stderr, "Did not receive status_bar in update status bar.\n");
    exit(EXIT_FAILURE);
  }

  GString* output = g_string_new(NULL);
  for (int i = 0; i < status_bar->n_monitors; i++) {
    void* m = status_bar->monitors[i];
    struct base_monitor* bm = ((struct base_monitor_base*)m)->base;

    g_mutex_lock(bm->mutex);
    g_string_append_printf(output, "%s", bm->text->str);
    g_mutex_unlock(bm->mutex);
  }

  fprintf(status_bar->display_pipe, "%s\n", output->str);
  fflush(status_bar->display_pipe);
  g_string_free(output, TRUE);

  return TRUE;
}

void close_status_bar(struct status_bar* status_bar) {
  pclose(status_bar->display_pipe);
  g_key_file_free(status_bar->configs);

  for (int i = 0; i < status_bar->n_monitors; i++) {
    void* m = status_bar->monitors[i];
    struct base_monitor* bm = ((struct base_monitor_base*)m)->base;
    bm->free(m);
  }

  free(status_bar->monitors);

  g_main_loop_unref(status_bar->loop);
}

void monitor_null_check(void* ptr, const char* monitor_name, const char* monitor_fn) {
  if (ptr == NULL) {
    fprintf(stderr, "%s not received in %s.\n", monitor_name, monitor_fn);
    exit(EXIT_FAILURE);
  }
}
