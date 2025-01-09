cc := gcc
CFLAGS := -Wall -Wextra -g

all: aesdsocket

aesdsocket: aesdsocket.o
	$(cc) $(CFLAGS) aesdsocket.o -o aesdsocket

aesdsocket.o: aesdsocket.c
	$(cc) $(CFLAGS) -c aesdsocket.c -o aesdsocket.o

clean:
	rm -f aesdsocket.o aesdsocket
