#include <errno.h>
#include "./includes/copy_st.h"

#define ATTACHMENT(key)     ( ( struct socks5 * )(key)->data)

char cmd[4] = {0};
int cmd_index = 0;

int user_index = 0;
int pass_index = 0;

//Check if socket connection was closed
bool is_socket_closed(int fd){
    int error = 0;
    socklen_t len = sizeof(error);
    getsockopt(fd, SOL_SOCKET, SO_ERROR, &error, &len);
    return error != 0;
}

//Init pop3 user and pass parsers
unsigned init_pop3_parsers(struct selector_key * key){
    struct socks5 * sock = ATTACHMENT(key);
    if(sock->pop3 == NULL){
        sock->pop3 = calloc(1, sizeof(struct pop3_st));
        if(sock->pop3 == NULL){
            plog(ERROR, "Error allocating memory for pop3 parser");
            return ERROR;
        }
    }
    sock->pop3->pop3_state = POP3_READING_COMMAND;
    memset(sock->pop3->user, 0, 255);
    memset(sock->pop3->pass, 0, 255);
    return COPY;
}

void to_upper_case(char * str, int n){
    int i = 0;
    while(str[i] != '\0' && i < n && cmd_index < 4){
        if(cmd[cmd_index] == '\0' && isalpha(str[i])){
            cmd[cmd_index] = toupper(str[i]);
            cmd_index++;
        }
        i++;
    }
}

//parse pop3 user and pass
void parse_pop3(struct selector_key * key, buffer * buffer) {
    struct socks5 *sock = ATTACHMENT(key);
    size_t n = 0;
    uint8_t *ptr = buffer_read_ptr(buffer, &n);
    int i = 0;
    while (i < n && !sock->sniffed) {
        switch (sock->pop3->pop3_state) {
            case POP3_READING_COMMAND:
                to_upper_case((char*) ptr, 4);
                if (ptr[i] == ' ' && !strncmp(cmd, "USER", 4)) {
                    sock->pop3->pop3_state = POP3_READING_USER;
                    //reset cmd
                    memset(cmd, 0, 4);
                    cmd_index = 0;
                    i++;
                }else if (ptr[i] == ' ' && !strncmp(cmd, "PASS", 4)) {
                    sock->pop3->pop3_state = POP3_READING_PASSWORD;
                    memset(cmd, 0, 4);
                    cmd_index = 0;
                    i++;
                } else {
                    i++;
                }
                break;
            case POP3_READING_USER:
                if (ptr[i] == '\r') {
                    sock->pop3->pop3_state = POP3_READING_COMMAND;
                    sock->pop3->user_done = true;
                    sock->pop3->user[user_index] = '\0';
                    user_index = 0;
                    i++;
                } else {
                    sock->pop3->user[user_index++] = ptr[i];
                    i++;
                }
                break;
            case POP3_READING_PASSWORD:
                if (ptr[i] == '\r') {
                    sock->pop3->pop3_state = POP3_PARSER_DONE;
                    sock->pop3->pass_done = true;
                    sock->pop3->pass[pass_index] = '\0';
                    pass_index = 0;
                    i++;
                } else {
                    sock->pop3->pass[pass_index++] = ptr[i];
                    i++;
                }
                break;
            case POP3_PARSER_DONE:
                sock->sniffed = true;
                plog(INFO, "Sniffed POP3 user \"%s\" with pass \"%s\"", sock->pop3->user, sock->pop3->pass);
                i++;
                break;

        }

    }
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
        add_bytes(ret);
        buffer_write_adv(buff, ret);
        if(key->fd == sock->client_fd && !sock->sniffed && sock->isPop && get_disector()) {
            if(sock->pop3 == NULL){
                if(init_pop3_parsers(key) == ERROR) {
                    return ERROR;
                }
            }
            parse_pop3(key, buff);
            if(sock->sniffed) {
                sock->pop3->pop3_state = POP3_READING_COMMAND;
                sock->sniffed = false;

            }


            //TODO: parsear pop3 y extraer user y pass
        }
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
        if(!buffer_can_read(buff)){
            return CLOSE_CONNECTION;
        } else {
            sock->closing = true;
        }
    } else {
        goto finally;
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
            if(sock->closing){
                return CLOSE_CONNECTION;
            }
        }
        if(!sock->closing && selector_add_interest(key->s, key->fd == sock->client_fd ? sock->origin_fd : sock->client_fd, OP_READ) != SELECTOR_SUCCESS) {
            goto finally;
        }

        if(sock->closing){
            return CLOSE_CONNECTION;
        }

    } else {
        goto finally;
    }
    return is_socket_closed(key->fd) ? ERROR : COPY;
finally:
    return ERROR;
}

