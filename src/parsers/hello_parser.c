#include "./includes/hello_parser.h"
#include "../includes/buffer.h"

void hello_parser_init(struct hello_parser *hp) {
    hp->state = hello_reading_version;
    hp->methods = NULL;
    hp->index = 0;
}

enum hello_state consume_hello_byte(uint8_t c, struct hello_parser * hp){
    switch(hp->state) {
        case hello_reading_version:
            if(c == PROXY_SOCKS5_V5){
                hp->state = hello_reading_nmethods;
            } else {
                plog(ERRORR, "Invalid socks version: %d", c);
                hp->state = hello_error;
            }
            break;
        case hello_reading_nmethods:
            if(c <= 0) {
                hp -> state = hello_error;
            } else {
                hp->nmethods= c;
                hp->methods = calloc(c, sizeof(c));
                if(hp->methods == NULL) {
                    hp->state = hello_error;
                    return hp->state;
                }
                hp->state = hello_reading_methods;
            }
            break;
        case hello_reading_methods:
            hp->methods[hp->index++] = c;
            if(hp->index == hp->nmethods){
                hp->state = hello_end;
            }
            break;
        case hello_end:
        case hello_error:
            break;
    }
    return hp->state;
}

enum hello_state consume_hello(buffer * b, struct hello_parser * hp) {
    enum hello_state state = hp->state;

    while(buffer_can_read(b)) {
        const uint8_t c = buffer_read(b);
        state = consume_hello_byte(c, hp);
        if(state == hello_end || state == hello_error) {
            break;
        }

    }
    return state;
}