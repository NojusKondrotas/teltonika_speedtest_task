#include "server.h"
#include "../cJSON/cJSON.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

Server create_server(char *country, char *city, char *provider, char *host, int id) {
    Server server = {
        .country = NULL,
        .city = NULL,
        .provider = NULL,
        .host = NULL,
        .id = id
    };

    server.country = strdup(country);
    if(!server.country) {
        return server;
    }

    server.city = strdup(city);
    if(!server.city) {
        free(server.country);
        server.country = NULL;
        return server;
    }

    server.provider = strdup(provider);
    if(!server.provider) {
        free(server.country);
        free(server.city);
        server.country = NULL;
        server.city = NULL;
        return server;
    }

    server.host = strdup(host);
    if(!server.host) {
        free(server.country);
        free(server.city);
        free(server.provider);
        server.country = NULL;
        server.city = NULL;
        server.provider = NULL;
        return server;
    }
    
    return server;
}

Server deepcopy_server(Server server) {
    Server copy = {
        .country = NULL,
        .city = NULL,
        .provider = NULL,
        .host = NULL,
        .id = server.id
    };

    if(server.country) {
        copy.country = strdup(server.country);
        if (!copy.country) {
            return copy;
        }
    }

    if(server.city) {
        copy.city = strdup(server.city);
        if(!copy.city) {
            free(copy.country);
            copy.country = NULL;
            return copy;
        }
    }
    
    if(server.provider) {
        copy.provider = strdup(server.provider);
        if(!copy.provider) {
            free(copy.country);
            free(copy.city);
            copy.country = NULL;
            copy.city = NULL;
            return copy;
        }
    }
    
    if(server.host) {
        copy.host = strdup(server.host);
        if(!copy.host) {
            free(copy.country);
            free(copy.city);
            free(copy.provider);
            copy.country = NULL;
            copy.city = NULL;
            copy.provider = NULL;
            return copy;
        }
    }
    
    return copy;
}

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
    for (int i = 0; i < array_size; ++i) {
        cJSON *item = cJSON_GetArrayItem(json, i);
        cJSON *country = cJSON_GetObjectItem(item, "country");
        cJSON *city = cJSON_GetObjectItem(item, "city");
        cJSON *provider = cJSON_GetObjectItem(item, "provider");
        cJSON *host = cJSON_GetObjectItem(item, "host");
        cJSON *id = cJSON_GetObjectItem(item, "id");

        if(!country || !city || !provider || !host || !id) {
            fprintf(stderr, "A required property was not found in a JSON object at index %d\n", i);
            cleanup_servers(servers, i);
            cJSON_Delete(json);
            return NULL;
        }

        if (!cJSON_IsString(country) || !cJSON_IsString(city) || 
            !cJSON_IsString(provider) || !cJSON_IsString(host) || 
            !cJSON_IsNumber(id)) {
            fprintf(stderr, "A required property is of an invalid type in a JSON object at index %d\n", i);
            cleanup_servers(servers, i);
            cJSON_Delete(json);
            return NULL;
        }

        Server server = create_server(country->valuestring, city->valuestring,
            provider->valuestring, host->valuestring, id->valueint);

        if(!server.country || !server.city || !server.provider || !server.host) {
            fprintf(stderr, "Failure allocating memory for server property\n");
            cleanup_servers(servers, i);
            cJSON_Delete(json);
            return NULL;
        }

        servers[i] = server;
    }
    
    *count = array_size;
    cJSON_Delete(json);
    return servers;
}

Server *get_servers_by_city(Server *servers, size_t n, size_t *filtered_count) {
    
}

Server *get_servers_by_country(Server *servers, size_t n, size_t *filtered_count) {

}

int print_servers(Server *servers, size_t count) {
    if(!servers) return EXIT_FAILURE;
    
    for(size_t i = 0; i < count; ++i) {
        printf("%s - %s, %s - %s, %i\n", servers[i].country, servers[i].city, servers[i].provider, servers[i].host, servers[i].id);
    }

    return EXIT_SUCCESS;
}