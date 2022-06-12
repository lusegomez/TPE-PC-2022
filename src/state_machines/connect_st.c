#include <stdint.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <string.h>
#include <netinet/in.h>
#include <arpa/inet.h>


#include "../includes/parser_utils.h"
#include "./includes/connect_st.h"
#include "../includes/socks5_states.h"
#include "../includes/socks5.h"

#define ATTACHMENT(key)     ( ( struct socks5 * )(key)->data)
#define IPV4 0x01
#define FQDN 0x03
#define IPV6 0x04

unsigned connect_origin_ipv4(struct connect_st *st, struct socks5 * sock){
    //Crear socket para origin
    sock->origin_fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if(sock->origin_fd < 0) {
        goto finally;
    }
    if(selector_fd_set_nio(sock->origin_fd) < 0){
        goto finally;
    }
    if(memset(st->origin_addr, 0, sizeof(struct sockaddr_in)) == NULL){
        goto finally;
    }
    if(memcpy(&st->origin_addr->sin_addr, sock->request_read->req_parser->destaddr, sock->request_read->req_parser->destaddr_len) == NULL){
        goto finally;
    }
    st->origin_addr->sin_port = htons(sock->request_read->req_parser->port);
    st->origin_addr->sin_family = AF_INET;

    
    if(connect(sock->origin_fd, (struct sockaddr *) &st->origin_addr, sizeof(st->origin_addr)) < 0){
        
    }

    return CONNECT_ORIGIN;
finally:
    //Tengo que informar del error
    return RESPONSE_WRITING;

}
unsigned connect_origin_fqdn(struct connect_st *st, struct socks5 * sock);     //TODO: Implementar funcion
unsigned connect_origin_ipv6(struct connect_st *st, struct socks5 * sock){}

unsigned connect_origin(struct connect_st * st, struct socks5 * sock) {
    switch (st->atype)
    {
    case IPV4:
        return connect_origin_ipv4(st, sock);
        break;
    case FQDN:
        return connect_origin_fqdn(st, sock);       //TODO: Implementar funcion
        break;
    case IPV6:
        return connect_origin_ipv6(st, sock);
        break;
    
    default:
        break;
    }
}


unsigned connect_init(const unsigned state, struct selector_key *key){
    struct socks5 * sock = ATTACHMENT(key);
    struct connect_st * st = sock->connect_origin;
    enum socks5_state ret_state = CONNECT_ORIGIN;
    //ACA TENGO EL PARSER CON LA INFO DE LA CONEXION:
    //DESTADDR Y ATYPE
    struct request_read_st * rqst_st = sock->request_read;  
    
    st->atype = rqst_st->req_parser->atype;
    st->destaddr = strcpy((char *)st->destaddr, (const char *) rqst_st->req_parser->destaddr);
    if(st->destaddr == NULL){
        return ERROR;
    }

    connect_origin(st, sock);
}
