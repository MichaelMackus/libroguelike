#include <stdio.h>
#include <time.h>

#define RL_IMPLEMENTATION
#include "../roguelike.h"

#define WIDTH 80
#define HEIGHT 30

int main()
{
    srand(time(0));
    RL_Map *map = rl_map_create(WIDTH, HEIGHT);
    if (rl_mapgen_bsp(map, RL_MAPGEN_BSP_DEFAULTS) != RL_OK) {
        fprintf(stderr, "Error while generating map!\n");
        return 1;
    }

    for (int y = 0; y < HEIGHT; ++y) {
        for (int x = 0; x < WIDTH; ++x) {
            RL_Tile t = (RL_Tile) map->tiles[map->width*y + x];
            switch (t) {
                case RL_TileRock:
                    {
                        int wall = rl_map_room_wall(map, x, y);
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
                case RL_TileDoorOpen:
                    printf("+");
                    break;
            }
        }
        printf("\n");
    }

    rl_map_destroy(map);

    return 0;
}
