#define RL_IMPLEMENTATION
#define RL_ENABLE_PATHFINDING 0 /* pathfinding currently requires c99 */
#include "../roguelike.h"

#include <stdio.h>
#include <time.h>

#define WIDTH 80
#define HEIGHT 30

int main(void)
{
    int x, y;

    RL_Map *map = rl_map_create(WIDTH, HEIGHT);
    RL_MapgenConfigBSP config = RL_MAPGEN_BSP_DEFAULTS;
    config.max_splits = 3;
    config.draw_corridors = RL_ConnectSimple;

    srand(time(0));
    if (rl_mapgen_bsp(map, config) != RL_OK) {
        fprintf(stderr, "Error while generating map!\n");
        return 1;
    }

    for (y=0; y<HEIGHT; ++y) {
        for (x=0; x<WIDTH; ++x) {
            printf("%c", map->tiles[x+y*WIDTH]);
        }
        printf("\n");
    }

    return 0;
}
