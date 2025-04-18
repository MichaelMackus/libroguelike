#define RL_IMPLEMENTATION

#include "../roguelike.h"
#include <stdio.h>
#include <time.h>

#define WIDTH 80
#define HEIGHT 30

// buffer & rendering function to render a basic layout of the BSP tree
char bsp_map[WIDTH * HEIGHT];
void render_bsp(RL_BSP *node)
{
    if (node == NULL) {
        return;
    }
    if (rl_bsp_is_leaf(node)) {
        // draw L/R & U/D in the center of the leaf nodes
        if (node->parent->width != node->width) {
            // L/R
            unsigned int x = node->x + node->width/2;
            unsigned int y = node->y + node->height/2;
            if (node->parent->left == node)
                bsp_map[x + y*WIDTH] = 'L';
            else
                bsp_map[x + y*WIDTH] = 'R';
        } else {
            // U/D
            unsigned int x = node->x + node->width/2;
            unsigned int y = node->y + node->height/2;
            if (node->parent->left == node)
                bsp_map[x + y*WIDTH] = 'U';
            else
                bsp_map[x + y*WIDTH] = 'D';
        }
    } else {
        // draw lines dividing splits
        if (node->left->width != node->width) {
            unsigned int x = node->x + node->width/2;
            // draw vertical line
            for (unsigned int y=node->y; y<node->height + node->y; ++y) {
                if (bsp_map[x + y*WIDTH] == '\0')
                    bsp_map[x + y*WIDTH] = node->parent == NULL ? '#' : '*';
            }
        } else {
            unsigned int y = node->y + node->height/2;
            // draw horizontal line
            for (unsigned int x=node->x; x<node->width + node->x; ++x) {
                if (bsp_map[x + y*WIDTH] == '\0')
                    bsp_map[x + y*WIDTH] = node->parent == NULL ? '#' : '*';
            }
        }
        render_bsp(node->left);
        render_bsp(node->right);
    }
}

int main(int argc, char **argv)
{
    unsigned int x, y;
    unsigned long seed = time(0);
    if (argc > 1) {
        seed = atol(argv[1]);
    }
    printf("Seed: %ld\n", seed);
    srand(seed);
    RL_Map *map = rl_map_create(WIDTH, HEIGHT);
    RL_MapgenConfigBSP config = {
        .room_min_width = 4,
        .room_max_width = 8,
        .room_min_height = 4,
        .room_max_height = 8,
        .room_padding = 0,
        .draw_corridors = RL_ConnectBSP,
        .draw_doors = true,
        .max_splits = 3,
    };
    RL_BSP *bsp = rl_bsp_create(WIDTH, HEIGHT);
    if (rl_mapgen_bsp_ex(map, bsp, &config) != RL_OK) {
        fprintf(stderr, "Error while generating map!\n");
        return 1;
    }

    printf("Leaf count: %zu\n", rl_bsp_leaf_count(bsp));

    // render the layout of the BSP first for debugging
    render_bsp(bsp);
    for (y = 0; y < HEIGHT; ++y) {
        for (x = 0; x < WIDTH; ++x) {
            if (bsp_map[x + y*WIDTH])
                printf("%c", bsp_map[x + y*WIDTH]);
            else
                printf(" ");
        }
        printf("\n");
    }

    // render the map
    for (y = 0; y < HEIGHT; ++y) {
        for (x = 0; x < WIDTH; ++x) {
            RL_Tile t = map->tiles[map->width*y + x];
            switch (t) {
                case RL_TileRock:
                    ;
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
                case RL_TileRoom:
                    printf(".");
                    break;
                case RL_TileCorridor:
                    printf("#");
                    break;
                case RL_TileDoor:
                    printf("+");
                    break;
                default:
                    printf("?");
                    break;
            }
        }
        printf("\n");
    }

    rl_map_destroy(map);
    rl_bsp_destroy(bsp);

    return 0;
}
