#ifndef SOCKS5_STATE
#define SOCKS5_STATE

#include "../state_machines/includes/hello_stm.h"
#include "stm.h"

enum socks5_state {
    HELLO,
    //HELLO_WRITING,
    HELLO_AUTH_WRITING,
    HELLO_AUTH_READING,
    REQUEST_READING,
 //   DNS_QUERY,
 //   DNS_RESPONSE,
    CONNECT_ORIGIN,
    REQUEST_WRITING,
    COPY,
    CLOSE_CONNECTION,
    ERROR


};

const struct state_definition states_definition[] = {
    {
        .state = HELLO,
        .on_arrival = hello_read_init,
        .on_read_ready = hello_read,
        .on_write_ready = hello_write
    },
    
    /*
    {
        .state = HELLO_WRITING,
        .on_arrival = hello_write_init,
        .on_write_ready = hello_write
    },
    {
        .state = HELLO_AUTH_READING,
        .on_arrival = hello_auth_read_init,
        .on_read_ready = hello__auth_read
    },
    {
        .state = HELLO_AUTH_WRITING,
        .on_arrival = hello_auth_write_init,
        .on_write_ready = hello_auth_write
    },
    {
        .state = REQUEST_READING,
        .on_arrival = request_reading_init,
        .on_read_ready = request_reading
    },
    {
        .state = DNS_QUERY,
        .on_arrival = dns_query_init,
        .on_departure = dns_query_close
    },
    {
        .state = DNS_RESPONSE,
        .on_arrival = dns_response_init,
        .on_read_ready = dns_response
    },
    {
        .state = CONNECT_ORIGIN,
        .on_arrival = connect_origin_init,
        .on_block_ready = connect_origin_block
    },
    {
        .state = REQUEST_WRITING,
        .on_arrival = request_write_init,
        .on_write_ready = request_write,
    },
    {
        .state = COPY,
        .on_arrival = copy_init,
        .on_read_ready = copy_read,
        .on_write_ready = copy_write,
    },
    {
        .state = CLOSE_CONNECTION,
        .on_arrival = close_connection_init,
    },
*/
    {
        .state = ERROR,
        .on_arrival = error_init,
    },
};

#endif