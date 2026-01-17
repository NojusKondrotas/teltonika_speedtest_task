#ifndef DRIVER_H
#define DRIVER_H

#include <stdlib.h>

typedef struct s_driver_flags{
    int d_flag, u_flag, s_flag, l_flag;
    char *path;
    char *host;
    int dutimeout;
    char *city;
    char *country;
    size_t server_directives;
}Flags;

int parse_cmd_args(int argc, char *argv[], Flags *flags);

#endif