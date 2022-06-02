#include "./include/hello_stm.h"
#include "../buffer.h"
#include "../socks5_states.h"
#include "../../parsers/include/hello_parser.h"
unsigned hello_read_init(const unsigned state, struct selector_key *key) {
    struct hello_stm * stm = &((struct socks5*)key->data)-> hello_stm;
    stm->select_method = -1;

    stm->read_buffer_data = calloc(get_buff_size(), sizeof(uint8_t));
    if(stm->read_buffer_data == NULL){
        goto finally;
    }
    buffer_init(&stm->read_buffer, get_buff_size(), sizeof(uint8_t));


    stm->write_buffer_data = calloc(get_buff_size(), sizeof(uint8_t));
    if(stm->write_buffer_data == NULL){
        goto finally;
    }
    buffer_init(&stm->write_buffer, get_buff_size(), sizeof(uint8_t));

    //TODO: Init parser
    return state;
finally:
    return ERROR;
}

unsigned hello_read(struct selector_key * key) {
    struct hello_stm * stm = &((struct socks5*)key->data)-> hello_stm;
    size_t nbytes;
    uint8_t * pointer = buffer_write_ptr(&stm->rb, &nbytes);
    ssize_t ret = recv(key->fd, pointer, nbytes, 0);

    uint8_t ret_state = HELLO_READING;
    if(ret > 0){
        buffer_write_adv(&stm->rb, ret);
        enum hello_state

    }
    
}