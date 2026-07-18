#define RL_IMPLEMENTATION
#include "../roguelike.h"
#include <assert.h>
#include <time.h>

#define WIDTH 80
#define HEIGHT 20

RL_Byte random_points[WIDTH*HEIGHT];
RL_Map map;
RL_BSP *root;

int main(void)
{
    unsigned int i;

    srand(time(0));
    map = rl_map_create(WIDTH, HEIGHT);
    root = rl_bsp_create(WIDTH, HEIGHT);
    if (rl_mapgen_bsp_ex(map, root, RL_MAPGEN_BSP_DEFAULTS) != RL_OK) {
        fprintf(stderr, "Error during mapgen\n");
        return 1;
    }

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

    rl_map_destroy(map);
    rl_bsp_destroy(root);

    return 0;
}
