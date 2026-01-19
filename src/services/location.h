#ifndef LOCATION_H
#define LOCATION_H

typedef struct {
    char *data;
    size_t size;
}HttpResponse;

int get_user_location(char *ip, char **city, char **country);
int get_user_ip(char **ip);

#endif