#include "speedtest.h"

#include <curl/curl.h>
#include <stdlib.h>
#include <string.h>

struct curl_slist *add_headers(CURL *curl) {
    struct curl_slist *headers = NULL;
    headers = curl_slist_append(headers, "User-Agent: Mozilla/5.0");
    headers = curl_slist_append(headers, "Content-Type: application/octet-stream");
    headers = curl_slist_append(headers, "Accept: */*");
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

    return headers;
}
void curl_upload_setopts(CURL *curl, UploadData *buf, int timeout) {
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, buf->buffer);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, UP_BUFFER_SIZE);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, timeout > 0 ? (long)timeout : 15L);
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);
}

void fill_buffer(UploadData *buf) {
    srand(time(NULL));
    for(size_t i = 0; i < UP_BUFFER_SIZE; i++) {
        buf->buffer[i] = rand() % 256;
    }
}

int get_download_speed(Server *servers, size_t count) {
    return EXIT_SUCCESS;
}

int get_upload_speed(Server *servers, size_t count) {
    return EXIT_SUCCESS;
}