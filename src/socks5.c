#include <sys/socket.h>
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
#include <netdb.h>

#include <arpa/inet.h>
#include "./includes/socks5.h"
#include "./includes/selector.h"
#include "./includes/socks5_states.h"


static unsigned pool_size = 0;
static struct socks5 *pool = NULL;

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

    sock->stm.initial = HELLO_READING;
    sock->stm.max_state = ERROR;
    sock->stm.states = states_definition;
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

void socks_pool_destroy() {
    struct socks5 *next, *s;
    for(s = pool; s != NULL ; s = next) {
        if (s != s->next)
        {
            next = s->next;
        }
        else
            next = NULL;
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
    struct state_machine *stm = &ATTACHMENT(key)->stm;

    stm_handler_close(stm, key);
    //TODO: Se deberia manejar error aca?

}

