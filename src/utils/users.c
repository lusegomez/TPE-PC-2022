#include "./includes/users.h"
#include <string.h>
bool can_login(uint8_t * user, uint8_t * pass){
    for (int i = 0; i < total_users; i++) {
        if(strcmp((char *)user, users[i].name) == 0 && strcmp((char *)pass, users[i].pass)) {
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