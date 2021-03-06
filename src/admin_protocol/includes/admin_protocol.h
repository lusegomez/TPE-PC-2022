#ifndef ADMIN_PROTOCOL_H
#define ADMIN_PROTOCOL_H


#include "../../includes/buffer.h"
#include "../../includes/stm.h"
#include "../../includes/args.h"
#include "../../utils/includes/logger.h"
#include "../../utils/includes/metrics.h"
#include "../../utils/includes/users.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <limits.h>
#include <errno.h>
#include <signal.h>

#include <unistd.h>
#include <sys/types.h>   // socket
#include <sys/socket.h>  // socket
#include <netdb.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <netinet/sctp.h>
#include <assert.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <ctype.h> // para toupper

#define N(x)                (sizeof(x)/sizeof((x)[0]))
#define ADMIN_ATTACHMENT(key)     ( ( struct admin * )(key)->data)
#define BUFFERSIZE 4096

typedef enum command_parser_states
{
    STATS=2,
    ADMIN_CLOSE_CONNECTION,
    DISECTOR_ACTIVATION, //tiene args
    GET_DISECTOR,
    ADD_USER, //tiene args
    DELETE_USER, //tiene args
    LIST_USERS,


} command_parser_states;


typedef enum admin_states
{
    GREETING,
    AUTH,
    COMMANDS,
    ADONE,
    AERROR,

} admin_states;


typedef struct admin
{
    int client_fd;
    struct sockaddr_storage       client_addr;
    socklen_t                     client_addr_len;

    buffer read_buffer, write_buffer;
    uint8_t raw_buff_a[BUFFERSIZE], raw_buff_b[BUFFERSIZE];
    struct state_machine stm;
    struct admin * next;
    unsigned                references;
    admin_states state;
} admin;



//static void
//hop(const unsigned state, struct selector_key *key);

struct admin *
new_admin(int client_fd);

void admin_read   (struct selector_key *key);

void admin_write  (struct selector_key *key);

void admin_block  (struct selector_key *key);

void admin_close  (struct selector_key *key);


static const struct fd_handler admin_handler = {
        .handle_read   = admin_read,
        .handle_write  = admin_write,
        .handle_close  = admin_close,
        .handle_block  = admin_block
};

void admin_connection(struct selector_key * key);

#endif