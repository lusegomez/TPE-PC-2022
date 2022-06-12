#ifndef TPE_PC_2022_DNS_QUERY_ST_H
#define TPE_PC_2022_DNS_QUERY_ST_H

#include "../../includes/buffer.h"
#include "../../includes/selector.h"

enum dns_query_status{
    dns_query_ok,
    dns_query_fail
};

struct dns_query_st{
    enum dns_query_status status;
};

void dns_query_init(const unsigned state, struct selector_key *key);
unsigned dns_query_close(struct selector_key * key);

#endif //TPE_PC_2022_DNS_QUERY_ST_H