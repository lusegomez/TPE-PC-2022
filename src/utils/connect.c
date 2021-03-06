#include <stdint.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <string.h>

#include <errno.h>

#include "./includes/connect.h"
#include "../includes/socks5_states.h"
#include "../includes/socks5.h"
#include "..//state_machines/includes/request_read_st.h"
//#include "../utils/includes/response.h"

#define ATTACHMENT(key)     ( ( struct socks5 * )(key)->data)
#define IPV4 0x01
#define FQDN 0x03
#define IPV6 0x04

#define POP3PORT 110

#define INET_ADDRSTRLEN 16
#define INET6_ADDRSTRLEN 46


unsigned connect_origin_ipv4(struct connect *conn, struct selector_key *key){
    struct socks5 * sock = ATTACHMENT(key);
    unsigned state = GENERAL_SOCKS_SERVER_FAILURE;
    int error = 0;
    socklen_t len = sizeof(error);
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
    conn->origin_addr.sin_port = htons(aux);
    conn->origin_addr.sin_family = AF_INET;
    
    if(connect(sock->origin_fd, (const struct sockaddr *)&conn->origin_addr, sizeof(conn->origin_addr)) < 0){
        switch (errno) {
            case EINPROGRESS:
                if(selector_register(key->s, sock->origin_fd, &socks5_active_handler, OP_READ, key->data) != SELECTOR_SUCCESS){
                    selector_unregister_fd(key->s, sock->origin_fd);
                    close(sock->origin_fd);
                    sock->origin_fd = -1;
                    state = GENERAL_SOCKS_SERVER_FAILURE;
                    goto finally;
                }
                if(getsockopt(sock->origin_fd, SOL_SOCKET, SO_ERROR, &error, &len) < 0 || error != 0) {
                    selector_unregister_fd(key->s, sock->origin_fd);
                    state = GENERAL_SOCKS_SERVER_FAILURE;
                    close(sock->origin_fd);
                    sock->origin_fd = -1;
                    goto finally;
                }

                sock->request_read->status = SUCCEDED;
                //response(sock, SUCCEDED);
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
    if(sock->origin_fd > 0){
        selector_unregister_fd(key->s, sock->origin_fd);
        close(sock->origin_fd);
        sock->origin_fd = -1;
    }
    sock->request_read->status = state;
    return RESPONSE_WRITING;

}


unsigned connect_origin_fqdn(struct selector_key *key){
    unsigned state = GENERAL_SOCKS_SERVER_FAILURE;
    struct socks5 * sock = ATTACHMENT(key);
    int error = 0;
    socklen_t len = sizeof(error);
    struct addrinfo *rp;
    if(sock->origin_resolution == NULL){
        state = HOST_UNREACHABLE;
        goto finally;
    }
    for(rp = sock->origin_resolution; rp != NULL && sock->origin_fd == -1; rp = rp->ai_next){
        sock->origin_fd = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
        if (sock->origin_fd == -1) {
            close(sock->origin_fd);
            sock->origin_fd = -1;
            continue;   //PRUEBO OTRO
        }
        if(selector_fd_set_nio(sock->origin_fd) < 0){
            close(sock->origin_fd);
            sock->origin_fd = -1;
            state = GENERAL_SOCKS_SERVER_FAILURE;
            continue;
        }
        if(connect(sock->origin_fd, rp->ai_addr, rp->ai_addrlen) < 0) {
            if(errno == EINPROGRESS) {
                if(selector_register(key->s, sock->origin_fd, &socks5_active_handler, OP_READ, key->data) != SELECTOR_SUCCESS){
                    selector_unregister_fd(key->s, sock->origin_fd);
                    close(sock->origin_fd);
                    sock->origin_fd = -1;
                    state = GENERAL_SOCKS_SERVER_FAILURE;
                    continue;
                }
                if(getsockopt(sock->origin_fd, SOL_SOCKET, SO_ERROR, &error, &len) < 0 || error != 0) {
                    selector_unregister_fd(key->s, sock->origin_fd);
                    state = HOST_UNREACHABLE;
                    close(sock->origin_fd);
                    sock->origin_fd = -1;
                    continue;
                }
                sock->request_read->status = SUCCEDED;
                return RESPONSE_WRITING;
            } else {
                switch (errno) {
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
        }
    }
    if (rp == NULL)
    {
        selector_unregister_fd(key->s, sock->origin_fd);
        close(sock->origin_fd);
        sock->origin_fd = -1;
    }
finally:
    if(sock->origin_fd > 0) {
        selector_unregister_fd(key->s, sock->origin_fd);
        close(sock->origin_fd);
        sock->origin_fd = -1;
    }
    sock->request_read->status = state;
    return RESPONSE_WRITING;
}   
unsigned connect_origin_ipv6(struct connect * conn, struct selector_key *key){
    struct socks5 * sock = ATTACHMENT(key);
    unsigned state = GENERAL_SOCKS_SERVER_FAILURE;
    int error = 0;
    socklen_t len = sizeof(error);
    //Crear socket para origin
    sock->origin_fd = socket(AF_INET6, SOCK_STREAM, IPPROTO_TCP);
    if(sock->origin_fd < 0) {
        goto finally;
    }
    if(selector_fd_set_nio(sock->origin_fd) < 0){
        goto finally;
    }

    if(memcpy(&conn->origin_addr6.sin6_addr, sock->request_read->req_parser->destaddr, sock->request_read->req_parser->destaddr_len) == NULL){
        goto finally;
    }

    uint16_t aux = ((uint16_t)sock->request_read->req_parser->port[0] << 8) | sock->request_read->req_parser->port[1];
    conn->origin_addr6.sin6_port = htons(aux);
    conn->origin_addr6.sin6_family = AF_INET6;

    if(connect(sock->origin_fd, (const struct sockaddr *)&conn->origin_addr6, sizeof(conn->origin_addr6)) < 0){
        switch (errno) {
            case EINPROGRESS:
                if(selector_register(key->s, sock->origin_fd, &socks5_active_handler, OP_READ, key->data) != SELECTOR_SUCCESS){
                    selector_unregister_fd(key->s, sock->origin_fd);
                    state = GENERAL_SOCKS_SERVER_FAILURE;
                    goto finally;
                }
                if(getsockopt(sock->origin_fd, SOL_SOCKET, SO_ERROR, &error, &len) < 0 || error != 0) {
                    selector_unregister_fd(key->s, sock->origin_fd);
                    state = GENERAL_SOCKS_SERVER_FAILURE;
                    close(sock->origin_fd);
                    sock->origin_fd = -1;
                    goto finally;
                }

                sock->request_read->status = SUCCEDED;
                //response(sock, SUCCEDED);
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
    if(sock->origin_fd > 0){
        selector_unregister_fd(key->s, sock->origin_fd);
        close(sock->origin_fd);
        sock->origin_fd = -1;
    }
    sock->request_read->status = state;
    //response(sock, state);
    return RESPONSE_WRITING;
}

unsigned connect_origin(struct connect * conn, struct selector_key *key) {
    switch (conn->atype)
    {
    case IPV4:
        return connect_origin_ipv4(conn, key);
        break;
    case FQDN:
        return connect_origin_fqdn(key);       //TODO: Implementar funcion
        break;
    case IPV6:
        return connect_origin_ipv6(conn, key);
        break;
    
    default:
        break;
    }
    return GENERAL_SOCKS_SERVER_FAILURE;
}


unsigned connect_init(struct selector_key *key){
    struct socks5 * sock = ATTACHMENT(key);
    struct connect * connection = sock->connect_origin;
    enum socks5_state ret_state = RESPONSE_WRITING;
    //ACA TENGO EL PARSER CON LA INFO DE LA CONEXION:
    //DESTADDR Y ATYPE
    struct request_read_st * rqst_st = sock->request_read;  
    connection->destaddr = malloc(rqst_st->req_parser->destaddr_len);
    memset(connection->destaddr, 0x00, rqst_st->req_parser->destaddr_len);

    connection->atype = rqst_st->req_parser->atype;
    memcpy(connection->destaddr, rqst_st->req_parser->destaddr, rqst_st->req_parser->destaddr_len);
    connection->destaddr_len = rqst_st->req_parser->destaddr_len;
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
