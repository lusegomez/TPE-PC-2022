#include <string.h>

#include "./includes/passive_sockets.h"
#include "./includes/args.h"

#define MAXPENDING 10 // Maximum outstanding connection requests

int create_passive_socket(int * s, struct socks5args args){
    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    if(inet_pton(AF_INET, args.socks_addr, &addr.sin_addr) == 0) {
        //TODO: manejar error
        return -1;
    }
    addr.sin_port = htons(args.socks_port);
    *s = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if(s < 0) {
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