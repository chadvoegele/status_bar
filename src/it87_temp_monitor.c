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

int convert_it87_temp(int temp) {
  return temp/1000;
}

void* it87_temp_init(GArray* arguments) {
  GArray* temp_filenames = g_array_new(FALSE, FALSE, sizeof(GString*));
  append_filename(temp_filenames,
      "/sys/devices/platform/it87.656/temp1_input");
  append_filename(temp_filenames,
      "/sys/devices/platform/it87.656/temp2_input");
  append_filename(temp_filenames,
      "/sys/devices/platform/it87.656/temp3_input");

  return sys_file_init_config(temp_filenames, convert_it87_temp, arguments);
}
