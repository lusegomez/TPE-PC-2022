#ifndef TPE_PC_2022_REQUEST_READ_ST_H
#define TPE_PC_2022_REQUEST_READ_ST_H

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include "../../includes/selector.h"
#include "../../parsers/includes/request_read_parser.h"

struct request_read_st {
    struct request_read_parser * req_parser;
};

void request_read_init(const unsigned state, struct selector_key * key);
unsigned request_read(struct selector_key * key);

#endif //TPE_PC_2022_REQUEST_READ_ST_H
