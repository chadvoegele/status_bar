/*
* Copyright 2015 Chad Voegele.
* See LICENSE file for license details.
*/

#pragma once

#include <glib.h>
#include <sys/un.h>

#include "status_bar.h"

struct dropbox_monitor {
  struct sockaddr_un remote;
  int addr_len;
  char* status_req;
  char* icon;
  char* err;
  int socket;
  int conn;
  GString* response;
};

void* dropbox_monitor(struct monitor_refs*);
void dropbox_init(void*, void*);
const char* dropbox_update_text(void*);
int dropbox_sleep_time(void*);
void dropbox_close(void*);

void setup_sockaddr(struct sockaddr_un*, int*);
int receive(int, GString*);
void format_response(GString*, char*);
void format_status(char*);
