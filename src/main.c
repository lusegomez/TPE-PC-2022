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
    parse_args(argc, (char **)argv, &args);
    //for every user in the list, add them to the user list
    for(int i = 0; i < N(args.users); i++) {
        add_user(&args.users[i]);
    }
 /*
    if(argc == 1) {
        // utilizamos el default
    } else if(argc == 2) {
        char *end     = 0;
        const long sl = strtol(argv[1], &end, 10);

        if (end == argv[1]|| '\0' != *end 
           || ((LONG_MIN == sl || LONG_MAX == sl) && ERANGE == errno)
           || sl < 0 || sl > USHRT_MAX) {
            fprintf(stderr, "port should be an integer: %s\n", argv[1]);
            return 1;
        }
        port = sl;
    } else {
        fprintf(stderr, "Usage: %s <port>\n", argv[0]);
        return 1;
    }
*/
    // no tenemos nada que leer de stdin
    close(0);

    const char       *err_msg = NULL;
    selector_status   ss      = SELECTOR_SUCCESS;
    fd_selector selector      = NULL;


    //TODO:Set ipv4/ipv6
    int passive_socket_ipv4 = -1;
    int passive_socket_ipv6 = -1;

    int errorIPv4 = create_passive_socket_ipv4(&passive_socket_ipv4, args);
    if(errorIPv4 == -1) {
        printf("Error al crear socket IPv4 \n");
    }
    int errorIPv6 = create_passive_socket_ipv6(&passive_socket_ipv6, args);
    if(errorIPv6 == -1) {
        printf("Error al crear socket IPv6 \n");
    }
    if (errorIPv4 == -1 && errorIPv6 == -1)
    {
        goto finally;
    }
    fprintf(stdout, "Listening on TCP port %d\n", args.socks_port);
/*    
    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family      = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port        = htons(port);
    const int server = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if(server < 0) {
        err_msg = "unable to create socket";
        goto finally;
    }


    // man 7 ip. no importa reportar nada si falla.
    setsockopt(server, SOL_SOCKET, SO_REUSEADDR, &(int){ 1 }, sizeof(int));

    if(bind(server, (struct sockaddr*) &addr, sizeof(addr)) < 0) {
        err_msg = "unable to bind socket";
        goto finally;
    }

    if (listen(server, 20) < 0) {
        err_msg = "unable to listen";
        goto finally;
    }

    // registrar sigterm es útil para terminar el programa normalmente.
    // esto ayuda mucho en herramientas como valgrind.
    signal(SIGTERM, sigterm_handler);
    signal(SIGINT,  sigterm_handler);

    if(selector_fd_set_nio(server) == -1) {
        err_msg = "getting server socket flags";
        goto finally;
    }
*/
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
        .handle_close      = NULL, // nada que liberar
    };

    bool ipv4_flag = false;
    bool ipv6_flag = false;
    if(passive_socket_ipv4 != -1){
        ss = selector_register(selector, passive_socket_ipv4, &socksv5, OP_READ, NULL);
        if(ss != SELECTOR_SUCCESS) {
            ipv4_flag = true;
            err_msg = "Error registering IPv4 Socket";
        }
    }
    if(passive_socket_ipv6 != -1){
        ss = selector_register(selector, passive_socket_ipv6, &socksv5, OP_READ, NULL);
        if(ss != SELECTOR_SUCCESS) {
            ipv6_flag = true;
            err_msg = "Error registering IPv6 Socket";
        }
    }

    if(ipv4_flag && ipv6_flag) goto finally;
    /*
    ss = selector_register(selector, server, &socksv5,
                                              OP_READ, NULL);
    if(ss != SELECTOR_SUCCESS) {
        err_msg = "registering fd";
        goto finally;
    }
    */
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
        fprintf(stderr, "%s: %s\n", (err_msg == NULL) ? "": err_msg,
                                  ss == SELECTOR_IO
                                      ? strerror(errno)
                                      : selector_error(ss));
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
    
    //socksv5_pool_destroy();
/*
    close(server);
    if(server >= 0) {
        close(server);
    }*/
    return ret;
}
