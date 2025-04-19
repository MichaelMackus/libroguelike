#define RL_IMPLEMENTATION
#include "../roguelike.h"

#include <stdio.h>
#include <time.h>

int main(void)
{
    srand(time(0));

    RL_Map *map = rl_map_create(100, 40);
    printf("%d x %d\n", map->width, map->height);
    for (unsigned int x=0; x < map->width; x++) {
        for (unsigned int y=0; y < map->height; y++) {
            if (rl_rng_generate(1, 10) <= 7)
                map->tiles[y*map->width + x] = RL_TileRoom;
            else
                map->tiles[y*map->width + x] = RL_TileRock;
        }
    }

    RL_Point start = RL_XY(rl_rng_generate(0, map->width - 1), rl_rng_generate(0, map->height - 1));
    RL_Graph *path_map = rl_dijkstra_create(map, start, rl_distance_manhattan, rl_map_is_passable);
    printf("Start: %f,%f\n", start.x, start.y);

    for (unsigned int y=0; y < map->height; y++) {
        for (unsigned int x=0; x < map->width; x++) {
            const RL_GraphNode *n;
            char sym = ' ';
            n = &path_map->nodes[y * map->width + x];
            /* if (!n->passable) { */
            /*     sym = '.'; */
            /*} else */if (n->score == 0) {
                sym = '@';
            } else if (n->score < 10) {
                sym = 48 + n->score;
            } else if (n->score < 36) {
                sym = 65 + (n->score - 10);
            } else if (n->score == FLT_MAX) {
                sym = '#';
            }
            printf("%c", sym);
        }
        printf("\n");
    }

    rl_map_destroy(map);
    rl_graph_destroy(path_map);
}
