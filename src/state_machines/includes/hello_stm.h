#ifndef HELLO_STM
#define HELLO_STM

#include "../../includes/buffer.h"
#include "../../includes/selector.h"

struct hello_stm {
   // struct hello_parser parser;
    buffer read_buffer;
    buffer write_buffer;
    uint8_t * read_buffer_data;
    uint8_t * write_buffer_data;


    int selected_method;
    
};

void hello_read_init(const unsigned state, struct selector_key *key);   //TODO: creo que tienen que retornar void *
unsigned hello_read(struct selector_key * key);

#endif
