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

int main(void)
{
    srand(time(0));

    RL_Map *map = rl_map_create(MAP_WIDTH, MAP_HEIGHT);
    system("clear");

    // draw a path not allowing diagonals
    RL_Point start = { rl_rng_generate(0, MAP_WIDTH - 1), rl_rng_generate(0, MAP_HEIGHT - 1) };
    RL_Point end = { rl_rng_generate(0, MAP_WIDTH - 1), rl_rng_generate(0, MAP_HEIGHT - 1) };
    printf("S: (%d, %d) | E: (%d, %d)\n", (int)start.x, (int)start.y, (int)end.x, (int)end.y);
    RL_Graph *graph = rl_graph_create(map, NULL, 0);
    rl_dijkstra_score(graph, end, rl_distance_manhattan);
    RL_Path *path = rl_path_create_from_graph(graph, start);
    while ((path = rl_path_walk(path))) {
        print_at(path->point.x, path->point.y, '*');
    }
    rl_graph_destroy(graph);
    print_at(start.x, start.y, 's');
    print_at(end.x, end.y, 'x');
    print_at(0, map->height, '\n');

    // draw a path allowing diagonals using euclidian distance
    int offset_y = MAP_HEIGHT + 2;
    path = rl_path_create(map, start, end, rl_distance_euclidian, NULL);
    while ((path = rl_path_walk(path))) {
        print_at(path->point.x, path->point.y + offset_y, '*');
    }
    print_at(start.x, start.y + offset_y, 's');
    print_at(end.x, end.y + offset_y, 'x');
    print_at(0, map->height + offset_y, '\n');

    rl_map_destroy(map);
}
