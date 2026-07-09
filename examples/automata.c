#define RL_IMPLEMENTATION

#include "../roguelike.h"
#include <stdio.h>
#include <time.h>

#define WIDTH 80
#define HEIGHT 30

int main(int argc, char **argv)
{
    unsigned long seed = time(0);
    if (argc > 1) {
        seed = atol(argv[1]);
    }
    printf("Seed: %ld\n", seed);
    srand(seed);

    RL_Map map = rl_map_create(WIDTH, HEIGHT);
    if (rl_mapgen_automata(map, RL_MAPGEN_AUTOMATA_DEFAULTS) != RL_OK) {
        fprintf(stderr, "Error during mapgen\n");
        return 1;
    }

    for (unsigned int y=0; y<map.height; ++y) {
        for (unsigned int x=0; x<map.width; ++x) {
            RL_Tile t = map.tiles[x + y*map.width];
            if (t == RL_TileRoom || t == RL_TileCorridor)
                printf("%c", '.');
            else
                printf("%c", rl_map_is_wall(map, x, y) ? '*' : ' ');
        }
        printf("\n");
    }

    rl_map_destroy(map);

    return 0;
}
