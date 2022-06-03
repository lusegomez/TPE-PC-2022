#ifndef HELLO_PARSER_H
#define HELLO_PARSER_H

#include <stdlib.h>

enum hello_state {
    hello_reading_version,
    hello_reading_nmethods,
    hello_reading_methods,
    hello_end
};

struct hello_parser {
    enum hello_state state;
    uint8_t * methods;
    uint8_t index;
};

enum hello_state consume_buffer(buffer * b, struct hello_parser * parser);

#endif