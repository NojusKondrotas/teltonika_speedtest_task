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
                } 
            else if(res == CURLE_OPERATION_TIMEDOUT) {
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