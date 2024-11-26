#define RL_IMPLEMENTATION

#include "../roguelike.h"
#include <stdio.h>
#include <time.h>

int WIDTH = 80;
int HEIGHT = 30;

int main()
{
    rl_rng_seed(time(0));

    RL_Map map = rl_map_create(WIDTH, HEIGHT);
    RL_BSP *bsp = rl_mapgen_bsp(&map, (RL_MapgenConfigBSP) { 3, 5, 3, 5, 1, 1, 1, 1 });

    int x, y;
    RL_PathMap floodfill = rl_map_largest_connected_area(&map);
    for (y = 0; y < HEIGHT; ++y) {
        for (x = 0; x < WIDTH; ++x) {
            RL_Tile t = map.tiles[map.width*y + x];
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
            if (floodfill.nodes[x + y*WIDTH].distance < DBL_MAX) {
                printf("*");
            } else {
                printf(" ");
            }
        }
        printf("\n");
    }

    rl_map_destroy(map);
    rl_bsp_destroy(bsp);
    rl_pathmap_destroy(floodfill);

    printf("Done\n");

    return 0;
}
