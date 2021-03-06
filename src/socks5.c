#include <netdb.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <errno.h>
#include <time.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/queue.h>

#include <arpa/inet.h>
#include "./includes/socks5.h"
#include "./includes/selector.h"
#include "./includes/socks5_states.h"
#include "./includes/parser.h"
#include "./utils/includes/logger.h"


struct socks5 * pool = NULL;


const struct state_definition states_definition[] = {
        {
            .state = HELLO,
            .on_arrival = hello_init,
            .on_read_ready = hello_read,
            .on_write_ready = hello_write
        },
        {
            .state = HELLO_AUTH,
            .on_arrival = hello_auth_init,
            .on_read_ready = hello_auth_read,
            .on_write_ready = hello_auth_write
        },
        {
            .state = REQUEST_READING,
            .on_arrival = request_read_init,
            .on_read_ready = request_read
        },
        {
            .state = DNS_QUERY,
            .on_arrival = dns_query_init,
            .on_block_ready = dns_query_close
        },
        {
            .state = RESPONSE_WRITING,
            .on_write_ready = response_write,
        },

        {
            .state = COPY,
            //.on_arrival = copy_init,
            .on_read_ready = copy_read,
            .on_write_ready = copy_write,
        },

        {
            .state = CLOSE_CONNECTION,
//            .on_arrival = close_connection_init,
        },

        {
                .state = ERROR,
//        .on_arrival = error_init,
        },
};

static struct socks5 * create_new_sock5(int client_fd) {

    struct socks5 * sock = malloc(sizeof(struct socks5));
    memset(sock, 0x00, sizeof(*sock));
    if(sock == NULL){
        goto finally;
    }
    if(pool == NULL){
        pool = sock;
        sock->next = NULL;
    } else {
        sock->next = pool;
        pool = sock;
    }

    sock->hello = malloc(sizeof(struct hello_st));
    sock->hello_auth = malloc(sizeof(struct hello_auth_st));
    sock->request_read = malloc(sizeof(struct request_read_st));
    sock->dns_query = malloc(sizeof(struct dns_query_st));
    sock->connect_origin = malloc(sizeof(struct connect));
    sock->pop3 = NULL;


    memset(sock->hello, 0x00, sizeof(struct hello_st));
    memset(sock->hello_auth, 0x00, sizeof(struct hello_auth_st));
    memset(sock->request_read, 0x00, sizeof(struct request_read_st));
    memset(sock->dns_query, 0x00, sizeof(struct dns_query_st));
    memset(sock->connect_origin, 0x00, sizeof(struct connect));

    sock->client_fd = client_fd;
    sock->origin_fd = -1;
    sock->closing = false;
    sock->sniffed = false;
    sock->isPop = false;
    sock->stm.initial = HELLO;
    sock->stm.max_state = ERROR;
    sock->stm.states = states_definition;

    stm_init(&sock->stm);


    buffer_init(&sock->read_buffer, BUFFER_SIZE, sock->read_raw_buff);
    buffer_init(&sock->write_buffer, BUFFER_SIZE, sock->write_raw_buff);

    add_concurrent();
    sock->references = 1;
finally:
    return sock;
}

void free_socks5(struct socks5 * sock);
//remove sock from pool
static void remove_sock(struct socks5 * sock) {
    if(sock == pool){
        pool = sock->next;
    } else {
        struct socks5 * prev = pool;
        while(prev != NULL && prev->next != sock){
            prev = prev->next;
        }
        prev->next = sock->next;
    }

    free_socks5(sock);
}

void free_socks5(struct socks5 * sock);
static void
destroy_socks5(struct selector_key *key) {
    struct socks5 * sock = ATTACHMENT(key);
    if(sock->client_fd != -1) {
        if(selector_unregister_fd(key->s, sock->client_fd) != SELECTOR_SUCCESS) {
            exit(EXIT_FAILURE);
        }
        close(sock->client_fd);
    }
    if (sock->origin_fd != -1) {
        if(selector_unregister_fd(key->s, sock->origin_fd) != SELECTOR_SUCCESS) {
            exit(EXIT_FAILURE);
        }
        close(sock->origin_fd);
    }
    remove_concurrent();
    remove_sock(sock);

}

