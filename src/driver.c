#include "driver.h"
#include "services/location.h"
#include "server/server.h"
#include "services/speedtest.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <getopt.h>

struct option long_options[] = {
    {"path", required_argument, 0, 'P'},
    {"host", required_argument, 0, 'H'},
    {"dutimeout", required_argument, 0, 'T'},
    {"city", required_argument, 0, 'c'},
    {"country", required_argument, 0, 'C'},
    {0, 0, 0, 0}
};

int parse_cmd_args(int argc, char *argv[], Flags *flags) {
    int opt;
    while((opt = getopt_long(argc, argv, "dusl", long_options, NULL)) != -1) {
        switch(opt) {
            case 'd':
                flags->d_flag = 1;
                break;
            case 'u':
                flags->u_flag = 1;
                break;
            case 's':
                flags->s_flag = 1;
                break;
            case 'l':
                flags->l_flag = 1;
                break;
            case 'P':
                flags->path = optarg;
                ++flags->server_directives;
                break;
            case 'T':
                char *endptr;
                long val;

                val = strtol(optarg, &endptr, 10);

                if(endptr == optarg) {
                    fprintf(stderr, "ERROR: --dutimeout: '%s' is not a valid number\n", optarg);
                    return EXIT_FAILURE;
                }

                if(*endptr != '\0') {
                    fprintf(stderr, "ERROR: --dutimeout: '%s' contains extra non-numerical characters\n", optarg);
                    return EXIT_FAILURE;
                }

                flags->dutimeout = (int)val;
                break;
            case 'H':
                flags->host = optarg;
                ++flags->server_directives;
                break;
            case 'c':
                flags->city = optarg;
                ++flags->server_filters;
                break;
            case 'C':
                flags->country = optarg;
                ++flags->server_filters;
                break;

            case '?':
                fprintf(stderr, "Use valid options\n");
                return EXIT_FAILURE;
            default:
                fprintf(stderr, "Unexpected option: %c\n", opt);
                return EXIT_FAILURE;
        }
    }

    return EXIT_SUCCESS;
}

int perform_download_speed_test(DownloadArgs *args) {
    get_download_speed(args->servers, args->count);
    return EXIT_SUCCESS;
}

int perform_upload_speed_test(UploadArgs *args) {
    get_upload_speed(args->servers, args->count);
    return EXIT_SUCCESS;
}

int main(int argc, char *argv[]) {
    Flags flags = {
        .d_flag = 0,
        .u_flag = 0,
        .s_flag = 0,
        .l_flag = 0,

        .path = NULL,
        .host = NULL,
        .dutimeout = 0,
        .city = NULL,
        .country = NULL,

        .server_directives = 0,
        .server_filters = 0
    };

    if(parse_cmd_args(argc, argv, &flags) == EXIT_FAILURE) {
        return EXIT_FAILURE;
    }

    if(flags.server_filters > 1) {
        fprintf(stderr, "Multiple server filters specified (--city and --country). Ambiguity between which one to apply\n");
        return EXIT_FAILURE;
    }
    
    size_t count;
    Server *servers = NULL;
    if(flags.d_flag || flags.u_flag) {
        if(flags.server_directives == 0) {
            fprintf(stderr, "A server directive must be specified with -d and -u flags. What servers to test?\n");
            return EXIT_FAILURE;
        } else if(flags.server_directives > 1) {
            fprintf(stderr, "Only one server directive must be specified with -d and -u flags (--path or --host). Ambiguity between which server(s) to test\n");
            return EXIT_FAILURE;
        } else {
            if(flags.path) {
                servers = load_servers(flags.path, &count);
                if(!servers) {
                    return EXIT_FAILURE;
                }
            } else if(flags.host) {
                count = 1;
                servers = malloc(sizeof(Server));
                if(!servers) {
                    fprintf(stderr, "Failure allocating memory for server\n");
                    return EXIT_FAILURE;
                }
                servers[0] = (Server){
                    .city = NULL,
                    .country = NULL,
                    .host = flags.host,
                    .id = -1,
                    .provider = NULL
                };
            } else {
                fprintf(stderr, "Internal error: server directive specified but neither path nor host found\n");
                return EXIT_FAILURE;
            }
                
            if(flags.city) {
                Server *tmp = get_servers_by_city(servers, count, &count);

                cleanup_servers(servers, count);
                if(!tmp) {
                    return EXIT_FAILURE;
                }

                servers = tmp;
            } else if(flags.country) {
                Server *tmp = get_servers_by_country(servers, count, &count);

                cleanup_servers(servers, count);
                if(!tmp) {
                    return EXIT_FAILURE;
                }
                
                servers = tmp;
            }
        }
    }

    if(flags.d_flag) {
        DownloadArgs args = {
            .servers = servers,
            .count = count,
            .timeout = flags.dutimeout > 0 ? flags.dutimeout : 15
        };
        if(perform_download_speed_test(&args) == EXIT_FAILURE) {
            cleanup_servers(servers, count);
            return EXIT_FAILURE;
        }
    }

    if(flags.u_flag) {
        UploadArgs args = {
            .servers = servers,
            .count = count,
            .timeout = flags.dutimeout > 0 ? flags.dutimeout : 15
        };
        if(perform_upload_speed_test(&args) == EXIT_FAILURE) {
            cleanup_servers(servers, count);
            return EXIT_FAILURE;
        }
    }

    if(flags.s_flag) {
        char *ip = "0";
        if(get_user_ip(&ip) != EXIT_SUCCESS) {
            return EXIT_FAILURE;
        }
        printf("User IP: %s\n", ip);
        free(ip);
    }

    return EXIT_SUCCESS;
}