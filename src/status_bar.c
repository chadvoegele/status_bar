/*
* Copyright 2015 Chad Voegele.
* See LICENSE file for license details.
*/

#include <stdio.h>
#include <stdlib.h>
#include <glib.h>
#include <sys/unistd.h>
#include <string.h>

#include "status_bar.h"
#include "signal_handler.h"

#include "configs.h"

int main(int argc, char** argv) {
  return status_bar();
}

int status_bar() {
  init_signal_handler();
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

  init_monitors(status_bar->configs, status_bar->monitors, &status_bar->n_monitors);

  int i;
  for (i = 0; i < status_bar->n_monitors; i++) {
    g_mutex_init(&status_bar->monitors[i].mutex);
    status_bar->monitors[i].configs = status_bar->configs;
  }
}

void run_status_bar(struct status_bar* status_bar) {
  is_running_global = TRUE;

  GThread* threads[MAX_MONITORS];
  int i;
  for (i = 0; i < status_bar->n_monitors; i++) {
    threads[i] = g_thread_new("",
        thread_fun,
        &status_bar->monitors[i]);
    g_mutex_lock(&status_bar->monitors[i].mutex);
  }

  char all_text[MAX_MONITORS*MAX_TEXT_LENGTH];
  struct monitor_refs* mr;
  char* ptr;
  while (is_running_global) {
    ptr = all_text;

    for (i = 0; i < status_bar->n_monitors-1; i++) {
      mr = &status_bar->monitors[i];

      if (i != 0) {
        ptr = ptr + sprintf(ptr, " | ");
      }

      g_mutex_lock(&mr->mutex);
      ptr = ptr + sprintf(ptr, "%s", mr->text);
      g_mutex_unlock(&mr->mutex);
    }

    mr = &status_bar->monitors[status_bar->n_monitors-1];
    g_mutex_lock(&mr->mutex);
    int last_length = strlen(mr->text);
    sprintf(ptr, "^p(_RIGHT)^p(%d)%s", -last_length*9, mr->text);
    g_mutex_unlock(&mr->mutex);

    fprintf(status_bar->dzen_pipe, "%s\n", all_text);
    fflush(status_bar->dzen_pipe);

    sleep(1);
  }

  for (i = 0; i < status_bar->n_monitors; i++) {
    g_thread_join(threads[i]);
  }
}

void close_status_bar(struct status_bar* status_bar) {
  pclose(status_bar->dzen_pipe);
  g_key_file_free(status_bar->configs);
}

void* thread_fun(void* pointer) {
  struct monitor_refs* mr;
  if ((mr = (struct monitor_refs*)pointer) != NULL) {
    return mr->monitor(mr);
  } else {
    fprintf(stderr, "monitor_refs not received in thread_fun.\n");
    exit(EXIT_FAILURE);
  }
}
