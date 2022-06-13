#ifndef TPE_PC_2022_REQUEST_READ_ST_H
#define TPE_PC_2022_REQUEST_READ_ST_H

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include "../../includes/selector.h"
#include "../../parsers/includes/request_read_parser.h"
#include "../../includes/socks5.h"


#define SUCCEDED                          0x00
#define GENERAL_SOCKS_SERVER_FAILURE      0X01
#define CONNECTION_NOT_ALLOWED_BY_RULESET 0x02
#define NETWORK_UNREACHABLE               0x03
#define HOST_UNREACHABLE                  0x04
#define CONNECTION_REFUSED                0x05
#define TTL_EXPIRED                       0x06
#define COMMAND_NOT_SUPPORTED             0x07
#define ADDRESS_TYPE_NOT_SUPPORTED        0x08


struct request_read_st {
    struct request_read_parser * req_parser;
    unsigned status;
};

void request_read_init(const unsigned state, struct selector_key * key);
void request_read_reset(struct request_read_st * rq);
unsigned request_read(struct selector_key * key);
unsigned response_write(struct selector_key * key);


#endif //TPE_PC_2022_REQUEST_READ_ST_H
