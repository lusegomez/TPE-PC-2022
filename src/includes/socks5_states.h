#ifndef SOCKS5_STATE
#define SOCKS5_STATE


#include "stm.h"
#include "../state_machines/includes/hello_st.h"
#include "../state_machines/includes/hello_auth_st.h"

enum socks5_state {
    HELLO,
    //HELLO_AUTH_WRITING,
    HELLO_AUTH,
    REQUEST_READING,
 //   DNS_QUERY,
 //   DNS_RESPONSE,
//    CONNECT_ORIGIN,
//    REQUEST_WRITING,
//    COPY,
    CLOSE_CONNECTION,
    ERROR


};


#endif