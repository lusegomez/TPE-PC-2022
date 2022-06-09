#include "includes/request_read_parser.h"
#include "../includes/buffer.h"

#define PROXY_SOCKS5_V5     0x05

void request_read_parser_init(struct request_read_parser *rp) {
    rp->state = request_reading_version;
    rp->version = 0;
    rp->command = 0;
    rp->destaddr_len = 0;
    rp->atype = 0;
    rp->index = 0;
}

enum request_read_state consume_request_byte(uint8_t c, struct request_read_parser * rp){
    switch(rp->state) {
        case request_reading_version:
            if(c == PROXY_SOCKS5_V5){
                rp->state = request_reading_command;
            } else {
                rp->state = request_reading_error;
            }
            break;
        case request_reading_command:
            if(c != 1) {
                rp -> state = request_reading_command_error;
            } else {
                rp->command = c;
                rp->state = request_reading_reserved;
            }
            break;
        case request_reading_reserved:
            if(c != 0){
                rp->state = request_reading_error;
            } else {
                rp->state = request_reading_atype;
            }
            break;
        case request_reading_atype:
            if(c == 3){
                rp->state = request_reading_destaddr_len;
            } else if(c == 1 || c == 4) {
                rp->atype = c;
                rp->state = request_reading_destaddr;
                rp->destaddr_len = c*4 + 1;
                rp->destaddr = calloc(c*4 + 1, sizeof(char));
            } else {
                rp->state = request_reading_atype_error;
            }
            break;
        case request_reading_destaddr_len:
            if(c <= 0){
                rp->state = request_reading_error;
            } else {
                rp ->state = request_reading_destaddr;
                rp->destaddr_len = c+1;
                rp->destaddr = calloc(c+1, sizeof(char));
            }
            break;
        case request_reading_destaddr:
            rp->destaddr[rp->index++] = c;
            if(rp->index == rp->destaddr_len-1){
                rp->destaddr[rp->index] = 0;
                rp->state = request_reading_destport;
                rp->index = 0;
            }
            break;
        case request_reading_destport:
            rp->port[rp->index++] = c;
            if(rp->index == 2){
                rp->state = request_reading_end;
            }
            break;
        case request_reading_end:
        case request_reading_command_error:
        case request_reading_atype_error:
        case request_reading_error:
            break;
    }
    return rp->state;
}

enum request_read_state consume_request(buffer * b, struct request_read_parser * rp) {
    enum request_read_state state = rp->state;

    while(buffer_can_read(b)) {
        const uint8_t c = buffer_read(b);
        state = consume_request_byte(c, rp);
        if(state == request_reading_end || state == request_reading_error) {
            break;
        }

    }
    return state;
}