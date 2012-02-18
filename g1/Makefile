CC=gcc
CFLAGS=-g -Wall -Wextra -pedantic -std=c99

calc: calc.o stack.o list.o
	$(CC) $(CFLAGS) calc.o stack.o list.o -o calc

calc.o: calc.c stack.h
	$(CC) $(CFLAGS) -c calc.c

stack.o: stack.c stack.h
	$(CC) $(CFLAGS) -c stack.c -o stack.o

list.o: list.c list.h
	$(CC) $(CFLAGS) -c list.c

clean:
	rm -f calc
	rm -f calc.o
	rm -f stack.o
	rm -f list.o

all: calc
