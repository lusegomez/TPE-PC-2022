#include "includes/response.h"
#define SOCKSVERSION 0x05

//Errors


#define RESPLEN 10
#define IPV4 0x01

void request_response(struct socks5 * sock, unsigned state){
    buffer * b = &sock->write_buffer;
    sock->request_read->status = state;
    size_t n;
    uint8_t * pointer = buffer_write_ptr(b,&n);
    pointer[0] = SOCKSVERSION;
    pointer[1] = state;
    pointer[2] = 0x00;
    pointer[3] = IPV4;
    pointer[4] = 0x00;
    pointer[5] = 0x00;
    pointer[6] = 0x00;
    pointer[7] = 0x00;
    pointer[8] = 0x00;
    pointer[9] = 0x00;

    buffer_write_adv(b, RESPLEN);
}