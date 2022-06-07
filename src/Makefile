.PHONY=clean all
CFLAGS = --std=c11 -pedantic -pedantic-errors -g -Wall -Wextra -Wno-unused-parameter -Wno-implicit-fallthrough -D_POSIX_C_SOURCE=200112L
LDFLAGS = -lpthread
all: main
clean:
	- rm -f *.o  main

COMMON = buffer.o
SERVER_SOURCES = main.o socks5.o selector.o stm.o parser.o parser_utils.o
STATES = state_machines/hello_st.o

buffer.o: includes/buffer.h
socks5.o: includes/socks5.h
selector.o: includes/selector.h
stm.o: includes/stm.h
parser.o: includes/parser.h
parser_utils.o: includes/parser_utils.h
state_machines/hello_st.o: state_machines/includes/hello_st.h


main: $(SERVER_SOURCES) $(STATES) $(COMMON)
	$(CC) $(CFLAGS) $(LDFLAGS) -o main $(SERVER_SOURCES) $(STATES) $(PARSERS) $(COMMON)
