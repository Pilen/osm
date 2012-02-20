CC=gcc
CFLAGS=-g -Wall -Wextra -pedantic -std=c99

all: calc brauntest

calc: calc.o stack.o list.o
	$(CC) $(CFLAGS) calc.o stack.o list.o -o calc

calc.o: calc.c stack.h
	$(CC) $(CFLAGS) -c calc.c

stack.o: stack.c stack.h
	$(CC) $(CFLAGS) -c stack.c

list.o: list.c list.h
	$(CC) $(CFLAGS) -c list.c


brauntest: brauntest.o braunseq.o
	$(CC) $(CFLAGS) brauntest.o braunseq.o -o brauntest
brauntest.o: brauntest.c braunseq.h
	$(CC) $(CFLAGS) -c brauntest.c
braunseq.o: braunseq.c braunseq.h
	$(CC) $(CFLAGS) -c braunseq.c

clean:
	rm -f calc
	rm -f calc.o
	rm -f stack.o
	rm -f list.o

	rm -f brauntest
	rm -f brauntest.o
	rm -f braunseq.o
