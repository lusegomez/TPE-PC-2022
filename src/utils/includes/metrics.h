#ifndef METRICS_H
#define METRICS_H

#include <stdint.h>
#include <stdbool.h>

typedef struct admin_data * admin_data_t;

admin_data_t
init_admin_data(void);

void get_stats(char * m);

void add_concurrent(void);

void remove_concurrent(void);

int get_concurrent(void);

void add_bytes(unsigned long bytes);

bool get_disector(void);

void
free_admin_data(admin_data_t  adminData);

bool disector_activation(bool activation);


#endif