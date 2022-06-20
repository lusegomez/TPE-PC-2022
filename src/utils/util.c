#include <stdio.h>
#include <string.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <stdbool.h>
#include "./includes/util.h"

const char *
printFamily(struct addrinfo *aip)
{
	switch (aip->ai_family) {
	case AF_INET:
		return "inet";
	case AF_INET6:
		return "inet6";
	case AF_UNIX:
		return "unix";
	case AF_UNSPEC:
		return "unspecified";
	default:
		return "unknown";
	}
}

const char *
printType(struct addrinfo *aip)
{
	switch (aip->ai_socktype) {
	case SOCK_STREAM:
		return "stream";
	case SOCK_DGRAM:
		return "datagram";
	case SOCK_SEQPACKET:
		return "seqpacket";
	case SOCK_RAW:
		return "raw";
	default:
		return "unknown ";
	}
}

const char *
printProtocol(struct addrinfo *aip)
{
	switch (aip->ai_protocol) {
	case 0:
		return "default";
	case IPPROTO_TCP:
		return "TCP";
	case IPPROTO_UDP:
		return "UDP";
	case IPPROTO_RAW:
		return "raw";
	default:
		return "unknown ";
	}
}

void
printFlags(struct addrinfo *aip)
{
	printf("flags");
	if (aip->ai_flags == 0) {
		printf(" 0");
	} else {
		if (aip->ai_flags & AI_PASSIVE)
			printf(" passive");
		if (aip->ai_flags & AI_CANONNAME)
			printf(" canon");
		if (aip->ai_flags & AI_NUMERICHOST)
			printf(" numhost");
		if (aip->ai_flags & AI_NUMERICSERV)
			printf(" numserv");
		if (aip->ai_flags & AI_V4MAPPED)
			printf(" v4mapped");
		if (aip->ai_flags & AI_ALL)
			printf(" all");
	}
}

char *
printAddressPort( const struct addrinfo *aip, char addr[]) 
{
	char abuf[INET6_ADDRSTRLEN];
	const char *addrAux ;
	if (aip->ai_family == AF_INET) {
		struct sockaddr_in	*sinp;
		sinp = (struct sockaddr_in *)aip->ai_addr;
		addrAux = inet_ntop(AF_INET, &sinp->sin_addr, abuf, INET_ADDRSTRLEN);
		if ( addrAux == NULL )
			addrAux = "unknown";
		strcpy(addr, addrAux);
		if ( sinp->sin_port != 0) {
			sprintf(addr + strlen(addr), ": %d", ntohs(sinp->sin_port));
		}
	} else if ( aip->ai_family ==AF_INET6) {
		struct sockaddr_in6	*sinp;
		sinp = (struct sockaddr_in6 *)aip->ai_addr;
		addrAux = inet_ntop(AF_INET6, &sinp->sin6_addr, abuf, INET6_ADDRSTRLEN);
		if ( addrAux == NULL )
			addrAux = "unknown";
		strcpy(addr, addrAux);			
		if ( sinp->sin6_port != 0)
			sprintf(addr + strlen(addr), ": %d", ntohs(sinp->sin6_port));
	} else
		strcpy(addr, "unknown");
	return addr;
}


int 
printSocketAddress(const struct sockaddr *address, char *addrBuffer) {

	void *numericAddress; 

	in_port_t port;

	switch (address->sa_family) {
		case AF_INET:
			numericAddress = &((struct sockaddr_in *) address)->sin_addr;
			port = ntohs(((struct sockaddr_in *) address)->sin_port);
			break;
		case AF_INET6:
			numericAddress = &((struct sockaddr_in6 *) address)->sin6_addr;
			port = ntohs(((struct sockaddr_in6 *) address)->sin6_port);
			break;
		default:
			strcpy(addrBuffer, "[unknown type]");    // Unhandled type
			return 0;
	}
	// Convert binary to printable address
	if (inet_ntop(address->sa_family, numericAddress, addrBuffer, INET6_ADDRSTRLEN) == NULL)
		strcpy(addrBuffer, "[invalid address]"); 
	else {
		if (port != 0)
			sprintf(addrBuffer + strlen(addrBuffer), ":%u", port);
	}
	return 1;
}

