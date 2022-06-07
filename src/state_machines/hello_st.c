#include <stdint.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <string.h>

#include "../includes/parser_utils.h"
#include "./includes/hello_st.h"
#include "../includes/socks5_states.h"
#include "../includes/socks5.h"


#define ATTACHMENT(key)     ( ( struct socks5 * )(key)->data)

struct hello_st * init_parser_definition(struct hello_st * st){
    st->ver_def = malloc(sizeof(struct parser_definition));
    char str[2] = "\0"; /* gives {\0, \0} */
    str[0] = 5;
    struct parser_definition pd = parser_utils_strcmpi(str);
    memcpy(st->ver_def, &pd, sizeof(struct parser_definition));
    return st;
}

void destroy_parser_definition(struct hello_st * st){
    parser_utils_strcmpi_destroy(st->ver_def);
    free(st->ver_def);
}

void initialize_parsers(struct selector_key * key) {
    struct socks5 * sock = ATTACHMENT(key);
    sock->hello->ver_parser = parser_init(parser_no_classes(), sock->hello->ver_def);

}

void hello_init(const unsigned state, struct selector_key *key) {
    struct hello_st * st = ATTACHMENT(key)->hello;
    st->selected_method = -1;

    //TODO: Init parser
    st = init_parser_definition(st);
    st->ver_parser = parser_init(parser_no_classes(),st->ver_def);
}

unsigned hello_read(struct selector_key * key) {
    struct socks5 * sock = ATTACHMENT(key);
    size_t nbytes;
    uint8_t * pointer = buffer_write_ptr(&sock->read_buffer, &nbytes);
    ssize_t ret = recv(key->fd, pointer, nbytes, 0);

    uint8_t ret_state = HELLO;
    for(int i = 0; i < ret; i++) {
        const struct parser_event * state = parser_feed(sock->hello->ver_parser, pointer[i]);
        if(state->type == STRING_CMP_EQ) {
            if(selector_set_interest(key->s, sock->client_fd, OP_WRITE) != SELECTOR_SUCCESS) {
                return ERROR;
            }
        } else if (state->type == STRING_CMP_NEQ){
            return ERROR;
        }
    }
    parser_reset(sock->hello->ver_parser);
    buffer_write_adv(&sock->write_buffer, ret);

    return ret_state;
}


unsigned hello_write(struct selector_key * key) {
    struct socks5 * sock = ATTACHMENT(key);
    size_t nbytes = 5;
    char * pointer = "0x05";
    ssize_t ret = send(key->fd, pointer, nbytes, 0);
    if(ret <= 0) {
        //TODO: Agregar logs
        return ERROR;
    }
    uint8_t ret_state = HELLO;
    
    return ret_state;
}
