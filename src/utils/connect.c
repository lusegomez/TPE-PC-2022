#include <stdint.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <string.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>

#include "../includes/parser_utils.h"
#include "./includes/connect.h"
#include "../includes/socks5_states.h"
#include "../includes/socks5.h"

#define ATTACHMENT(key)     ( ( struct socks5 * )(key)->data)
#define IPV4 0x01
#define FQDN 0x03
#define IPV6 0x04

unsigned connect_origin_ipv4(struct connect *conn, struct selector_key *key){
    struct socks5 * sock = ATTACHMENT(key);
    //Crear socket para origin
    sock->origin_fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if(sock->origin_fd < 0) {
        goto finally;
    }
    if(selector_fd_set_nio(sock->origin_fd) < 0){
        goto finally;
    }
    if(memset(conn->origin_addr, 0, sizeof(struct sockaddr_in)) == NULL){
        goto finally;
    }
    if(memcpy(&conn->origin_addr->sin_addr, sock->request_read->req_parser->destaddr, sock->request_read->req_parser->destaddr_len) == NULL){
        goto finally;
    }
    uint16_t aux = 0;
    aux |= (uint16_t)sock->request_read->req_parser->port[0] << 8;
    aux |= (uint16_t)sock->request_read->req_parser->port[1] << 8;
    conn->origin_addr->sin_port = htons(aux);
    conn->origin_addr->sin_family = AF_INET;

    
    if(connect(sock->origin_fd, (struct sockaddr *) &conn->origin_addr, sizeof(conn->origin_addr)) < 0){
        if(errno == EINPROGRESS){   //La conexion es bloqueante y no se resuelve inmediatamente
            if(selector_register(key->s, sock->origin_fd, &socks5_active_handler, OP_WRITE, key->data) != SELECTOR_SUCCESS){
                goto finally;
            }
            return CONNECT_ORIGIN;
        } else {
            goto finally;
        }
    }

    return CONNECT_ORIGIN;
finally:
    //Tengo que informar del error
    return RESPONSE_WRITING;

}
unsigned connect_origin_fqdn(struct connect *conn, struct selector_key *key){}    //TODO: Implementar funcion
unsigned connect_origin_ipv6(struct connect *conn, struct selector_key *key){}

unsigned connect_origin(struct connect * conn, struct selector_key *key) {
    switch (conn->atype)
    {
    case IPV4:
        return connect_origin_ipv4(conn, key);
        break;
    case FQDN:
        return connect_origin_fqdn(conn, key);       //TODO: Implementar funcion
        break;
    case IPV6:
        return connect_origin_ipv6(conn, key);
        break;
    
    default:
        break;
    }
}


unsigned connect_init(struct selector_key *key){
    struct socks5 * sock = ATTACHMENT(key);
    struct connect * connection = sock->connect_origin;
    enum socks5_state ret_state = CONNECT_ORIGIN;
    //ACA TENGO EL PARSER CON LA INFO DE LA CONEXION:
    //DESTADDR Y ATYPE
    struct request_read_st * rqst_st = sock->request_read;  
    
    connection->atype = rqst_st->req_parser->atype;
    connection->destaddr = (uint8_t *)strcpy((char *)connection->destaddr, (const char *) rqst_st->req_parser->destaddr);
    if(connection->destaddr == NULL){
        return ERROR;
    }

    ret_state = connect_origin(connection, key);
    if(ret_state == RESPONSE_WRITING) {
        if(selector_set_interest(key->s, sock->client_fd, OP_WRITE) != SELECTOR_SUCCESS){
            return ERROR;
        }
    }
    return ret_state;
}