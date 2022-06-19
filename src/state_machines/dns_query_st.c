#include <stdint.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <string.h>
#include <pthread.h>
#include <netdb.h>

#include <arpa/inet.h>

#include "../includes/parser_utils.h"
#include "./includes/dns_query_st.h"
#include "../includes/socks5_states.h"
#include "../includes/socks5.h"
#include "../utils/includes/connect.h"
#include "../utils/includes/util.h"

#define FQDN 0x03

#define ATTACHMENT(key)     ( ( struct socks5 * )(key)->data)

static void * resolve_blocking(void * data) {
    struct selector_key* key = (struct selector_key*)data;
    struct socks5 * socks = ATTACHMENT(key);

    pthread_detach(pthread_self());
    socks->origin_resolution = 0;
    struct addrinfo hints = {
            .ai_family    = AF_UNSPEC,
            .ai_socktype  = SOCK_STREAM,
            .ai_flags     = AI_PASSIVE,
            .ai_protocol  = 0,
            .ai_canonname = NULL,
            .ai_addr      = NULL,
            .ai_next      = NULL,
    };

    char buff[7];
    snprintf(buff, sizeof(buff), "%d", ((uint16_t)socks->request_read->req_parser->port[0] << 8) | socks->request_read->req_parser->port[1]);

    if (getaddrinfo((const char *)socks->request_read->req_parser->destaddr, buff, &hints, &socks->origin_resolution) != 0) {
        socks->dns_query->status = dns_query_fail;
        goto finally;
    }
    socks->current_origin_resolution = socks->origin_resolution;
    socks->dns_query->status = dns_query_ok;
    finally:
    selector_notify_block(key->s, key->fd);
    free(data);
    return 0;
}

void dns_query_init(const unsigned state, struct selector_key *key){
    struct socks5 * sock = ATTACHMENT(key);
    pthread_t thread;

    struct selector_key * new_key = malloc(sizeof(*key));
    if (new_key == NULL){
        sock->dns_query->status = dns_query_fail;
    }
    new_key->s = key->s;
    new_key->fd = sock->client_fd;
    new_key->data = sock;
    if (pthread_create(&thread, 0, resolve_blocking, new_key) == -1 ){
        sock->dns_query->status = dns_query_fail;
    }


}

unsigned dns_query_close(struct selector_key * key){
    return connect_init(key);
}

