#ifndef HELLO_AUTH_ST_H
#define HELLO_AUTH_ST_H

#include "../../includes/buffer.h"
#include "../../includes/selector.h"
#include "../../parsers/includes/hello_auth_parser.h"

struct hello_auth_st {
    struct hello_auth_parser * hello_auth_parser;
    uint8_t status;
    uint8_t version;
    uint8_t user_len;
    uint8_t username[255];
    uint8_t pass_len;
    uint8_t password[255];
};


void hello_auth_init(const unsigned state, struct selector_key *key);
unsigned hello_auth_read(struct selector_key * key);
unsigned hello_auth_write(struct selector_key * key);

#endif