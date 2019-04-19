#ifndef LIBCURL_H
#define LIBCURL_H
#include <stdio.h>
#include <curl/curl.h>
#include <string>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <cstring>
#include <stdlib.h>
#include <stdarg.h>
#include <time.h>

char* concat(int, ...);
int curl_get(const char*,const char*);
time_t curl_get_info(const char*,const char*);
#endif