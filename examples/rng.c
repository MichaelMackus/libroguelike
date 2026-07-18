#define RL_IMPLEMENTATION
#include "../roguelike.h"
#include <assert.h>
#include <time.h>

#define WIDTH 80
#define HEIGHT 20

RL_Byte random_points[WIDTH*HEIGHT];
RL_Map map;
RL_BSP *root;

bool generate_map()
{
    RL_Status ret;
    RL_MapgenConfigBSP config = RL_MAPGEN_BSP_DEFAULTS;
    root = rl_bsp_create(WIDTH, HEIGHT);
    assert(root != NULL);

    map = rl_map_create(WIDTH, HEIGHT);
    ret = rl_mapgen_bsp_recursive_split(root, config.room_max_width + config.room_padding*2, config.room_max_height + config.room_padding*2, config.max_splits);
    if (ret != RL_OK) return ret;
    ret = rl_mapgen_bsp_generate_rooms(root, map, config.room_min_width, config.room_max_width, config.room_min_height, config.room_max_height, config.room_padding);
    if (ret != RL_OK) return ret;
    ret = rl_mapgen_connect_corridors(map, root, config.draw_doors, config.draw_corridors);
    if (ret != RL_OK) return ret;

    return true;
}

int main()
{
    unsigned int i;

    srand(time(0));
    if (!generate_map()) return 1;

    // random corridor tiles
    for (i=0; i<20; i++) {
        unsigned int x, y;

        RL_Status r = rl_rng_map_point(map, RL_TileCorridor, &x, &y);
        if (r != RL_OK) {
            printf("Error %s from rl_rng_map_point\n", rl_status_str(r));
            return 1;
        }

        random_points[x + y*WIDTH] = '*';
    }

    // random passable tiles
    for (i=0; i<20; i++) {
        unsigned int x, y;

        RL_Status r = rl_rng_map_passable(map, &x, &y);
        if (r != RL_OK) {
            printf("Error %s from rl_rng_map_point\n", rl_status_str(r));
            return 1;
        }

        random_points[x + y*WIDTH] = '*';
    }

    // random middle of rooms
    for (i=0; i<10; i++) {
        unsigned int x, y;

        RL_Status r = rl_rng_map_room(map, root, &x, &y);
        if (r != RL_OK) {
            printf("Error %s from rl_rng_map_point\n", rl_status_str(r));
            return 1;
        }

        random_points[x + y*WIDTH] = '_';
    }

    {
        unsigned int x, y;
        for (y=0; y<map.height; y++) {
            for (x=0; x<map.width; x++) {
                if (random_points[x + y*WIDTH]) {
                    printf("%c", random_points[x + y*WIDTH]);
                } else {
                    RL_Tile t = map.tiles[x + y*WIDTH];
                    if (t != RL_TileRock && t != RL_TileDoor) printf(".");
                    else printf("%c", t);
                }
            }
            printf("\n");
        }
    }

    return 0;
}
