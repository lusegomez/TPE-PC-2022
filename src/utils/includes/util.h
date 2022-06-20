#ifndef UTIL_H_
#define UTIL_H_
#include <netdb.h>
#include <stdbool.h>
#include <stdio.h>
#include <sys/socket.h>
#include <time.h>


int printSocketAddress(const struct sockaddr *address, char * addrBuffer);

const char * printFamily(struct addrinfo *aip);
const char * printType(struct addrinfo *aip);
const char * printProtocol(struct addrinfo *aip);
void printFlags(struct addrinfo *aip);
char * printAddressPort( const struct addrinfo *aip, char addr[]);
int getAddressType(char *ipAddress);
char *getIpAddress(struct sockaddr_storage *sa);
int getPort(struct sockaddr_storage *sa);

// Determina si dos sockets son iguales (misma direccion y puerto)
int sockAddrsEqual(const struct sockaddr *addr1, const struct sockaddr *addr2);
extern const char *
sockaddr_to_human(char *buff, const size_t buffsize,
                  const struct sockaddr *addr);
char * getDateTime(void);
#endif 
