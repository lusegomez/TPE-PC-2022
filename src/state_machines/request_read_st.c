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

#define POP3PORT 110

#define INET_ADDRSTRLEN 16
#define INET6_ADDRSTRLEN 46

#define SOCKSVERSION 0x05

#define ATTACHMENT(key)     ( ( struct socks5 * )(key)->data)

#define RESPLEN 10

void connection_logger(struct socks5 * sock);
void response(struct socks5 * sock){
    buffer * b = &sock->write_buffer;
    size_t n;
    uint8_t * pointer = buffer_write_ptr(b,&n);
    pointer[0] = SOCKSVERSION;
    pointer[1] = sock->request_read->status;
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

unsigned request_read(struct selector_key * key) {
    struct socks5 * sock = ATTACHMENT(key);
    struct request_read_parser * rp = sock->request_read->req_parser;
    size_t nbytes;
    uint8_t * pointer = buffer_write_ptr(&sock->read_buffer, &nbytes); //Agarro el write pointer para escribir en el read_buff con el recv
    ssize_t ret = recv(key->fd, pointer, nbytes, 0);
    unsigned state = GENERAL_SOCKS_SERVER_FAILURE;
    if(ret > 0) {
        buffer_write_adv(&sock->read_buffer, ret);
        enum request_read_state ret_state = consume_request(&sock->read_buffer, rp);
        if(ret_state == request_reading_end){

            if(selector_set_interest_key(key, OP_NOOP) != SELECTOR_SUCCESS){
                state = GENERAL_SOCKS_SERVER_FAILURE;
                goto finally;
            }

        } else if(ret_state == request_reading_error){

            switch (rp->state) {
                case request_reading_command_error:
                    state = COMMAND_NOT_SUPPORTED;
                    break;
                case request_reading_atype_error:
                    state = ADDRESS_TYPE_NOT_SUPPORTED;
                    break;
                case request_reading_error:
                    state = GENERAL_SOCKS_SERVER_FAILURE;
                    break;
                default:
                    break;
            }
             sock->request_read->status = state;

            //response(sock, state);

            if(selector_set_interest_key(key, OP_WRITE) != SELECTOR_SUCCESS){
                state = GENERAL_SOCKS_SERVER_FAILURE;
                goto finally;
            }
            return RESPONSE_WRITING;

        }
    }
    return rp->atype == FQDN ? DNS_QUERY : connect_init(key);
    finally:
    switch (rp->state) {
        case request_reading_command_error:
            state = COMMAND_NOT_SUPPORTED;
            break;
        case request_reading_atype_error:
            state = ADDRESS_TYPE_NOT_SUPPORTED;
            break;
        case request_reading_error:
            state = GENERAL_SOCKS_SERVER_FAILURE;
            break;
        default:
            break;
    }
    sock->request_read->status = state;
    //response(sock, state);
    return RESPONSE_WRITING;
}

unsigned response_write(struct selector_key * key){
    struct socks5 * sock = ATTACHMENT(key);
    response(sock);
    size_t n;
    uint8_t * pointer = buffer_read_ptr(&sock->write_buffer, &n);
    uint8_t ret_state = COPY;
    ssize_t ret = send(key->fd, pointer, n, MSG_NOSIGNAL);
    if(ret > 0) {
        buffer_read_adv(&sock->write_buffer, n);
        if(!buffer_can_read(&sock->write_buffer)){
            if(selector_set_interest_key(key, OP_READ) != SELECTOR_SUCCESS){
                goto finally;
            }
            if(sock->request_read->status != SUCCEDED){
                goto finally;
            }
        } else {
            ret_state = RESPONSE_WRITING;
        }
    }else {
        goto finally;
    }
    connection_logger(sock);
    return ret_state;
    finally:
    connection_logger(sock);
    return ERROR;
}

void connection_logger(struct socks5 * sock){
    struct request_read_st * rqst_st = sock->request_read;
    char buff[INET6_ADDRSTRLEN]={0};
    char *clientaddr = getIpAddress(&sock->client_addr);
    if(rqst_st->req_parser->atype != FQDN){
        inet_ntop(rqst_st->req_parser->atype == IPV4 ? AF_INET : AF_INET6, rqst_st->req_parser->destaddr, buff, rqst_st->req_parser->atype == IPV4 ? INET_ADDRSTRLEN : INET6_ADDRSTRLEN);
    }
    if ((((uint16_t)rqst_st->req_parser->port[0] << 8) | rqst_st->req_parser->port[1]) == POP3PORT){
        sock->isPop = true;
    }
    plog(INFO, "\t%s\tA\t%s\t%d\t%s\t%d\t%d",
         sock->hello_auth->hello_auth_parser != NULL ? (char*)sock->hello_auth->hello_auth_parser->user : "Unknown user",
         clientaddr,
         getPort(&sock->client_addr),
         rqst_st->req_parser->atype == FQDN ? (char *)rqst_st->req_parser->destaddr : buff,
         ((uint16_t)rqst_st->req_parser->port[0] << 8) | rqst_st->req_parser->port[1],
         rqst_st->status);
    free(clientaddr);
}

