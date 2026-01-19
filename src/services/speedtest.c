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

int get_download_speed(Server *servers, size_t count) {
    return EXIT_SUCCESS;
}

int get_upload_speed(Server *servers, size_t count) {
    return EXIT_SUCCESS;
}