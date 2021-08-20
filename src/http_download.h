/*
* Copyright 2015-2017 Chad Voegele.
* See LICENSE file for license details.
*/

#pragma once

struct http_data {
  guint id;
  CURLM* curl;
};

gpointer http_init();
void http_free(struct http_data*);
CURLcode download_data(struct http_data*, char*, void*, void*, void*);
