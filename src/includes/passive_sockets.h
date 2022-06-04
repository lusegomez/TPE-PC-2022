#ifndef PASSIVE_SOCKETS_H
#define PASSIVE_SOCKETS_H

#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>

#include "../includes/args.h"
#include "../includes/selector.h"
int create_passive_socket(int * socket, struct socks5args args);

#endif