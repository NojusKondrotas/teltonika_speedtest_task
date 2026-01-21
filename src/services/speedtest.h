#ifndef SPEEDTEST_H
#define SPEEDTEST_H

#include "../server/server.h"

#include <curl/curl.h>
#include <stdlib.h>

#define UP_BUFFER_SIZE (2 * 1024 * 1024)
#define KB_SIZE 1024

#define DEFAULT_TIMEOUT 15L

typedef struct s_upload_data{
    char *buffer;
}UploadData;

struct curl_slist *add_headers(CURL *curl);
void curl_download_setopts(CURL *curl, size_t timeout, int disableSSL);
void curl_upload_setopts(CURL *curl, UploadData *buf, size_t timeout, int disableSSL);

void fill_buffer(UploadData *buf);

char *probe_download_endpoint(CURL *curl, const char *host, size_t timeout, int disableSSL);

int get_download_speed_single(CURL *curl, Server *server, size_t timeout, int disableSSL);
int get_upload_speed_single(CURL *curl, Server *server, size_t timeout, int disableSSL);
int get_download_speed(Server *servers, size_t count, size_t timeout, int disableSSL);
int get_upload_speed(Server *servers, size_t count, size_t timeout, int disableSSL);

#endif