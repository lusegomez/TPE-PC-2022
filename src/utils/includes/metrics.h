#ifndef METRICS_H
#define METRICS_H

#include <stdint.h>

typedef struct metrics * metrics_t;

static metrics_t metrics;

struct metrics {
    unsigned long concurrent_connections;
    unsigned long total_connections;
    unsigned long bytes_transfered;
};

char * get_stats(void);

metrics_t
init_metrics(void);

void
free_metrics(metrics_t metrics);


#endif
