#define RL_IMPLEMENTATION
#define RL_ENABLE_FOV 0
#define RL_ENABLE_PATHFINDING 0
#include "../roguelike.h"

#include <stdio.h>
#include <time.h>

#define WIDTH 80
#define HEIGHT 30

int main()
{
    int x, y;

    RL_Map *map = rl_map_create(WIDTH, HEIGHT);
    RL_MapgenConfigBSP config = RL_MAPGEN_BSP_DEFAULTS;
    config.draw_corridors = RL_ConnectSimple;

    srand(time(0));
    rl_mapgen_bsp(map, config);

    for (y=0; y<HEIGHT; ++y) {
        for (x=0; x<WIDTH; ++x) {
            printf("%c", map->tiles[x+y*WIDTH]);
        }
        printf("\n");
    }

    return 0;
}
