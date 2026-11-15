CC     = gcc
CFLAGS = -Wall -Wextra -g -fsanitize=address

all: tests browser

tests: history.c tests.c history.h
	$(CC) $(CFLAGS) history.c tests.c -o tests

browser: history.c main.c history.h
	$(CC) $(CFLAGS) history.c main.c -o browser

test: tests
	./tests

clean:
	rm -f tests browser *.o
	rm -rf tests.dSYM
