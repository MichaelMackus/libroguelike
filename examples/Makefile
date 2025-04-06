CC=gcc
CFLAGS=-lm -Wall -Wextra -pedantic -pedantic-errors -Werror -ggdb -std=c99
SRCS=$(wildcard *.c)
BINS=$(SRCS:%.c=%)

all: $(BINS)

player_movement: player_movement.c ../roguelike.h
	$(CC) $(CFLAGS) -lcurses -o $@ $<
%: %.c ../roguelike.h
	$(CC) $(CFLAGS) -o $@ $<

clean:
	rm $(BINS)
