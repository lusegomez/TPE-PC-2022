#ifndef CONNECT_H
#define CONNECT_H

#include "../../includes/selector.h"
#include <netinet/in.h>
#include <arpa/inet.h>

//BND.PORT contanins the port number that the server assigned to connect to the target host
//BND.ADDR contains the associated IP address
/*
enum connection_type {
    IPV4,
    IPV6,
    FQDN
};
*/
struct connect {
    uint8_t atype;
    uint8_t destaddr_len;
    uint8_t * destaddr;
    uint8_t port[2];

    struct sockaddr_in origin_addr;
    struct sockaddr_in6 origin_addr6;
};

unsigned connect_init(struct selector_key *key);

#endif