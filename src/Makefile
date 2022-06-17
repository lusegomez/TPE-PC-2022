.PHONY=clean all
CFLAGS = --std=c11 -pedantic -pedantic-errors -g -Wall -Wextra -Wno-unused-parameter -Wno-implicit-fallthrough -D_POSIX_C_SOURCE=200112L -fsanitize=address
LDFLAGS = -lpthread -pthread
SCTPFLAGS = -lsctp
all: main admin
clean:
	- rm -f *.o ./state_machines/*.o ./parsers/*.o socks5d ./utils/*.o ./admin_protocol/*.o

COMMON = buffer.o utils/logger.o utils/util.o utils/metrics.o
DEP = selector.o stm.o parser.o parser_utils.o
SERVER_SOURCES = main.o socks5.o args.o passive_sockets.o utils/users.o utils/connect.o admin_protocol/admin_protocol.o
ADMIN_SOURCES = admin_protocol/adminctl.o admin_protocol/argsctl.o
PARSERS = parsers/hello_parser.o parsers/hello_auth_parser.o parsers/request_read_parser.o
STATES = state_machines/hello_st.o state_machines/hello_auth_st.o state_machines/request_read_st.o state_machines/dns_query_st.o state_machines/copy_st.o

buffer.o: includes/buffer.h
passive_sockets.o: includes/passive_sockets.h
args.o: includes/args.h
socks5.o: includes/socks5.h
selector.o: includes/selector.h
stm.o: includes/stm.h
parser.o: includes/parser.h
parser_utils.o: includes/parser_utils.h
utils/logger.o: utils/includes/logger.h
utils/util.o: utils/includes/util.h
utils/users.o: utils/includes/users.h
utils/connect.o: utils/includes/connect.h
utils/metrics.o: utils/includes/metrics.h
state_machines/hello_st.o: state_machines/includes/hello_st.h
state_machines/hello_auth_st.o: state_machines/includes/hello_auth_st.h
state_machines/request_read_st.o: state_machines/includes/request_read_st.h
state_machines/dns_query_st.o: state_machines/includes/dns_query_st.h 
state_machines/copy_st.o: state_machines/includes/copy_st.h
parsers/hello_parser.o: parsers/includes/hello_parser.h
parsers/hello_auth_parser.o: parsers/includes/hello_auth_parser.h
parsers/request_read_parser.o: parsers/includes/request_read_parser.h

admin_protocol/admin_protocol.o: admin_protocol/includes/admin_protocol.h
admin_protocol/argsctl.o: admin_protocol/includes/argsctl.h


main: $(SERVER_SOURCES) $(DEP) $(STATES) $(PARSERS) $(COMMON) 
	$(CC) $(CFLAGS) $(SCTPFLAGS) $(LDFLAGS) -o socks5d $(SERVER_SOURCES) $(DEP) $(STATES) $(PARSERS) $(COMMON) 
admin: $(ADMIN_SOURCES) $(DEP) $(COMMON)
	$(CC) $(CFLAGS) $(SCTPFLAGS) $(LDFLAGS) -o socks5dctl $(ADMIN_SOURCES) $(DEP) $(COMMON)
