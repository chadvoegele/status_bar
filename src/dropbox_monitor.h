/*
* Copyright 2015 Chad Voegele.
* See LICENSE file for license details.
*/

#pragma once

#include <glib.h>
#include <sys/un.h>

#include "status_bar.h"

struct dropbox_monitor {
  GString* bar_text;
  GMutex* mutex;

  struct sockaddr_un remote;
  int addr_len;
  char* status_req;
  char* icon;
  char* err;
  int socket;
  int conn;
  GString* response;
};

struct monitor_fns dropbox_monitor_fns();
void* dropbox_init(GString*, GMutex*, GKeyFile*);
gboolean dropbox_update_text(void*);
int dropbox_sleep_time(void*);
void dropbox_free(void*);

void setup_sockaddr(struct sockaddr_un*, int*);
int receive(int, GString*);
void format_response(GString*, char*);
void format_status(char*);
