#include "../cJSON/cJSON.h"
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

int get_user_location(char *ip, char **city, char **country) {
    if(!city || !country) {
        return EXIT_FAILURE;
    }
    *city = NULL;
    *country = NULL;

    char url[64];
    if(ip && ip[0] != '\0') {
        snprintf(url, sizeof(url), "http://ip-api.com/json/%s", ip);
    } else {
        snprintf(url, sizeof(url), "http://ip-api.com/json/");
    }

    CURL *curl = curl_easy_init();
    if(!curl) {
        fprintf(stderr, "curl init failed\n");
        return EXIT_FAILURE;
    }

    char *response;

    curl_easy_setopt(curl, CURLOPT_URL, url);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_cb);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);

    CURLcode res = curl_easy_perform(curl);
    if(res != CURLE_OK) {
        fprintf(stderr, "curl encountered an error: %s\n", curl_easy_strerror(res));
        free(response);
        curl_easy_cleanup(curl);
        return EXIT_FAILURE;
    }
    curl_easy_cleanup(curl);
    
    cJSON *json = cJSON_Parse(response);
    free(response);

    if(!json) {
        fprintf(stderr, "Failure parsing JSON\n");
        return EXIT_FAILURE;
    }

    cJSON *status = cJSON_GetObjectItem(json, "status");
    if(!status || strcmp(status->valuestring, "success") != 0) {
        fprintf(stderr, "Not able to get response status\n");
        cJSON_Delete(json);
        return EXIT_FAILURE;
    }

    cJSON *city_json = cJSON_GetObjectItem(json, "city");
    cJSON *country_json = cJSON_GetObjectItem(json, "country");

    if(city_json && cJSON_IsString(city_json) && city_json->valuestring) {
        *city = strdup(city_json->valuestring);
    } else {
        fprintf(stderr, "Not able to geolocate city\n");
        cJSON_Delete(json);
        return EXIT_FAILURE;
    }

    if(country_json && cJSON_IsString(country_json) && country_json->valuestring) {
        *country = strdup(country_json->valuestring);
    } else {
        fprintf(stderr, "Not able to geolocate country\n");
        free(*city);
        cJSON_Delete(json);
        return EXIT_FAILURE;
    }

    cJSON_Delete(json);
    return EXIT_SUCCESS;
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