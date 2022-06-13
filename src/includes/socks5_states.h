#ifndef SOCKS5_STATE_H
#define SOCKS5_STATE_H


#include "stm.h"
#include "../state_machines/includes/hello_st.h"
#include "../state_machines/includes/hello_auth_st.h"

enum socks5_state {
    HELLO,
    HELLO_AUTH,
    REQUEST_READING,
    DNS_QUERY,
    RESPONSE_WRITING,
    COPY,
    CLOSE_CONNECTION,
    ERROR


};


#endif