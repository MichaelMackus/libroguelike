#define RL_IMPLEMENTATION

#include "../roguelike.h"
#include <stdio.h>
#include <time.h>

#define WIDTH 80
#define HEIGHT 30

char bsp_map[WIDTH * HEIGHT];
int leaf_count;
void render_bsp(RL_BSP *node)
{
    if (node == NULL) {
        return;
    }
    // draw
    render_bsp(node->left);
    render_bsp(node->right);
    for (int y=node->y; y<node->height + node->y; ++y) {
        for (int x=node->x; x<node->width + node->x; ++x) {
            if (node->left && node->left->left && !node->left->left->left) {
                rl_assert(node->parent);
                if (x == node->x || y == node->y || x == node->x + node->width - 1 || y == node->y + node->height - 1) {
                    bsp_map[x + y*WIDTH] = '#';
                }
                if (x == node->x + node->width/2 && y == node->y + node->height/2) {
                    if (node->parent && node->parent->left == node) {
                        bsp_map[x + y*WIDTH] = 'L';
                    } else {
                        bsp_map[x + y*WIDTH] = 'R';
                    }
                }
            }
        }
    }
}

int main(int argc, char **argv)
{
    int x, y;
    unsigned long seed = time(0);
    if (argc > 1) {
        seed = atol(argv[1]);
    }
    printf("Seed: %ld\n", seed);
    rl_rng_seed(seed);
    RL_Map map = rl_map_create(WIDTH, HEIGHT);
    RL_BSP bsp = rl_mapgen_bsp(&map, (RL_MapgenConfigBSP) { 3, 5, 3, 5, 1, 1, 1, 1 });

    render_bsp(&bsp);
    for (y = 0; y < HEIGHT; ++y) {
        for (x = 0; x < WIDTH; ++x) {
            if (bsp_map[x + y*WIDTH])
                printf("%c", bsp_map[x + y*WIDTH]);
            else
                printf(" ");
        }
        printf("\n");
    }

    for (y = 0; y < HEIGHT; ++y) {
        for (x = 0; x < WIDTH; ++x) {
            RL_Tile t = map.tiles[map.width*y + x];
            switch (t) {
                case RL_TileRock:
                    ;
                    int wall = rl_map_room_wall(&map, RL_XY(x, y));
                    if (wall & RL_WallToEast || wall & RL_WallToWest)
                        printf("-");
                    else if (wall & RL_WallToSouth || wall & RL_WallToNorth)
                        printf("|");
                    else if (wall)
                        printf("0");
                    else
                        printf(" ");
                    break;
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

    rl_map_destroy(&map);
    rl_bsp_destroy(&bsp);

    return 0;
}
