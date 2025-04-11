CC=gcc
CLANG=clang
CFLAGS=-lm -Wall -Wextra -pedantic -pedantic-errors -Werror -Wswitch -Wswitch-enum -ggdb -std=c99
CLANGFLAGS=-lm -Wall -Wextra -pedantic -std=c99
SRCS=$(wildcard *.c)
BINS=$(SRCS:%.c=%) test-cpp

all: $(BINS)

clang: clang.c ../roguelike.h
	clang $(CLANGFLAGS) -lm -o $@ $<
player_movement: player_movement.c ../roguelike.h
	$(CC) $(CFLAGS) -lcurses -o $@ $<
simple: simple.c ../roguelike.h
	$(CC) $(CFLAGS) -lcurses -o $@ $<
test-cpp: test.cpp
	gcc -lm -o $@ $<
%: %.c ../roguelike.h
	$(CC) $(CFLAGS) -o $@ $<

clean:
	rm $(BINS)
