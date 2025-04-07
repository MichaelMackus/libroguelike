#include <stdio.h>

#define RL_IMPLEMENTATION
#include "../roguelike.h"

#define WIDTH 80
#define HEIGHT 30

int main()
{
    rl_rng_seed(time(0));
    RL_Map *map = rl_map_create(WIDTH, HEIGHT);
    RL_BSP *bsp = rl_mapgen_bsp(map, (RL_MapgenConfigBSP) { 3, 5, 3, 5, 1, 1, 1, 1 });

    for (int y = 0; y < HEIGHT; ++y) {
        for (int x = 0; x < WIDTH; ++x) {
            RL_Tile t = map->tiles[map->width*y + x];
            switch (t) {
                case RL_TileRock:
                    {
                        int wall = rl_map_room_wall(map, RL_XY(x, y));
                        if (wall & RL_WallToEast || wall & RL_WallToWest)
                            printf("-");
                        else if (wall & RL_WallToSouth || wall & RL_WallToNorth)
                            printf("|");
                        else if (wall)
                            printf("0");
                        else
                            printf(" ");
                        break;
                    }
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
