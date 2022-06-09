.PHONY=clean all
CFLAGS = --std=c11 -pedantic -pedantic-errors -g -Wall -Wextra -Wno-unused-parameter -Wno-implicit-fallthrough -D_POSIX_C_SOURCE=200112L
LDFLAGS = -lpthread -pthread
all: main
clean:
	- rm -f *.o ./state_machines/*.o ./parsers/*.o socks5d

COMMON = buffer.o args.o passive_sockets.o utils/users.o
SERVER_SOURCES = main.o socks5.o selector.o stm.o parser.o parser_utils.o
PARSERS = parsers/hello_parser.o parsers/hello_auth_parser.o parsers/request_read_parser.o
STATES = state_machines/hello_st.o state_machines/hello_auth_st.o state_machines/request_read_st.o

buffer.o: includes/buffer.h
passive_sockets.o: includes/passive_sockets.h
args.o: includes/args.h
socks5.o: includes/socks5.h
selector.o: includes/selector.h
stm.o: includes/stm.h
parser.o: includes/parser.h
parser_utils.o: includes/parser_utils.h
utils/users.o: utils/includes/users.h
state_machines/hello_st.o: state_machines/includes/hello_st.h
state_machines/hello_auth_st.o: state_machines/includes/hello_auth_st.h
state_machines/request_read_st.o: state_machines/includes/request_read_st.h
parsers/hello_parser.o: parsers/includes/hello_parser.h
parsers/hello_auth_parser.o: parsers/includes/hello_auth_parser.h
parsers/request_read_parser.o: parsers/includes/request_read_parser.h


main: $(SERVER_SOURCES) $(STATES) $(PARSERS) $(COMMON)
	$(CC) $(CFLAGS) $(LDFLAGS) -o socks5d $(SERVER_SOURCES) $(STATES) $(PARSERS) $(COMMON)
