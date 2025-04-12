#include <stdio.h>
#include <stdbool.h>

#include "curses.h"

#define RL_IMPLEMENTATION
#include "../roguelike.h"

#define WIDTH 80
#define HEIGHT 20
#define FOV_RADIUS 8

char map_str[WIDTH * HEIGHT] = "--------------------------------------------------------------------------------"
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

RL_TileSize visibility[WIDTH * HEIGHT];

int main()
{
    RL_Map map = { .width = WIDTH, .height = HEIGHT, .tiles = (unsigned char*) map_str };
    RL_FOV fov = { .width = WIDTH, .height = HEIGHT, .visibility = (unsigned char*) visibility };
    RL_Point player_loc = RL_XY(17, 7);

    // initialize curses
    initscr();
    noecho();
    keypad(stdscr, true);
    curs_set(0);

    // game loop
    bool quit = 0;
    while (!quit) {
        // calculate FOV
        rl_fov_calculate(&fov, &map, player_loc, FOV_RADIUS, rl_distance_manhattan);
        // draw map
        move(0, 0);
        for (int y=0; y < HEIGHT; ++y) {
            for (int x=0; x < WIDTH; ++x) {
                if (player_loc.x == x && player_loc.y == y) {
                    addch('@');
                } else if (rl_fov_is_visible(&fov, RL_XY(x, y))) {
                    /* if (rl_map_tile_is(&map, RL_XY(x, y), RL_TileRock)) */
                    /*     addch('#'); */
                    /* else if (rl_map_tile_is(&map, RL_XY(x, y), RL_TileCorridor)) */
                    /*     addch('.'); */
                    /* else */
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
        RL_Point new_loc = player_loc;
        switch (ch) {
            case 'q':
                quit = true;
                break;
            case 'h':
            case KEY_LEFT:
                new_loc.x -= 1;
                break;
            case 'j':
            case KEY_DOWN:
                new_loc.y += 1;
                break;
            case 'k':
            case KEY_UP:
                new_loc.y -= 1;
                break;
            case 'l':
            case KEY_RIGHT:
                new_loc.x += 1;
                break;
        }
        if (rl_map_tile_is(&map, new_loc, '>') || ch == '>') {
            // generate a new map
            rl_mapgen_bsp(&map, (RL_MapgenConfigBSP) { 3, 5, 3, 5, 1, 1, 1, 1, 1 });
            // find passable tile for player
            player_loc.x = player_loc.y = -1;
            for (int y=0; y < HEIGHT; ++y) {
                for (int x=0; x < WIDTH; ++x) {
                    if (rl_map_is_passable(&map, RL_XY(x, y))) {
                        player_loc = RL_XY(x, y);
                        break;
                    }
                }
                if (player_loc.x > -1 && player_loc.y > -1) break;
            }
        } else if (rl_map_is_passable(&map, new_loc)) {
            player_loc = new_loc;
        }
    }

    // clean-up
    endwin();

    return 0;
}
