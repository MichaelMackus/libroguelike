#define RL_IMPLEMENTATION

#include "../roguelike.h"
#include <stdio.h>
#include <time.h>
#include <locale.h>

#define WIDTH 80
#define HEIGHT 30

int main(int argc, const char **argv)
{
    unsigned long seed = time(0);

    setlocale(LC_ALL, "");
    if (argc > 1) {
        seed = atol(argv[1]);
    }
    printf("Seed: %ld\n", seed);
    srand(seed);
    srand(seed);

    RL_Map *map = rl_map_create(WIDTH, HEIGHT);
    if (rl_mapgen_maze(map) != RL_OK) {
        fprintf(stderr, "Error during mapgen\n");
        return 1;
    }

    /* floodfill to ensure it is a proper maze */
    RL_Graph *floodfill = rl_graph_floodfill_largest_area(map);
    for (unsigned int y = 0; y < HEIGHT; ++y) {
        for (unsigned int x = 0; x < WIDTH; ++x) {
            if (floodfill->nodes[x + y*WIDTH].score < FLT_MAX) {
                printf("*");
            } else {
                printf(" ");
            }
        }
        printf("\n");
    }
    for (unsigned int x = 0; x < map->width; ++x) {
        for (unsigned int y = 0; y < map->height; ++y) {
            if (!rl_map_tile_is(map, x, y, RL_TileRock) && floodfill->nodes[x + y*map->width].score == FLT_MAX) {
                assert("ERROR: Unreachable tile found!" == 0);
            }
        }
    }
    rl_graph_destroy(floodfill);

    /* print the maze */
    for (unsigned int y=0; y<map->height; ++y) {
        for (unsigned int x=0; x<map->width; ++x) {
            RL_Tile t = map->tiles[x + y*map->width];
            if (t == RL_TileRock) {
                wchar_t ch;
                long connections = rl_map_wall(map, x, y);
                if (connections & RL_WallToEast && connections & RL_WallToWest && connections & RL_WallToNorth && connections & RL_WallToSouth)
                    ch = L'┼';
                else if (connections & RL_WallToEast && connections & RL_WallToWest && connections & RL_WallToSouth)
                    ch = L'┬';
                else if (connections & RL_WallToEast && connections & RL_WallToWest && connections & RL_WallToNorth)
                    ch = L'┴';
                else if (connections & RL_WallToNorth && connections & RL_WallToSouth && connections & RL_WallToWest)
                    ch = L'┤';
                else if (connections & RL_WallToNorth && connections & RL_WallToSouth && connections & RL_WallToEast)
                    ch = L'├';
                else if (connections & RL_WallToSouth && connections & RL_WallToEast)
                    ch = L'┌';
                else if (connections & RL_WallToNorth && connections & RL_WallToEast)
                    ch = L'└';
                else if (connections & RL_WallToNorth && connections & RL_WallToWest)
                    ch = L'┘';
                else if (connections & RL_WallToSouth && connections & RL_WallToWest)
                    ch = L'┐';
                else if (connections & RL_WallToEast || connections & RL_WallToWest)
                    ch = L'─';
                else if (connections & RL_WallToNorth || connections & RL_WallToSouth)
                    ch = L'│';
                else if (connections)
                    ch = '0';
                else
                    ch = ' ';
                printf("%lc", ch);
            } else
                printf("%c", '.');
        }
        printf("\n");
    }

    rl_map_destroy(map);

    return 0;
}
