/*
* Copyright 2015-2017 Chad Voegele.
* See LICENSE file for license details.
*/

#pragma once

#include <glib.h>
#include <sys/un.h>

#include "status_bar.h"
#include "base_monitor.h"

struct dropbox_monitor {
  struct base_monitor* base;

  struct sockaddr_un remote;
  int addr_len;
  char* status_req;
  char* err;
  int socket;
  int conn;
  GString* response;
};

void* dropbox_init(GArray*);
gboolean dropbox_update_text(void*);
int dropbox_sleep_time(void*);
void dropbox_free(void*);

void setup_sockaddr(struct sockaddr_un*, int*);
int receive(int, GString*);
void format_response(GString*);
void format_status(char*);
