#include "driver.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <getopt.h>

int main(int argc, char *argv[]) {
    int opt;
    
    int d_flag = 0, u_flag = 0, s_flag = 0, l_flag = 0;

    int all_flag = 0;
    char *path = NULL;
    char *host = NULL;
    int dutimeout = 0;
    char *city = NULL;
    char *country = NULL;

    struct option long_options[] = {
        {"all", no_argument, 0, 'a'},             // --all
        {"path", required_argument, 0, 'P'},      // --path
        {"host", required_argument, 0, 'H'},      // --host
        {"dutimeout", required_argument, 0, 'T'}, // --dutimeout
        {"city", required_argument, 0, 'c'},      // --city
        {"country", required_argument, 0, 'C'},   // --country
        {0, 0, 0, 0}
    };

    while((opt = getopt_long(argc, argv, "dusl", long_options, NULL)) != -1) {
        switch(opt) {
            case 'd':
                d_flag = 1;
                break;
            case 'u':
                u_flag = 1;
                break;
            case 's':
                s_flag = 1;
                break;
            case 'l':
                l_flag = 1;
                break;

            case 'a': // --all
                all_flag = 1;
                break;
            case 'P': // --path
                path = optarg;
                break;
            case 'T': // --dutimeout
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

                dutimeout = (int)val;
                break;
            case 'H': // --host
                host = optarg;
                break;
            case 'c': // --city
                city = optarg;
                break;
            case 'C': // --country
                country = optarg;
                break;

            case '?':
                fprintf(stderr, "Use valid options\n");
                return 1;
            default:
                fprintf(stderr, "Unexpected option: %c\n", opt);
                return 1;
        }
    }

    return EXIT_SUCCESS;
}