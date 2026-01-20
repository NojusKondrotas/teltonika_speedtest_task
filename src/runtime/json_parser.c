#include "../libs/cJSON/cJSON.h"
#include "json_parser.h"

#include <stdio.h>
#include <stdlib.h>

cJSON *parse_json_file(const char *filepath) {
    FILE *fptr;

    if((fptr = fopen(filepath, "r")) == NULL) {
        fprintf(stderr, "Failure opening file: %s\n", filepath);
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

    return json;
}