void free_socks5(struct socks5 * sock) {
    if(sock->hello != NULL) {
        if(sock->hello->hello_parser != NULL) {
            if(sock->hello->hello_parser->methods != NULL) {
                free(sock->hello->hello_parser->methods);
            }
            free(sock->hello->hello_parser);
        }
        free(sock->hello);
    }
    if(sock->hello_auth != NULL) {
        if(sock->hello_auth->hello_auth_parser != NULL) {
            free(sock->hello_auth->hello_auth_parser);
        }
        free(sock->hello_auth);
    }
    if(sock->request_read != NULL) {
        if(sock->request_read->req_parser != NULL) {
            if(sock->request_read->req_parser->destaddr != NULL) {
                free(sock->request_read->req_parser->destaddr);
            }
            free(sock->request_read->req_parser);
        }
        free(sock->request_read);
    }
    if(sock->dns_query != NULL) {
        free(sock->dns_query);
    }
    if(sock->connect_origin != NULL) {
        if(sock->connect_origin->destaddr != NULL) {
            free(sock->connect_origin->destaddr);
        }
        free(sock->connect_origin);
    }

    if(sock->origin_resolution != NULL) {
        freeaddrinfo(sock->origin_resolution);
    }

    if(sock->pop3 != NULL) {
        free(sock->pop3);
    }
    free(sock);
}

//
//struct socks5 * create_new_sock5(int client_fd) {
//
//    struct socks5 * sock;
//
//    if (pool == NULL) {
//        sock = malloc(sizeof(*sock));
//        initialize_parsers(sock);
//    }
//    else {
//        sock       = pool;
//        pool        = pool->next;
//        sock->next = 0;
//        //Resetear parsers
//    }
//
//    struct socks5 * sock = calloc(1,sizeof(struct socks5));
//    if(sock == NULL) {
//        return NULL;
//    }
//
//    sock->client_ip = calloc(INET6_ADDRSTRLEN, sizeof(char));
//    if(sock->client_ip == NULL) {
//        goto fail;
//    }
//    sock->client_fd = client_fd;
//    sock->origin_fd = -1;
//    return sock;
//fail:
//    if(sock->client_ip != NULL) free(sock->client_ip);
//    if(sock->origin_ip != NULL) free(sock->origin_ip);
//    if(sock != NULL) free(sock);
//    return NULL;
//
//}

//static void destroy_socks5(struct selector_key *key){
//    struct socks5 * sock = ((struct socks5 *) key -> data);
//    if(sock->origin_fd != -1){
//        if(selector_unregister_fd(key->s, sock->origin_fd) != SELECTOR_SUCCESS){
//            exit(EXIT_FAILURE);
//        }
//        close(sock->origin_fd);
//    }
//    if(sock->client_fd != -1) {
//        if(selector_unregister_fd(key->s, sock->client_fd) != SELECTOR_SUCCESS){
//            exit(EXIT_FAILURE);
//        }
//        close(sock->client_fd);
//    }
//
//    free(sock);
//}
void socksv5_pool_destroy(){
    struct socks5 * next, *s;
    for(s = pool; s != NULL ; s = next) {
        next = s->next;
        free_socks5(s);
    }
}
void socksv5_passive_accept(struct selector_key * key) {
    int client_sock = -1;
    struct socks5 * state = NULL;
    if(get_concurrent() == MAX_POOL){
        plog(ERRORR, "Max pool reached");
        goto fail;
    }
    struct sockaddr_storage new_client;
    socklen_t new_client_len = sizeof(new_client);



    client_sock = accept(key->fd, (struct sockaddr * )&new_client, &new_client_len);
    if(client_sock == -1) {
        goto fail;
    }

    if(selector_fd_set_nio(client_sock) == -1) {
        goto fail;
    }

    state = create_new_sock5(client_sock);
    if(state == NULL) {
        goto fail;
    }
    memcpy(&state->client_addr, &new_client, new_client_len);
    state->client_addr_len = new_client_len;

    if(selector_register(key->s, client_sock, &socks5_active_handler, OP_READ, state) != SELECTOR_SUCCESS) {
        goto fail;
    }

    return;
fail:
    if(client_sock != -1) {
        close(client_sock);
    }
    if(state != NULL) {
        destroy_socks5(key);
    }
}


void socks5_handle_read(struct selector_key * key) {
    struct state_machine *stm = &ATTACHMENT(key)->stm;
    enum socks5_state state = stm_handler_read(stm, key);
    if(state == ERROR || state == CLOSE_CONNECTION) {
        destroy_socks5(key);
    }

}
void socks5_handle_write(struct selector_key * key){
    struct state_machine *stm = &ATTACHMENT(key)->stm;

    enum socks5_state state = stm_handler_write(stm, key);
    if(state == ERROR || state == CLOSE_CONNECTION) {
        destroy_socks5(key);

    }

}
void socks5_handle_block(struct selector_key * key) {
    struct state_machine *stm = &ATTACHMENT(key)->stm;

    enum socks5_state state = stm_handler_block(stm, key);
    if(state == ERROR || state == CLOSE_CONNECTION) {
        destroy_socks5(key);
    }

}

void socks5_handle_close(struct selector_key * key) {
    destroy_socks5(key);
    //TODO: Se deberia manejar error aca?

}



