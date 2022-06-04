#include <netinet/in.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <string.h>
#include "./includes/socks5.h"
#include "./includes/selector.h"
#include "./includes/socks5_states.h"



struct socks5 * create_new_sock5(int client_fd) {
    struct socks5 * sock = calloc(1,sizeof(struct socks5));
    if(sock == NULL) {
        return NULL;
    }
    
    sock->client_ip = calloc(INET6_ADDRSTRLEN, sizeof(char));
    if(sock->client_ip == NULL) {
        goto fail;
    }
    sock->client_fd = client_fd;
    sock->origin_fd = -1;
    return sock;
fail:
    if(sock->client_ip != NULL) free(sock->client_ip);
    if(sock->origin_ip != NULL) free(sock->origin_ip);
    if(sock != NULL) free(sock);
    return NULL;

}

static void destroy_socks5(struct selector_key *key){
    struct socks5 * sock = ((struct socks5 *) key -> data);
    if(sock->origin_fd != -1){
        if(selector_unregister_fd(key->s, sock->origin_fd) != SELECTOR_SUCCESS){
            exit(EXIT_FAILURE);
        }
        close(sock->origin_fd);
    }
    if(sock->client_fd != -1) {
        if(selector_unregister_fd(key->s, sock->client_fd) != SELECTOR_SUCCESS){ 
            exit(EXIT_FAILURE);
        }
        close(sock->client_fd);
    }

    free(sock);
}
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
    memcpy(&state->client_ip, &new_client, new_client_len);
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
        destroy_socks5(key);    //TODO: destroy
    }
}


void socks5_handle_read(struct selector_key * key) {
    struct socks5 * sock = ((struct socks5*)(key)-> data);
    struct state_machine *stm = &(sock->stm);

    enum socks5_state state = stm_handler_read(stm, key);
    if(state == ERROR || state == CLOSE_CONNECTION) {
        destroy_socks5(key);
    }

}
void socks5_handle_write(struct selector_key * key){
    struct socks5 * sock = ((struct socks5*)key-> data);
    struct state_machine *stm = &(sock->stm);

    enum socks5_state state = stm_handler_write(stm, key);
    if(state == ERROR || state == CLOSE_CONNECTION) {
        destroy_socks5(key);

    }

}
void socks5_handle_block(struct selector_key * key) {
    struct socks5 * sock = ((struct socks5*)key-> data);
    struct state_machine *stm = &(sock->stm);

    enum socks5_state state = stm_handler_block(stm, key);
    if(state == ERROR || state == CLOSE_CONNECTION) {
        destroy_socks5(key);
    }

}
void socks5_handle_close(struct selector_key * key) {
    struct socks5 * sock = ((struct socks5*)key-> data);
    struct state_machine *stm = &(sock->stm);

    stm_handler_close(stm, key);
    //TODO: Se deberia manejar error aca?

}


