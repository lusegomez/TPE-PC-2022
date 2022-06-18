#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include "./includes/metrics.h"


struct admin_data {
    unsigned long concurrent_connections;
    unsigned long total_connections;
    unsigned long bytes_transfered;
    bool disector;
    char * disector_data[100];

} ;

struct admin_data * adminData;

void add_concurrent(void){
    adminData->concurrent_connections++;
    adminData->total_connections++;
}

void remove_concurrent(void){
    adminData->concurrent_connections--;
}


void add_bytes(unsigned long bytes){
    adminData->bytes_transfered += bytes;
}

bool get_disector(void){
    return adminData->disector;
}



void get_stats(char * m)
{
    sprintf(m, "+2 \n%lu %lu %lu\n", adminData->total_connections, adminData->concurrent_connections, adminData->bytes_transfered);

}


struct admin_data *
init_admin_data(void) {
    adminData = malloc(sizeof(struct admin_data));
    memset(adminData,0,sizeof(struct admin_data));
    adminData->disector = true;
    if (adminData == NULL) {
        perror("Error initializing admin data");
        exit(EXIT_FAILURE);
    }
    return adminData;
}

void
free_admin_data(struct admin_data *  adminData) {
    free(adminData);
}

bool disector_activation(bool activation)
{
    adminData->disector = activation;
    return adminData->disector;
}