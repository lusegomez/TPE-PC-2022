#ifndef HELLO_STM
#define HELLO_STM

#include "../../includes/buffer.h"
#include "../../includes/selector.h"
#include "../../parsers/includes/hello_parser.h"


struct hello_st {
    struct hello_parser * hello_parser;
    int selected_method;
    
};

void hello_init(const unsigned state, struct selector_key *key);   
unsigned hello_read(struct selector_key * key);
unsigned hello_write(struct selector_key * key);

#endif
