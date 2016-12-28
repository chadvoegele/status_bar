/** Copyright 2015-2017 Chad Voegele.
* See LICENSE file for license details.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <glib.h>

#include "status_bar.h"
#include "nct6775_fan_monitor.h"
#include "sys_file_monitor.h"

int convert_nct6775_fan(int temp) {
  return temp;
}

void* nct6775_fan_init(GKeyFile* configs) {
  GArray* temp_filenames = g_array_new(FALSE, FALSE, sizeof(GString*));
  append_filename(temp_filenames,
      "/sys/devices/platform/nct6775.2576/hwmon/hwmon1/fan2_input");
  append_filename(temp_filenames,
      "/sys/devices/platform/nct6775.2576/hwmon/hwmon1/fan3_input");
  append_filename(temp_filenames,
      "/sys/devices/platform/nct6775.2576/hwmon/hwmon1/fan5_input");

  gunichar icon;
  sscanf("U+62384", "U+%06"G_GINT32_FORMAT"X", &icon);
  return sys_file_init_config(icon, temp_filenames, convert_nct6775_fan, configs);
}
