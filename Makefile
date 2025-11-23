CC=gcc
CFLAGS=-Wall -Wextra -O2 -Iinclude -pthread

SRC=$(wildcard src/*.c)
OBJ=$(SRC:.c=.o)

http-server: $(OBJ)
	$(CC) $(CFLAGS) -o $@ $^

clean:
	rm -f src/*.o http-server
