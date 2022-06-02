#include "socks5.h"
#include "selector.h"
#include "socks5_states.h"
#include <stdlib.h>
#include <sys/socket.h>



void destroy_socks5(struct selector_key *key){
    //TODO: destroy
}

struct socks5 * create_new_sock5(int client_fd) {
    struct socks5 * s = calloc(1,sizeof(struct socks5));
    if(s == NULL) {
        return NULL;
    }

    //TODO: stm
    s->stm.initial = HELLO_READING;
    s->stm.max_state = ERROR;
    s->states = states_definition;
    s->current = NULL;
    stm_init(&(s->stm));

    
    
    S->client_ip = calloc(INET6_ADDRSTRLEN, sizeof(char));
    if(s->client_ip == NULL) {
        goto finally;
    }
    s->client_fd = client_fd;
    s->origin_fd = -1;
    return s;
finally:
    if(s->client_ip != NULL) free(s->client_ip);
    if(s->origin_ip != NULL) free(s->origin_ip);
    if(s != NULL) free(s);
    return NULL;

}


void socksv5_passive_accept(struct selector_key * key) {
    struct sockaddr_storage new_client;
    sturct socks5 * s5 = NULL;
    
    socklen_t new_client_len = sizeof(new_client)

    int client_sock = accept(key->fd, (struct sockaddr * )&new_client, &new_client_len);
    if(client_sock == -1) {
        goto finally;
    }

    if(selector_fd_set_nio(client_sock) == -1) {
        goto finally;
    }

    s = create_new_sock5(client_sock);
    if(s == NULL) {
        goto finally;
    }

    if(selector_register(key->s, client_sock, &socks5_active_hanlder, OP_READ, s) != SELECTOR_SUCCESS) {
        goto finally;
    }

finally:
    if(client_sock != -1) {
        close(client_sock);
    }
    if(s != NULL) {
        destroy_socks5(key);    //TODO: destroy
    }
}


void socks5_handle_read(struct selector_key * key) {
    struct socks5 * sock = &((struct socks5*)key)-> data;
    struct state_machine *stm = sock->stm;

    enum socks5_state state = stm_handler_read(stm, key);
    if(state == ERROR || state == CLOSE_CONNECTION) {
        //TODO: destroy
    }

}
void socks5_handle_write(struct selector_key * key){
    struct socks5 * sock = &((struct socks5*)key)-> data;
    struct state_machine *stm = sock->stm;

    enum socks5_state state = stm_handler_write(stm, key);
    if(state == ERROR || state == CLOSE_CONNECTION) {
        //TODO: destroy
    }

}
void socks5_handle_block(struct selector_key * key) {
    struct socks5 * sock = &((struct socks5*)key)-> data;
    struct state_machine *stm = sock->stm;

    enum socks5_state state = stm_handler_block(stm, key);
    if(state == ERROR || state == CLOSE_CONNECTION) {
        //TODO: destroy
    }

}
void socks5_handle_close(struct selector_key * key) {
    struct socks5 * sock = &((struct socks5*)key)-> data;
    struct state_machine *stm = sock->stm;

    enum socks5_state state = stm_handler_close(stm, key);
    if(state == ERROR || state == CLOSE_CONNECTION) {
        //TODO: destroy
    }

}


