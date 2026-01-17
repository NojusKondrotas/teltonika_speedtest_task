#ifndef SERVER_H
#define SERVER_H

#include <stdlib.h>

typedef struct s_server {
    char *country;
    char *city;
    char *provider;
    char *host;
    int id;
}Server;

Server deepcopy_server(Server server);

void cleanup_servers(Server *servers, size_t count);

Server *load_servers(const char *filepath, size_t *count);
Server *get_servers_by_city(Server *servers, size_t n, size_t *filtered_count);
Server *get_servers_by_country(Server *servers, size_t n, size_t *filtered_count);

int print_servers(Server *servers, size_t count);

#endif