.PHONY=clean all
CFLAGS = --std=c11 -pedantic -pedantic-errors -g -Wall -Wextra -Wno-unused-parameter -Wno-implicit-fallthrough -D_POSIX_C_SOURCE=200112L
LDFLAGS = -lpthread
all: main
clean:
	- rm -f *.o  main

COMMON = buffer.o
SERVER_SOURCES = main.o socks5.o passive_sockets.o selector.o stm.o parser.o parser_utils.o
STATES = state_machines/hello_st.o
socks5.o: includes/socks5.h

main: $(SERVER_SOURCES) $(STATES) $(COMMON)
	$(CC) $(CFLAGS) $(LDFLAGS) -o main $(SERVER_SOURCES) $(STATES) $(PARSERS) $(COMMON)
