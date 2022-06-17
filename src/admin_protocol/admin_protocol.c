#include "./includes/admin_protocol.h"



static const struct state_definition client_statbl[] =
        {
        {
                .state          = GREETING,
                .on_write_ready = greet

        },
        {
                .state          = AUTH,
                .on_read_ready  = authenticate,

        },
        {
                .state          = COMMANDS,
                .on_read_ready  = parse_command,
                .on_write_ready = command_response,
        },
        {
                .state          = ADONE,

        },
        {
                .state          = AERROR,

        }
};



static const struct state_definition *
admin_describe_states(void)
{
    return client_statbl;
}


void admin_read(struct selector_key *key) {
    struct state_machine *stm = &ADMIN_ATTACHMENT(key)->stm;
    const enum admin_states st = stm_handler_read(stm,key);

    if (AERROR == st || ADONE == st) {
        admin_done(key);
    }
}

void admin_write(struct selector_key *key) {
    struct state_machine *stm = &ADMIN_ATTACHMENT(key)->stm;
    const enum admin_states st = stm_handler_write(stm,key);

    if (AERROR == st || ADONE == st) {
        admin_done(key);
    }
}

void admin_block(struct selector_key *key) {
  //NOTHING TO DO HERE
}

static void
admin_destroy(struct admin * admin) {
    if(admin != NULL) {
        // struct connection * aux = connections;
        // while(aux->next != NULL && aux->next != con){
        //     aux = aux->next;
        // }
        // aux->next = con->next;
        // if(con->origin_resolution != NULL){
        //     free(con->origin_resolution);
        // }
        // if(con->origin_resolution_current != NULL){
        //     free(con->origin_resolution_current);
        // }
        if(admin->references == 1) {
            free(admin);
        }
    } else {
         admin->references -= 1;
    }
}


void admin_close(struct selector_key *key) {
    admin_destroy(ADMIN_ATTACHMENT(key));
}


static void
admin_done(struct selector_key* key) {

    const int fds[] = {

            ADMIN_ATTACHMENT(key)->client_fd,

    };

    for(unsigned i = 0; i < N(fds); i++) {

        if(fds[i] != -1) {

            if(SELECTOR_SUCCESS != selector_unregister_fd(key->s, fds[i])) {

                abort();

            }

            close(fds[i]);

        }

    }
}
// FALTA IMPLEMENTAR DESTROYERS

void
admin_connection(struct selector_key *key) {
    struct sockaddr_storage       client_addr;
    socklen_t                     client_addr_len = sizeof(client_addr);

    const int client = accept(key->fd, (struct sockaddr*) &client_addr, &client_addr_len);
    if(client == -1) {
        goto fail;
    }
    if(selector_fd_set_nio(client) == -1) {
        goto fail;
    }
    struct admin *admin = new_admin(client);
    if(admin == NULL) {
        goto fail;
    }

    memcpy(&admin->client_addr, &client_addr, client_addr_len);
    admin->client_addr_len = client_addr_len;

    if(SELECTOR_SUCCESS != selector_register(key->s, client, &admin_handler, OP_WRITE, admin)) {
        goto fail;
    }

    return;

    fail:
    if(client != -1) {
        close(client);
    }
}

struct admin *
new_admin(int client_fd) {
    struct admin * admin;
    admin = malloc(sizeof(*admin));

    if (admin != NULL) {
        memset(admin, 0x00, sizeof(*admin));

        admin->next = 0;
        admin->client_fd = client_fd;
        admin->state = GREETING;

        admin->stm    .initial = GREETING;
        admin->stm    .max_state = AERROR;
        admin->stm    .states = admin_describe_states();

        admin->references = 1;
        stm_init(&admin->stm);

        buffer_init(&admin->read_buffer, N(admin->raw_buff_a), admin->raw_buff_a);
        buffer_init(&admin->write_buffer, N(admin->raw_buff_b), admin->raw_buff_b);
    }

    return admin;
}

static unsigned
greeting(struct selector_key* key) {

    admin * admin = ADMIN_ATTACHMENT(key);
    size_t greeting_size;
    buffer * buff = &admin->write_buffer;
    size_t buff_size;

    char * greeting = "+0 \n";
    greeting_size = strlen(greeting);

    uint8_t  * ptr = buffer_write_ptr(buff, &buff_size);
    memcpy(ptr, greeting, greeting_size);
    buffer_write_adv(buff, greeting_size);
    int n = send_to_client(key);
    
    return n;
}

static unsigned
pass_auth(struct selector_key* key, bool number) {

    admin * admin = ADMIN_ATTACHMENT(key);
    size_t auth_error_size;
    buffer * buff = &admin->write_buffer;
    size_t buff_size;
    int n;
    if (number == 0) {
        char *auth_error = "-1 \n";
        auth_error_size = strlen(auth_error);
        uint8_t *ptr = buffer_write_ptr(buff, &buff_size);
        memcpy(ptr, auth_error, auth_error_size);
        buffer_write_adv(buff, auth_error_size);
        n = send_to_client(key);
    } else {
        char *auth_error = "+1 \n";
        auth_error_size = strlen(auth_error);
        uint8_t *ptr = buffer_write_ptr(buff, &buff_size);
        memcpy(ptr, auth_error, auth_error_size);
        buffer_write_adv(buff, auth_error_size);
        n = send_to_client(key);
    }
    return n;
}

static unsigned
greet(struct selector_key* key) {
    int status = greeting(key);
    if (status < 0) {
        ADMIN_ATTACHMENT(key)->state = AERROR;
        return AERROR;
    }
    selector_set_interest(key->s, ADMIN_ATTACHMENT(key)->client_fd, OP_READ);
    ADMIN_ATTACHMENT(key)->state = AUTH;
    return AUTH;
}

