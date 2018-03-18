# Makefile for CSSE2310 Assignment 3

CC=gcc
CFLAGS=-Wall -pedantic -std=gnu99
DEBUG=-g

all: hub.o acrophobe.o bandit.o spoiler.o player.o shared.o comms.o
		$(CC) $(CFLAGS) -o 2310express hub.o shared.o comms.o -lm
		$(CC) $(CFLAGS) -o acrophobe acrophobe.o player.o shared.o comms.o -lm
		$(CC) $(CFLAGS) -o bandit bandit.o player.o shared.o comms.o -lm
		$(CC) $(CFLAGS) -o spoiler spoiler.o player.o shared.o comms.o -lm
		@echo "Compiled!"

hub.o: hub.c
		$(CC) $(CFLAGS) -c hub.c

acrophobe.o: acrophobe.c
		$(CC) $(CFLAGS) -c acrophobe.c

bandit.o: bandit.c
		$(CC) $(CFLAGS) -c bandit.c

spoiler.o: spoiler.c
		$(CC) $(CFLAGS) -c spoiler.c

player.o: player.c
		$(CC) $(CFLAGS) -c player.c

shared.o: shared.c
		$(CC) $(CFLAGS) -c shared.c

comms.o: comms.c
		$(CC) $(CFLAGS) -c comms.c

clean:
		rm -f *.o 2310express acrophobe bandit spoiler
		@echo "Clean successful!"
