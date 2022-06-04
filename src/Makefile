.PHONY=clean all
COMPILER=gcc
CFLAGS = -Wall -fsanitize=address -g -pthread -std=c11
all: main 
clean:	
	- rm -f *.o  main

COMMON = socks5.c passive_sockets.c buffer.c selector.c stm.c

main:      
	$(COMPILER) $(CFLAGS) -o main main.c $(COMMON)
