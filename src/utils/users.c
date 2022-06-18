#include "./includes/users.h"
#include <string.h>
#include <stdlib.h>

#define MAX_USER_NAME_LEN 255
#define MAX_USER_PASS_LEN 255

static int total_users = 0;
static int total_admins = 0;

static struct users * users = NULL;

int add_user(struct users * usr){
    if(total_users == MAX_USERS){
        return -2;
    }

    if(usr->name[0] == 0 || usr->pass[0] == 0){
        plog(ERRORR, "Error adding user %s", usr->name);
        return -1;
    }

    if(users == NULL){
        users = calloc(MAX_USERS, sizeof(struct users));
        if (users == NULL) {
            plog(ERRORR, "Error adding user %s", usr->name);
            return -1;
        }
    }
    for(int i = 0; i < total_users; i++){
        if(strcmp(users[i].name, usr->name) == 0) {
            plog(ERRORR, "Error adding user %s, user already exists", usr->name);
            return -3;
        }
    }


    size_t name_len = strlen(usr->name) + 1;
    size_t pass_len = strlen(usr->pass) + 1;
    for(int j = 0; j < pass_len; j++){
        if(usr->pass[j] == ':'){
            plog(ERRORR, "Error adding user %s, contains ':' in password", usr->name);
            return -4;
        }
    }
    if(name_len > MAX_USER_NAME_LEN || pass_len > MAX_USER_PASS_LEN){
        plog(ERRORR, "Error adding user %s, name or password too long", usr->name);
        return -4;
    }
    users[total_users].name = malloc(name_len);
    users[total_users].pass = malloc(pass_len);
    memcpy(users[total_users].name, (const char *)usr->name, name_len);
    memcpy(users[total_users++].pass, (const char *)usr->pass, pass_len);
    plog(INFO, "User %s added", usr->name);
    return 0;
}

//Delete user from the system
int delete_user(char * username){
    if(total_users == 0){
        plog(ERRORR, "Error deleting user %s", username);
        return -1;
    }

    for(int i = 0; i < total_users; i++) {
        if (!strcmp(users[i].name, username)) {
            free(users[i].name);
            free(users[i].pass);
            for (int j = i; j < total_users - 1; j++) {
                users[j] = users[j + 1];
            }
            total_users--;
            plog(ERRORR, "User %s deleted", username);
            return 0;
        }
    }
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

int get_total_users(){
    return total_users;
}


char * get_users() {
    if(total_users == 0){
        return NULL;
    }
    char *to_ret = calloc(total_users, MAX_USER_NAME_LEN);
    for (int i = 0; i < total_users; i++) {
        strcat(to_ret, users[i].name);
        if(i < total_users - 1){
            strcat(to_ret, " ");
        }
    }
    strcat(to_ret, "\n");
    return to_ret;
}