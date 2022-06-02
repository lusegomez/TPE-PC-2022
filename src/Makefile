.PHONY=clean all
COMPILER=gcc
CFLAGS = -Wall -fsanitize=address -g
all: main 
clean:	
	- rm -f *.o  main

COMMON = stm.c selector.c socks5.c

main:      
	$(COMPILER) $(CFLAGS) -o main main.c $(COMMON)