static unsigned
authenticate(struct selector_key * key) {
    size_t size;
    int status;
    buffer * buff = &ADMIN_ATTACHMENT(key)->read_buffer;
    char * login_key = "password";
    int bytes = recieve_from_client(key);
    if(bytes < 0) {
        return AERROR;
    }
    selector_set_interest(key->s, ADMIN_ATTACHMENT(key)->client_fd, OP_READ);
    uint8_t * ptr = buffer_read_ptr(buff,&size);
    if(strncmp(login_key, (const char *)ptr, strlen(login_key)) == 0) {
        status = pass_auth(key,1);
        if (status < 0) {
            return AERROR;
        }
        buffer_read_adv(buff,bytes);
        ADMIN_ATTACHMENT(key)->state = COMMANDS;
        return COMMANDS;
    } else {
        status = pass_auth(key,0);
        if (status < 0) {
            return AERROR;
        }
        buffer_read_adv(buff,bytes);
        return AUTH;
    }
}



static unsigned
command_response(struct selector_key * key) {
    int status;
    admin * admin = ADMIN_ATTACHMENT(key);
    status = send_to_client(key);
    if(status < 0) {
        return AERROR;
    }
    selector_set_interest(key->s, ADMIN_ATTACHMENT(key)->client_fd, OP_READ);
    return admin->state;
}


static unsigned
send_to_client(struct selector_key * key) {

    admin * admin = ADMIN_ATTACHMENT(key);
    buffer * buff = &admin->write_buffer;
    size_t size;
    int n;
    uint8_t * ptr = buffer_read_ptr(buff, &size);
     if(n = sctp_sendmsg(key->fd, ptr , size,
                               NULL, 0, 0, 0, 0, 0, 0) < 0) {
         log(ERRORR, "%s\n", "Error sending message to client");
         return -1;
     }
    buffer_read_adv(buff, size);
    return n;
}

static unsigned
recieve_from_client(struct selector_key * key) {
    admin * admin = ADMIN_ATTACHMENT(key);
    buffer * buff = &admin->read_buffer;
    size_t size;
    int n;
    uint8_t * ptr = buffer_write_ptr(buff, &size);
    n = sctp_recvmsg(key->fd, ptr, size, NULL, 0, 0, 0);
    if(n<= 0) {
     return -1;
    }
    buffer_write_adv(buff, n);
    return n;

}



static unsigned
parse_command(struct selector_key * key) {

    char buf[2048] = {0};
    admin *admin = ADMIN_ATTACHMENT(key);
    buffer *buff = &admin->read_buffer;
    int bytes = recieve_from_client(key);
    if (bytes < 0) {
        return AERROR;
    }
    size_t size;
    uint8_t *ptr = buffer_read_ptr(buff, &size);
    ptr[size - 1] = '\0';
    char * token = strtok((char *)ptr, " ");
    int comando = atoi(token);
    char * arg = strtok(NULL, " ");
    bool valid = true;
    if(comando == DISECTOR_ACTIVATION || comando == ADD_USER || comando == DELETE_USER || comando == USER_ACCESS_HISTORY){
        if (arg != NULL) {
            if(strtok(NULL, " ")) {
                valid = false;
                return -1;
            }
        }
    } else {
        if(strtok(NULL, " ")) {
            valid = false;
            return -1;
        }
    }
    selector_set_interest(key->s, ADMIN_ATTACHMENT(key)->client_fd, OP_WRITE);
//    buffer_read_adv(buff, 1);
    buff = &admin->write_buffer;
    ptr = buffer_write_ptr(buff, &size);
    if(!valid) {
        *ptr = '-';
        buffer_write_adv(buff, 1);
        *ptr = comando + '0';
        buffer_write_adv(buff, 1);
        *ptr = ' ';
        buffer_write_adv(buff, 1);
        *ptr = '\n';
        buffer_write_adv(buff, 1);
        admin->state = COMMANDS;
        return COMMANDS;
    }
    char *message = malloc(255);
    bool disector_ret;

    switch (comando) {

        case STATS:

            get_stats(message);
            if(message == NULL) {
                message = "-2 \n";
                memcpy(ptr, message, strlen(message));
            } else {
                memcpy(ptr, message, strlen(message));
            }
            buffer_write_adv(buff, strlen(message));
            admin->state = COMMANDS;
            break;

        case ADMIN_CLOSE_CONNECTION:

            message = "+3 \n";
            memcpy(ptr, message, strlen(message));
            buffer_write_adv(buff, strlen(message));
            admin->state = ADONE;
            break;

        case DISECTOR_ACTIVATION:
            message = "+4 \n";
            if(*arg == '+'){
                disector_ret = disector_activation(true);
            } else if(*arg == '-') {
                disector_ret = disector_activation(false);
            } else {
                message = "-4 \n";
            }
            memcpy(ptr, message, strlen(message));
            buffer_write_adv(buff, strlen(message));
            admin->state = COMMANDS;
            break;
        case DISECTOR_DATA:
            break;
        case ADD_USER:
            break;
        case DELETE_USER:
            break;
        case LIST_USERS:
            break;
        case USER_ACCESS_HISTORY:
            break;
        default:
            message = "-ERR\n";
            memcpy(ptr, message, strlen(message));
            buffer_write_adv(buff, strlen(message));
            admin->state = COMMANDS;
            break;

    }
    free(message);
    return COMMANDS;
}