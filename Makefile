CC = gcc
CFLAGS = -Wall -Wextra -std=c99

all: basic

basic: shell.o basic.o
	$(CC) $(CFLAGS) -o $@ $^ -lm

shell.o: shell.c basic.h
	$(CC) $(CFLAGS) -c $<

basic.o: basic.c basic.h
	$(CC) $(CFLAGS) -c $<

clean:
	rm -f basic shell.o basic.o

test:
	./basic
