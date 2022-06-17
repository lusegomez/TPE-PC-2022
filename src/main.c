/**
 * main.c - servidor proxy socks concurrente
 *
 * Interpreta los argumentos de línea de comandos, y monta un socket
 * pasivo.
 *
 * Todas las conexiones entrantes se manejarán en éste hilo.
 *
 * Se descargará en otro hilos las operaciones bloqueantes (resolución de
 * DNS utilizando getaddrinfo), pero toda esa complejidad está oculta en
 * el selector.
 */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <limits.h>
#include <errno.h>
#include <signal.h>

#include <unistd.h>
#include <sys/types.h>   // socket
#include <sys/socket.h>  // socket
#include <netinet/in.h>
#include <netinet/tcp.h>
#include "./includes/selector.h"
#include "./includes/socks5.h"
#include "./includes/passive_sockets.h"
#include "./includes/args.h"
#include "./utils/includes/users.h"
#include "./admin_protocol/includes/admin_protocol.h"

#define N(x) (sizeof(x)/sizeof((x)[0]))

struct socks5args args;
static bool done = false;

static void
sigterm_handler(const int signal) {
    printf("signal %d, cleaning up and exiting\n",signal);
    done = true;
}

int
main(const int argc, const char **argv) {
    log(INFO, "%s", "Iniciando proxy...");
    parse_args(argc, (char **)argv, &args);
    //for every user in the list, add them to the user list
    if(args.users[0].name != 0) {
        for(int i = 0; i < N(args.users); i++) {
            if(add_user(&args.users[i]) == -1){
                log(ERRORR, "Error adding user %s\n", args.users[i].name);
            }
        }
    }
    init_admin_data();


    // no tenemos nada que leer de stdin
    close(0);

    const char       *err_msg = NULL;
    selector_status   ss      = SELECTOR_SUCCESS;
    fd_selector selector      = NULL;


    //TODO:Set ipv4/ipv6
    int passive_socket_ipv4 = -1;
    int passive_socket_ipv6 = -1;
    int passive_socket_mngt_ipv4 = -1;
    int passive_socket_mngt_ipv6 = -1;

    int error_ipv4 = create_passive_socket_ipv4(&passive_socket_ipv4, args);
    if(error_ipv4 == -1) {
        log(INFO, "%s ", "Error al crear socket IPv4");
    }
    int error_ipv6 = create_passive_socket_ipv6(&passive_socket_ipv6, args);
    if(error_ipv6 == -1) {
        log(INFO, "%s ", "Error al crear socket IPv6");
    }
    int error_mngt_ipv4 = create_passive_socket_mngt_ipv4(&passive_socket_mngt_ipv4, args);
    if(error_mngt_ipv4 == -1){
        log(INFO, "%s ", "Error al crear socket  SCTP IPv4");
    }
    int error_mngt_ipv6 = create_passive_socket_mngt_ipv6(&passive_socket_mngt_ipv6, args);
    if(error_mngt_ipv6 == -1){
        log(INFO, "%s ", "Error al crear socket  SCTP IPv6");
    }
    if ((error_ipv4 == -1 && error_ipv6 == -1) || (error_mngt_ipv4 == -1 && error_mngt_ipv6 == -1))
    {
        err_msg = "No se pudieron crear los sockets pasivos";
        goto finally;
    }
    log(INFO, "%s %d", "[PROXY] Listening on TCP port", args.socks_port);
    log(INFO, "%s %d ", "[PROXY] Listening on SCTP port", args.mng_port);

    signal(SIGTERM, sigterm_handler);
    signal(SIGINT,  sigterm_handler);

    const struct selector_init conf = {
        .signal = SIGALRM,
        .select_timeout = {
            .tv_sec  = 10,
            .tv_nsec = 0,
        },
    };
    if(0 != selector_init(&conf)) {
        err_msg = "initializing selector";
        goto finally;
    }

    selector = selector_new(1024);
    if(selector == NULL) {
        err_msg = "unable to create selector";
        goto finally;
    }
    const struct fd_handler socksv5 = {
        .handle_read       = socksv5_passive_accept,
        .handle_write      = NULL,
        .handle_close      = NULL,
    };
    const struct fd_handler mngt_socksv5 = {
        .handle_read = admin_connection,
        .handle_write = NULL,
        .handle_close = NULL,
    };
    bool ipv4_flag = false;
    bool ipv6_flag = false;
    bool ipv4_mngt_flag = false;
    bool ipv6_mngt_flag = false;
    if(passive_socket_ipv4 != -1){
        ss = selector_register(selector, passive_socket_ipv4, &socksv5, OP_READ, NULL);
        if(ss != SELECTOR_SUCCESS) {
            ipv4_flag = true;
        }
    }
    if(passive_socket_ipv6 != -1){
        ss = selector_register(selector, passive_socket_ipv6, &socksv5, OP_READ, NULL);
        if(ss != SELECTOR_SUCCESS) {
            ipv6_flag = true;
        }
    }
    if(passive_socket_mngt_ipv4 != -1){
        ss = selector_register(selector, passive_socket_mngt_ipv4, &mngt_socksv5, OP_READ, NULL);
        if(ss != SELECTOR_SUCCESS) {
            ipv4_mngt_flag = true;
        }
    }
    if(passive_socket_mngt_ipv6 != -1){
        ss = selector_register(selector, passive_socket_mngt_ipv6, &mngt_socksv5, OP_READ, NULL);
        if(ss != SELECTOR_SUCCESS) {
            ipv6_mngt_flag = true;
        }
    }

    if((ipv4_flag && ipv6_flag)){
        err_msg = "Error registrando los sockets para el proxy";
        goto finally;
    }

    if((ipv4_mngt_flag && ipv6_mngt_flag)){
        err_msg = "Error registrando los sockets para el protocolo de monitoreo";
        goto finally;
    }       
  
    for(;!done;) {
        err_msg = NULL;
        ss = selector_select(selector);
        if(ss != SELECTOR_SUCCESS) {
            err_msg = "serving";
            goto finally;
        }
    }
    if(err_msg == NULL) {
        err_msg = "closing";
    }

    int ret = 0;
finally:
    if(ss != SELECTOR_SUCCESS) {
        log(ERROR, "%s: %s", err_msg, ss == SELECTOR_IO? strerror(errno) : selector_error(ss));
        /*
        fprintf(stderr, "%s: %s\n", (err_msg == NULL) ? "": err_msg,
                                  ss == SELECTOR_IO
                                      ? strerror(errno)
                                      : selector_error(ss));
        */
        ret = 2;
    } else if(err_msg) {
        perror(err_msg);
        ret = 1;
    }
    if(selector != NULL) {
        selector_destroy(selector);
    }
    selector_close();

    if (passive_socket_ipv4 != -1) {
        close(passive_socket_ipv4);
    }
    if (passive_socket_ipv6 != -1) {
        close(passive_socket_ipv6);
    }
    if (passive_socket_mngt_ipv4 != -1) {
        close(passive_socket_mngt_ipv4);
    }
    if (passive_socket_mngt_ipv6 != -1) {
        close(passive_socket_mngt_ipv6);
    }
    socksv5_pool_destroy();

    return ret;
}
