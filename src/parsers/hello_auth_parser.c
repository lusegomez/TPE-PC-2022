#include "./includes/hello_auth_parser.h"
#include "../includes/buffer.h"

#define AUTH_VERSION 0x01

void hello_auth_parser_init(struct hello_auth_parser * hap){
    hap-> state = hello_auth_reading_version;
    hap-> user_index = 0;
    hap-> pass_index = 0;
}

enum hello_auth_state consume_hello_auth_byte(uint8_t c, struct hello_auth_parser * hap){
    switch (hap->state){
    case hello_auth_reading_version:
        if(c == AUTH_VERSION){
            hap->state = hello_auth_reading_ulen;
        }else {
            hap-> state =hello_auth_error;
        }
        break;
    case hello_auth_reading_ulen:
        if(c <= 0) {
            hap->state = hello_auth_error;
        } else{
            hap->ulen = c;
            hap->state = hello_auth_reading_uname;
        }
        break;
    case hello_auth_reading_uname:
        hap->user[hap->user_index++] = c;
        if(hap->user_index == hap->ulen){
            hap->state = hello_auth_reading_plen;
        } else{
            hap->state = hello_auth_reading_uname;
        }
        break;
    case hello_auth_reading_plen:
        if(c <= 0) {
            hap->state = hello_auth_error;
        } else{
            hap->plen = c;
            hap->state = hello_auth_reading_password;
        }
        break;
    case hello_auth_reading_password:
        hap->pass[hap->pass_index++]  = c;
        if(hap->pass_index == hap->plen){
            hap->state = hello_auth_end;
        } else{
            hap->state = hello_auth_reading_password;
        }
        break;
    case hello_auth_end:
    case hello_auth_error:
        break;
    }
    return hap->state;
}

enum hello_auth_state consume_hello_auth(buffer * b, struct hello_auth_parser * hap){
    enum hello_auth_state state = hap->state;
    while(buffer_can_read(b)) {
        const uint8_t c = buffer_read(b);
        state = consume_hello_auth_byte(c, hap);
        if(state == hello_auth_end || state == hello_auth_error){
            break;
        }
    }
    return state;
}
