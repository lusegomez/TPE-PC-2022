#ifndef PASSIVE_SOCKETS_H
#define PASSIVE_SOCKETS_H

#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>

#include "../includes/args.h"
#include "../includes/selector.h"
int create_passive_socket_ipv4(int * s, struct socks5args args);
int create_passive_socket_ipv6(int * s, struct socks5args args);
int create_passive_socket_mngt_ipv4(int * s, struct socks5args args);
int create_passive_socket_mngt_ipv6(int * s, struct socks5args args);

#endif