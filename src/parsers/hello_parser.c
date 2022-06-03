#include "./includes/hello_parser.h"
#include "../includes/buffer.h"

enum hello_state parse_char(uint8_t c, struct hello_parser * parser) {
    //TODO: parse char and return state
}

 enum hello_state consume_buffer(buffer * b, struct hello_parser * parser) {
    enum hello_state state = parser->state;

    while(buffer_can_read(b)) {
        uint8_t c = buffer_read(b);
        state = parse_char(c, parser); 
        if(state == hello_end){
            break;
        }
    }
    return state;
}