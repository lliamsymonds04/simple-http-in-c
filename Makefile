CC=gcc
CFLAGS=-Wall -Wextra -O2 -Iinclude -pthread

SRC=$(wildcard src/*.c)
OBJ=$(SRC:.c=.o)

libhttp.a: $(OBJ)
	ar rcs $@ $^

clean:
	rm -f src/*.o libhttp.a
