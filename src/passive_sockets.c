#include <string.h>
#include <stdio.h>
#include "./includes/passive_sockets.h"
#include "./includes/args.h"

#define MAXPENDING 10 // Maximum outstanding connection requests

int create_passive_socket_ipv4(int * s, struct socks5args args){
    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    if(inet_pton(AF_INET, args.socks_addr, &addr.sin_addr) == 0) {
        //TODO: manejar error
        return -1;
    }
    addr.sin_port = htons(args.socks_port);
    *s = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if(*s < 0) {
        //TODO: Manejar error
        return -1;
    }
    //Argumentos: fd, level (setear en SOL_SOCKET), optname, optval (mandar 0 si esta deshabilitada), optlen
    setsockopt(*s, SOL_SOCKET, SO_REUSEADDR, &(int){1},sizeof(int));

    if(bind(*s, (struct sockaddr*) &addr, sizeof(addr)) <0){
        //TODO: manejar error
        return -1;
    }
    //Argumentos: fd, backlog (indica la maxima longitud de la cola de conexiones pendientes)
    //Si la cola esta llena el cliente recibe un error
    if(listen(*s, MAXPENDING) < 0) {
        //TODO: manejar error
        return -1;
    }

    if(selector_fd_set_nio(*s) == -1) {
        //TODO: manejar error
        return -1;
    }
    return 0;
}


int create_passive_socket_ipv6(int * s, struct socks5args args){
    struct sockaddr_in6 addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin6_family = AF_INET6;
    if(inet_pton(AF_INET6, args.socks_addr6, &addr.sin6_addr) == 0) {
        //TODO: manejar error
        printf("a");
        return -1;
    }
    addr.sin6_port = htons(args.socks_port);
    *s = socket(AF_INET6, SOCK_STREAM, IPPROTO_TCP);
    if(*s < 0) {
        //TODO: Manejar error
        printf("b");
        return -1;
    }
    //Argumentos: fd, level (setear en SOL_SOCKET), optname, optval (mandar 0 si esta deshabilitada), optlen
    setsockopt(*s, SOL_SOCKET, SO_REUSEADDR, &(int){1},sizeof(int));
    if(setsockopt(*s, SOL_IPV6, IPV6_V6ONLY, &(int) {1}, sizeof(int)) < 0){
        printf("c");
        return -1;
    }
    if(bind(*s, (struct sockaddr*) &addr, sizeof(addr)) <0){
        //TODO: manejar error
        printf("d");
        return -1;
    }
    //Argumentos: fd, backlog (indica la maxima longitud de la cola de conexiones pendientes)
    //Si la cola esta llena el cliente recibe un error
    if(listen(*s, MAXPENDING) < 0) {
        //TODO: manejar error
        printf("e");
        return -1;
    }

    if(selector_fd_set_nio(*s) == -1) {
        //TODO: manejar error
        printf("f");
        return -1;
    }
    return 0;
}

int create_passive_socket_mngt_ipv4( int * s, struct socks5args args){
    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    if(inet_pton(AF_INET, args.mng_addr, &addr.sin_addr) == 0) {
        //TODO: manejar error
        return -1;
    }
    addr.sin_port = htons(args.mng_port);
    *s = socket(AF_INET, SOCK_STREAM, IPPROTO_SCTP);
    if(*s < 0) {
        //TODO: Manejar error
        return -1;
    }
    //Argumentos: fd, level (setear en SOL_SOCKET), optname, optval (mandar 0 si esta deshabilitada), optlen
    setsockopt(*s, SOL_SOCKET, SO_REUSEADDR, &(int){1},sizeof(int));

    if(bind(*s, (struct sockaddr*) &addr, sizeof(addr)) <0){
        //TODO: manejar error
        return -1;
    }
    //Argumentos: fd, backlog (indica la maxima longitud de la cola de conexiones pendientes)
    //Si la cola esta llena el cliente recibe un error
    if(listen(*s, MAXPENDING) < 0) {
        //TODO: manejar error
        return -1;
    }

    if(selector_fd_set_nio(*s) == -1) {
        //TODO: manejar error
        return -1;
    }
    return 0;
}

int create_passive_socket_mngt_ipv6(int * s, struct socks5args args){
    struct sockaddr_in6 addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin6_family = AF_INET6;
    if(args.mng_addr6 == NULL) return -1;
    if(inet_pton(AF_INET6, args.mng_addr6, &addr.sin6_addr) == 0) {
        //TODO: manejar error
        printf("a");
        return -1;
    }
    addr.sin6_port = htons(args.mng_port);
    *s = socket(AF_INET6, SOCK_STREAM, IPPROTO_SCTP);
    if(*s < 0) {
        //TODO: Manejar error
        printf("b");
        return -1;
    }
    //Argumentos: fd, level (setear en SOL_SOCKET), optname, optval (mandar 0 si esta deshabilitada), optlen
    setsockopt(*s, SOL_SOCKET, SO_REUSEADDR, &(int){1},sizeof(int));
    if(setsockopt(*s, SOL_IPV6, IPV6_V6ONLY, &(int) {1}, sizeof(int)) < 0){
        printf("c");
        return -1;
    }
    if(bind(*s, (struct sockaddr*) &addr, sizeof(addr)) <0){
        //TODO: manejar error
        printf("d");
        return -1;
    }
    //Argumentos: fd, backlog (indica la maxima longitud de la cola de conexiones pendientes)
    //Si la cola esta llena el cliente recibe un error
    if(listen(*s, MAXPENDING) < 0) {
        //TODO: manejar error
        printf("e");
        return -1;
    }

    if(selector_fd_set_nio(*s) == -1) {
        //TODO: manejar error
        printf("f");
        return -1;
    }
    return 0;
}