int sockAddrsEqual(const struct sockaddr *addr1, const struct sockaddr *addr2) {
	if (addr1 == NULL || addr2 == NULL)
		return addr1 == addr2;
	else if (addr1->sa_family != addr2->sa_family)
		return 0;
	else if (addr1->sa_family == AF_INET) {
		struct sockaddr_in *ipv4Addr1 = (struct sockaddr_in *) addr1;
		struct sockaddr_in *ipv4Addr2 = (struct sockaddr_in *) addr2;
		return ipv4Addr1->sin_addr.s_addr == ipv4Addr2->sin_addr.s_addr && ipv4Addr1->sin_port == ipv4Addr2->sin_port;
	} else if (addr1->sa_family == AF_INET6) {
		struct sockaddr_in6 *ipv6Addr1 = (struct sockaddr_in6 *) addr1;
		struct sockaddr_in6 *ipv6Addr2 = (struct sockaddr_in6 *) addr2;
		return memcmp(&ipv6Addr1->sin6_addr, &ipv6Addr2->sin6_addr, sizeof(struct in6_addr)) == 0 
			&& ipv6Addr1->sin6_port == ipv6Addr2->sin6_port;
	} else
		return 0;
}

extern const char *
sockaddr_to_human(char *buff, const size_t buffsize,
                  const struct sockaddr *addr) {
    if(addr == 0) {
        strncpy(buff, "null", buffsize);
        return buff;
    }
    in_port_t port;
    void *p = 0x00;
    bool handled = false;

    switch(addr->sa_family) {
        case AF_INET:
            p    = &((struct sockaddr_in *) addr)->sin_addr;
            port =  ((struct sockaddr_in *) addr)->sin_port;
            handled = true;
            break;
        case AF_INET6:
            p    = &((struct sockaddr_in6 *) addr)->sin6_addr;
            port =  ((struct sockaddr_in6 *) addr)->sin6_port;
            handled = true;
            break;
    }
    if(handled) {
        if (inet_ntop(addr->sa_family, p,  buff, buffsize) == 0) {
            strncpy(buff, "unknown ip", buffsize);
            buff[buffsize - 1] = 0;
        }
    } else {
        strncpy(buff, "unknown", buffsize);
    }

    strncat(buff, ":", buffsize);
    buff[buffsize - 1] = 0;
    const size_t len = strlen(buff);

    if(handled) {
        snprintf(buff + len, buffsize - len, "%d", ntohs(port));
    }
    buff[buffsize - 1] = 0;

    return buff;
}

//function that returns date and time
char *getDateTime() {
    time_t rawtime;
    struct tm * timeinfo;
    char *dateTime = malloc(sizeof(char) * 25);
    time(&rawtime);
    timeinfo = localtime(&rawtime);
    strftime(dateTime, 25, "%Y-%m-%dT%H:%M:%SZ", timeinfo);
    return dateTime;
}

//function that recieves ip address as string and returns type of address
int getAddressType(char *ipAddress) {
    struct sockaddr_in sa;
    struct sockaddr_in6 sa6;
    if(inet_pton(AF_INET, ipAddress, &(sa.sin_addr)) == 1) {
        return AF_INET;
    } else if(inet_pton(AF_INET6, ipAddress, &(sa6.sin6_addr)) == 1) {
        return AF_INET6;
    } else {
        return -1;
    }
}

//function that returns ip address as string from sockaddr_storage
char *getIpAddress(struct sockaddr_storage *addr) {
    char *ipAddress = malloc(sizeof(char) * INET6_ADDRSTRLEN);
    if(addr->ss_family == AF_INET) {
        struct sockaddr_in *ipv4Addr = (struct sockaddr_in *) addr;
        inet_ntop(AF_INET, &(ipv4Addr->sin_addr), ipAddress, INET6_ADDRSTRLEN);
    } else if(addr->ss_family == AF_INET6) {
        struct sockaddr_in6 *ipv6Addr = (struct sockaddr_in6 *) addr;
        inet_ntop(AF_INET6, &(ipv6Addr->sin6_addr), ipAddress, INET6_ADDRSTRLEN);
    } else {
        strcpy(ipAddress, "unknown");
    }
    return ipAddress;
}

//function that returns port number from sockaddr_storage
int getPort(struct sockaddr_storage *addr) {
    if(addr->ss_family == AF_INET) {
        struct sockaddr_in *ipv4Addr = (struct sockaddr_in *) addr;
        return ntohs(ipv4Addr->sin_port);
    } else if(addr->ss_family == AF_INET6) {
        struct sockaddr_in6 *ipv6Addr = (struct sockaddr_in6 *) addr;
        return ntohs(ipv6Addr->sin6_port);
    } else {
        return -1;
    }
}