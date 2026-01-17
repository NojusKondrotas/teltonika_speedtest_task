#include "server.h"
#include "../cJSON/cJSON.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void cleanup_servers(Server *servers, size_t count) {
    for (int i = 0; i < count; i++) {
        free(servers[i].country);
        free(servers[i].city);
        free(servers[i].provider);
        free(servers[i].host);
    }
    free(servers);
}

Server *load_servers(const char *filepath, size_t *count) {
    FILE *fptr;

    if((fptr = fopen(filepath, "r")) == NULL) {
        fprintf(stderr, "Failure opening servers' file: %s\n", filepath);
        return NULL;
    }

    long file_size = 0;
    fseek(fptr, 0, SEEK_END);
    file_size = ftell(fptr);
    fseek(fptr, 0, SEEK_SET);

    char *json_str = malloc(file_size + 1);
    if(!json_str) {
        fprintf(stderr, "Failure allocating memory for json reading\n");
        fclose(fptr);
        return NULL;
    }

    fread(json_str, 1, file_size, fptr);
    json_str[file_size] = '\0';
    fclose(fptr);

    cJSON *json = cJSON_Parse(json_str);
    free(json_str);

    if(!json) {
        fprintf(stderr, "Failure parsing JSON\n");
        return NULL;
    }

    if(!cJSON_IsArray(json)) {
        fprintf(stderr, "Failure: JSON root element must be an array, it is not\n");
        cJSON_Delete(json);
        return NULL;
    }

    int array_size = cJSON_GetArraySize(json);
    Server *servers = calloc(array_size, sizeof(Server));
    if(!servers) {
        fprintf(stderr, "Failure allocating memory for servers' array\n");
        cJSON_Delete(json);
        return NULL;
    }
    for (int i = 0; i < array_size; i++) {
        cJSON *item = cJSON_GetArrayItem(json, i);
        cJSON *country = cJSON_GetObjectItem(item, "country");
        cJSON *city = cJSON_GetObjectItem(item, "city");
        cJSON *provider = cJSON_GetObjectItem(item, "provider");
        cJSON *host = cJSON_GetObjectItem(item, "host");
        cJSON *id = cJSON_GetObjectItem(item, "id");
        
        if (country && country->valuestring) {
            servers[i].country = strdup(country->valuestring);
            if(!servers[i].country) {
                fprintf(stderr, "Failure allocating memory for server property\n");
                cleanup_servers(servers, i);
                cJSON_Delete(json);
                return NULL;
            }
        }
        if (city && city->valuestring) {
            servers[i].city = strdup(city->valuestring);
            if(!servers[i].city) {
                fprintf(stderr, "Failure allocating memory for server property\n");
                cleanup_servers(servers, i);
                cJSON_Delete(json);
                return NULL;
            }
        }
        if (provider && provider->valuestring) {
            servers[i].provider = strdup(provider->valuestring);
            if(!servers[i].provider) {
                fprintf(stderr, "Failure allocating memory for server property\n");
                cleanup_servers(servers, i);
                cJSON_Delete(json);
                return NULL;
            }
        }
        if (host && host->valuestring) {
            servers[i].host = strdup(host->valuestring);
            if(!servers[i].host) {
                fprintf(stderr, "Failure allocating memory for server property\n");
                cleanup_servers(servers, i);
                cJSON_Delete(json);
                return NULL;
            }
        }
        if (id) {
            servers[i].id = id->valueint;
        }
    }
    
    *count = array_size;
    cJSON_Delete(json);
    return servers;
}

Server *get_servers_by_city(Server *servers, size_t n) {
    
}

Server *get_servers_by_country(Server *servers, size_t n) {

}