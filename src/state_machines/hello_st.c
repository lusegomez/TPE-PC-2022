#include <stdint.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <string.h>

#include "../includes/parser_utils.h"
#include "./includes/hello_st.h"
#include "../includes/socks5_states.h"
#include "../includes/socks5.h"

#define HELLORESPONSE 2
#define SOCKSVERSION 0x05
#define NOAUTH 0X00
#define AUTH 0x02
#define NOMETHOD 0xFF

#define ATTACHMENT(key)     ( ( struct socks5 * )(key)->data)

//struct hello_st * init_parser_definition(struct hello_st * st){
//    st->ver_def = malloc(sizeof(struct parser_definition));
//    char str[2] = "\0"; /* gives {\0, \0} */
//    str[0] = 5;
//    struct parser_definition pd = parser_utils_strcmpi(str);
//    memcpy(st->ver_def, &pd, sizeof(struct parser_definition));
//    return st;
//}
//
//void destroy_parser_definition(struct hello_st * st){
//    parser_utils_strcmpi_destroy(st->ver_def);
//    free(st->ver_def);
//}
//
//void initialize_parsers(struct selector_key * key) {
//    struct socks5 * sock = ATTACHMENT(key);
//    sock->hello->ver_parser = parser_init(parser_no_classes(), sock->hello->ver_def);
//
//}

void hello_response(buffer * b, struct hello_st * hello);

void hello_init(const unsigned state, struct selector_key *key) {
    struct hello_st * st = ATTACHMENT(key)->hello;
    st->selected_method = -1;
    st->hello_parser = malloc(sizeof(struct hello_parser));
    //TODO: Init parser
//    st = init_parser_definition(st);
//    st->ver_parser = parser_init(parser_no_classes(),st->ver_def);
}

unsigned hello_read(struct selector_key * key) {
    struct socks5 * sock = ATTACHMENT(key);
    struct hello_parser * hp = sock->hello->hello_parser;
    size_t nbytes;
    uint8_t * pointer = buffer_write_ptr(&sock->read_buffer, &nbytes);
    ssize_t ret = recv(key->fd, pointer, nbytes, 0);

    if(ret > 0) {
        buffer_write_adv(&sock->read_buffer, ret);
        enum hello_state state = consume_hello(&sock->read_buffer, hp);
        if(state == hello_end){
            if(hp->methods != NULL){
                for(int i = 0; i < hp->nmethods; i++ ){
                    if(hp->methods[i] == AUTH){
                        sock->hello->selected_method = hp->methods[i];
                    } else if (hp->methods[i] == NOAUTH) {
                        if(sock->hello->selected_method != AUTH){
                            sock->hello->selected_method = hp->methods[i];
                        }
                    } else {
                        if(sock->hello->selected_method != AUTH && sock->hello->selected_method != NOAUTH){
                            sock->hello->selected_method = NOMETHOD;
                        }
                    }
                }
            }
            if(selector_set_interest_key(key, OP_WRITE) != SELECTOR_SUCCESS){
                goto finally;
            }
            hello_response(&sock->write_buffer, sock->hello);
        } else if(state == hello_error){
            hello_response(&sock->write_buffer, sock->hello);

            sock->hello->selected_method = NOMETHOD;
            if(selector_set_interest_key(key, OP_WRITE) != SELECTOR_SUCCESS){
                goto finally;
            }
        }
    }
    return HELLO;
    finally:
    sock->hello->selected_method = NOMETHOD;
    hello_response(&sock->write_buffer, sock->hello);
    return ERROR;
//    for(int i = 0; i < ret; i++) {
//        const struct parser_event * state = parser_feed(sock->hello->ver_parser, pointer[i]);
//        if(state->type == STRING_CMP_EQ) {
//            if(selector_set_interest(key->s, sock->client_fd, OP_WRITE) != SELECTOR_SUCCESS) {
//                return ERROR;
//            }
//        } else if (state->type == STRING_CMP_NEQ){
//            return ERROR;
//        }
//    }
//    parser_reset(sock->hello->ver_parser);
//    buffer_write_adv(&sock->write_buffer, ret);

}

void hello_response(buffer * b, struct hello_st * hello){
    size_t n;
    uint8_t * pointer = buffer_write_ptr(b,&n);
    //TODO: checkear que se puedan escribir 2 bytes
    pointer[0] = SOCKSVERSION;
    pointer[1] = hello->selected_method;
    buffer_write_adv(b, HELLORESPONSE);
}


unsigned hello_write(struct selector_key * key) {
    struct socks5 * sock = ATTACHMENT(key);
    size_t n;
    uint8_t * pointer = buffer_read_ptr(&sock->write_buffer, &n);
    uint8_t ret_state = HELLO;
    ssize_t ret = send(key->fd, pointer, n, MSG_NOSIGNAL);
    if(ret > 0) {
        buffer_read_adv(&sock->write_buffer, n);
        if(!buffer_can_read(&sock->write_buffer)){
            if(selector_set_interest_key(key, OP_READ) != SELECTOR_SUCCESS){
                goto finally;
            }
            if(sock->hello->selected_method == AUTH) {
                return HELLO_AUTH;
            }
            if(sock->hello->selected_method == NOAUTH) {
                return REQUEST_READING;
            }
            if(sock -> hello->selected_method == NOMETHOD){
                goto finally;
            }
        }
    }else {
        goto finally;
    }
    return ret_state;
finally:
    return ERROR;
}
