#include <stdint.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <string.h>

#include <errno.h>

#include "./includes/connect.h"
#include "../includes/socks5_states.h"
#include "../includes/socks5.h"
#include "../utils/includes/response.h"

#define ATTACHMENT(key)     ( ( struct socks5 * )(key)->data)
#define IPV4 0x01
#define FQDN 0x03
#define IPV6 0x04

unsigned connect_origin_ipv4(struct connect *conn, struct selector_key *key){
    struct socks5 * sock = ATTACHMENT(key);
    unsigned state = GENERAL_SOCKS_SERVER_FAILURE;
    //Crear socket para origin
    sock->origin_fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if(sock->origin_fd < 0) {
        goto finally;
    }
    if(selector_fd_set_nio(sock->origin_fd) < 0){
        goto finally;
    }

    if(memcpy(&conn->origin_addr.sin_addr, sock->request_read->req_parser->destaddr, sock->request_read->req_parser->destaddr_len) == NULL){
        goto finally;
    }

    uint16_t aux = ((uint16_t)sock->request_read->req_parser->port[0] << 8) | sock->request_read->req_parser->port[1];
    conn->origin_addr.sin_port = aux;
    conn->origin_addr.sin_family = AF_INET;
    
    if(connect(sock->origin_fd, (const struct sockaddr *)&conn->origin_addr, sizeof(conn->origin_addr)) < 0){
        switch (errno) {
            case EINPROGRESS:
                if(selector_register(key->s, sock->origin_fd, &socks5_active_handler, OP_WRITE, key->data) != SELECTOR_SUCCESS){
                    state = GENERAL_SOCKS_SERVER_FAILURE;
                    goto finally;
                }
                request_response(sock, SUCCEDED);
                return RESPONSE_WRITING;
            case ECONNREFUSED:
                state = CONNECTION_REFUSED;
                break;
            case EHOSTUNREACH:
                state = HOST_UNREACHABLE;
                break;
            case ENETUNREACH:
                state = NETWORK_UNREACHABLE;
                break;
            default:
                state = GENERAL_SOCKS_SERVER_FAILURE;
                break;
        }
        goto finally;

    }
    return RESPONSE_WRITING;
finally:
    request_response(sock, state);
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
    enum socks5_state ret_state = RESPONSE_WRITING;
    //ACA TENGO EL PARSER CON LA INFO DE LA CONEXION:
    //DESTADDR Y ATYPE
    struct request_read_st * rqst_st = sock->request_read;  
    connection->destaddr = malloc(sizeof(rqst_st->req_parser->destaddr));
    memset(connection->destaddr, 0x00, sizeof(rqst_st->req_parser->destaddr));

    connection->atype = rqst_st->req_parser->atype;
    connection->destaddr = (uint8_t *)strcpy((char *)connection->destaddr, (const char *) rqst_st->req_parser->destaddr);
    connection->destaddr_len = strlen((const char *)connection->destaddr);
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
