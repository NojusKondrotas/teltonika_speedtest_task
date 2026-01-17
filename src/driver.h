#ifndef DRIVER_H
#define DRIVER_H

#include "server/server.h"

#include <stdlib.h>

typedef struct s_driver_flags{
    int d_flag, u_flag, s_flag, l_flag;
    char *path;
    char *host;
    int dutimeout;
    char *city;
    char *country;
    size_t server_directives;
    size_t server_filters;
}Flags;

typedef struct s_download_args{
    Server *servers;
    size_t count;
    size_t timeout;
}DownloadArgs;

typedef struct s_upload_args{
    Server *servers;
    size_t count;
    size_t timeout;
}UploadArgs;

int parse_cmd_args(int argc, char *argv[], Flags *flags);

int perform_download_speed_test(DownloadArgs *args);

int perform_upload_speed_test(UploadArgs *args);

#endif