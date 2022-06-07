#ifndef SOCKS5_STATE
#define SOCKS5_STATE


#include "stm.h"
#include "../state_machines/includes/hello_st.h"

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


#endif