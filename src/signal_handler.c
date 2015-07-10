/*
* Copyright 2015 Chad Voegele.
* See LICENSE file for license details.
*/

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <glib.h>

volatile sig_atomic_t is_running_global;

void term(int signum) {
  printf("Exiting...\n");
  is_running_global = FALSE;
}

void init_signal_handler() {
  struct sigaction action;
  memset(&action, 0, sizeof(struct sigaction));
  action.sa_handler = term;
  if (sigaction(SIGTERM, &action, NULL) != 0) {
    exit(EXIT_FAILURE);
  }
}
