#include "./includes/users.h"
#include <string.h>
#include <stdlib.h>

static int total_users = 0;
static int total_admins = 0;

static struct users * users;
static struct users * users_admins;

int add_user(struct users * usr){
    if(users == NULL){
        users = calloc(MAX_USERS, sizeof(struct users));
        if (users == NULL) {
            return -1;
        }
    }
    size_t name_len = strlen(usr->name) + 1;
    size_t pass_len = strlen(usr->pass) + 1;
    users[total_users].name = malloc(name_len);
    users[total_users].pass = malloc(pass_len);
    memcpy(users[total_users].name, (const char *)usr->name, name_len);
    memcpy(users[total_users++].pass, (const char *)usr->pass, pass_len);
    return 0;
}

bool can_login(uint8_t * user, uint8_t * pass){
    for (int i = 0; i < total_users; i++) {
        const char * name = users[i].name;
        const char * password = users[i].pass;
        if(strcmp((const char *)user, name) == 0 && strcmp((const char *)pass, password) == 0) {
            return true;
        }
    }
    return false;
}


bool is_admin(uint8_t * user, uint8_t * pass){
    for (int i = 0; i < total_admins; i++) {
        if(strcmp((char *)user, users_admins[i].name) == 0 && strcmp((char *)pass, users_admins[i].pass)) {
            return true;
        }
    }
    return false;
}