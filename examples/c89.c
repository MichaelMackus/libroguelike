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

    RL_Map map = rl_map_create(WIDTH, HEIGHT);
    RL_MapgenConfigBSP config = RL_MAPGEN_BSP_DEFAULTS;
    RL_BSP *bsp = rl_bsp_create(WIDTH, HEIGHT);
    RL_Status status;
    assert(bsp != NULL);
    config.max_splits = 3;
    config.draw_corridors = RL_ConnectBSP;

    srand(time(0));

    status = rl_mapgen_bsp_recursive_split(bsp, config.room_min_width + config.room_padding*2, config.room_min_height + config.room_padding*2, config.max_splits);
    assert(status == RL_OK);
    rl_mapgen_bsp_generate_rooms(bsp, map, config.room_min_width, config.room_max_width, config.room_min_height, config.room_max_height, config.room_padding);
    status = rl_mapgen_connect_corridors(map, bsp, config.draw_doors, config.draw_corridors);
    assert(status == RL_OK);

    for (y=0; y<HEIGHT; ++y) {
        for (x=0; x<WIDTH; ++x) {
            printf("%c", map.tiles[x+y*WIDTH]);
        }
        printf("\n");
    }

    return 0;
}
