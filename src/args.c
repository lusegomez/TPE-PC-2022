#include <stdio.h>     /* for printf */
#include <stdlib.h>    /* for exit */
#include <limits.h>    /* LONG_MIN et al */
#include <string.h>    /* memset */
#include <errno.h>
#include <getopt.h>

#include "./includes/args.h"
#include "./utils/includes/util.h"

static unsigned short
port(const char *s) {
     char *end     = 0;
     const long sl = strtol(s, &end, 10);

     if (end == s|| '\0' != *end
        || ((LONG_MIN == sl || LONG_MAX == sl) && ERANGE == errno)
        || sl < 0 || sl > USHRT_MAX) {
         fprintf(stderr, "port should in in the range of 1-65536: %s\n", s);
         exit(1);
         return 1;
     }
     return (unsigned short)sl;
}

static void
user(char *s, struct users *user) {
    char *p = strchr(s, ':');
    if(p == NULL) {
        fprintf(stderr, "password not found\n");
        exit(1);
    } else {
        *p = 0;
        p++;
        user->name = s;
        user->pass = p;
    }

}

static void
version(void) {
    fprintf(stderr, "socks5v version 1.0\n"
                    "ITBA Protocolos de Comunicación 2022/2 -- Grupo 12\n"
                    "LICENCIA\n");
}

static void
usage(const char *progname) {
    fprintf(stderr,
        "Usage: %s [OPTION]...\n"
        "\n"
        "   -h               Imprime la ayuda y termina.\n"
        "   -l <SOCKS addr>  Dirección donde servirá el proxy SOCKS.\n"
        "   -L <conf  addr>  Dirección donde servirá el servicio de management.\n"
        "   -p <SOCKS port>  Puerto entrante conexiones SOCKS.\n"
        "   -P <conf port>   Puerto entrante conexiones configuracion\n"
        "   -u <name>:<pass> Usuario y contraseña de usuario que puede usar el proxy. Hasta 10.\n"
        "   -v               Imprime información sobre la versión versión y termina.\n"
        "   -N               Deshabilita los disectors.\n"
        "\n",
        progname);
    exit(1);
}

void 
parse_args(const int argc, char **argv, struct socks5args *args) {
    memset(args, 0, sizeof(*args)); 

    args->socks_addr = "0.0.0.0";
    args->socks_addr6 = "::";
    args->socks_port = 1080;


    args->mng_addr   = "0.0.0.0";
    args->mng_port   = 8080;

    args->disectors_enabled = true;


    int c;
    args->nusers = 0;
    while (true) {
        c = getopt(argc, argv, "hl:L:Np:P:u:v");
        if (c == -1)
            break;

        switch (c) {
            case 'h':
                usage(argv[0]);
                break;
            case 'l':
                if(getAddressType(optarg) == AF_INET6) {
                    args->socks_addr6 = optarg;
                    args->socks_addr = NULL;
                } else if (getAddressType(optarg) == AF_INET) {
                    args->socks_addr = optarg;
                    args->socks_addr6 = NULL;
                } else {
                    fprintf(stderr, "Invalid proxy address: %s\n", optarg);
                    exit(1);
                }
                break;
            case 'L':
                if(getAddressType(optarg) == AF_INET6) {
                    args->mng_addr6 = optarg;
                    args->mng_addr = NULL;
                } else if (getAddressType(optarg) == AF_INET) {
                    args->mng_addr = optarg;
                    args->mng_addr6 = NULL;
                } else {
                    fprintf(stderr, "Invalid management address: %s\n", optarg);
                    exit(1);
                }
                break;
            case 'N':
                args->disectors_enabled = false;
                break;
            case 'p':
                args->socks_port = port(optarg);
                break;
            case 'P':
                args->mng_port   = port(optarg);
                break;
            case 'u':
                if(args->nusers >= MAX_USERS) {
                    fprintf(stderr, "maximun number of command line users reached: %d.\n", MAX_USERS);
                    exit(1);
                } else {
                    user(optarg, args->users + args->nusers);
                    args->nusers++;
                }
                break;
            case 'v':
                version();
                exit(0);
                break;
            default:
                fprintf(stderr, "unknown argument %d.\n", c);
                exit(1);
        }

    }
    if (optind < argc) {
        fprintf(stderr, "argument not accepted: ");
        while (optind < argc) {
            fprintf(stderr, "%s ", argv[optind++]);
        }
        fprintf(stderr, "\n");
        exit(1);
    }
}
