CC=gcc
CFLAGS=-lm -Wall -Wextra -pedantic -pedantic-errors -Werror -ggdb -std=c99
SRCS=$(wildcard *.c)
BINS=$(SRCS:%.c=%) test-cpp

all: $(BINS)

player_movement: player_movement.c ../roguelike.h
	$(CC) $(CFLAGS) -lcurses -o $@ $<
test-cpp: test.cpp
	gcc -lm -o $@ $<
%: %.c ../roguelike.h
	$(CC) $(CFLAGS) -o $@ $<

clean:
	rm $(BINS)
