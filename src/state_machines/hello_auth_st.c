#include <stdint.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <string.h>

#include "../includes/buffer.h"
#include "../includes/parser_utils.h"
#include "./includes/hello_auth_st.h"
#include "../includes/socks5_states.h"
#include "../includes/socks5.h"
#include "../utils/includes/users.h"

#define AUTH_VERSION 0x01
#define AUTH_RESPONSE 2
#define STATUS_SUCCESS 0x00
#define STATUS_FAILURE 0x01
#define ATTACHMENT(key)     ( ( struct socks5 * )(key)->data)


void hello_auth_init(const unsigned state, struct selector_key *key){
    struct hello_auth_st * st = ATTACHMENT(key)->hello_auth;
    st->hello_auth_parser = malloc(sizeof(struct hello_auth_parser));
    hello_auth_parser_init(st->hello_auth_parser);

}

void hello_auth_reset(struct hello_auth_st * ha){
    ha->status = -1;
    if(ha->hello_auth_parser != NULL){
        free(ha->hello_auth_parser);
    }
}

void hello_auth_response(buffer * b, struct hello_auth_st * hello_auth){
    size_t n;
    uint8_t * pointer = buffer_write_ptr(b,&n);
    pointer[0] = AUTH_VERSION;
    pointer[1] = hello_auth->status;
    buffer_write_adv(b, AUTH_RESPONSE);
} 

unsigned hello_auth_read(struct selector_key * key){
    struct socks5 * sock = ATTACHMENT(key);
    struct hello_auth_parser * hap = sock->hello_auth->hello_auth_parser;
    size_t nbytes;
    uint8_t * pointer = buffer_write_ptr(&sock->read_buffer, &nbytes);
    ssize_t ret = recv(key->fd, pointer, nbytes, 0);

    if (ret > 0) {
        buffer_write_adv(&sock->read_buffer, ret);
        enum hello_auth_state state = consume_hello_auth(&sock->read_buffer, hap); 
        if(state == hello_auth_end){
            if(!can_login(hap->user, hap->pass)) {
                sock->hello_auth->status = STATUS_FAILURE;
            }
            if(selector_set_interest_key(key, OP_WRITE) != SELECTOR_SUCCESS){
                goto finally;
            }   
            hello_auth_response(&sock->write_buffer, sock->hello_auth); 
        } else if(state == hello_auth_error) {
            hello_auth_response(&sock->write_buffer, sock->hello_auth); 
            if(selector_set_interest_key(key, OP_WRITE) != SELECTOR_SUCCESS){
                goto finally;
            }
        }
    }
    return HELLO_AUTH;
finally:
    hello_auth_response(&sock->write_buffer, sock->hello_auth);
    return ERROR;
}


unsigned hello_auth_write(struct selector_key * key) {
    struct socks5 * sock = ATTACHMENT(key);
    size_t n;
    uint8_t * pointer = buffer_read_ptr(&sock->write_buffer, &n);
    uint8_t ret_state = HELLO_AUTH;
    ssize_t ret = send(key->fd, pointer, n, MSG_NOSIGNAL);
    if(ret > 0){
        buffer_read_adv(&sock->write_buffer, n);
        if(!buffer_can_read(&sock->write_buffer)){
            if(sock->hello_auth->status == STATUS_SUCCESS){
                if(selector_set_interest_key(key, OP_READ) != SELECTOR_SUCCESS){
                    goto finally;
                }
                return REQUEST_READING;
            } else {
                    goto finally;
            }

            
        }
    } else {
        goto finally;
    }
    return ret_state;
finally:
    return ERROR;

}


