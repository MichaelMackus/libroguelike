CC=gcc
CFLAGS=-lm -Wall -Wextra -pedantic -Werror -std=c99
SRCS=$(wildcard *.c)
BINS=$(SRCS:%.c=%)

all: $(BINS)

fov: fov.c ../roguelike.h
	$(CC) $(CFLAGS) -lcurses -o $@ $<
%: %.c ../roguelike.h
	$(CC) $(CFLAGS) -o $@ $<

clean:
	rm $(BINS)
