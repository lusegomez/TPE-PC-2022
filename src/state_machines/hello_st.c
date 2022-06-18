#include <stdint.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <string.h>

#include "../includes/parser_utils.h"
#include "./includes/hello_st.h"
#include "../includes/socks5_states.h"
#include "../includes/socks5.h"

#include "../utils/includes/logger.h"

#define HELLORESPONSE 2
#define SOCKSVERSION 0x05
#define NOAUTH 0X00
#define AUTH 0x02
#define NOMETHOD 0xFF

#define ATTACHMENT(key)     ( ( struct socks5 * )(key)->data)

void hello_response(buffer * b, struct hello_st * hello);

void hello_init(const unsigned state, struct selector_key *key) {
    struct hello_st * st = ATTACHMENT(key)->hello;
    st->selected_method = -1;
    st->hello_parser = malloc(sizeof(struct hello_parser));
    hello_parser_init(st->hello_parser);
    plog(DEBUG, "%s: %s:%d", "Hello parser initialized", __FILE__, __LINE__);
}

void hello_reset(struct hello_st * hello){
    hello->selected_method = -1;
    if(hello->hello_parser != NULL && hello->hello_parser->methods != NULL){
        free(hello->hello_parser->methods);
        free(hello->hello_parser);
        plog(DEBUG, "%s: %s:%d", "Hello parser resources freed", __FILE__, __LINE__);
    }
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
                plog(ERRORR, "%s", "Failed to set WRITE interest", __FILE__, __LINE__);
                goto finally;
            }
            hello_response(&sock->write_buffer, sock->hello);
        } else if(state == hello_error){
            hello_response(&sock->write_buffer, sock->hello);

            sock->hello->selected_method = NOMETHOD;
            if(selector_set_interest_key(key, OP_WRITE) != SELECTOR_SUCCESS){
                plog(ERRORR, "%s", "Failed to set WRITE interest", __FILE__, __LINE__);
                goto finally;
            }
        }
    }
    return HELLO;
    finally:
    sock->hello->selected_method = NOMETHOD;
    hello_response(&sock->write_buffer, sock->hello);
    return ERROR;
}

void hello_response(buffer * b, struct hello_st * hello){
    size_t n;
    uint8_t * pointer = buffer_write_ptr(b,&n);
    if(n >= 2){
        pointer[0] = SOCKSVERSION;
        pointer[1] = hello->selected_method;
        buffer_write_adv(b, HELLORESPONSE);
    }
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
                plog(ERRORR, "%s", "Failed to set READ interest", __FILE__, __LINE__);
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
