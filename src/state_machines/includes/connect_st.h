#ifndef CONNECT_ST_H
#define CONNECT_ST_H

#include "../../includes/buffer.h"
#include "../../includes/selector.h"

//BND.PORT contanins the port number that the server assigned to connect to the target host
//BND.ADDR contains the associated IP address
/*
enum connection_type {
    IPV4,
    IPV6,
    FQDN
};
*/
struct connect_st {
    uint8_t atype;
    uint8_t destaddr_len;
    uint8_t * destaddr;
    uint8_t port[2];

    struct sockaddr_in * origin_addr;
    struct sockaddr_in6 * origin_addr6;
};

unsigned connect_init(const unsigned state, struct selector_key *key);
unsigned connect_write(struct selector_key * key);

#endif