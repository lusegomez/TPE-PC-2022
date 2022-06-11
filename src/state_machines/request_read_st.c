#include <stdint.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <string.h>

#include "../includes/parser_utils.h"
#include "./includes/request_read_st.h"
#include "../includes/socks5_states.h"
#include "../includes/socks5.h"

#define IPV4 0x01
#define FQDN 0x03
#define IPV6 0x04
//Errors
#define GERROR 0x01
#define CERROR 0x07
#define ATERROR 0x08

#define RESPLEN 10

#define SOCKSVERSION 0x05

#define ATTACHMENT(key)     ( ( struct socks5 * )(key)->data)

void request_read_init(const unsigned state, struct selector_key * key){
    struct request_read_st * st = ATTACHMENT(key)->request_read;
    st->req_parser = malloc(sizeof(struct request_read_parser));
    request_read_parser_init(st->req_parser);
}

void request_read_reset(struct request_read_st * rq){
    if(rq->req_parser != NULL){
        free(rq->req_parser);
    }
}

void request_response(buffer * b, struct request_read_st * req);

unsigned request_read(struct selector_key * key) {
    struct socks5 * sock = ATTACHMENT(key);
    struct request_read_parser * rp = sock->request_read->req_parser;
    size_t nbytes;
    uint8_t * pointer = buffer_write_ptr(&sock->read_buffer, &nbytes); //Agarro el write pointer para escribir en el read_buff con el recv
    ssize_t ret = recv(key->fd, pointer, nbytes, 0);

    if(ret > 0) {
        buffer_write_adv(&sock->read_buffer, ret);
        enum request_read_state state = consume_request(&sock->read_buffer, rp);
        if(state == request_reading_end){

            if(selector_set_interest_key(key, OP_NOOP) != SELECTOR_SUCCESS){
                goto finally;
            }

        } else if(state == request_reading_error){

            request_response(&sock->write_buffer, sock->request_read);

            if(selector_set_interest_key(key, OP_WRITE) != SELECTOR_SUCCESS){
                goto finally;
            }
            return RESPONSE_WRITING;

        }
    }
    return rp->atype == FQDN ? DNS_QUERY : CONNECT_ORIGIN;
    finally:
    request_response(&sock->write_buffer, sock->request_read);
    return ERROR;
}

void request_response(buffer * b, struct request_read_st * req){
    size_t n;
    uint8_t * pointer = buffer_write_ptr(b,&n);
    //TODO: checkear que se puedan escribir 2 bytes
    pointer[0] = SOCKSVERSION;
    switch (req->req_parser->state) {
        case request_reading_command_error:
            pointer[1] = CERROR;
            break;
        case request_reading_atype_error:
            pointer[1] = ATERROR;
            break;
        case request_reading_error:
            pointer[1] = GERROR;
            break;
    }
    pointer[2] = 0x00;
    pointer[3] = IPV4;
    pointer[4] = 0x00;
    pointer[5] = 0x00;
    pointer[6] = 0x00;
    pointer[7] = 0x00;
    pointer[8] = 0x00;
    pointer[9] = 0x00;

    buffer_write_adv(b, RESPLEN);
}
