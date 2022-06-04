#ifndef HELLO_PARSER_H
#define HELLO_PARSER_H

#include <stdlib.h>

enum hello_state {
    hello_reading_version,
    hello_reading_nmethods,
    hello_reading_methods,
    hello_end
};

static const struct parser_state_transition hello_reading_version [] =  {
    {.when = '5',        .dest = S0,        .act1 = foo,},
    {.when = 'f',        .dest = S0,        .act1 = foo,},
    {.when = ANY,        .dest = S1,        .act1 = bar,},
};

#define N(x) (sizeof(x)/sizeof((x)[0]))


static const size_t states_n [] = {
    N(ST_S0),
    N(ST_S1),
};

static struct parser_definition definition = {
    .states_count = N(hello_state),
    .states       = hello_state,
    .states_n     = states_n,
    .start_state  = S0,
};


struct hello_parser {
    enum hello_state state;
    uint8_t * methods;
    uint8_t index;
};

enum hello_state consume_buffer(buffer * b, struct hello_parser * parser);

#endif