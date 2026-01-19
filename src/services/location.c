#include "location.h"

#include <curl/curl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static size_t write_cb(void *ptr, size_t size, size_t nmemb, void *userdata) {
    size_t total = size * nmemb;
    char **ip = (char **)userdata;

    static size_t len = 0;

    char *tmp = realloc(*ip, len + total + 1);
    if (!tmp) return 0;

    *ip = tmp;
    memcpy(*ip + len, ptr, total);
    len += total;
    (*ip)[len] = '\0';

    return total;
}

int get_user_location(char *ip) {
    
}

int get_user_ip(char **ip) {
    *ip = NULL;

    CURL *curl = curl_easy_init();
    if(!curl) {
        fprintf(stderr, "curl init failed\n");
        return EXIT_FAILURE;
    }

    curl_easy_setopt(curl, CURLOPT_URL, "https://api.ipify.org");
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_cb);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, ip);

    CURLcode res = curl_easy_perform(curl);
    if(res != CURLE_OK) {
        fprintf(stderr, "curl encountered an error: %s\n", curl_easy_strerror(res));
        curl_easy_cleanup(curl);
        return EXIT_FAILURE;
    }

    curl_easy_cleanup(curl);
    return EXIT_SUCCESS;
}