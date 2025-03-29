all: examples/*.c
	make -C examples all

clean:
	make -C examples clean
