/*
* Copyright 2015 Chad Voegele.
* See LICENSE file for license details.
*/

#pragma once

CURLcode download_data(CURL*, char*, GString*);
size_t writefunc(void*, size_t, size_t, GString*);
