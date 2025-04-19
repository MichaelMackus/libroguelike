#include <stdio.h>
#include <time.h>

#define RL_IMPLEMENTATION
#include "../roguelike.h"

int main(void)
{
    srand(time(0)); /* seed the RNG */
    RL_Map *map = rl_map_create(80, 25); /* generate tiles memory */
    if (rl_mapgen_bsp(map, RL_MAPGEN_BSP_DEFAULTS) != RL_OK) { /* generate random map */
        printf("Error occurred during mapgen!\n");
        return 1;
    }
    /* print map */
    for (unsigned int y=0; y<map->height; ++y) {
        for (unsigned int x=0; x<map->width; ++x) {
            printf("%c", map->tiles[x + y*map->width]);
        }
        printf("\n");
    }
    rl_map_destroy(map);

    return 0;
}
