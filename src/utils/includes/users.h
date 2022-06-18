#ifndef USERS_H
#define USERS_H

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#include "../../includes/args.h"
#include "logger.h"



int add_user(struct users * usr);
int delete_user(char * username);
char * get_users(void);
int get_total_users();

bool can_login(uint8_t * user, uint8_t * pass);
bool is_admin(uint8_t * user, uint8_t * pass);

#endif