#include "stm.h"
#include "selector.h"
#include <stdint.h>

struct socks5 {
    int client_fd;
    int origin_fd;

    char * client_ip;
    uint16_t client_port;
    char * origin_ip;
    uint16_t origin_port;
    //struct users current_users;

    struct hello_stm hello_stm;
    struct hello_auth_stm hello_auth_stm;
    struct request_stm request_stm;
    struct dns_stm dns_stm;
    struct connect_origin_stm connect_origin_stm;
    struct copy_stm copy_stm;
    struct state_machine state_machine;
    struct error_stm error_stm;


};

void socksv5_passive_accept(struct selector_key * key);


void socks5_handle_read(struct selector_key * key);
void socks5_handle_write(struct selector_key * key);
void socks5_handle_block(struct selector_key * key);
void socks5_handle_close(struct selector_key * key);

const fd_handler socks5_active_handler = {
    .handle_read = socks5_handle_read,
    .handle_write = socks5_handle_write,
    .handle_block = socks5_handle_block,
    .handle_close = socks5_handle_close
};
