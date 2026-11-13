CC     = gcc
CFLAGS = -Wall -Wextra -g -fsanitize=address

all: tests

tests: history.c tests.c history.h
	$(CC) $(CFLAGS) history.c tests.c -o tests

test: tests
	./tests

clean:
	rm -f tests browser *.o
	rm -rf tests.dSYM
