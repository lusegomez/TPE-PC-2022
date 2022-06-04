.PHONY=clean all
COMPILER=gcc
CFLAGS = -Wall -fsanitize=address -g -pthread
all: main 
clean:	
	- rm -f *.o  main

COMMON = stm.c selector.c socks5.c passive_sockets.c

main:      
	$(COMPILER) $(CFLAGS) -o main main.c $(COMMON)
