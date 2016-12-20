/*
* Copyright 2015-2017 Chad Voegele.
* See LICENSE file for license details.
*/

#include <glib.h>
#include <curl/curl.h>

size_t writefunc(void *res_str, size_t size, size_t nmemb, GString* res) {
  res = g_string_append_len(res, res_str, size*nmemb);
  return size*nmemb;
}

CURLcode download_data(CURL* curl, char* request_str, GString* res) {
  res = g_string_set_size(res, 0);
  // Modified example from Alexandre Jasim on Stack Overflow.
  curl_easy_setopt(curl, CURLOPT_USERAGENT, "c");
  curl_easy_setopt(curl, CURLOPT_URL, request_str);
  curl_easy_setopt(curl, CURLOPT_TIMEOUT, 30L);
  curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writefunc);
  curl_easy_setopt(curl, CURLOPT_WRITEDATA, res);
  CURLcode code = curl_easy_perform(curl);
  return code;
}
