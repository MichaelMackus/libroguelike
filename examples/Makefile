CC=cc
CFLAGS=-Wall -Wextra -pedantic -pedantic-errors -Wswitch -Wswitch-enum -ggdb -std=c99
LIBFLAGS=-lm
LIBFLAGS_CURSES=-lcurses
SRCS=$(wildcard *.c)
BINS=$(SRCS:%.c=%) test-cpp

all: $(BINS) roguelike.o

roguelike.o: ../roguelike.h
	$(CC) $(CFLAGS) -x c -c -DRL_IMPLEMENTATION -o $@ $<
player_movement: player_movement.c ../roguelike.h
	$(CC) $(CFLAGS) -o $@ $< $(LIBFLAGS) $(LIBFLAGS_CURSES)
simple: simple.c ../roguelike.h
	$(CC) $(CFLAGS) -o $@ $< $(LIBFLAGS) $(LIBFLAGS_CURSES)
c89: c89.c ../roguelike.h
	$(CC) $(CFLAGS) -std=c89 -o $@ $< $(LIBFLAGS) $(LIBFLAGS_CURSES)
test-cpp: test.cpp ../roguelike.h
	$(CC) -lm -Wno-narrowing -o $@ $< $(LIBFLAGS)
%: %.c ../roguelike.h
	$(CC) $(CFLAGS) -o $@ $< $(LIBFLAGS)

clean:
	rm $(BINS)
