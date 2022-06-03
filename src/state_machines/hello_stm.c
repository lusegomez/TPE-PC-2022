#include <stdint.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>

#include "../includes/buffer.h"
#include "./includes/hello_stm.h"
#include "../includes/socks5.h"
#include "../includes/socks5_states.h"
#include "../parsers/includes/hello_parser.h"

void hello_read_init(const unsigned state, struct selector_key *key) {
    struct hello_stm * stm = &((struct socks5*)key->data)-> hello_stm;
    stm->selected_method = -1;

    stm->read_buffer_data = calloc(get_buff_size(), sizeof(uint8_t));
    if(stm->read_buffer_data == NULL){
    //TODO: Handle error
    }
    buffer_init(&stm->read_buffer, get_buff_size(), stm->read_buffer_data);

    stm->write_buffer_data = calloc(get_buff_size(), sizeof(uint8_t));
    if(stm->write_buffer_data == NULL){
    //TODO: Handle error
    }
    buffer_init(&stm->write_buffer, get_buff_size(), stm->write_buffer_data);

    //TODO: Init parser
}

unsigned hello_read(struct selector_key * key) {
    struct hello_stm * stm = &((struct socks5*)key->data)-> hello_stm;
    size_t nbytes;
    uint8_t * pointer = buffer_write_ptr(&stm->read_buffer, &nbytes);
    ssize_t ret = recv(key->fd, pointer, nbytes, 0);

    uint8_t ret_state = HELLO_READING;
    if(ret > 0){
        buffer_write_adv(&stm->read_buffer, ret);
        enum hello_state state = consume_buffer(&stm->read_buffer, NULL /*TODO: parser*/);
        if(state == hello_end) {
            //TODO: manejar los distintos estados
            return 0;
        }
    }
    return ret_state;

    
}