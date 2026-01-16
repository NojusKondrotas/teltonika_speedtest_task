#include "driver.h"

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

                break;
            case 'P':
                flags->path = optarg;
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
                break;
            case 'c':
                flags->city = optarg;
                break;
            case 'C':
                flags->country = optarg;
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
        .country = NULL
    };

    if(parse_cmd_args(argc, argv, &flags) == EXIT_FAILURE) {
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}