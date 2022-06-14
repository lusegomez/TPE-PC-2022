#include <errno.h>
#include "./includes/copy_st.h"

#define ATTACHMENT(key)     ( ( struct socks5 * )(key)->data)

//void copy_init(const unsigned state, struct selector_key *key){
//    struct socks5 * sock = ATTACHMENT(key);
//    struct copy_st * copy = sock->copy;
//
//    copy->clt2srv = calloc(1, sizeof(buffer));
//    //if(copy->clt2srv == NULL) return ERROR;
//
//    copy->srv2org = calloc(1, sizeof(buffer));
//    //if(copy->srv2org == NULL) return ERROR;
//
//}
unsigned copy_read(struct selector_key * key){
    struct socks5 *sock = ATTACHMENT(key);
    struct copy_st * copy = sock->copy;
    buffer * buff;
    if(key->fd == sock->origin_fd) {
        buff = &sock->write_buffer;
    } else if(key->fd == sock->client_fd) {
        buff = &sock->read_buffer;
    } else {
        return ERROR;
    }
    size_t nbytes;
    uint8_t * prt = buffer_write_ptr(buff, &nbytes);
    ssize_t ret = recv(key->fd, prt, nbytes, 0);
    if(ret > 0) {
        buffer_write_adv(buff, ret);

        if(!buffer_can_write(buff)){
            //TENGO QUE SACAR UN INTERES SIN SACAR LOS OTROS
            if(selector_remove_interest(key->s, key->fd, OP_READ) != SELECTOR_SUCCESS){
                goto finally;
            }
        }
        if(selector_add_interest(key->s, key->fd == sock->client_fd ? sock->origin_fd : sock->client_fd, OP_WRITE) != SELECTOR_SUCCESS)  { //LO QUIERO IR ESCRIBIENDO
            goto finally;
        }
    } else if(ret == 0 || errno == ECONNRESET) {
        if(selector_remove_interest(key->s, key->fd, OP_READ) != SELECTOR_SUCCESS){ //YA LEI TODO
            goto finally;
        }
    }

    return COPY;
finally:
    return ERROR;
}

unsigned copy_write(struct selector_key * key){
    struct socks5 * sock = ATTACHMENT(key);
    size_t nbytes;
    buffer * buff;
    if(key->fd == sock->origin_fd) {
        buff = &sock->read_buffer;
    } else if(key->fd == sock->client_fd) {
        buff = &sock->write_buffer;
    } else {
        return ERROR;
    }
    uint8_t * prt = buffer_read_ptr(buff, &nbytes);
    ssize_t ret = send(key->fd, prt, nbytes, MSG_NOSIGNAL);
    if(ret > 0) {
        buffer_read_adv(buff, ret);
        if(!buffer_can_read(buff)){
            if(selector_remove_interest(key->s, key->fd, OP_WRITE) != SELECTOR_SUCCESS){
                goto finally;
            }
        }
        if(selector_add_interest(key->s, key->fd, OP_READ) != SELECTOR_SUCCESS) {
            goto finally;
        }
        return COPY;
    }
finally:
    return ERROR;
}