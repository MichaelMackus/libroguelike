#define RL_IMPLEMENTATION
#include "../roguelike.h"

#include <stdio.h>
#include <time.h>

#define MAP_WIDTH 80
#define MAP_HEIGHT 20

void print_at(int x, int y, char c)
{
   printf("\033[%d;%dH%c", y, x, c);
}

int main(int argc, char **argv)
{
    unsigned long seed = time(0);
    if (argc > 1) {
        seed = atol(argv[1]); // parse seed from CLI arg
    }
    srand(seed);

    RL_Map *map = rl_map_create(MAP_WIDTH, MAP_HEIGHT);
    system("clear");

    // draw a line on the screen with step of 1
    RL_Point start = { rl_rng_generate(0, MAP_WIDTH - 1), rl_rng_generate(0, MAP_HEIGHT - 1) };
    RL_Point end = { rl_rng_generate(0, MAP_WIDTH - 1), rl_rng_generate(0, MAP_HEIGHT - 1) };
    printf("S: (%d, %d) | E: (%d, %d)\n", (int)start.x, (int)start.y, (int)end.x, (int)end.y);
    RL_Path *path = rl_line_create(start, end, 1);
    while ((path = rl_path_walk(path))) {
        print_at(path->point.x, path->point.y, '*');
    }
    print_at(start.x, start.y, 's');
    print_at(end.x, end.y, 'x');
    print_at(0, map->height, '\n');

    // draw the same line on the screen with step of 0.5
    int offset = MAP_HEIGHT + 2;
    start.x /= 2.0;
    start.y /= 2.0;
    end.x /= 2.0;
    end.y /= 2.0;
    printf("S: (%d, %d) | E: (%d, %d)\n", (int)start.x, (int)start.y, (int)end.x, (int)end.y);
    path = rl_line_create(start, end, 0.5);
    while ((path = rl_path_walk(path))) {
        print_at(path->point.x*2, offset + path->point.y*2, '*');
    }
    print_at(start.x*2, offset + start.y*2, 's');
    print_at(end.x*2, offset + end.y*2, 'x');
    print_at(0, offset + map->height, '\0');
    printf("Seed: %zu\n", seed);

    rl_map_destroy(map);
}
