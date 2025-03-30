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

int main()
{
    rl_rng_seed(time(0));

    RL_Map map = rl_map_create(MAP_WIDTH, MAP_HEIGHT);
    system("clear");

    RL_Point start = { rl_rng_generate(0, MAP_WIDTH - 1), rl_rng_generate(0, MAP_HEIGHT - 1) };
    RL_Point end = { rl_rng_generate(0, MAP_WIDTH - 1), rl_rng_generate(0, MAP_HEIGHT - 1) };
    RL_Path *path = rl_path_create(&map, start, end, rl_distance_manhattan, NULL, 0);
    while ((path = rl_path_walk(path))) {
        print_at(path->point.x, path->point.y, '*');
    }
    print_at(start.x, start.y, 's');
    print_at(end.x, end.y, 'x');
    print_at(0, map.height, '\n');

    rl_map_destroy(&map);
}
