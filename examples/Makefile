all: bsp dijkstra heap floodfill path

floodfill: floodfill.c ../roguelike.h
	gcc floodfill.c -lm -o floodfill -Wall -Wextra -pedantic -Werror -std=c99
heap: heap.c ../roguelike.h
	gcc heap.c -lm -o heap -Wall -Wextra -pedantic -Werror -std=c99
bsp: bsp.c ../roguelike.h
	gcc bsp.c -lm -o bsp -Wall -Wextra -pedantic -Werror -std=c99
dijkstra: dijkstra.c ../roguelike.h
	gcc dijkstra.c -lm -o dijkstra -Wextra -pedantic -Werror -std=c99
path: path.c ../roguelike.h
	gcc path.c -lm -o path -Wextra -pedantic -Werror -std=c99

clean:
	rm bsp dijkstra heap floodfill path
