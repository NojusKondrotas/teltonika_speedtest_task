#include "driver.h"
#include "services/location.h"
#include "server/server.h"
#include "services/speedtest.h"

#include <curl/curl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <getopt.h>

struct option long_options[] = {
    {"path", required_argument, 0, 'P'},
    {"host", required_argument, 0, 'H'},
    {"timeout", required_argument, 0, 'T'},
    {"city", required_argument, 0, 'c'},
    {"country", required_argument, 0, 'C'},
    {"user", no_argument, 0, 'U'},
    {"disableSSL", no_argument, 0, 'D'},
    {"joint", no_argument, 0, 'J'},
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
                    fprintf(stderr, "ERROR: --timeout: '%s' is not a valid number\n", optarg);
                    return EXIT_FAILURE;
                }

                if(*endptr != '\0') {
                    fprintf(stderr, "ERROR: --timeout: '%s' contains extra non-numerical characters\n", optarg);
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
            case 'U':
                flags->user = 1;
                break;
            case 'D':
                flags->disableSSL = 1;
                break;
            case 'J':
                flags->joint = 1;
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
    get_download_speed(args->servers, args->count, args->timeout, args->disableSSL);
    return EXIT_SUCCESS;
}

int perform_upload_speed_test(UploadArgs *args) {
    get_upload_speed(args->servers, args->count, args->timeout, args->disableSSL);
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
        .user = 0,
        .disableSSL = 0,
        .joint = 0,

        .server_directives = 0,
        .server_filters = 0
    };

    if(parse_cmd_args(argc, argv, &flags) == EXIT_FAILURE) {
        return EXIT_FAILURE;
    }

    if(flags.server_directives > 1) {
        fprintf(stderr, "Multiple server directives specified (--path and --host). Ambiguity between which server(s) to test\n");
        return EXIT_FAILURE;
    }

    if(flags.server_filters > 1) {
        fprintf(stderr, "Multiple server filters specified (--city and --country). Ambiguity between which one to apply\n");
        return EXIT_FAILURE;
    }

    if(flags.dutimeout < 0) {
        fprintf(stderr, "Timeout cannot be < 0\n");
        return EXIT_FAILURE;
    }
    
    size_t s_count;
    Server *servers = NULL;
    if(flags.server_directives == 1) {
        if(flags.path) {
            servers = load_servers(flags.path, &s_count);
            if(!servers) {
                return EXIT_FAILURE;
            }
        } else if(flags.host) {
            s_count = 1;
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
    }

    if(flags.server_filters == 1) {
        if(flags.city) {
            size_t new_count;
            Server *tmp = get_servers_by_city(servers, s_count, flags.city, &new_count);

            cleanup_servers(servers, s_count);
            if(!tmp) {
                return EXIT_FAILURE;
            }

            servers = tmp;
            s_count = new_count;
        } else if(flags.country) {
            size_t new_count;
            Server *tmp = get_servers_by_country(servers, s_count, flags.country, &new_count);

            cleanup_servers(servers, s_count);
            if(!tmp) {
                return EXIT_FAILURE;
            }

            servers = tmp;
            s_count = new_count;
        } else {
            fprintf(stderr, "Internal error: server filter specified but neither country nor city found\n");
            return EXIT_FAILURE;
        }
    }

    if(flags.joint) {
        if(flags.server_directives == 0) {
            fprintf(stderr, "A server directive must be specified when executing download or upload tests. What servers to test?\n");
            return EXIT_FAILURE;
        }

        CURL *curl_DL = curl_easy_init();
        CURL *curl_UL = curl_easy_init();
        if(!curl_DL || !curl_UL) {
            fprintf(stderr, "curl init failed\n");
            cleanup_servers(servers, s_count);
            return EXIT_FAILURE;
        }
        UploadData buf;
        buf.buffer = malloc(UP_BUFFER_SIZE);
        if(!buf.buffer) {
            fprintf(stderr, "Upload buffer alloc failed\n");
            cleanup_servers(servers, s_count);
            return EXIT_FAILURE;
        }
        fill_buffer(&buf);
        curl_download_setopts(curl_DL, flags.dutimeout, flags.disableSSL);
        struct curl_slist *headers_DL = add_headers(curl_DL);
        curl_upload_setopts(curl_UL, &buf, flags.dutimeout, flags.disableSSL);
        struct curl_slist *headers_UL = add_headers(curl_UL);

        for(size_t i = 0; i < s_count; ++i) {
            double mbpsDL = get_download_speed_single(curl_DL, &servers[i], flags.dutimeout, flags.disableSSL);
            double mbpsUL = get_upload_speed_single(curl_UL, &servers[i], flags.dutimeout, flags.disableSSL);
            if(mbpsDL != -1) {
                printf("Download speed:   \t%.2f Mbps\n", mbpsDL);
            }
            if(mbpsUL != -1) {
                printf("Upload speed:     \t%.2f Mbps\n", mbpsUL);
            }
            char *host_clean = remove_port(servers[i].host);
            if (!host_clean) {
                fprintf(stderr, "Failed to remove port for host %s\n", servers[i].host);
                continue;
            }
            printf("Tested host: %s\n\n", servers[i].host);
            free(host_clean);
        }

        free(buf.buffer);
        curl_slist_free_all(headers_DL);
        curl_slist_free_all(headers_UL);
        curl_easy_cleanup(curl_DL);
        curl_easy_cleanup(curl_UL);

        char *ip;
        if(get_user_ip(&ip) == EXIT_FAILURE) {
            return EXIT_FAILURE;
        }
        printf("User IP: %s\n", ip);
        free(ip);

        char *city, *country;
        if(get_user_location("", &city, &country) == EXIT_FAILURE) {
            free(city);
            free(country);
            cleanup_servers(servers, s_count);
            return EXIT_FAILURE;
        }

        printf("Country: %s, City: %s\n", country, city);
        free(city);
        free(country);
        printf("\n");
    }
    
    if(flags.d_flag || flags.u_flag) {
        if(flags.server_directives == 0) {
            fprintf(stderr, "A server directive must be specified with -d and -u flags. What servers to test?\n");
            return EXIT_FAILURE;
        }
    }

    if(flags.d_flag) {
        DownloadArgs args = {
            .servers = servers,
            .count = s_count,
            .timeout = flags.dutimeout > 0 ? (size_t)flags.dutimeout : DEFAULT_TIMEOUT,
            .disableSSL = flags.disableSSL
        };
        if(perform_download_speed_test(&args) == EXIT_FAILURE) {
            cleanup_servers(servers, s_count);
            return EXIT_FAILURE;
        }
    }

    if(flags.u_flag) {
        UploadArgs args = {
            .servers = servers,
            .count = s_count,
            .timeout = flags.dutimeout > 0 ? (size_t)flags.dutimeout : DEFAULT_TIMEOUT,
            .disableSSL = flags.disableSSL
        };
        if(perform_upload_speed_test(&args) == EXIT_FAILURE) {
            cleanup_servers(servers, s_count);
            return EXIT_FAILURE;
        }
    }

    if(flags.s_flag) {
        char *ip;
        if(get_user_ip(&ip) == EXIT_FAILURE) {
            return EXIT_FAILURE;
        }
        printf("User IP: %s\n", ip);
        free(ip);
    }

    if(flags.l_flag) {
        char *city = NULL, *country = NULL;
        if(flags.user || (!flags.path && !flags.host)) {
            if(get_user_location("", &city, &country) == EXIT_FAILURE) {
                free(city);
                free(country);
                cleanup_servers(servers, s_count);
                return EXIT_FAILURE;
            }
            printf("User location:\nCountry: %s, City: %s\n", country, city);
            free(city);
            free(country);
            printf("\n");
        } else {
            char *host_clean;
            for(size_t i = 0; i < s_count; ++i) {
                host_clean = remove_port(servers[i].host);
                if (!host_clean) {
                    fprintf(stderr, "Failed to remove port for host %s\n", servers[i].host);
                    continue;
                }

                if (get_user_location(host_clean, &city, &country) == EXIT_SUCCESS) {
                    printf("Host's %s location:\nCountry: %s, City: %s\n", host_clean, country, city);
                }

                free(host_clean);
                free(city);
                free(country);
                city = country = NULL;
                printf("\n");
            }
        }
    }

    cleanup_servers(servers, s_count);
    return EXIT_SUCCESS;
}