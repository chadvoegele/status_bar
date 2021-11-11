/*
* Copyright 2015-2021 Chad Voegele.
* See LICENSE file for license details.
*/

#include <glib.h>
#include <stdlib.h>
#include <stdio.h>

gboolean quit_loop(void* ptr) {
  GMainLoop* loop = (GMainLoop*)ptr;
  if (loop != NULL && g_main_loop_is_running(loop)) {
    printf("Exiting...\n");
    g_main_loop_quit(loop);
  } else {
    fprintf(stderr, "GMainLoop* not received in quit_loop signal handler.\n");
    exit(EXIT_FAILURE);
  }
  return FALSE;
}
