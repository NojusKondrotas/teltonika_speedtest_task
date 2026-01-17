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

Server *load_servers(const char *filepath, size_t *count);

#endif