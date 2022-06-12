#ifndef TPE_PC_2022_RESPONSE_H
#define TPE_PC_2022_RESPONSE_H

#include "../../includes/buffer.h"
#include "../../includes/socks5.h"

#define SUCCEDED                          0x00
#define GENERAL_SOCKS_SERVER_FAILURE      0X01
#define CONNECTION_NOT_ALLOWED_BY_RULESET 0x02
#define NETWORK_UNREACHABLE               0x03
#define HOST_UNREACHABLE                  0x04
#define CONNECTION_REFUSED                0x05
#define TTL_EXPIRED                       0x06
#define COMMAND_NOT_SUPPORTED             0x07
#define ADDRESS_TYPE_NOT_SUPPORTED        0x08

void request_response(struct socks5 * sock, unsigned state);
#endif //TPE_PC_2022_RESPONSE_H
