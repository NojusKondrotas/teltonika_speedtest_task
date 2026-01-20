#ifndef JSON_PARSER_H
#define JSON_PARSER_H

#include "../libs/cJSON/cJSON.h"

cJSON *parse_json_file(const char *filepath);

#endif