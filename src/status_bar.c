/*
* Copyright 2015 Chad Voegele.
* See LICENSE file for license details.
*/

#include <stdio.h>
#include <stdlib.h>
#include <glib.h>
#include <glib-unix.h>
#include <sys/unistd.h>
#include <string.h>

#include "status_bar.h"
#include "signal_handler.h"

#include "configs.h"

int main(int argc, char** argv) {
  return status_bar();
}

int status_bar() {
  struct status_bar status_bar;
  init_status_bar(&status_bar);
  run_status_bar(&status_bar);
  close_status_bar(&status_bar);
  return EXIT_SUCCESS;
}

void init_status_bar(struct status_bar* status_bar) {
  status_bar->configs = g_key_file_new();
  load_configs(status_bar->configs);

  GString* dzen_str = g_string_new(NULL);
  build_dzen_str(status_bar->configs, dzen_str);
  status_bar->dzen_pipe = popen(dzen_str->str, "w");
  g_string_free(dzen_str, TRUE);

  status_bar->one_char_width = get_one_char_width(status_bar->configs);

  status_bar->loop = g_main_loop_new(g_main_context_default(), FALSE);

  g_unix_signal_add(SIGINT, quit_loop, status_bar->loop);
  g_unix_signal_add(SIGTERM, quit_loop, status_bar->loop);

  GArray* fns;
  init_monitors(status_bar->configs, &fns);
  status_bar->n_monitors = fns->len;

  status_bar->monitors = malloc(sizeof(struct monitor_refs)*status_bar->n_monitors);

  int min_update = -1;
  int i;
  for (i = 0; i < status_bar->n_monitors; i++) {
    struct monitor_refs* mr = &(status_bar->monitors[i]);

    g_mutex_init(&mr->mutex);
    mr->text = g_string_new(NULL);
    mr->fns = g_array_index(fns, struct monitor_fns, i);

    mr->monitor = mr->fns.init(mr->text, &mr->mutex, status_bar->configs);

    int seconds = mr->fns.sleep_time(mr->monitor);
    g_timeout_add_seconds(seconds, mr->fns.update_text, mr->monitor);

    if (min_update == -1 || min_update > seconds)
      min_update = seconds;
  }

  g_timeout_add_seconds(min_update, update_status_bar, status_bar);

  g_array_free(fns, TRUE);
}

void run_status_bar(struct status_bar* status_bar) {
  int i;
  for (i = 0; i < status_bar->n_monitors; i++) {
    struct monitor_refs* mr = &(status_bar->monitors[i]);
    mr->fns.update_text(mr->monitor);
  }
  update_status_bar(status_bar);

  g_main_loop_run(status_bar->loop);
}

gboolean update_status_bar(void* ptr) {
  struct status_bar* status_bar;
  if ((status_bar = (struct status_bar*)ptr) != NULL) {
    struct monitor_refs* mr;
    GString* output = g_string_new(NULL);
    int i;
    for (i = 0; i < status_bar->n_monitors-1; i++) {
      mr = &status_bar->monitors[i];

      if (i != 0) {
        output = g_string_append(output, " | ");
      }

      g_mutex_lock(&mr->mutex);
      g_string_append_printf(output, "%s", mr->text->str);
      g_mutex_unlock(&mr->mutex);
    }

    mr = &status_bar->monitors[status_bar->n_monitors-1];
    g_mutex_lock(&mr->mutex);
    g_string_append_printf(output, "%%{r}%s",
        mr->text->str);
    g_mutex_unlock(&mr->mutex);

    fprintf(status_bar->dzen_pipe, "%s\n", output->str);
    fflush(status_bar->dzen_pipe);

  } else {
    fprintf(stderr, "Did not receive status_bar in update status bar.\n");
    exit(EXIT_FAILURE);
  }

  return TRUE;
}

void close_status_bar(struct status_bar* status_bar) {
  pclose(status_bar->dzen_pipe);
  g_key_file_free(status_bar->configs);

  int i;
  for (i = 0; i < status_bar->n_monitors; i++) {
    struct monitor_refs* mr = &(status_bar->monitors[i]);
    g_string_free(mr->text, TRUE);
    mr->fns.free(mr->monitor);
    g_mutex_clear(&mr->mutex);
  }

  free(status_bar->monitors);

  g_main_loop_unref(status_bar->loop);
}
