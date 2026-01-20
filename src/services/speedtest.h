#ifndef SPEEDTEST_H
#define SPEEDTEST_H

#include "../server/server.h"

#include <curl/curl.h>
#include <stdlib.h>

#define UP_BUFFER_SIZE (2 * 1024 * 1024)

typedef struct s_upload_data{
    char *buffer;
}UploadData;

struct curl_slist *add_headers(CURL *curl);
void curl_upload_setopts(CURL *curl, UploadData *buf, int timeout);

void fill_buffer(UploadData *buf);

char *probe_download_endpoint(CURL *curl, const char *host);

int get_download_speed(Server *servers, size_t count, size_t timeout);
int get_upload_speed(Server *servers, size_t count);

#endif