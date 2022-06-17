#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "./includes/metrics.h"

char * get_stats()
{
    char to_ret[100];
    sprintf(to_ret, "+Total connections: %lu\nConcurrent connections: %lu\nTotal bytes transferred: %lu\n", metrics->total_connections, metrics->concurrent_connections, metrics->bytes_transfered);
    return to_ret;
}


metrics_t
init_metrics(void) {
    metrics_t ret = (metrics_t)malloc(sizeof(*ret));
    memset(ret,0,sizeof (*ret));
    if (ret == NULL) {
        perror("Error initializing metrics");
        exit(EXIT_FAILURE);
    }
    return ret;
}

void
free_metrics(metrics_t metrics) {
    free(metrics);
}