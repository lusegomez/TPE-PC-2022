#ifndef COPY_H
#define COPY_H

#include "../../includes/socks5.h"
#include "../../includes/socks5_states.h"
#include "../../includes/buffer.h"

#define BUFFER_SIZE 2048
struct copy_st {
    buffer * clt2srv;
    buffer * srv2org;
    uint8_t clt2srv_buff[BUFFER_SIZE];
    uint8_t srv2org_buff[BUFFER_SIZE];

};

void copy_init(const unsigned state, struct selector_key *key);  
unsigned copy_read(struct selector_key * key);
unsigned copy_write(struct selector_key * key);


#endif