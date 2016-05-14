/*
* Copyright 2015 Chad Voegele.
* See LICENSE file for license details.
*/

#include <glib.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/unistd.h>
#include <sys/socket.h>
#include <sys/un.h>

#include "status_bar.h"
#include "dropbox_monitor.h"

struct monitor_fns dropbox_monitor_fns() {
  struct monitor_fns f;
  f.init = dropbox_init;
  f.sleep_time = dropbox_sleep_time;
  f.update_text = dropbox_update_text;
  f.free = dropbox_free;

  return f;
}

void* dropbox_init(GString* bar_text, GMutex* mutex, GKeyFile* configs) {
  struct dropbox_monitor* m = malloc(sizeof(struct dropbox_monitor));

  m->bar_text = bar_text;
  m->mutex = mutex;

  setup_sockaddr(&m->remote, &m->addr_len);
  m->status_req = "get_dropbox_status\ndone\n";
  m->icon = "";

  m->err = malloc((strlen(m->icon) + 2)*sizeof(char));
  sprintf(m->err, "%s!", m->icon);

  m->socket = -1;
  m->conn = -1;

  m->response = g_string_new(NULL);

  return m;
}

gboolean dropbox_update_text(void* ptr) {
  struct dropbox_monitor* m;
  if ((m = (struct dropbox_monitor*)ptr) != NULL) {
    char* output;
    output = m->err;

    if (m->socket == -1) {
      m->socket = socket(AF_UNIX, SOCK_STREAM, 0);
    }

    if (m->socket == -1) {
      fprintf(stderr, "Failed to create dropbox socket.\n");
    } else {
      if (m->conn == -1) {
        m->conn = connect(m->socket, (struct sockaddr*)&m->remote, m->addr_len);
      }
      if (m->conn == -1) {
        fprintf(stderr, "Failed to connect to dropbox socket.\n");
        close(m->socket);
        m->socket = -1;
      } else {
        int bytes_sent = send(m->socket, m->status_req, strlen(m->status_req), MSG_NOSIGNAL);
        if (bytes_sent == -1) {
          fprintf(stderr, "Failed to send request to dropbox socket.\n");
          close(m->socket);
          m->socket = -1;
          m->conn = -1;
        } else {
          m->response = g_string_truncate(m->response, 0);
          int rec_status = receive(m->socket, m->response);

          if (rec_status == -1) {
            fprintf(stderr, "Bad dropbox response: %s\n", m->response->str);
          } else {
            format_response(m->response, m->icon);
            output = m->response->str;
          }
        }
      }
    }

    g_mutex_lock(m->mutex);
    m->bar_text = g_string_assign(m->bar_text, output);
    g_mutex_unlock(m->mutex);

    return TRUE;

  } else {
    fprintf(stderr, "Dropbox monitor not received in update.\n");
    exit(EXIT_FAILURE);
  }
}

int dropbox_sleep_time(void* ptr) {
  struct dropbox_monitor* m;
  if ((m = (struct dropbox_monitor*)ptr) != NULL) {
    return 1;
  } else {
    fprintf(stderr, "Dropbox monitor not received in sleep_time.\n");
    exit(EXIT_FAILURE);
  }
}

void dropbox_free(void* ptr) {
  struct dropbox_monitor* m;
  if ((m = (struct dropbox_monitor*)ptr) != NULL) {
    free(m->err);

    if (m->socket != -1) {
      close(m->socket);
    }

    g_string_free(m->response, TRUE);
    free(m);

  } else {
    fprintf(stderr, "Dropbox monitor not received in close.\n");
    exit(EXIT_FAILURE);
  }
}

void setup_sockaddr(struct sockaddr_un* remote, int* socklen) {
  remote->sun_family = AF_UNIX;
  char* home = getenv("HOME");
  sprintf(remote->sun_path, "%s/.dropbox/command_socket", home);
  *socklen = strlen(remote->sun_path) + sizeof(remote->sun_family);
}

int receive(int sock, GString* response) {
  char buf[101];
  int buf_len = 0;
  int i = 0;
  fd_set sockds;
  FD_ZERO(&sockds);
  FD_SET(sock, &sockds);

  struct timeval tv;
  tv.tv_sec = 0;
  tv.tv_usec = 250000;  // 0.25 sec

  // magic 20 from python dropbox-cli script
  while(i < 20 && select(sock + 1, &sockds, NULL, NULL, &tv) > 0) {
    buf_len = recv(sock, buf, 100, 0);
    if (buf_len < 0)
      break;
    buf[buf_len] = '\0';
    response = g_string_append(response, buf);
    i++;
  }

  if (i < 20 && buf_len >= 5
      && strncmp(buf + buf_len -1 - 4, "done", 4) == 0) {
    return 0;

  } else {
    return -1;
  }
}

void format_response(GString* response, char* icon) {
  char** words = g_strsplit(response->str, "\n", -1);

  char** word = words;
  while (*word != 0) {
    if (strstr(*word, "status") != NULL) {
      format_status(*word);
      g_strstrip(*word);

      response = g_string_truncate(response, 0);
      response = g_string_append(response, icon);
      response = g_string_append(response, *word);

      break;
    }
    word = word + 1;
  }

  g_strfreev(words);
  return;
}

void format_status(char* word) {
  if (strstr(word, "Up to date") != NULL) {
    strcpy(word, "Up to date");
  } else if (strstr(word, "Downloading file list")) {
    strcpy(word, "Downloading file list");
  } else if (strstr(word, "Uploading")) {
    strcpy(word, "Uploading");
  } else if (strstr(word, "Downloading")) {
    strcpy(word, "Downloading");
  } else if (strstr(word, "Indexing")) {
    strcpy(word, "Indexing");
  } else if (strstr(word, "Starting")) {
    strcpy(word, "Starting");
  } else if (strstr(word, "Connecting")) {
    strcpy(word, "Connecting");
  } else {
    fprintf(stderr, "Unable to parse: %s\n", word);
    strcpy(word, "?");
  }
}
