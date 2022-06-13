#ifndef COPY_H
#define COPY_H

#include "../../includes/socks5.h"
#include "../../includes/socks5_states.h"
#include "../../includes/buffer.h"


void copy_init(const unsigned state, struct selector_key *key);  
unsigned copy_read(struct selector_key * key);
unsigned copy_write(struct selector_key * key);


#endif