#include "./includes/users.h"
#include <string.h>
#include <stdlib.h>

static int total_users = 0;
static int total_admins = 0;

static struct users * users = NULL;
static struct users * users_admins = NULL;
/*
int add_user(char * user, char * pass){
    if(users == NULL){
        users = calloc(255, sizeof(struct users));
        if (users == NULL) {
            return -1;
        }
    }
    if (total_users >= 255) {
        return -1;
    }
    memcpy(users[total_users].name, (const char *)user, strlen(user + 1));
    memcpy(users[total_users++].pass, (const char *)pass, strlen(pass + 1));
    return 0;
}

*/
bool can_login(uint8_t * user, uint8_t * pass){
    /*for (int i = 0; i < total_users; i++) {
        if(strcmp((char *)user, users[i].name) == 0 && strcmp((char *)pass, users[i].pass)) {
            return true;
        }
    }
    return false;
    */
}


bool is_admin(uint8_t * user, uint8_t * pass){
  /*  for (int i = 0; i < total_admins; i++) {
        if(strcmp((char *)user, users_admins[i].name) == 0 && strcmp((char *)pass, users_admins[i].pass)) {
            return true;
        }
    }
    return false;
    */
}