#define RL_IMPLEMENTATION

#define RL_HEX_ODD_OFFSET 1
#include "../roguelike.h"
#include <curses.h>
#include <stdio.h>
#include <time.h>

#define WIDTH 40
#define HEIGHT 25

typedef enum {
    AUTOMATA,
    BSP,
    MAZE,
} MapgenType;

RL_Byte tiles[WIDTH * HEIGHT] = {0};
RL_Map map = { .width = WIDTH, .height = HEIGHT, .tiles = (RL_Byte*) tiles };
MapgenType mapgen_type = AUTOMATA;
RL_Point player, new_player;

// generate new map depending on mapgen type selected
bool mapgen(void)
{
    RL_Status status;
    switch (mapgen_type) {
        case AUTOMATA:
            status = rl_mapgen_automata(map, RL_MAPGEN_AUTOMATA_DEFAULTS);
            break;
        case BSP:
            status = rl_mapgen_bsp(map, RL_MAPGEN_BSP_DEFAULTS);
            break;
        case MAZE:
            status = rl_mapgen_maze(map);
            break;
    }

    if (status != RL_OK) {
        fprintf(stderr, "Error during mapgen\n");
        return false;
    }

    // generate a random starting tile for player
    int player_x = 0, player_y = 0;
    while (!rl_map_tile_is(map, player_x, player_y, RL_TileRoom)) {
        player_x = rl_rng_generate(0, WIDTH - 1);
        player_y = rl_rng_generate(0, HEIGHT - 1);
    }
    player = rl_point_axial(player_x, player_y);
    new_player = player;

    return true;
}

int main(int argc, char **argv)
{
    unsigned long seed = time(0);
    if (argc > 1) {
        seed = atol(argv[1]);
    }
    srand(seed);

    if (!mapgen()) return 1;

    // initialize curses (for player movement)
    initscr();
    curs_set(0);
    noecho();
    keypad(stdscr, 1);
    mousemask(ALL_MOUSE_EVENTS | REPORT_MOUSE_POSITION, NULL);
    mouseinterval(0);
    start_color();
    if (can_change_color()) {
        init_pair(1, 255, COLOR_BLACK);
        init_pair(2, 240, COLOR_BLACK);
    } else {
        init_pair(1, COLOR_WHITE, COLOR_BLACK);
        init_pair(2, COLOR_WHITE, COLOR_BLACK);
    }

    bool hex_display = true;
    bool outline_display = false;
    int camera_x = 0, camera_y = 0, input = 0;
    do {
        // handle input
        switch (input) {
            case 'r':
                // generate new map
                if (!mapgen()) return 1;
                break;
            case 'b':
                // set BSP mapgen type
                mapgen_type = BSP;
                mapgen();
                break;
            case 'c':
                // set BSP mapgen type
                mapgen_type = AUTOMATA;
                mapgen();
                break;
            case 'm':
                // set BSP mapgen type
                mapgen_type = MAZE;
                mapgen();
                break;
            case 'h':
                // toggle hex rendering mode
                hex_display = !hex_display;
                break;
            case 'o':
                // toggle outline rendering
                outline_display = !outline_display;
                break;
            // WASD = camera control
            case 'w':
                camera_y -= 1;
                break;
            case 'a':
                camera_x -= 1;
                break;
            case 's':
                camera_y += 1;
                break;
            case 'd':
                camera_x += 1;
                break;
            // player movement (in axial coordinates) - numpad
            case KEY_LEFT:
            case '4':
                new_player.x -= 1;
                break;
            case KEY_RIGHT:
            case '6':
                new_player.x += 1;
                break;
            case KEY_A1:
            case '7':
                new_player.y -= 1;
                break;
            case KEY_A3:
            case '9':
                new_player.x += 1;
                new_player.y -= 1;
                break;
            case KEY_C1:
            case '1':
                new_player.x -= 1;
                new_player.y += 1;
                break;
            case KEY_C3:
            case '3':
                new_player.y += 1;
                break;
        }
        // update the player position if the target is passable
        if (rl_map_is_passable(map, rl_axial_to_map_x(new_player), rl_axial_to_map_y(new_player))) {
            player = new_player;
        } else {
            new_player = player;
        }
        // display map
        clear();
        int player_col = rl_axial_to_map_x(player);
        int player_row = rl_axial_to_map_y(player);
        for (int y=camera_y; y<HEIGHT; y++) {
            for (int x=camera_x; x<WIDTH; x++) {
                if (x < 0 || y < 0 || x >= WIDTH || y >= WIDTH) continue;
                int disp_x = x - camera_x, disp_y = y - camera_y;
                if (hex_display) {
                    disp_x *= 4;
                    disp_y *= 2;
                    if (y % 2 != 0)
                        disp_x += 2;
                    // display hex outline
                    if (outline_display) {
                        attron(COLOR_PAIR(2));
                        mvaddch(disp_y, disp_x - 1, '|');
                        mvaddch(disp_y, disp_x + 1, '|');
                        mvaddch(disp_y + 1, disp_x - 1, '\\');
                        mvaddch(disp_y + 1, disp_x, 'v');
                        mvaddch(disp_y + 1, disp_x + 1, '/');
                        mvaddch(disp_y - 1, disp_x, '^');
                        attroff(COLOR_PAIR(2));
                    }
                }
                char ch = map.tiles[x + y*WIDTH];
                attron(A_BOLD);
                attron(COLOR_PAIR(1));
                if (x == player_col && y == player_row)
                    mvaddch(disp_y, disp_x, '@');
                else
                    mvaddch(disp_y, disp_x, ch);
                attroff(COLOR_PAIR(1));
                attroff(A_BOLD);
            }
        }
        refresh();
    } while ((input = getch()) && input != 'q');

    endwin();
    printf("Seed: %ld\n", seed);

    return 0;
}
