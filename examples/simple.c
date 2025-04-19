#include <stdio.h>
#include <stdbool.h>

#include "curses.h"

#define RL_IMPLEMENTATION
#include "../roguelike.h"

#define WIDTH 80
#define HEIGHT 20
#define FOV_RADIUS 8

unsigned char map_str[WIDTH * HEIGHT] = "--------------------------------------------------------------------------------"
                                        "|                                                                              |"
                                        "|                                                                              |"
                                        "|                                                                              |"
                                        "|                                                                              |"
                                        "|          -------------             -------                                   |"
                                        "|          |.....>.....|             |.....|                                   |"
                                        "|##########+...........+#############+.....|                                   |"
                                        "|          |...........|             |.....|                                   |"
                                        "|          -------------             -------                                   |"
                                        "|                                                                              |"
                                        "|                                                                              |"
                                        "|                                                                              |"
                                        "|                                                                              |"
                                        "|                                                                              |"
                                        "|                                                                              |"
                                        "|                                                                              |"
                                        "|                                                                              |"
                                        "|                                                                              |"
                                        "--------------------------------------------------------------------------------";

RL_Byte visibility[WIDTH * HEIGHT];

int main(void)
{
    RL_Map map = { .width = WIDTH, .height = HEIGHT, .tiles =  map_str };
    RL_FOV fov = { .width = WIDTH, .height = HEIGHT, .visibility =  visibility };
    unsigned int player_x = 17, player_y = 7;

    // initialize curses
    initscr();
    noecho();
    keypad(stdscr, true);
    curs_set(0);

    // game loop
    bool quit = 0;
    while (!quit) {
        // calculate FOV
        rl_fov_calculate(&fov, &map, player_x, player_y, FOV_RADIUS);
        // draw map
        move(0, 0);
        for (unsigned int y=0; y < HEIGHT; ++y) {
            for (unsigned int x=0; x < WIDTH; ++x) {
                if (player_x == x && player_y == y) {
                    addch('@');
                } else if (rl_fov_is_visible(&fov, x, y)) {
                    addch(map.tiles[y*WIDTH + x]);
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
        if (rl_map_tile_is(&map, new_x, new_y, '>') || ch == '>') {
            // generate a new map
            RL_MapgenConfigBSP config = RL_MAPGEN_BSP_DEFAULTS;
            config.draw_corridors = RL_ConnectSimple;
            if (rl_mapgen_bsp(&map, config) != RL_OK) {
                fprintf(stderr, "Error while generating map!\n");
                return 1;
            }
            // find passable tile for player
            player_x = player_y = -1;
            for (int y=0; y < HEIGHT; ++y) {
                for (int x=0; x < WIDTH; ++x) {
                    if (rl_map_is_passable(&map, x, y)) {
                        player_x = x;
                        player_y = y;
                        break;
                    }
                }
                if (rl_map_is_passable(&map, player_x, player_y)) {
                    break;
                }
            }
            assert(rl_map_is_passable(&map, player_x, player_y));
        } else if (rl_map_is_passable(&map, new_x, new_y)) {
            player_x = new_x;
            player_y = new_y;
        }
    }

    // clean-up
    endwin();

    return 0;
}
