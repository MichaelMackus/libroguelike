CC=gcc
CFLAGS=-lm -Wall -Wextra -pedantic -Werror -std=c99
SRCS=$(wildcard *.c)
BINS=$(SRCS:%.c=%)

all: $(BINS)

%: %.c ../roguelike.h
	$(CC) $(CFLAGS) -o $@ $<

clean:
	rm $(BINS)
