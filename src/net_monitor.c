/*
* Copyright 2015 Chad Voegele.
* See LICENSE file for license details.
*/

#include <glib.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "net_monitor.h"
#include "status_bar.h"

struct monitor_fns net_monitor_fns() {
  struct monitor_fns f;
  f.init = net_init;
  f.sleep_time = net_sleep_time;
  f.update_text = net_update_text;
  f.free = net_free;

  return f;
}

void* net_init(GString* bar_text, GMutex* mutex, GKeyFile* configs) {
  struct net_monitor* m = malloc(sizeof(struct net_monitor));

  m->bar_text = bar_text;
  m->mutex = mutex;

  m->str = g_string_new(NULL);

  m->rx = g_array_new(FALSE, FALSE, sizeof(GString*));
  m->tx = g_array_new(FALSE, FALSE, sizeof(GString*));

  find_interfaces(m->rx, m->tx);

  m->last_rx = total_bytes(m->rx);
  m->last_tx = total_bytes(m->tx);

  return m;
}

gboolean net_update_text(void* ptr) {
  struct net_monitor* m;
  if ((m = (struct net_monitor*)ptr) == NULL) {
    fprintf(stderr, "net monitor not received in update.\n");
    exit(EXIT_FAILURE);
  }

  int this_rx = total_bytes(m->rx);
  int this_tx = total_bytes(m->tx);

  int rx_speed = (this_rx - m->last_rx)*8/1000;
  int tx_speed = (this_tx - m->last_tx)*8/1000;

  m->last_rx = this_rx;
  m->last_tx = this_tx;

  g_string_printf(m->str, "%d%d", tx_speed, rx_speed);

  g_mutex_lock(m->mutex);
  m->bar_text = g_string_assign(m->bar_text, m->str->str);
  g_mutex_unlock(m->mutex);

  return TRUE;
}

int net_sleep_time(void* ptr) {
  struct net_monitor* m;
  if ((m = (struct net_monitor*)ptr) == NULL) {
    fprintf(stderr, "net monitor not received in sleep_time.\n");
    exit(EXIT_FAILURE);
  }

  return 1;
}

void net_free(void* ptr) {
  struct net_monitor* m;
  if ((m = (struct net_monitor*)ptr) == NULL) {
    fprintf(stderr, "net monitor not received in close.\n");
    exit(EXIT_FAILURE);
  }

  g_string_free(m->str, TRUE);

  size_t i = 0;
  for (i = 0; i < m->rx->len; i++) {
    g_string_free(g_array_index(m->rx, GString*, i), TRUE);
  }

  for (i = 0; i < m->rx->len; i++) {
    g_string_free(g_array_index(m->tx, GString*, i), TRUE);
  }

  g_array_free(m->rx, TRUE);
  g_array_free(m->tx, TRUE);

  free(m);
}

void find_interfaces(GArray* rx, GArray* tx) {
  char* net_dir = "/sys/class/net";
  GDir* dir;
  dir = g_dir_open (net_dir, 0, NULL);
  if (dir == NULL) {
    fprintf(stderr, "Can't open net dir %s!\n", net_dir);
    exit(EXIT_FAILURE);
  }

  const char* listing;
  while ((listing = g_dir_read_name(dir)) != NULL) {
    if (strncmp(listing, "lo", 2) != 0) {
      GString* rx_int = g_string_new_len(NULL, 100);
      g_string_printf(rx_int, "/sys/class/net/%s/statistics/rx_bytes", listing);
      g_array_append_val(rx, rx_int);

      GString* tx_int = g_string_new_len(NULL, 100);
      g_string_printf(tx_int, "/sys/class/net/%s/statistics/tx_bytes", listing);
      g_array_append_val(tx, tx_int);
    }
  }

  g_dir_close(dir);
}

int total_bytes(GArray* arr) {
  FILE* fps;
  int bytes;
  int total_bytes = 0;
  size_t i;
  for (i = 0; i < arr->len; i++) {
    char* file = g_array_index(arr, GString*, i)->str;
    fps = fopen(file, "r");

    if (fps != NULL)
      fscanf(fps, "%d", &bytes);
    else
      bytes = 0;

    total_bytes = total_bytes + bytes;

    int close_ret = 1;
    if (fps != NULL) {
      close_ret = fclose(fps);

      if (close_ret != 0) {
        fprintf(stderr, "Could not close %s\n", file);
        exit(EXIT_FAILURE);
      }
    }
  }
  return total_bytes;
}
