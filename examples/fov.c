#define RL_IMPLEMENTATION

#include "../roguelike.h"
#include <stdio.h>
#include <time.h>
#include <curses.h>

#define WIDTH 80
#define HEIGHT 30

int main(int argc, char **argv)
{
    int x, y;
    unsigned long seed = time(0);
    if (argc > 1) {
        seed = atol(argv[1]);
    }
    rl_rng_seed(seed);
    RL_Map map = rl_map_create(WIDTH, HEIGHT);
    RL_BSP bsp = rl_mapgen_bsp(&map, (RL_MapgenConfigBSP) { 3, 5, 3, 5, 1, 1, 1, 1 });

    // initialize curses (for player movement)
    initscr();
    curs_set(0);
    noecho();
    keypad(stdscr, 1);
    start_color();
    use_default_colors();
    init_pair(1, COLOR_WHITE, -1);
    init_pair(2, -1, -1); // Pair 2: Green text on black background
    attron(COLOR_PAIR(1));

    RL_Point player = { 0 };
    while (!rl_map_tile_is(&map, player, RL_TileRoom)) {
        player.x = rl_rng_generate(0, WIDTH - 1);
        player.y = rl_rng_generate(0, HEIGHT - 1);
    }
    rl_map_fov_calculate(&map, player, 8, 0, rl_distance_euclidian);
    int quit = 0;
    while (!quit) {
        rl_map_fov_calculate(&map, player, 8, 1, rl_distance_euclidian);
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
        refresh();
        RL_Point new_player = player;
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
        }
        if (rl_map_is_passable(&map, new_player)) {
            player = new_player;
        }
    }
    endwin();

    printf("Seed: %ld\n", seed);

    rl_map_destroy(&map);
    rl_bsp_destroy(&bsp);

    return 0;
}
