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

    // check all tiles reachable with random corridor generation
    RL_MapgenConfigBSP config = {
        .room_min_width = 3,
        .room_max_width = 5,
        .room_min_height = 3,
        .room_max_height = 5,
        .room_padding = 0,
        .draw_corridors = RL_ConnectRandomly,
        .draw_doors = true,
    };
    rl_mapgen_bsp(map, config);
    RL_Graph *floodfill = rl_map_largest_connected_area(map);
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

    // check all tiles reachable with sequential leaf corridor generation
    rl_graph_destroy(floodfill);
    config.draw_corridors = RL_ConnectBSP;
    rl_mapgen_bsp(map, config);
    floodfill = rl_map_largest_connected_area(map);
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
