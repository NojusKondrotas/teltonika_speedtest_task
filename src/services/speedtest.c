#include "speedtest.h"

#include <curl/curl.h>
#include <stdlib.h>
#include <string.h>

static size_t discard_cb(void *ptr, size_t size, size_t nmemb, void *userdata) {
    return size * nmemb;
}

struct curl_slist *add_headers(CURL *curl) {
    struct curl_slist *headers = NULL;
    headers = curl_slist_append(headers, "User-Agent: Mozilla/5.0");
    headers = curl_slist_append(headers, "Content-Type: application/octet-stream");
    headers = curl_slist_append(headers, "Accept: */*");
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

    return headers;
}

void curl_download_setopts(CURL *curl, int timeout) {
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, discard_cb);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, NULL);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, timeout > 0 ? (long)timeout : 15L);
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);
}

void curl_upload_setopts(CURL *curl, UploadData *buf, int timeout) {
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, discard_cb);
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

char *probe_download_endpoint(CURL *curl, const char *host) {
    CURLcode res;

    const char *patterns[3] = {
        "/download?size=1000000",
        "/random4000x4000.jpg",
        "/100MB.bin"
    };

    size_t count = sizeof(patterns)/sizeof(patterns[0]);
    for(size_t i = 0; i < count; i++) {
        curl_easy_reset(curl);
        curl_download_setopts(curl, 15);
        struct curl_slist *headers = add_headers(curl);

        char url[512];
        snprintf(url, sizeof(url), "%s%s", host, patterns[i]);

        curl_easy_setopt(curl, CURLOPT_URL, url);

        res = curl_easy_perform(curl);
        curl_slist_free_all(headers);
        if(res != CURLE_OK) {
            continue;
        }

        curl_off_t dl_size = 0;
        curl_easy_getinfo(curl, CURLINFO_SIZE_DOWNLOAD_T, &dl_size);

        if (dl_size >= 1024) {
            return strdup(url);
        }
    }

    return NULL;
}

int get_download_speed(Server *servers, size_t count) {
    CURL *curl;

    curl = curl_easy_init();
    if(!curl) {
        fprintf(stderr, "curl init failed\n");
        return EXIT_FAILURE;
    }

    struct curl_slist *headers = add_headers(curl);
    curl_download_setopts(curl, 15);

    int max_reqs = 5;
    curl_off_t total_time_all, total_data_all;
    printf("Download speeds from specified hosts:\n\n");
    for(size_t i = 0; i < count; ++i) {
        total_time_all = 0, total_data_all = 0;
        char *host_dl = probe_download_endpoint(curl, servers[i].host);
        if(!host_dl) {
            fprintf(stderr, "No endpoint for download found on host %s\n", servers[i].host);
            continue;
        }
        printf("Using endpoint %s\n", host_dl);
        curl_easy_reset(curl);
        curl_easy_setopt(curl, CURLOPT_URL, host_dl);
        curl_download_setopts(curl, 15);
        struct curl_slist *headers_local = add_headers(curl);

        for(int req = 0; req < max_reqs; ++req) {
            CURLcode res = curl_easy_perform(curl);

            if(res == CURLE_OK) {
                curl_off_t total_time;
                curl_off_t dl_size;
                curl_easy_getinfo(curl, CURLINFO_TOTAL_TIME_T, &total_time);
                curl_easy_getinfo(curl, CURLINFO_SIZE_DOWNLOAD_T, &dl_size);
                total_time_all += total_time;
                total_data_all += dl_size;
            } else if(res == CURLE_OPERATION_TIMEDOUT) {
                fprintf(stderr, "Timeout from %s\n", servers[i].host);
                break;
            } else if(res == CURLE_COULDNT_CONNECT) {
                fprintf(stderr, "Connection timeout to %s\n", servers[i].host);
                break;
            } else if(res == CURLE_COULDNT_RESOLVE_HOST) {
                fprintf(stderr, "Could not resolve hostname for %s\n", servers[i].host);
                break;
            } else {
                fprintf(stderr, "Failed to download from %s: %s\n", servers[i].host, curl_easy_strerror(res));
            }
        }

        curl_slist_free_all(headers_local);
        free(host_dl);

        if(total_time_all > 0 && total_data_all > 0) {
            double total_time_seconds = (double)total_time_all / 1000000.0;
            double mbps = ((double)total_data_all * 8.0) / 1000000.0 / total_time_seconds;

            printf("%s: %.2f Mbps\n\n", servers[i].host, mbps);
        } else {
            printf("No successful downloads to calculate speed.\n\n");
        }
    }

    curl_slist_free_all(headers);
    curl_easy_cleanup(curl);
    return EXIT_SUCCESS;
}

int get_upload_speed(Server *servers, size_t count) {
    CURL *curl;
    CURLcode res;
    UploadData buf;
    buf.buffer = malloc(UP_BUFFER_SIZE);
    if(!buf.buffer) {
        return EXIT_FAILURE;
    }
    fill_buffer(&buf);

    curl = curl_easy_init();
    if(!curl) {
        fprintf(stderr, "curl init failed\n");
        free(buf.buffer);
        return EXIT_FAILURE;
    }

    struct curl_slist *headers = add_headers(curl);
    curl_upload_setopts(curl, &buf, 15L);

    int max_reqs = 5;
    curl_off_t total_time_all, total_data_all;
    printf("Upload speeds to specified hosts:\n\n");
    for(size_t i = 0; i < count; ++i) {
        total_time_all = 0, total_data_all = 0;
        curl_easy_setopt(curl, CURLOPT_URL, servers[i].host);

        for(int req = 0; req < max_reqs; ++req) {
            res = curl_easy_perform(curl);

            if(res == CURLE_OK) {
                curl_off_t total_time;
                curl_easy_getinfo(curl, CURLINFO_TOTAL_TIME_T, &total_time);
                total_time_all += total_time;
                total_data_all += UP_BUFFER_SIZE;
            } else if(res == CURLE_OPERATION_TIMEDOUT) {
                fprintf(stderr, "Timeout to %s\n", servers[i].host);
                break;
            } else if(res == CURLE_COULDNT_CONNECT) {
                fprintf(stderr, "Connection timeout to %s\n", servers[i].host);
                break;
            } else if(res == CURLE_COULDNT_RESOLVE_HOST) {
                fprintf(stderr, "Could not resolve hostname for %s\n", servers[i].host);
                break;
            } else {
                fprintf(stderr, "Failed to upload to %s: %s\n", servers[i].host, curl_easy_strerror(res));
            }
        }

        if (total_time_all > 0) {
            double total_time_seconds = (double)total_time_all / 1000000.0;
            
            double mbps = ((double)total_data_all * 8.0) / 1000000.0 / total_time_seconds;
            
            printf("%s: %.2f Mbps\n\n", servers[i].host, mbps);
        } else {
            printf("No successful uploads to calculate speed.\n\n");
        }
    }

    free(buf.buffer);
    curl_slist_free_all(headers);
    curl_easy_cleanup(curl);
    return EXIT_SUCCESS;
}