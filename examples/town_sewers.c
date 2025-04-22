#include <stdio.h>
#include <stdbool.h>
#include <time.h>

#include "curses.h"

#define WIDTH 80
#define HEIGHT 20
#define FOV_RADIUS 8

#define RL_IMPLEMENTATION
#include "../roguelike.h"

// map filled with empty space (the corridor tile is used so map generation properly places doors in town rooms; otherwise, if everything is floor tile, the corridor generation cannot find the rooms to connect)
unsigned char map_str[WIDTH * HEIGHT] = "################################################################################"
                                        "################################################################################"
                                        "################################################################################"
                                        "################################################################################"
                                        "################################################################################"
                                        "################################################################################"
                                        "################################################################################"
                                        "################################################################################"
                                        "################################################################################"
                                        "################################################################################"
                                        "################################################################################"
                                        "################################################################################"
                                        "################################################################################"
                                        "################################################################################"
                                        "################################################################################"
                                        "################################################################################"
                                        "################################################################################"
                                        "################################################################################"
                                        "################################################################################"
                                        "################################################################################";

RL_Byte visibility[WIDTH * HEIGHT];

int main(void)
{
    srand(time(0));
    RL_Map map = { .width = WIDTH, .height = HEIGHT, .tiles = map_str };
    RL_FOV fov = { .width = WIDTH, .height = HEIGHT, .visibility =  visibility };

    // initialize curses
    initscr();
    noecho();
    keypad(stdscr, true);
    curs_set(0);

    // generate town map
    RL_MapgenConfigBSP config = RL_MAPGEN_BSP_DEFAULTS;
    RL_BSP bsp = { .width = WIDTH, .height = HEIGHT };
    rl_mapgen_bsp_ex(&map, &bsp, &config); // ex is used since rl_mapgen will reset our tiles to RL_TileRock
    // find random corridor tile to place downstair & player
    unsigned int downstair_x = WIDTH, downstair_y = HEIGHT;
    while (!(rl_map_tile_is(&map, downstair_x, downstair_y, RL_TileCorridor))) {
        downstair_x = rl_rng_generate(0, WIDTH - 1);
        downstair_y = rl_rng_generate(0, HEIGHT - 1);
    }
    unsigned int player_x = WIDTH, player_y = HEIGHT;
    while (!(rl_map_tile_is(&map, player_x, player_y, RL_TileCorridor))) {
        player_x = rl_rng_generate(0, WIDTH - 1);
        player_y = rl_rng_generate(0, HEIGHT - 1);
    }

    // game loop
    bool quit = 0;
    while (!quit) {
        // calculate FOV
        rl_fov_calculate(&fov, &map, player_x, player_y, -1);
        // draw map
        move(0, 0);
        for (unsigned int y=0; y < HEIGHT; ++y) {
            for (unsigned int x=0; x < WIDTH; ++x) {
                if (rl_fov_is_visible(&fov, x, y)) {
                    RL_Byte t = map.tiles[y*WIDTH + x];
                    if (x == downstair_x && y == downstair_y) t = '>';
                    if (x == player_x && y == player_y) t = '@';
                    switch (t) {
                        case RL_TileCorridor:
                            addch('.');
                            break;
                        case RL_TileRock:
                            addch('#');
                            break;
                        case RL_TileDoor:
                        case RL_TileDoorOpen:
                        case RL_TileRoom:
                        default:
                            addch(t);
                    }
                } else {
                    addch(' ');
                }
            }
            addch('\n');
        }
        refresh();
        // handle user input
        int ch = getch();
        unsigned int new_x = player_x, new_y = player_y;
        switch (ch) {
            case 'q':
                quit = true;
                break;
            case 'h':
            case KEY_LEFT:
                new_x -= 1;
                break;
            case 'j':
            case KEY_DOWN:
                new_y += 1;
                break;
            case 'k':
            case KEY_UP:
                new_y -= 1;
                break;
            case 'l':
            case KEY_RIGHT:
                new_x += 1;
                break;
        }
        if ((new_x == downstair_x && new_y == downstair_y) || ch == '>') {
            // generate a new map - for this one, we generate a maze first then BSP rooms
            if (rl_mapgen_maze(&map) != RL_OK) {
                fprintf(stderr, "Error while generating map!\n");
                return 1;
            }
            rl_bsp_destroy(bsp.left);
            rl_bsp_destroy(bsp.right);
            bsp.left = bsp.right = NULL;
            if (rl_mapgen_bsp_ex(&map, &bsp, &config) != RL_OK) {
                fprintf(stderr, "Error while generating map!\n");
                return 1;
            }
            // find passable tile for player & downstair
            player_x = player_y = downstair_x = downstair_y -1;
            while (!(rl_map_tile_is(&map, downstair_x, downstair_y, RL_TileRoom))) {
                downstair_x = rl_rng_generate(0, WIDTH - 1);
                downstair_y = rl_rng_generate(0, HEIGHT - 1);
            }
            while (!(rl_map_tile_is(&map, player_x, player_y, RL_TileRoom))) {
                player_x = rl_rng_generate(0, WIDTH - 1);
                player_y = rl_rng_generate(0, HEIGHT - 1);
            }
            assert(rl_map_is_passable(&map, player_x, player_y));
        } else if (rl_map_is_passable(&map, new_x, new_y)) {
            player_x = new_x;
            player_y = new_y;
        }
    }

    if (bsp.left) rl_bsp_destroy(bsp.left);
    if (bsp.right) rl_bsp_destroy(bsp.right);

    // clean-up
    endwin();

    return 0;
}
