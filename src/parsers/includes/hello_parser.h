#ifndef HELLO_PARSER_H
#define HELLO_PARSER_H

#include <stdlib.h>
#include <stdio.h>
#include "../../includes/buffer.h"
#include "../../utils/includes/logger.h"

#define PROXY_SOCKS5_V5     0x05

enum hello_state {
    hello_reading_version,
    hello_reading_nmethods,
    hello_reading_methods,
    hello_end,
    hello_error
};


struct hello_parser {
    enum hello_state state;
    uint8_t * methods;
    uint8_t nmethods;
    uint8_t index;
};

void hello_parser_init(struct hello_parser *hp);
enum hello_state consume_hello(buffer * b, struct hello_parser * hp);


#endif