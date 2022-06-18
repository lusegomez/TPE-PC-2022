#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stddef.h>
#include <ctype.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <netinet/sctp.h>
#include <fcntl.h>
#include <stdint.h>
#include <signal.h>
#include <unistd.h>
#include <stdbool.h>

#include "includes/argsctl.h"

#define STATSC 2
#define LOGOUTC 3
#define DISECTOR_ACTIVATION 4
#define DISECTOR_DATA 5
#define ADD_USER 6
#define DELETE_USER 7
#define LIST_USERS 8
#define USER_ACCESS_HISTORY 9
#define HELPC    10
int totalCommands = 9;
char * commands[] = {
        "help",
        "stats",
        "logout",
        "disector_activation",
        "disector_data",
        "add_user",
        "delete_user",
        "list_users",
        "user_access_history",
};

static int isCommand(char * name){
    for (int i = 0; i < totalCommands; i++) {
        if (!strcmp(commands[i],name)){
            return i;
        }
    }
    return -1;
}

char * help_message = "These are all available commmands:\n"
                      "STATS\n\tPrints useful statistics about the proxy server\n"
                      "CLOSE_CONNECTION\n\tDisconnects admin client\n"
                      "DISECTOR_ACTIVATION\n\tActivates or deactivates the disector\n"
                      "DISECTOR_DATA\n\tGets data from the disector\n"
                      "ADD_USER <user:pass>\n\tAdds a new user to the system\n"
                      "DELETE_USER <user>\n\tDeletes a user from the system\n"
                      "LIST_USERS\n\tLists all users in the system\n"
                      "USER_ACCESS_HISTORY <user>\n\tGets the access history of a user\n"
                      "HELP\n\tPrints this message\n";

static int
parse_command(int sock, char * in_buff, char * out_buffer) {
    memset(out_buffer, 0, BUFF_SIZE);
    memset(in_buff, 0, BUFF_SIZE);
    fgets(out_buffer, BUFF_SIZE, stdin);
    /*      strlen siempre devuelve >=0
    if (strlen(out_buffer) < 0) {
        return -1;
    }*/
    //char args[100] = {0};
    out_buffer[strlen(out_buffer) - 1] = '\0';
    char * token = strtok(out_buffer, " ");
    char * command = token;
    while (*token){
        *token = tolower(*token);
        token++;
    }
    int command_index = isCommand(command);
    if(command_index == -1) {
        printf("-Err \n");
        printf("Invalid command!\n");
        return -1;
    } else {
        command_index += 1;
    }
    char * arg = NULL;
    if(command_index == 4 || command_index == 6 || command_index == 7 || command_index == 9){
        arg = strtok(NULL, " ");
        if (arg != NULL) {
            if(strtok(NULL, " ")) {
                printf("-Err \n");
                printf("Too many arguments!\n");
                return -1;
            }
        }
    } else if( command_index == 1){
        printf("%s", help_message);
        return 1;
    } else {
        if(strtok(NULL, " ")) {
            printf("-Err \n");
            printf("Too many arguments!\n");
            return -1;
        }
    }


    int n;

    memset(out_buffer, 0, BUFF_SIZE);
    memset(in_buff, 0, BUFF_SIZE);
    sprintf(out_buffer, "%d ", command_index);
    if(arg != NULL){
        strcat(out_buffer, arg);
    }
    strcat(out_buffer, "\n");
    n = sctp_sendmsg(sock, out_buffer, strlen(out_buffer), NULL, 0,0,0,0,0,0);
    if(n < 0) {
        printf("Error sending command!\n");
        return -1;
    }
    n = sctp_recvmsg(sock,in_buff, BUFF_SIZE, NULL, 0,0,0);
    if(n < 0) {
        printf("Error receiving response!\n");
        return -1;
    } else {
        printf("%s", in_buff);
    }

//    switch (command) {
//        case STATSC:
//            if(in_buff[0] == '+') {
//                printf(in_buff+1);
//            } else if (in_buff[0] == '-') {
//                printf("Failed to get stats\n");
//            }
//            break;
//        case LOGOUTC:
//            if(in_buff[0] == '+') {
//                printf("Logging out...\n");
//            } else if (in_buff[0] == '-') {
//                printf("Failed to log out\n");
//            }
//            break;
//        case HELPC:
//            printf(help_message);
//            break;
//    }

    return 1;
}

int
get_authentication(int sock,char * in_buffer,char * out_buffer) {
    int n;
    int nr;
    memset(in_buffer, 0, BUFF_SIZE);
    memset(out_buffer, 0, BUFF_SIZE);
    fgets(out_buffer, BUFF_SIZE, stdin);
    strtok(out_buffer, "\n");
    if ((n = sctp_sendmsg(sock, out_buffer, strlen(out_buffer),
                         NULL, 0, 0, 0, 0, 0, 0)) < 0) {
        printf("ERROR\n");
        return -1;
    }
    nr = sctp_recvmsg(sock, in_buffer, BUFF_SIZE, NULL, 0, 0, 0);
    if (nr <= 0) {
        printf("ERROR\n");
        return -1;
    }
    if (in_buffer[0] == '+') {
        //printf(in_buffer);
        printf("%sWelcome Administrator!\n", in_buffer);
        return  1;
    } else {
        //printf(in_buffer);
        printf("%sWrong Password, Try Again\n", in_buffer);
        return 0;
    }
}


int
main(const int argc, char **argv) {

    int status = 0;
    struct admin_opt opt;
    struct address_data addr;
    char out[BUFF_SIZE] = {0};
    char incoming[BUFF_SIZE] = {0};
    parse_admin_options(argc, argv, &opt);
    set_mgmt_address(&addr, opt.mgmt_addr, &opt);
    int sock = socket(addr.mgmt_domain, SOCK_STREAM, IPPROTO_SCTP);
    if(sock < 0) {
        printf("Failed to create socket\n");
        close(sock);
        exit(EXIT_FAILURE);
    }
    int con = connect(sock, (struct sockaddr*)&addr.mgmt_addr, addr.mgmt_addr_len);
    if (con < 0) {
        printf("Failed to connect to management\n");
        close(sock);
        exit(EXIT_FAILURE);
    }
    int n = sctp_recvmsg(sock, incoming, BUFF_SIZE, NULL, 0,0,0);
    if(n < 0) {
        printf("Error getting greeting\n");
        close(sock);
        exit(EXIT_FAILURE);
    } else if (n > 0){
        if(incoming[0] == '+'){
            printf("%sGREETINGS\n", incoming);
        }
    }

    printf("Please enter Password to login.\nPassword: \n");
    while(!status) {
        status = get_authentication(sock,incoming, out);

    }
    printf("%s", help_message);
    while(1) {
        parse_command(sock, incoming, out);
    }
    return 0;
}
