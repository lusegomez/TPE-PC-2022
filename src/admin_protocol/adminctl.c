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
#include "../utils/includes/logger.h"

#define STATSC 2
#define LOGOUTC 3
#define DISECTOR_ACTIVATION 4
#define GET_DISECTOR 5
#define ADD_USER 6
#define DELETE_USER 7
#define LIST_USERS 8
#define HELPC    9
bool close_flag = false;
int totalCommands = 8;
char * commands[] = {
        "help",
        "stats",
        "close_connection",
        "disector_activation",
        "get_disector",
        "add_user",
        "delete_user",
        "list_users",
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
                      "GET_DISECTOR\n\tGets disector status\n"
                      "ADD_USER <user:pass>\n\tAdds a new user to the system\n"
                      "DELETE_USER <user>\n\tDeletes a user from the system\n"
                      "LIST_USERS\n\tLists all users in the system\n"
                      "HELP\n\tPrints this message\n\n";

static int
parse_command(int sock, char * in_buff, char * out_buffer) {
    memset(out_buffer, 0, BUFF_SIZE);
    memset(in_buff, 0, BUFF_SIZE);
    fgets(out_buffer, BUFF_SIZE, stdin);
    if (strlen(out_buffer) > 0) {
        out_buffer[strlen(out_buffer) - 1] = '\0';
    }
    char * token = strtok(out_buffer, " ");
    char * command = token;
    if (token == NULL) {
        plog(DEBUG, "-ERR \n");
        plog(INFO, "Invalid command!\n");
        //printf("-ERR \n");
        //printf("Invalid command!\n");
        return -1;
    }
    while (*token){
        *token = tolower(*token);
        token++;
    }
    int command_index = isCommand(command);
    if(command_index == -1) {
        plog(DEBUG, "-ERR \n");
        plog(INFO, "Invalid command!\n");
        //printf("-ERR \n");
        //printf("Invalid command!\n");
        return -1;
    } else {
        command_index += 1;
    }
    char arg[BUFF_SIZE] = {0};
    if(command_index == DISECTOR_ACTIVATION || command_index == ADD_USER || command_index == DELETE_USER ){
        char * aux = strtok(NULL, " ");
        if (aux == NULL) {
            plog(DEBUG, "-ERR \n");
            plog(INFO, "No arguments!\n");
            return -1;
        }
        strcpy(arg,aux);
        if (arg[0] != 0) {
            if(strtok(NULL, " ")) {
                plog(DEBUG, "-ERR \n");
                plog(INFO, "Too many arguments!\n");
                //printf("-Err \n");
                //printf("Too many arguments!\n");
                return -1;
            }
        }
    } else if( command_index == 1){
        plog(INFO, "%s", help_message);
        //printf("%s", help_message);
        return 1;
    } else if(command_index == 3) {
        close_flag = true;
    } else {
        if(strtok(NULL, " ")) {
            plog(DEBUG, "-ERR \n");
            plog(INFO, "Too many arguments!\n");
            //printf("-Err \n");
            //printf("Too many arguments!\n");
            return -1;
        }
    }


    int n;

    memset(out_buffer, 0, BUFF_SIZE);
    memset(in_buff, 0, BUFF_SIZE);
    sprintf(out_buffer, "%d ", command_index);
    if(arg[0] != 0){
        strcat(out_buffer, arg);
    }
    strcat(out_buffer, "\n");
    n = sctp_sendmsg(sock, out_buffer, strlen(out_buffer), NULL, 0,0,0,0,0,0);
    if(n < 0) {
        plog(INFO, "Error sending command!\n");
        //printf("Error sending command!\n");
        return -1;
    }
    n = sctp_recvmsg(sock,in_buff, BUFF_SIZE, NULL, 0,0,0);
    if(n < 0) {
        plog(INFO, "Error receiving response!\n");
        //printf("Error receiving response!\n");
        return -1;
    } else {
        plog(DEBUG, "%s", in_buff);
        //printf("%s", in_buff);
    }
    if(close_flag){
        return 0;
    }

    char * status_and_command = strtok(in_buff, " ");
    int status = status_and_command[0] == '+';
    status_and_command +=1;
    int rec_command = atoi(status_and_command);

    switch (rec_command) {
        case STATSC:
            if(status) {
                plog(INFO, "STATS:\nTotal conections: %s \nCurrent connections: %s\n Total bytes transfered: %s", strtok(NULL, " "), strtok(NULL, " "), strtok(NULL, " "));
            } else {
                plog(INFO, "Server error recieving stats\n");
            }
             break;
        case LOGOUTC:
            if(status) {
                plog(INFO, "Connection closed\n");
            } else {
                plog(INFO, "Server error closing connection\n");
            }
            break;
        case DISECTOR_ACTIVATION:
            if(status) {
                plog(INFO, "Disector status updated\n");
            } else {
                plog(INFO, "Server error setting disector status\n");
            }
            break;
        case GET_DISECTOR:
            if(status) {
                plog(INFO, "Disector status: %s\n", strtok(NULL, " "));
            } else {
                plog(INFO, "Server error getting disector status\n");
            }
            break;
        case ADD_USER:
            if(status) {
                plog(INFO, "User added\n");
            } else {
                char * error_message = strtok(NULL, " ");
                int error_code = error_message[0] - '0';
                switch (error_code) {
                    case 0:
                        plog(INFO, "Server error adding user\n");
                        break;
                    case 1:
                        plog(INFO, "User already exists\n");
                        break;
                    case 2:
                        plog(INFO, "Max amount of users reached\n");
                        break;
                    case 3:
                        plog(INFO, "Invalid user\n");
                        break;
                }
            }
            break;
        case DELETE_USER:
            if(status) {
                plog(INFO, "User deleted\n");
            } else {
                plog(INFO, "Server error deleting user\n");
            }
            break;
        case LIST_USERS:
            if(status) {
                plog(INFO, "Users:\n");
                char * user = strtok(NULL, " ");
                while (user != NULL) {
                    plog(INFO, "\t%s", user);
                    user = strtok(NULL, " ");
                }
            } else {
                plog(INFO, "Server error listing users\n");
            }
            break;
        default:
            break;
    }

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
        plog(INFO, "ERROR\n");
        //printf("ERROR\n");
        return -1;
    }
    nr = sctp_recvmsg(sock, in_buffer, BUFF_SIZE, NULL, 0, 0, 0);
    if (nr <= 0) {
        plog(INFO, "ERROR\n");
        //printf("ERROR\n");
        return -1;
    }
    if (in_buffer[0] == '+') {
        //printf(in_buffer);
        plog(INFO, "%sWelcome Administrator!\n", in_buffer);
        //printf("%sWelcome Administrator!\n", in_buffer);
        return  1;
    } else {
        //printf(in_buffer);
        plog(INFO, "%sWrong Password, Try Again\n", in_buffer);
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
    bool stop = false;
    parse_admin_options(argc, argv, &opt);
    set_mgmt_address(&addr, opt.mgmt_addr, &opt);
    int sock = socket(addr.mgmt_domain, SOCK_STREAM, IPPROTO_SCTP);
    if(sock < 0) {
        plog(INFO, "Failed to create socket\n");
        close(sock);
        exit(EXIT_FAILURE);
    }
    int con = connect(sock, (struct sockaddr*)&addr.mgmt_addr, addr.mgmt_addr_len);
    if (con < 0) {
        plog(INFO, "Failed to connect to management\n");
        close(sock);
        exit(EXIT_FAILURE);
    }
    int n = sctp_recvmsg(sock, incoming, BUFF_SIZE, NULL, 0,0,0);
    if(n < 0) {
        plog(INFO, "Error getting greeting\n");
        close(sock);
        exit(EXIT_FAILURE);
    } else if (n > 0){
        if(incoming[0] == '+'){
            plog(INFO, "%sGREETINGS\n", incoming);
        }
    }

    plog(INFO, "Please enter Password to login.\nPassword: \n");
    while(!status) {
        status = get_authentication(sock,incoming, out);

    }
    plog(INFO, "%s", help_message);
    while(!stop) {
        parse_command(sock, incoming, out);
        if(close_flag){
            plog(INFO, "Logging out...\n");
            stop = true;
        }
    }
    return 0;
}
