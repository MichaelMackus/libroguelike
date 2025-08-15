#define RL_IMPLEMENTATION

/**
 * Example with custom tiles "." is floor "#" is wall and "+" is door
 */

#define RL_IS_PASSABLE(t, x, y) t == '.' || t == '+' || t == '>'
#define RL_IS_OPAQUE(t, x, y)   t == '#' || t == '+' || t == ' '

#include "../roguelike.h"
#include <stdio.h>
#include <time.h>
#include <curses.h>

#define WIDTH 45
#define HEIGHT 15

// buffer & rendering function to render a basic layout of the BSP tree
RL_Byte bsp_map[WIDTH * HEIGHT] = "                                             "
                                  "                                             "
                                  "                        #                    "
                                  "           ##########  #.#                   "
                                  "           #........# #...#                  "
                                  "           #........##.....#                 "
                                  "           #....>...+.......#                "
                                  "           #........##.....#                 "
                                  "           #........# #...#                  "
                                  "           ##########  #.#                   "
                                  "                        #                    "
                                  "                                             "
                                  "                                             "
                                  "                                             "
                                  "                                             ";

int main(void)
{
    unsigned int x, y;
    RL_Map map = { .width = WIDTH, .height = HEIGHT, .tiles = bsp_map };
    RL_FOV fov = { .width = WIDTH, .height = HEIGHT, .visibility = (RL_Byte[WIDTH * HEIGHT]) {0} };
    RL_Point player = { 24, 6 };

    // initialize curses
    initscr();
    curs_set(0);
    noecho();
    keypad(stdscr, 1);
    mousemask(ALL_MOUSE_EVENTS | REPORT_MOUSE_POSITION, NULL);
    mouseinterval(0);

    // game loop
    int quit = 0;
    while (!quit) {
        // regenerate FOV
        rl_fov_calculate(&fov, &map, player.x, player.y, 16);

        // draw the map, only drawing previously seen tiles or tiles within the FOV
        for (y = 0; y < map.height; ++y) {
            for (x = 0; x < map.width; ++x) {
                if (rl_fov_is_visible(&fov, x, y) || rl_fov_is_seen(&fov, x, y)) {
                    RL_Tile t = map.tiles[map.width*y + x];
                    char ch = t;
                    // handle player & downstair tiles
                    if (y == player.y && x == player.x) {
                        ch = '@';
                    }
                    if (!rl_fov_is_visible(&fov, x, y)) {
                        // if not visible but previously seen, we draw in a muted grey
                        attroff(A_BOLD);
                        mvaddch(y, x, ch);
                    } else {
                        attron(A_BOLD);
                        mvaddch(y, x, ch);
                    }
                } else {
                    mvaddch(y, x, ' ');
                }
            }
        }
        // refresh the window (draw to the screen)
        refresh();

        // handle user input
        RL_Point new_player = player;
        int ch = getch();
        switch (ch) {
            case 'k':
            case KEY_UP:
            case '8':
                new_player.y -= 1;
                break;
            case 'j':
            case KEY_DOWN:
            case '2':
                new_player.y += 1;
                break;
            case 'h':
            case KEY_LEFT:
            case '4':
                new_player.x -= 1;
                break;
            case 'l':
            case KEY_RIGHT:
            case '6':
                new_player.x += 1;
                break;
            case 'y':
            case KEY_A1:
            case '7':
                new_player.x -= 1;
                new_player.y -= 1;
                break;
            case 'u':
            case KEY_A3:
            case '9':
                new_player.x += 1;
                new_player.y -= 1;
                break;
            case 'b':
            case KEY_C1:
            case '1':
                new_player.x -= 1;
                new_player.y += 1;
                break;
            case 'n':
            case KEY_C3:
            case '3':
                new_player.x += 1;
                new_player.y += 1;
                break;
            case 'q':
                quit = 1;
                break;
        }
        // update the player position if the target is passable
        if (rl_map_is_passable(&map, new_player.x, new_player.y)) {
            player = new_player;
        }
    }

    endwin();

    return 0;
}
