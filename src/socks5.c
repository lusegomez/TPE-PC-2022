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


static unsigned pool_size = 0;
static struct socks5 *pool = NULL;


const struct state_definition states_definition[] = {
        {
                .state = HELLO,
                .on_arrival = hello_init,
                .on_read_ready = hello_read,
                .on_write_ready = hello_write
        },

        /*
        {
            .state = HELLO_WRITING,
            .on_arrival = hello_write_init,
            .on_write_ready = hello_write
        },
        {
            .state = HELLO_AUTH_READING,
            .on_arrival = hello_auth_read_init,
            .on_read_ready = hello__auth_read
        },
        {
            .state = HELLO_AUTH_WRITING,
            .on_arrival = hello_auth_write_init,
            .on_write_ready = hello_auth_write
        },
        {
            .state = REQUEST_READING,
            .on_arrival = request_reading_init,
            .on_read_ready = request_reading
        },
        {
            .state = DNS_QUERY,
            .on_arrival = dns_query_init,
            .on_departure = dns_query_close
        },
        {
            .state = DNS_RESPONSE,
            .on_arrival = dns_response_init,
            .on_read_ready = dns_response
        },
        {
            .state = CONNECT_ORIGIN,
            .on_arrival = connect_origin_init,
            .on_block_ready = connect_origin_block
        },
        {
            .state = REQUEST_WRITING,
            .on_arrival = request_write_init,
            .on_write_ready = request_write,
        },
        {
            .state = COPY,
            .on_arrival = copy_init,
            .on_read_ready = copy_read,
            .on_write_ready = copy_write,
        },
         */
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

    struct socks5 * sock;

    if (pool == NULL) {
        sock = malloc(sizeof(*sock));
        //parsers_init(sock);
    }
    else {
        sock       = pool;
        pool        = pool->next;
        sock->next = 0;
        //Resetear parsers
    }

    if(sock == NULL){
        goto finally;
    }

    memset(sock, 0x00, sizeof(*sock));

    sock->client_fd = client_fd;
    sock->origin_fd = -1;


    sock->stm.initial = HELLO;
    sock->stm.max_state = ERROR;
    sock->stm.states = states_definition;
    sock->hello = malloc(sizeof(struct hello_st));
    memset(sock->hello, 0x00, sizeof(struct hello_st));
    stm_init(&sock->stm);


    buffer_init(&sock->read_buffer, BUFFER_SIZE, sock->read_raw_buff);
    buffer_init(&sock->write_buffer, BUFFER_SIZE, sock->write_raw_buff);


    sock->references = 1;
finally:
    return sock;
}

/* realmente destruye */
static void destroy_socks5_(struct socks5 * socks) {
    if(socks->origin_resolution != NULL) {
        freeaddrinfo(socks->origin_resolution);
        socks->origin_resolution = NULL;
    }
    if(socks->current_origin_resolution != NULL) {
        freeaddrinfo(socks->current_origin_resolution);
        socks->current_origin_resolution = NULL;
    }

    //Destroy parsers

    free(socks);
}

static void
destroy_socks5(struct socks5 *s) {
    if(s == NULL) {
        // nada para hacer
    } else if(s->references == 1) {
        if(s != NULL) {
            if(pool_size < MAX_POOL) {
                s->next = pool;
                pool    = s;
                pool_size++;
            } else {
                destroy_socks5_(s);
            }
        }
    } else {
        s->references -= 1;
    }
}

void
socksv5_pool_destroy(void) {
     struct socks5 *next, *s;
     for(s = pool; s != NULL ; s = next) {
            next = s->next;
             free(s);
         }
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
/*
void socksv5_pool_destroy(){
    struct socks5 * next, *s;
    for(s = pool; s != NULL ; s = next) {
        next = s->next;
        free(s);
    }
}
*/
void socksv5_passive_accept(struct selector_key * key) {
    struct sockaddr_storage new_client;
    socklen_t new_client_len = sizeof(new_client);
    struct socks5 * state = NULL;
    

    int client_sock = accept(key->fd, (struct sockaddr * )&new_client, &new_client_len);
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
        destroy_socks5(state);
    }
}


void socks5_handle_read(struct selector_key * key) {
    struct state_machine *stm = &ATTACHMENT(key)->stm;
    enum socks5_state state = stm_handler_read(stm, key);
    if(state == ERROR || state == CLOSE_CONNECTION) {
        destroy_socks5((struct socks5 *) key); //TODO: Cambiar por socks5_done (aca significa que hubo un error y hay que terminar el proxy)
    }

}
void socks5_handle_write(struct selector_key * key){
    struct state_machine *stm = &ATTACHMENT(key)->stm;

    enum socks5_state state = stm_handler_write(stm, key);
    if(state == ERROR || state == CLOSE_CONNECTION) {
        destroy_socks5((struct socks5 *)key);//TODO: Cambiar por socks5_done (aca significa que hubo un error y hay que terminar el proxy)

    }

}
void socks5_handle_block(struct selector_key * key) {
    struct state_machine *stm = &ATTACHMENT(key)->stm;

    enum socks5_state state = stm_handler_block(stm, key);
    if(state == ERROR || state == CLOSE_CONNECTION) {
        destroy_socks5((struct socks5 *)key);//TODO: Cambiar por socks5_done (aca significa que hubo un error y hay que terminar el proxy)
    }

}
void socks5_handle_close(struct selector_key * key) {
    struct state_machine *stm = &ATTACHMENT(key)->stm;

    stm_handler_close(stm, key);
    //TODO: Se deberia manejar error aca?

}


