#define RL_IMPLEMENTATION

#define _DEFAULT_SOURCE
#include "../roguelike.h"
#include <stdio.h>
#include <time.h>
#include <curses.h>

#if defined(WIN32) || defined(_WIN32)
#include <windows.h>
#else
#include <unistd.h>
#define Sleep(x) usleep((x)*1000)
#endif

#define WIDTH 80
#define HEIGHT 30

/**
 * This file represents a very crude implementation of player movement around a
 * randomly generated map. It includes direction key & mouse movement, as well
 * as using our FOV algorithm.
 *
 * Needs to link with curses to work.
 */
int main(int argc, char **argv)
{
    int x, y;
    unsigned long seed = time(0);
    if (argc > 1) {
        seed = atol(argv[1]); // parse seed from CLI arg
    }
    rl_rng_seed(seed);

    // generate a random map with recursive BSP generation
    RL_Map map = rl_map_create(WIDTH, HEIGHT);
    RL_BSP bsp = rl_mapgen_bsp(&map, (RL_MapgenConfigBSP) { 3, 5, 3, 5, 1, 1, 1, 1 });

    // initialize curses (for player movement)
    initscr();
    curs_set(0);
    noecho();
    keypad(stdscr, 1);
    mousemask(ALL_MOUSE_EVENTS | REPORT_MOUSE_POSITION, NULL);
    mouseinterval(0);
    start_color();
    use_default_colors();
    init_pair(1, COLOR_WHITE, -1);
    init_pair(2, -1, -1); // Pair 2: Green text on black background
    attron(COLOR_PAIR(1));

    RL_Point player = { 0 }; // point the player is at
    RL_Path *player_path = NULL; // path for mouse movement
    // generate a random starting tile for player
    while (!rl_map_tile_is(&map, player, RL_TileRoom)) {
        player.x = rl_rng_generate(0, WIDTH - 1);
        player.y = rl_rng_generate(0, HEIGHT - 1);
    }
    int quit = 0;
    while (!quit) {
        // regenerate FOV
        rl_map_fov_calculate(&map, player, 8, 1, rl_distance_euclidian, rl_map_is_passable);
        // draw the map, only drawing previously seen tiles or tiles within the FOV
        for (y = 0; y < HEIGHT; ++y) {
            for (x = 0; x < WIDTH; ++x) {
                if (y == player.y && x == player.x) {
                    mvaddch(y, x, '@');
                } else if (rl_map_is_visible(&map, RL_XY(x, y)) || rl_map_is_seen(&map, RL_XY(x, y))) {
                    RL_Tile t = map.tiles[map.width*y + x];
                    char ch = ' ';
                    switch (t) {
                        case RL_TileRock:
                            ;
                            int wall = rl_map_room_wall(&map, RL_XY(x, y));
                            if (wall & RL_WallToEast || wall & RL_WallToWest)
                                ch = '-';
                            else if (wall & RL_WallToSouth || wall & RL_WallToNorth)
                                ch = '|';
                            else if (wall)
                                ch = '0';
                            else
                                ch = ' ';
                            break;
                        case RL_TileRoom:
                            ch = '.';
                            break;
                        case RL_TileCorridor:
                            ch = '#';
                            break;
                        case RL_TileDoor:
                            ch = '+';
                            break;
                    }
                    if (!rl_map_is_visible(&map, RL_XY(x, y))) {
                        // if not visible but previously seen, we draw in a muted grey
                        attroff(COLOR_PAIR(1));
                        attron(COLOR_PAIR(2));
                        mvaddch(y, x, ch);
                        attroff(COLOR_PAIR(2));
                        attron(COLOR_PAIR(1));
                    } else {
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
        if (!player_path) { // only handle input if we're not moving to a destination path
            switch (getch()) {
                case 'k':
                case KEY_UP:
                    new_player.y -= 1;
                    break;
                case 'j':
                case KEY_DOWN:
                    new_player.y += 1;
                    break;
                case 'h':
                case KEY_LEFT:
                    new_player.x -= 1;
                    break;
                case 'l':
                case KEY_RIGHT:
                    new_player.x += 1;
                    break;
                case 'q':
                    quit = 1;
                    break;
                case KEY_MOUSE:
                    {
                        MEVENT ev;
                        if (getmouse(&ev) == OK)
                        {
                            // if we have a mouse event & the destination is seen & passable, create a path to the destination
                            RL_Point dest = RL_XY(ev.x, ev.y);
                            if ((rl_map_is_seen(&map, dest) || rl_map_is_visible(&map, dest)) && rl_map_is_passable(&map, dest)) {
                                player_path = rl_path_create(&map, player, dest, rl_distance_chebyshev, rl_map_is_passable);
                                player_path = rl_path_walk(player_path); // skip first point
                            }
                        }
                        break;
                    }

            }
        } else { // update player to the next point towards the clicked destination
            new_player = player_path->point;
            player_path = rl_path_walk(player_path);
            Sleep(50); // so we can see the movement
        }
        // update the player position if the target is passable
        if (rl_map_is_passable(&map, new_player)) {
            player = new_player;
        }
    }

    // cleanup
    endwin();
    printf("Seed: %ld\n", seed); // pass seed as first argument to re-use RNG
    rl_map_destroy(&map);
    rl_bsp_destroy(&bsp);

    return 0;
}
