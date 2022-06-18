#ifndef COPY_H
#define COPY_H

#include "../../includes/socks5.h"
#include "../../includes/socks5_states.h"
#include "../../includes/buffer.h"
#include "../../utils/includes/metrics.h"
#include <stdbool.h>
#include <stdio.h>
#include <ctype.h>

enum pop3_state {
    POP3_READING_COMMAND,
    POP3_READING_USER,
    POP3_READING_PASSWORD,
    POP3_PARSER_DONE,
};

struct pop3_st {
    enum pop3_state pop3_state;
    char user[255];
    bool user_done;
    char pass[255];
    bool pass_done;
};

unsigned copy_read(struct selector_key * key);
unsigned copy_write(struct selector_key * key);


#endif