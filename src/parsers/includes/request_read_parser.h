#ifndef TPE_PC_2022_REQUEST_READ_PARSER_H
#define TPE_PC_2022_REQUEST_READ_PARSER_H

#include <stdint.h>
#include <stdint.h>
#include <stdlib.h>
#include "../../includes/buffer.h"

enum request_read_state {
    request_reading_version,
    request_reading_command,
    request_reading_reserved,
    request_reading_atype,
    request_reading_destaddr_len,
    request_reading_destaddr,
    request_reading_destport,
    request_reading_end,
    request_reading_command_error,
    request_reading_atype_error,
    request_reading_error
};

struct request_read_parser {
    enum request_read_state state;
    uint8_t version;
    uint8_t command;
    uint8_t atype;
    uint8_t index;
    uint8_t destaddr_len;
    uint8_t * destaddr;
    uint8_t port[2];
};

void request_read_parser_init(struct request_read_parser *hp);
enum request_read_state consume_request(buffer * b, struct request_read_parser * hp);

#endif //TPE_PC_2022_REQUEST_READ_PARSER_H
