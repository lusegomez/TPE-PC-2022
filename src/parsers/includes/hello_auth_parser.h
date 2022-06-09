#ifndef HELLO_AUTH_PARSER_H
#define HELLO_AUTH_PARSER_H

#include <stdint.h>
#include "../../includes/buffer.h"

enum hello_auth_state {
    hello_auth_reading_version,
    hello_auth_reading_ulen,
    hello_auth_reading_uname,
    hello_auth_reading_plen,
    hello_auth_reading_password,
    hello_auth_end,
    hello_auth_error,
};

struct hello_auth_parser {
    enum hello_auth_state state;
    uint8_t user[255];
    uint8_t pass[255];
    uint8_t user_index;
    uint8_t ulen;
    uint8_t pass_index;
    uint8_t plen;

};

enum hello_auth_state consume_hello_auth(buffer * b, struct hello_auth_parser * hap);

#endif