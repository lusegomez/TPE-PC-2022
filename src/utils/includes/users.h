#ifndef USERS_H
#define USERS_H

#include <stdbool.h>
#include <stdint.h>
#include "../../includes/args.h"


static int total_users = 0;
static int total_admins = 0;

static struct users * users;
static struct users * users_admins;


bool can_login(uint8_t * user, uint8_t * pass);

#endif