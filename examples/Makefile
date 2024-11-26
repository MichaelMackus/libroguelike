all: bsp dijkstra heap floodfill

floodfill: floodfill.c ../roguelike.h
	gcc floodfill.c -lm -o floodfill -Wall -Wextra -pedantic -std=c99
heap: heap.c ../roguelike.h
	gcc heap.c -lm -o heap -Wall -Wextra -pedantic -std=c99
bsp: bsp.c ../roguelike.h
	gcc bsp.c -lm -o bsp -Wall -Wextra -pedantic -std=c99
dijkstra: dijkstra.c ../roguelike.h
	gcc dijkstra.c -lm -o dijkstra -Wextra -pedantic -std=c99

clean:
	rm bsp dijkstra heap floodfill
