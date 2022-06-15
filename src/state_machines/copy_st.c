#include <errno.h>
#include "./includes/copy_st.h"

#define ATTACHMENT(key)     ( ( struct socks5 * )(key)->data)

//Check if socket connection was closed
bool is_socket_closed(int fd){
    int error = 0;
    socklen_t len = sizeof(error);
    getsockopt(fd, SOL_SOCKET, SO_ERROR, &error, &len);
    return error != 0;
}


unsigned copy_read(struct selector_key * key){
    struct socks5 *sock = ATTACHMENT(key);
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

    return is_socket_closed(key->fd) ? ERROR : COPY;
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
        if(selector_add_interest(key->s, key->fd == sock->client_fd ? sock->origin_fd : sock->client_fd, OP_READ) != SELECTOR_SUCCESS) {
            goto finally;
        }

    }
    return is_socket_closed(key->fd) ? ERROR : COPY;
finally:
    return ERROR;
}