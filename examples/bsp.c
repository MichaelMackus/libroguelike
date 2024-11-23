#define RL_IMPLEMENTATION

#include "../roguelike.h"
#include <stdio.h>
#include <time.h>

#define WIDTH 80
#define HEIGHT 25

int main()
{
    rl_rng_seed(time(0));
    RL_Map map = rl_map_create(WIDTH, HEIGHT);
    RL_BSP *bsp = rl_mapgen_bsp(&map, (RL_MapgenConfigBSP) { 3, 5, 3, 5, 1, 1, 1, 0 });

    int x, y;
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

    rl_map_destroy(map);
    rl_bsp_destroy(bsp);

    return 0;
}
