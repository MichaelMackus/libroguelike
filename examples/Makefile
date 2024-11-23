all: bsp dijkstra heap floodfill

floodfill: floodfill.c ../roguelike.h
	gcc floodfill.c -lm -o floodfill -Wall -pedantic
heap: heap.c ../roguelike.h
	gcc heap.c -lm -o heap -Wall -pedantic
bsp: bsp.c ../roguelike.h
	gcc bsp.c -lm -o bsp -Wall -pedantic
dijkstra: dijkstra.c ../roguelike.h
	gcc dijkstra.c -lm -o dijkstra -Wall -pedantic

clean:
	rm bsp dijkstra heap floodfill
