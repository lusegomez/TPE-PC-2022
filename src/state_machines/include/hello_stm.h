#include "../../buffer.h"
struct hello_stm {
    struct hello_parser parser;
    buffer read_buffer;
    buffer write_buffer;
    uint8_t * read_buffer_data;
    uint8_t * write_buffer_data;


    int selected_method;
    
}

