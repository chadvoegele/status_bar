/** Copyright 2015-2017 Chad Voegele.
* See LICENSE file for license details.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <glib.h>

#include "status_bar.h"
#include "core_temp_monitor.h"
#include "sys_file_monitor.h"

int convert_core_temp(int temp) {
  return temp/1000;
}

void* core_temp_init(GArray* arguments) {
  GArray* temp_filenames = g_array_new(FALSE, FALSE, sizeof(GString*));
  //TODO: Search directory for *input files instead of hardcode
  append_filename(temp_filenames,
      "/sys/devices/platform/coretemp.0/hwmon/hwmon0/temp1_input");
  append_filename(temp_filenames,
      "/sys/devices/platform/coretemp.0/hwmon/hwmon0/temp2_input");
  append_filename(temp_filenames,
      "/sys/devices/platform/coretemp.0/hwmon/hwmon0/temp3_input");
  append_filename(temp_filenames,
      "/sys/devices/platform/coretemp.0/hwmon/hwmon0/temp4_input");
  append_filename(temp_filenames,
      "/sys/devices/platform/coretemp.0/hwmon/hwmon0/temp5_input");
  append_filename(temp_filenames,
      "/sys/devices/platform/coretemp.0/hwmon/hwmon0/temp6_input");
  append_filename(temp_filenames,
      "/sys/devices/platform/coretemp.0/hwmon/hwmon0/temp7_input");

  gunichar icon;
  sscanf("U+62529", "U+%06"G_GINT32_FORMAT"X", &icon);
  return sys_file_init_config(icon, temp_filenames, convert_core_temp, arguments);
}
