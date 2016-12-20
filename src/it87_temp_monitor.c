/** Copyright 2015-2017 Chad Voegele.
* See LICENSE file for license details.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <glib.h>

#include "status_bar.h"
#include "it87_temp_monitor.h"
#include "sys_file_monitor.h"

struct monitor_fns it87_temp_monitor_fns() {
  struct monitor_fns f;
  f.init = it87_temp_init;
  f.sleep_time = sys_file_sleep_time;
  f.update_text = sys_file_update_text;
  f.free = sys_file_free;

  return f;
}

int convert_it87_temp(int temp) {
  return temp/1000;
}

void* it87_temp_init(GString* bar_text, GMutex* mutex,
    GKeyFile* configs) {
  GArray* temp_filenames = g_array_new(FALSE, FALSE, sizeof(GString*));
  append_filename(temp_filenames,
      "/sys/devices/platform/it87.656/temp1_input");
  append_filename(temp_filenames,
      "/sys/devices/platform/it87.656/temp2_input");
  append_filename(temp_filenames,
      "/sys/devices/platform/it87.656/temp3_input");

  gunichar icon;
  sscanf("U+62529", "U+%06"G_GINT32_FORMAT"X", &icon);
  return sys_file_init_config(icon, temp_filenames, convert_it87_temp,
      bar_text, mutex, configs);
}
