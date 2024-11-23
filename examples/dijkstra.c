#define RL_IMPLEMENTATION
#include "../roguelike.h"

#include <stdio.h>
#include <time.h>

int main()
{
    rl_rng_seed(time(0));

    RL_Map map = rl_map_create(100, 40);
    printf("%d x %d\n", map.width, map.height);
    for (int x=0; x < map.width; x++) {
        for (int y=0; y < map.height; y++) {
            map.tiles[y*map.width + x] = 1;
        }
    }

    RL_Point start = RL_XY(rl_rng_generate(0, map.width - 1), rl_rng_generate(0, map.height - 1));
    RL_PathMap path_map = rl_pathmap_create(map, start, rl_distance_manhattan, rl_map_is_passable);
    printf("Start: %d,%d\n", start.x, start.y);

    for (int y=0; y < map.height; y++) {
        for (int x=0; x < map.width; x++) {
            const RL_PathNode *n;
            char sym = ' ';
            n = &path_map.nodes[y * map.width + x];
            /* if (!n->passable) { */
            /*     sym = '.'; */
            /*} else */if (n->distance == 0) {
                sym = '@';
            } else if (n->distance < 10) {
                sym = 48 + n->distance;
            } else if (n->distance < 36) {
                sym = 65 + (n->distance - 10);
            } else if (n->distance == DBL_MAX) {
                sym = '?';
            }
            printf("%c", sym);
        }
        printf("\n");
    }

    rl_map_destroy(map);
    rl_pathmap_destroy(path_map);
}
