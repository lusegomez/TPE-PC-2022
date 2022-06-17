#ifndef SOCKS5_H
#define SOCKS5_H

#include "stm.h"
#include "selector.h"
#include <stdint.h>
#include "buffer.h"
#include "../state_machines/includes/hello_st.h"
#include "../state_machines/includes/hello_auth_st.h"
#include "../state_machines/includes/request_read_st.h"
#include "../utils/includes/connect.h"
#include "../state_machines/includes/dns_query_st.h"
#include "../state_machines/includes/copy_st.h"


#define ATTACHMENT(key)     ( ( struct socks5 * )(key)->data)
#define N(x) (sizeof(x)/sizeof((x)[0]))
#define MAX_POOL 500
#define BUFFER_SIZE 4096

struct socks5 {
    //Client data
    int client_fd;
    struct sockaddr_storage client_addr;
    socklen_t client_addr_len;

    //Origin data
    int origin_fd;
    struct sockaddr_storage origin_addr;
    socklen_t origin_addr_len;
    int origin_domain;
    struct addrinfo *origin_resolution;
    struct addrinfo *current_origin_resolution;

    //struct users current_users;

    struct hello_st * hello;
    struct hello_auth_st * hello_auth;
    struct request_read_st * request_read;
    struct connect * connect_origin; 
    struct dns_query_st * dns_query;
    struct copy_st * copy;
    bool closing;


    struct state_machine stm;

    // Buffers
    buffer read_buffer; // client --> origin
    uint8_t read_raw_buff[BUFFER_SIZE];
    buffer write_buffer; // origin --> client
    uint8_t write_raw_buff[BUFFER_SIZE];

    unsigned int references;
    struct socks5 * next;

};

void socksv5_passive_accept(struct selector_key * key);


void socks5_handle_read(struct selector_key * key);
void socks5_handle_write(struct selector_key * key);
void socks5_handle_block(struct selector_key * key);
void socks5_handle_close(struct selector_key * key);
void socksv5_pool_destroy();

static const fd_handler socks5_active_handler = {
    .handle_read = socks5_handle_read,
    .handle_write = socks5_handle_write,
    .handle_block = socks5_handle_block,
};


#endif