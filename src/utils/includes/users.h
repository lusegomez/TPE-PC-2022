#ifndef USERS_H
#define USERS_H

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include "../../includes/args.h"


int add_user(struct users * usr);
//int add_user(char * user, char * pass);

bool can_login(uint8_t * user, uint8_t * pass);
bool is_admin(uint8_t * user, uint8_t * pass);

#endif