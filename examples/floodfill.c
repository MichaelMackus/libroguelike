#define RL_IMPLEMENTATION

#include "../roguelike.h"
#include <stdio.h>
#include <time.h>

#define WIDTH 80
#define HEIGHT 30

int main(int argc, char **argv)
{
    int x, y;
    unsigned long seed = time(0);
    if (argc > 1) {
        seed = atol(argv[1]);
    }
    printf("Seed: %ld\n", seed);
    srand(seed);

    RL_Map *map = rl_map_create(WIDTH, HEIGHT);
    rl_mapgen_bsp(map, (RL_MapgenConfigBSP) { 3, 5, 3, 5, 1, 1, 1, 1 });

    RL_Graph *floodfill = rl_map_largest_connected_area(map);
    for (y = 0; y < HEIGHT; ++y) {
        for (x = 0; x < WIDTH; ++x) {
            RL_Tile t = map->tiles[map->width*y + x];
            switch (t) {
                case RL_TileRock:
                    printf(" ");
                    break;
                case RL_TileRoom:
                    printf(".");
                    break;
                case RL_TileCorridor:
                    printf("#");
                    break;
                case RL_TileDoor:
                    printf("+");
                    break;
            }
        }
        printf("\n");
    }
    for (y = 0; y < HEIGHT; ++y) {
        for (x = 0; x < WIDTH; ++x) {
            if (floodfill->nodes[x + y*WIDTH].score < DBL_MAX) {
                printf("*");
            } else {
                printf(" ");
            }
        }
        printf("\n");
    }

    // check to ensure map tiles are all reachable
    for (unsigned int x = 0; x < map->width; ++x) {
        for (unsigned int y = 0; y < map->height; ++y) {
            if (!rl_map_tile_is(map, RL_XY(x, y), RL_TileRock) && floodfill->nodes[x + y*map->width].score == DBL_MAX) {
                assert("ERROR: Unreachable tile found!" == 0);
            }
        }
    }

    rl_map_destroy(map);
    rl_graph_destroy(floodfill);

    printf("Done\n");

    return 0;
}
