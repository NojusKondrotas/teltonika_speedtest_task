#ifndef SPEEDTEST_H
#define SPEEDTEST_H

#include "../server/server.h"

#include <curl/curl.h>
#include <stdlib.h>

#define UP_BUFFER_SIZE (100 * 1024 * 1024)

typedef struct s_upload_data{
    char *buffer[UP_BUFFER_SIZE];
}UploadData;

struct curl_slist *add_headers(CURL *curl);
void curl_upload_setopts(CURL *curl, UploadData *buf, int timeout);

void fill_buffer(UploadData *buf);

int get_download_speed(Server *servers, size_t count);
int get_upload_speed(Server *servers, size_t count);

#endif