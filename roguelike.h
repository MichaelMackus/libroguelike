/**
 * MIT License
 *
 * Copyright (c) 2024 Michael H. Mackus
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 *
 * roguelike.h
 *
 *
 * A single header library for tile-based games. Most features have a
 * "rl_*_create" and a "rl_*_destroy" function. The create function allocates
 * memory and returns a pointer that is assumed to be freed with rl_*_destroy.
 * You can avoid using malloc & free by defining RL_MALLOC (and optionally
 * RL_CALLOC and RL_REALLOC) and RL_FREE, or simply manage the memory yourself.
 *
 * Make sure to define RL_IMPLEMENTATION once and only once before including
 * "roguelike.h" to compile the library.
 *
 * The primary feature of this library deals with the tile-based maps and map
 * generation. The functions are prefixed with rl_map and rl_mapgen.
 *
 * To generate a map, create the map via rl_map_create then call the function
 * with the algorithm you wish to use for mapgen. For example:
 *
 *   RL_Map *map = rl_map_create(80, 25);
 *   if (rl_mapgen_bsp(map, RL_MAPGEN_BSP_DEFAULTS) != RL_OK) {
 *     printf("Error occurred during mapgen!\n");
 *   }
 *   ....
 *   rl_map_destroy(map); // frees the map pointer & internal data
 *
 * The rl_bsp methods correspond to a BSP graph containing data for rectangles.
 * Note that the rl_bsp_split function does allocate memory for the split and
 * assigns the new left & right nodes to the BSP tree (this data is freed with
 * rl_bsp_destroy).
 *
 * There is also functionality for a simple min heap (or priority queue).
 * These functions are prefixed with rl_heap. To use these you need to create
 * a heap with rl_heap_create then insert items with rl_heap_insert. The heap
 * does not free or allocate memory for items you insert into the heap.
 *
 *  RL_Heap *q = rl_heap_create(1, NULL); // NULL comparison function acts as dynamic array
 *  int val = 5;
 *  rl_heap_insert(q, &val);
 *  ....
 *  int r;
 *  while ((r = rl_heap_pop(eq))) { ... }
 *  rl_heap_destroy(q);
 *
 * There is also a set of FOV functions - these functions use a simple
 * shadowcasting algorithm to implement FOV. Create the RL_FOV struct with
 * rl_fov_create (making sure to free it with rl_fov_destroy), and each time
 * you want to "update" the FOV you should call "rl_fov_calculate" or
 * "rl_fov_calculate_ex".
 *
 *  RL_FOV *fov = rl_fov_create(80, 25);
 *  for (;;) { // gameloop
 *      rl_fov_calculate(fov, map, player_x, player_y, 8); // last arg is FOV radius
 *      ... // draw map, handle input, etc.
 *  }
 *  rl_fov_destroy(fov);
 *
 * There is also a set of pathfinding functions - these functions primarily
 * create and manage Dijkstra graphs for pathfinding. These functions are
 * prefixed with rl_path, rl_dijkstra, and rl_graph. Paths should be "walked"
 * with "rl_path_walk" which frees each part of the path passed, returning the
 * next part of the path; or you can alternatively call "rl_path_destroy".
 *
 *  RL_Path *path = rl_path_create(map, RL_XY(0,0), RL_XY(20,20), rl_distance_euclidian, rl_map_is_passable);
 *  while ((path = rl_path_walk(path))) { ...  } // frees the path
 *
 * Dijkstra graphs can be created & scored with rl_dijkstra_create or manually
 * scored via rl_dijkstra_score* functions. After the graph is scored the
 * graph is can be walked by finding a "start" node in the graph, and
 * recursively walking the graph by choosing the lowest scored neighbor. If a
 * RL_GraphNode has a score of FLT_MAX it has not been scored.
 *
 *  // Typically you provide destination for the initial Dijkstra graph
 *  RL_Graph *graph = rl_dijkstra_create(map, dest, rl_distance_manhattan, rl_map_is_passable);
 *  // Then find start point in graph
 *  RL_GraphNode *node = rl_graph_node(graph, start);
 *  // Then, you "roll downhill" from the start point
 *  if (node != NULL) {
 *    RL_GraphNode *lowest_neighbor = rl_graph_lowest_scored_neighbor(graph, node);
 *    // The next point in the path is lowest_neighbor->point
 *    if (lowest_neighbor) move_player(lowest_neighbor->point);
 *    ...
 *  }
 *  rl_graph_destroy(graph);
 *
 *
 * Preprocessor definitions (define these before including roguelike.h to customize internals):
 *
 *
 *  RL_IMPLEMENTATION                 Define this to compile the library - should only be defined once in one file
 *  RL_MAX_NEIGHBOR_COUNT             Maximum neighbor count for Dijkstra graphs (defaults to 8). Note this is needed in the function definitions - if you override this, you'll have to define it everywhere you include roguelike.h (just make a wrapper).
 *  RL_FOV_SYMMETRIC                  Set this to 0 to disable symmetric FOV (defaults to 1)
 *  RL_MAX_RECURSION                  Maximum recursion (defaults to 100). This is used in FOV to limit recursion when fov_radius is large or -1 (unlimited).
 *  RL_MAPGEN_BSP_RANDOMISE_ROOM_LOC  Set this to 0 to disable randomizing room locations within bsp (used in rl_mapgen_bsp - defaults to 1)
 *  RL_ENABLE_PATHFINDING             Set this to 0 to disable pathfinding functionality (defaults to 1)
 *  RL_ENABLE_FOV                     Set this to 0 to disable field of view functionality (defaults to 1)
 *  RL_PASSABLE_F                     Set this to your default passable function (defaults to rl_map_is_passable).
 *  RL_OPAQUE_F                       Set this to your default opaque function (defaults to rl_map_is_opaque).
 *  RL_WALL_F                         Set this to your default is_wall function (defaults to rl_map_is_wall).
 *  RL_FOV_DISTANCE_F                 Set this to your default FOV distance function (defaults to rl_distance_euclidian).
 *  RL_RNG_F                          Set this to your default RNG generation function (defaults to rl_rng_generate).
 *  RL_ASSERT                         Define this to override the assert function used by the library (defaults to "assert")
 *  RL_MALLOC                         Define this to override the malloc function used by the library (defaults to "malloc")
 *  RL_CALLOC                         Define this to override the calloc function used by the library (defaults to "calloc")
 *  RL_REALLOC                        Define this to override the realloc function used by the library, used in rl_heap_* (defaults to "realloc")
 *  RL_FREE                           Define this to override the free function used by the library (defaults to "free")
 */

#ifdef __cplusplus
extern "C" {
#endif

#ifndef RL_ROGUELIKE_H
#define RL_ROGUELIKE_H

#include <stddef.h>

/* This is a helper since MSVC & c89 don't support compound literals */
#ifndef RL_CLITERAL
#if _MSVC_LANG || __cplusplus
#define RL_CLITERAL(type) type
#elif __STDC_VERSION__ < 199409L
#define RL_CLITERAL(type)
#else
#define RL_CLITERAL(type) (type)
#endif
#endif

/* Bool type in c89 */
#if __STDC_VERSION__ < 199409L && !__cplusplus
typedef int bool;
#define true 1
#define false 0
#else
#include <stdbool.h>
#endif

/**
 * Generic structs for library.
 */

/* each tile is the size of 1 byte, so it can be casted back & forth from char <-> RL_Tile */
typedef unsigned char RL_Byte;

/* Generic dungeon map structure, supporting hex & square 2d maps, along with the associated tile enum. */
typedef enum {
    RL_TileRock = ' ',
    RL_TileRoom = '.',
    RL_TileCorridor = '#',
    RL_TileDoor = '+',
    RL_TileDoorOpen = '='
} RL_Tile;
typedef struct RL_Map {
    unsigned int width;
    unsigned int height;
    RL_Byte *tiles; /* a sequential array of RL_Tiles, stride for each row equals the map width. */
} RL_Map;

/* BSP tree */
typedef struct RL_BSP {
    unsigned int width;
    unsigned int height;
    unsigned int x;
    unsigned int y;
    struct RL_BSP *parent;
    struct RL_BSP *left;  /* left child */
    struct RL_BSP *right; /* right child */
} RL_BSP;

/* BSP split direction */
typedef enum {
    RL_SplitHorizontally, /* split the BSP node on the x axis (splits width) */
    RL_SplitVertically   /* split the BSP node on the y axis (splits height) */
} RL_SplitDirection;

/**
 * Random map generation
 */

/* Creates an empty map. Make sure to call rl_map_destroy to clear memory. */
RL_Map *rl_map_create(unsigned int width, unsigned int height);

/* Frees the map & internal memory. */
void rl_map_destroy(RL_Map *map);

/* Enum representing the type of corridor connection algorithm. RL_ConnectRandomly is the default and results in the
 * most interesting & aesthetic maps. */
typedef enum {
    RL_ConnectNone = 0,       /* don't connect corridors */
    RL_ConnectRandomly,       /* connect corridors to random leaf nodes (requires RL_ENABLE_PATHFINDING, by default this is on) */
    RL_ConnectBSP,            /* connect corridors by traversing the BSP graph (faster than above but less circular/interesting maps, requires RL_ENABLE_PATHFINDING) */
    RL_ConnectSimple          /* connect corridors by traversing the BSP graph without Dijkstra pathfinding (fastest) */
} RL_MapgenCorridorConnection;

/* The config for BSP map generation - note that the dimensions *include* the walls on both sides, so the min room width
 * & height the library accepts is 3. */
typedef struct {
    unsigned int room_min_width;
    unsigned int room_max_width;
    unsigned int room_min_height;
    unsigned int room_max_height;
    unsigned int room_padding;
    RL_MapgenCorridorConnection draw_corridors; /* type of corridor connection algorithm to use */
    bool draw_doors; /* whether to draw doors while connecting corridors */
    int max_splits; /* max times to split BSP - set lower for less rooms */
} RL_MapgenConfigBSP;

/* Provide some defaults for mapgen. */
#define RL_MAPGEN_BSP_DEFAULTS RL_CLITERAL(RL_MapgenConfigBSP) { \
    /*.room_min_width =*/      4, \
    /*.room_max_width =*/      6, \
    /*.room_min_height =*/     4, \
    /*.room_max_height =*/     6, \
    /*.room_padding =*/        1, \
    /*.draw_corridors =*/      RL_ConnectRandomly, \
    /*.draw_doors =*/          true, \
    /*.max_splits =*/          100 \
}

typedef enum {
    RL_OK = 0,
    RL_ErrorMemory,
    RL_ErrorNullParameter,
    RL_ErrorMapgenInvalidConfig
} RL_Status;

/* Generate map with recursive BSP split algorithm. This fills the map tiles with RL_TileRock before generation. */
RL_Status rl_mapgen_bsp(RL_Map *map, RL_MapgenConfigBSP config);

/* Generates map with recursive BSP split algorithm. This splits the BSP pointer passed, and uses the BSP to constrain
 * the dimensions of the map generation.
 *
 * This allocates memory for the BSP children - make sure to use rl_bsp_destroy or free them yourself. Note that this
 * does not set the tiles to RL_TileRock before generation. This way you can have separate regions of the map with
 * different mapgen algorithms. */
RL_Status rl_mapgen_bsp_ex(RL_Map *map, RL_BSP *bsp, const RL_MapgenConfigBSP *config);

/* The config for BSP map generation - note that the dimensions *include* the walls on both sides, so the min room width
 * & height the library accepts is 3. */
typedef struct {
    unsigned int chance_cell_initialized; /* chance (from 1-100) a cell is initialized with rock */
    unsigned int birth_threshold;         /* threshold of neighbors for a cell to be born */
    unsigned int survival_threshold;      /* threshold of neighbors for a cell to die from overpopulation */
    unsigned int max_iterations;          /* recursion limit */
    bool draw_corridors;                  /* after generation, whether to randomly draw corridors to unconnected space
                                           * note - you still need cull_unconnected if you want a fully connected map
                                           *
                                           * requires RL_ENABLE_PATHFINDING */
    bool cull_unconnected;                /* after generation, whether to remove unconnected space from the larger map - requires RL_ENABLE_PATHFINDING */
    bool fill_border;                     /* after generation, whether to fill the border with rock to ensure enclosed map*/
} RL_MapgenConfigAutomata;

/* Provide some defaults for automata mapgen. */
#define RL_MAPGEN_AUTOMATA_DEFAULTS RL_CLITERAL(RL_MapgenConfigAutomata) { \
    /*.chance_cell_initialized =*/  45, \
    /*.birth_threshold =*/          5, \
    /*.survival_threshold =*/       4, \
    /*.max_iterations =*/           3, \
    /*.draw_corridors = */          true, \
    /*.cull_unconnected =*/         true, \
    /*.fill_border =*/              true \
}

/* Generate map with cellular automata. This clears out the previous tiles before generation. */
RL_Status rl_mapgen_automata(RL_Map *map, RL_MapgenConfigAutomata config);

/* Same as above function, but constrains generation according to passed dimensions. */
RL_Status rl_mapgen_automata_ex(RL_Map *map, unsigned int x, unsigned int y, unsigned int width, unsigned int height,  const RL_MapgenConfigAutomata *config);

/* Generate map with a random maze (via simplistic BFS). Tiles are carved with RL_TileCorridor. Fully connected. */
RL_Status rl_mapgen_maze(RL_Map *map);

/* Generate map with a random maze (via simplistic BFS). Tiles are carved with RL_TileCorridor. Fully connected. */
RL_Status rl_mapgen_maze_ex(RL_Map *map, unsigned int x, unsigned int y, unsigned int width, unsigned int height);

/* Connect map via corridors using the supplied BSP graph. */
RL_Status rl_mapgen_connect_corridors(RL_Map *map, RL_BSP *root, bool draw_doors, RL_MapgenCorridorConnection connection_algorithm);

/**
 * Generic map helper functions.
 */

/* Verifies a coordinates is within bounds of map. */
bool rl_map_in_bounds(const RL_Map *map, unsigned int x, unsigned int y);

/* Checks if a tile is passable. */
bool rl_map_is_passable(const RL_Map *map, unsigned int x, unsigned int y);

/* Checks if a tile is opaque (for FOV calculations). */
bool rl_map_is_opaque(const RL_Map *map, unsigned int x, unsigned int y);

/* Get tile at point */
RL_Byte *rl_map_tile(const RL_Map *map, unsigned int x, unsigned int y);

/* Returns 1 if tile at point matches given parameter. */
bool rl_map_tile_is(const RL_Map *map, unsigned int x, unsigned int y, RL_Byte tile);

/* Type of wall on the map - idea is they can be bitmasked together (e.g. for corners). See rl_map_wall and other
 * related functions. */
typedef enum {
    RL_WallToWest  = 1,
    RL_WallToEast  = 1 << 1,
    RL_WallToNorth = 1 << 2,
    RL_WallToSouth = 1 << 3,
    RL_WallOther   = 1 << 7 /* e.g. a wall that has no connecting walls */
} RL_Wall;

/* A tile is considered a wall if it is touching a passable tile.
 *
 * Returns a bitmask of the RL_Wall enum. For example, a wall with a wall tile to the south, west, and east would have a
 * bitmask of 0b1011. */
RL_Byte rl_map_wall(const RL_Map *map, unsigned int x, unsigned int y);

/* Is the tile a wall tile? */
bool rl_map_is_wall(const RL_Map *map, unsigned int x, unsigned int y);

/* Is the wall a corner? */
bool rl_map_is_corner_wall(const RL_Map *map, unsigned int x, unsigned int y);

/* Is this a wall that is touching a room tile? */
bool rl_map_is_room_wall(const RL_Map *map, unsigned int x, unsigned int y);

/* A wall that is touching a room tile (e.g. to display it lit). */
RL_Byte rl_map_room_wall(const RL_Map *map, unsigned int x, unsigned int y);

/**
 * Simple priority queue implementation
 */

typedef struct {
    void **heap;
    int cap;
    int len;
    int (*comparison_f)(const void *heap_item_a, const void *heap_item_b);
} RL_Heap;

/* Allocates memory for the heap. Make sure to call rl_heap_destroy after you are done.
 *
 * capacity - initial capacity for the heap
 * comparison_f - A comparison function that returns 1 if heap_item_a should be
 *  popped from the queue before heap_item_b. If NULL the heap will still work
 *  but order will be undefined. */
RL_Heap *rl_heap_create(int capacity, int (*comparison_f)(const void *heap_item_a, const void *heap_item_b));

/* Frees the heap & internal memory. */
void rl_heap_destroy(RL_Heap *h);

/* Return the length of the heap items */
int rl_heap_length(const RL_Heap *h);

/* Insert item into the heap. This will resize the heap if necessary. */
bool rl_heap_insert(RL_Heap *h, void *item);

/* Returns & removes an item from the queue. */
void *rl_heap_pop(RL_Heap *h);

/* Peek at the first item in the queue. This does not remove the item from the queue. */
void *rl_heap_peek(RL_Heap *h);

/**
 * BSP Manipulation
 */

/* Params width & height must be positive. Make sure to free with rl_bsp_destroy. */
RL_BSP *rl_bsp_create(unsigned int width, unsigned int height);

/* Frees the BSP root & all children */
void rl_bsp_destroy(RL_BSP *root);

/* Split the BSP by direction - this creates the left & right leaf and */
/* populates them in the BSP node. Position must be positive and within */
/* the BSP root node. Also node->left & node->right must be NULL */
void rl_bsp_split(RL_BSP *node, unsigned int position, RL_SplitDirection direction);

/* Recursively split the BSP. Used for map generation. */
/* */
/* Returns true if the BSP was able to split at least once */
RL_Status rl_bsp_recursive_split(RL_BSP *root, unsigned int min_width, unsigned int min_height, unsigned int max_recursion);

/* Returns 1 if the node is a leaf node. */
bool rl_bsp_is_leaf(const RL_BSP *node);

/* Return sibling node. Returns NULL if there is no parent (i.e. for the root */
/* node). */
RL_BSP *rl_bsp_sibling(const RL_BSP *node);

/* Returns amount of leaves in tree. */
size_t rl_bsp_leaf_count(const RL_BSP *root);

/* Return the next leaf node to the right if it exists. */
RL_BSP *rl_bsp_next_leaf(const RL_BSP *node);

/* Returns a random leaf node beneath root */
RL_BSP* rl_bsp_random_leaf(const RL_BSP *root);

/**
 * Pathfinding - disable with #define RL_ENABLE_PATHFINDING 0
 */

/* A point on the map used for pathfinding. The points are a float type for flexibility since pathfinding works for maps */
/* of all data types. */
typedef struct RL_Point {
    float x, y;
} RL_Point;

/* Macro to easily create a RL_Point (compound literals only available in C99, which MSVC doesn't support). */
#define RL_XY(x, y) RL_CLITERAL(RL_Point) { (float)(x), (float)(y) }

/* Max neighbors for a pathfinding node. */
#ifndef RL_MAX_NEIGHBOR_COUNT
#define RL_MAX_NEIGHBOR_COUNT 8
#endif

/* Represents a graph of pathfinding nodes that has been scored for pathfinding (e.g. with the Dijkstra algorithm). */
/* TODO store weights on graph nodes ? */
typedef struct RL_GraphNode {
    float score; /* will be FLT_MAX for an unreachable/unscored node in the Dijkstra algorithm */
    RL_Point point;
    size_t neighbors_length;
    struct RL_GraphNode *neighbors[RL_MAX_NEIGHBOR_COUNT];
} RL_GraphNode;
typedef struct RL_Graph {
    size_t length; /* length of nodes */
    RL_GraphNode *nodes; /* array of nodes - length will be the size of the map.width * map.height */
} RL_Graph;

/* A path is a linked list of paths. You can "walk" a path using rl_path_walk which will simultaneously free the
 * previous path. */
typedef struct RL_Path {
    RL_Point point;
    struct RL_Path *next;
} RL_Path;

/* Useful distance functions for pathfinding. */
float rl_distance_manhattan(RL_Point node, RL_Point end);
float rl_distance_euclidian(RL_Point node, RL_Point end);
float rl_distance_chebyshev(RL_Point node, RL_Point end);

/* Custom distance function for pathfinding - calculates distance between map nodes */
typedef float (*RL_DistanceFun)(RL_Point from, RL_Point to);

/* Custom passable function for pathfinding. Return 0 to prevent neighbor from being included in graph. */
typedef bool (*RL_PassableFun)(const RL_Map *map, unsigned int x, unsigned int y);

/* Custom score function for pathfinding - most users won't need this, but it gives flexibility in weighting the
 * Dijkstra graph. Note that Dijkstra expects you to add the current node's score to the newly calculated score. */
typedef float (*RL_ScoreFun)(const RL_GraphNode *current, const RL_GraphNode *neighbor, void *context);

/* Generates a line starting at from ending at to. Each path in the line will be incremented by step. */
RL_Path *rl_line_create(RL_Point from, RL_Point to, float step);

/* Find a path between start and end via Dijkstra algorithm. Make sure to call rl_path_destroy when done with path.
 * Pass NULL to distance_f to use rough approximation for euclidian. */
RL_Path *rl_path_create(const RL_Map *map, RL_Point start, RL_Point end, RL_DistanceFun distance_f);

/* Find a path between start and end via the scored Dijkstra graph. Make sure to call rl_path_destroy when done with path (or
 * use rl_path_walk). */
RL_Path *rl_path_create_from_graph(const RL_Graph *graph, RL_Point start);

/* Convenience function to "walk" the path. This will return the next path, freeing the current path. You do not need to
 * call rl_path_destroy if you walk the full path. */
RL_Path *rl_path_walk(RL_Path *path);

/* Frees the path & all linked nodes. */
void rl_path_destroy(RL_Path *path);

/* Dijkstra pathfinding algorithm. Pass NULL to distance_f to use rough approximation for euclidian.
 *
 * You can use Dijkstra maps for pathfinding, simple AI, and much more. For example, by setting the player point to
 * "start" then you can pick the highest scored tile in the map and set that as the new "start" point. As with all
 * Dijkstra maps, you just walk the map by picking the lowest scored neighbor. This is a simplistic AI resembling a
 * wounded NPC fleeing from the player.
 *
 * Make sure to destroy the resulting RL_Graph with rl_graph_destroy. */
RL_Graph *rl_dijkstra_create(const RL_Map *map,
                            RL_Point start,
                            RL_DistanceFun distance_f);

/* Dijkstra pathfinding algorithm. Uses RL_Graph so that your code doesn't need to rely on RL_Map. Each node's
 * distance should equal FLT_MAX in the resulting graph if it is impassable. */
void rl_dijkstra_score(RL_Graph *graph, RL_Point start, RL_DistanceFun distance_f);

/* Dijkstra pathfinding algorithm for advanced use cases such as weighting certain tiles higher than others. Uses
 * RL_Graph so that your code doesn't need to rely on RL_Map. Each node's distance should equal FLT_MAX in the resulting
 * graph if it is impassable. Most users should just use rl_dijkstra_score - only use this if you have a specific need. */
void rl_dijkstra_score_ex(RL_Graph *graph, RL_Point start, RL_ScoreFun score_f, void *score_context);

/* Returns a the largest connected area (of passable tiles) on the map. Make sure to destroy the graph with
 * rl_graph_destroy after you are done. */
RL_Graph *rl_graph_floodfill_largest_area(const RL_Map *map);

/* Create an unscored graph based on the 2d map. Make sure to call rl_graph_destroy when finished. */
RL_Graph *rl_graph_create(const RL_Map *map);

/* Create an unscored graph based on the 2d map. Make sure to call rl_graph_destroy when finished. */
RL_Graph *rl_graph_create_ex(const RL_Map *map, RL_PassableFun passable_f, bool allow_diagonal_neighbors);

/* Frees the graph & internal memory. */
void rl_graph_destroy(RL_Graph *graph);

/* Checks if coordinate is scored in graph (e.g. its score is less than FLT_MAX). */
bool rl_graph_is_scored(const RL_Graph *graph, RL_Point point);

/* Returns the node of a point within a graph if it exists. */
RL_GraphNode *rl_graph_node(const RL_Graph *graph, RL_Point point);

/* Returns the lowest scored neighbor within a graph if it exists - returns NULL if the lowest scored neighbor is scored
 * with FLT_MAX (meaning it is unscored). */
RL_GraphNode *rl_graph_lowest_scored_neighbor(const RL_Graph *graph, const RL_GraphNode *node);

/**
 * FOV - disable with #define RL_ENABLE_FOV 0
 */

/* Structure containing information for the FOV algorithm, along with the associated visibility enum. */
typedef enum {
    RL_TileCannotSee = 0,
    RL_TileVisible,
    RL_TileSeen
} RL_TileVisibility;
typedef struct {
    unsigned int width;
    unsigned int height;
    RL_Byte *visibility; /* a sequential array of RL_Visibility, stride for each row = the map width */
} RL_FOV;

/* Creates empty FOV and fills it with opaque tiles. Make sure to call rl_fov_destroy to clear memory. */
RL_FOV *rl_fov_create(unsigned int width, unsigned int height);

/* Frees the FOV & internal memory. */
void rl_fov_destroy(RL_FOV *fov);

/* Function to determine if a tile is within the range of the FOV. Returns true if point is in range. */
typedef bool (*RL_IsInRangeFun)(unsigned int x, unsigned int y, void *context);
/* Function to determine if a tile is considered Opaque for FOV calculation. Make sure you do bounds checking that the
 * point is within your map. Returns true if point is considered "opaque" (i.e. unable to see through). */
typedef bool (*RL_IsOpaqueFun)(unsigned int x, unsigned int y, void *context);
/* Function to mark a tile as visible within the FOV. Make sure you do bounds checking that the point is within your map. */
typedef void (*RL_MarkAsVisibleFun)(unsigned int x, unsigned int y, void *context);

/* Calculate FOV using simple shadowcasting algorithm. Set fov_radius to a negative value to have unlimited FOV (note
 * this is limited by RL_MAX_RECURSION).
 *
 * Note that this sets previously visible tiles to RL_TileSeen. */
void rl_fov_calculate(RL_FOV *fov, const RL_Map *map, unsigned int x, unsigned int y, int fov_radius);

/* Calculate FOV using simple shadowcasting algorithm. Set fov_radius to a negative value to have unlimited FOV (note
 * this is limited by RL_MAX_RECURSION).
 *
 * Generic version of above function. */
void rl_fov_calculate_ex(void *context, unsigned int x, unsigned int y, RL_IsInRangeFun in_range_f, RL_IsOpaqueFun opaque_f, RL_MarkAsVisibleFun mark_visible_f);

/* Checks if a point is visible within FOV. Make sure to call rl_fov_calculate first. */
bool rl_fov_is_visible(const RL_FOV *map, unsigned int x, unsigned int y);

/* Checks if a point has been seen within FOV. Make sure to call rl_fov_calculate first. */
bool rl_fov_is_seen(const RL_FOV *map, unsigned int x, unsigned int y);
#endif /* RL_ENABLE_FOV */

/**
 * Random number generation
 */

/* Default implementation of RNG using standard library. */
unsigned int rl_rng_generate(unsigned int min, unsigned int max);

#ifdef RL_IMPLEMENTATION

#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <limits.h>

#ifndef RL_FOV_SYMMETRIC
#define RL_FOV_SYMMETRIC 1
#endif

#ifndef RL_MAX_RECURSION
#define RL_MAX_RECURSION 100
#endif

/* define this to 0 to put the rooms in the middle of the BSP leaf during dungeon generation */
#ifndef RL_MAPGEN_BSP_RANDOMISE_ROOM_LOC
#define RL_MAPGEN_BSP_RANDOMISE_ROOM_LOC 1
#endif

/* define to 0 to disable pathfinding */
#ifndef RL_ENABLE_PATHFINDING
#define RL_ENABLE_PATHFINDING 1
#endif

/* define to 0 to disable FOV */
#ifndef RL_ENABLE_FOV
#define RL_ENABLE_FOV 1
#endif

#ifndef RL_PASSABLE_F
#define RL_PASSABLE_F rl_map_is_passable
#endif
#ifndef RL_OPAQUE_F
#define RL_OPAQUE_F rl_map_is_opaque
#endif
#ifndef RL_WALL_F
#define RL_WALL_F rl_map_is_wall
#endif
#ifndef RL_FOV_DISTANCE_F
#define RL_FOV_DISTANCE_F rl_distance_euclidian
#endif
#ifndef RL_RNG_F
#define RL_RNG_F rl_rng_generate
#endif
#ifndef RL_ASSERT
#include <assert.h>
#define RL_ASSERT(expr)		(assert(expr));
#endif
#ifndef RL_MALLOC
#define RL_MALLOC malloc
#endif
#ifndef RL_CALLOC
#define RL_CALLOC calloc
#endif
#ifndef RL_REALLOC
#define RL_REALLOC realloc
#endif
#ifndef RL_FREE
#define RL_FREE free
#endif

#define RL_UNUSED(x) (void)x

#if RL_ENABLE_PATHFINDING
#include <float.h>
#include <math.h>
#endif

RL_Map *rl_map_create(unsigned int width, unsigned int height)
{
    RL_Map *map;
    unsigned char *memory;
    RL_ASSERT(width*height < UINT_MAX);
    RL_ASSERT(width > 0 && height > 0);
    map = NULL;
    /* allocate all the memory we need at once */
    memory = (unsigned char*) RL_MALLOC(sizeof(*map) + sizeof(*map->tiles)*width*height);
    RL_ASSERT(memory);
    if (memory == NULL) return NULL;
    map = (RL_Map*) memory;
    RL_ASSERT(map);
    map->width = width;
    map->height = height;
    map->tiles = (RL_Byte*) (memory + sizeof(*map));
    RL_ASSERT(map->tiles);
    memset(map->tiles, RL_TileRock, sizeof(*map->tiles)*map->width*map->height);

    return map;
}

void rl_map_destroy(RL_Map *map)
{
    if (map) {
        RL_FREE(map);
    }
}

bool rl_map_in_bounds(const RL_Map *map, unsigned int x, unsigned int y)
{
    return x < map->width && y < map->height;
}

bool rl_map_is_passable(const RL_Map *map, unsigned int x, unsigned int y)
{
    if (rl_map_in_bounds(map, x, y)) {
        return map->tiles[y * map->width + x] == RL_TileRoom ||
               map->tiles[y * map->width + x] == RL_TileCorridor ||
               map->tiles[y * map->width + x] == RL_TileDoor ||
               map->tiles[y * map->width + x] == RL_TileDoorOpen;
    }

    return 0;
}

bool rl_map_is_opaque(const RL_Map *map, unsigned int x, unsigned int y)
{
    if (!rl_map_in_bounds(map, x, y)) {
        return true;
    }
    if (rl_map_tile_is(map, x, y, RL_TileDoor)) {
        /* doors are opaque by default, unless they are open */
        return true;
    }
    return !RL_PASSABLE_F(map, x, y);
}

RL_Byte *rl_map_tile(const RL_Map *map, unsigned int x, unsigned int y)
{
    if (rl_map_in_bounds(map, x, y)) {
        return &map->tiles[x + y*map->width];
    }

    return NULL;
}

bool rl_map_is_wall(const RL_Map *map, unsigned int x, unsigned int y)
{
    if (!rl_map_in_bounds(map, x, y))
        return 0;
    if (!RL_PASSABLE_F(map, x, y) || rl_map_tile_is(map, x, y, RL_TileDoor) || rl_map_tile_is(map, x, y, RL_TileDoorOpen)) {
        return RL_PASSABLE_F(map, x, y + 1) ||
               RL_PASSABLE_F(map, x, y - 1) ||
               RL_PASSABLE_F(map, x + 1, y) ||
               RL_PASSABLE_F(map, x - 1, y) ||
               RL_PASSABLE_F(map, x + 1, y - 1) ||
               RL_PASSABLE_F(map, x - 1, y - 1) ||
               RL_PASSABLE_F(map, x + 1, y + 1) ||
               RL_PASSABLE_F(map, x - 1, y + 1);
    }

    return 0;
}

/* checks if target tile is connecting from source (e.g. they can reach it) */
bool rl_map_is_connecting(const RL_Map *map, unsigned int from_x, unsigned int from_y, unsigned int target_x, unsigned int target_y)
{
    int x, y, x2, y2;
    RL_ASSERT(target_x < INT_MAX && target_y < INT_MAX && from_y < INT_MAX && target_y < INT_MAX);
    /* check that from passable neighbors can connect to target */
    for (x = (int)from_x - 1; x <= (int)from_x + 1; ++x) {
        for (y = (int)from_y - 1; y <= (int)from_y + 1; ++y) {
            if (!rl_map_in_bounds(map, x, y) || !RL_PASSABLE_F(map, x, y))
                continue;
            if (rl_map_tile_is(map, x, y, RL_TileDoor) || rl_map_tile_is(map, x, y, RL_TileDoorOpen))
                continue;
            /* this is a passable neighbor - check its neighbors to see if it can reach target */
            for (x2 = x - 1; x2 <= x + 1; ++x2) {
                for (y2 = y - 1; y2 <= y + 1; ++y2) {
                    if (!rl_map_in_bounds(map, x2, y2)) continue;
                    if ((unsigned int) x2 == target_x && (unsigned int) y2 == target_y)
                        return true;
                }
            }
        }
    }

    return false;
}

RL_Byte rl_map_wall(const RL_Map *map, unsigned int x, unsigned int y)
{
    RL_Byte mask = 0;
    if (!RL_WALL_F(map, x, y))
        return mask;
    if (RL_WALL_F(map, x + 1, y    ) && rl_map_is_connecting(map, x, y, x + 1, y))
        mask |= RL_WallToEast;
    if (RL_WALL_F(map, x - 1, y    ) && rl_map_is_connecting(map, x, y, x - 1, y))
        mask |= RL_WallToWest;
    if (RL_WALL_F(map, x,     y - 1) && rl_map_is_connecting(map, x, y, x,     y - 1))
        mask |= RL_WallToNorth;
    if (RL_WALL_F(map, x,     y + 1) && rl_map_is_connecting(map, x, y, x,     y + 1))
        mask |= RL_WallToSouth;
    return mask ? mask : RL_WallOther;
}

bool rl_map_is_corner_wall(const RL_Map *map, unsigned int x, unsigned int y)
{
    int wall = rl_map_wall(map, x, y);
    if (!wall) return 0;
    return (wall & RL_WallToWest && wall & RL_WallToNorth) ||
           (wall & RL_WallToWest && wall & RL_WallToSouth) ||
           (wall & RL_WallToEast && wall & RL_WallToNorth) ||
           (wall & RL_WallToEast && wall & RL_WallToSouth);
}

bool rl_map_tile_is(const RL_Map *map, unsigned int x, unsigned int y, RL_Byte tile)
{
    if (!rl_map_in_bounds(map, x, y)) return 0;
    return map->tiles[x + y*map->width] == tile;
}

bool rl_map_is_room_wall(const RL_Map *map, unsigned int x, unsigned int y)
{
    if (!RL_WALL_F(map, x, y))
        return 0;

    return rl_map_tile_is(map, x, y + 1,     RL_TileRoom) ||
           rl_map_tile_is(map, x, y - 1,     RL_TileRoom) ||
           rl_map_tile_is(map, x + 1, y,     RL_TileRoom) ||
           rl_map_tile_is(map, x - 1, y,     RL_TileRoom) ||
           rl_map_tile_is(map, x + 1, y - 1, RL_TileRoom) ||
           rl_map_tile_is(map, x - 1, y - 1, RL_TileRoom) ||
           rl_map_tile_is(map, x + 1, y + 1, RL_TileRoom) ||
           rl_map_tile_is(map, x - 1, y + 1, RL_TileRoom);
}

RL_Byte rl_map_room_wall(const RL_Map *map, unsigned int x, unsigned int y)
{
    RL_Byte mask = 0;
    if (!rl_map_is_room_wall(map, x,     y))
        return mask;
    if (rl_map_is_room_wall(map,  x + 1, y))
        mask |= RL_WallToEast;
    if (rl_map_is_room_wall(map,  x - 1, y))
        mask |= RL_WallToWest;
    if (rl_map_is_room_wall(map,  x,     y - 1))
        mask |= RL_WallToNorth;
    if (rl_map_is_room_wall(map,  x,     y + 1))
        mask |= RL_WallToSouth;
    return mask ? mask : RL_WallOther;
}

unsigned int rl_rng_generate(unsigned int min, unsigned int max)
{
    int rnd;

    RL_ASSERT(max >= min);
    RL_ASSERT(max < RAND_MAX);
    RL_ASSERT(max < UINT_MAX);

    if (max < min || max >= RAND_MAX || max >= UINT_MAX)
        return min;
    if (min == max)
        return min;

    rnd = rand();
    if (rnd < 0) rnd *= -1; /* fixes issue on LLVM MOS */

    /* produces more uniformity than using mod */
    return min + rnd / (RAND_MAX / (max - min + 1) + 1);
}

RL_BSP *rl_bsp_create(unsigned int width, unsigned int height)
{
    RL_BSP *bsp;

    RL_ASSERT(width > 0 && height > 0);
    bsp = (RL_BSP*) RL_CALLOC(sizeof(*bsp), 1);
    if (bsp == NULL) return NULL;
    bsp->width = width;
    bsp->height = height;

    return bsp;
}
void rl_bsp_destroy(RL_BSP* root)
{
    if (root) {
        if (root->left) {
            rl_bsp_destroy(root->left);
            root->left = NULL;
        }
        if (root->right) {
            rl_bsp_destroy(root->right);
            root->right = NULL;
        }
        RL_FREE(root);
    }
}

void rl_bsp_split(RL_BSP *node, unsigned int position, RL_SplitDirection direction)
{
    RL_BSP *left, *right;

    /* can't split something already split */
    RL_ASSERT(node->left == NULL && node->right == NULL);

    if (node->left || node->right)
        return;

    if (direction == RL_SplitVertically && position >= node->height)
        return;
    if (direction == RL_SplitHorizontally && position >= node->width)
        return;

    left = (RL_BSP*) RL_CALLOC(1, sizeof(RL_BSP));
    if (left == NULL)
        return;
    right = (RL_BSP*) RL_CALLOC(1, sizeof(RL_BSP));
    if (right == NULL) {
        RL_FREE(left);
        return;
    }

    if (direction == RL_SplitVertically) {
        left->width = node->width;
        left->height = position;
        left->x = node->x;
        left->y = node->y;
        right->width = node->width;
        right->height = node->height - position;
        right->x = node->x;
        right->y = node->y + position;
    } else {
        left->width = position;
        left->height = node->height;
        left->x = node->x;
        left->y = node->y;
        right->width = node->width - position;
        right->height = node->height;
        right->x = node->x + position;
        right->y = node->y;
    }

    left->parent = right->parent = node;
    node->left = left;
    node->right = right;
}

RL_Status rl_bsp_recursive_split(RL_BSP *root, unsigned int min_width, unsigned int min_height, unsigned int max_recursion)
{
    unsigned int width, height, split_position;
    RL_SplitDirection dir;
    RL_BSP *left, *right;
    RL_Status ret;

    RL_ASSERT(root);
    RL_ASSERT(min_width > 0 && min_height > 0 && root != NULL);
    RL_ASSERT(min_width <= root->width && min_height <= root->height);

    if (root == NULL)
        return RL_ErrorNullParameter;
    if (max_recursion <= 0)
        return RL_OK;

    width = root->width;
    height = root->height;

    /* determine split dir & split */
    if (RL_RNG_F(0, 1)) {
        if (width < min_width*2)
            dir = RL_SplitVertically;
        else
            dir = RL_SplitHorizontally;
    } else {
        if (height < min_height*2)
            dir = RL_SplitHorizontally;
        else
            dir = RL_SplitVertically;
    }

    if (dir == RL_SplitHorizontally) {
        /* cannot split if current node size is too small - end splitting */
        if (width < min_width*2)
            return RL_OK;
        split_position = width / 2;
    } else {
        /* cannot split if current node size is too small - end splitting */
        if (height < min_height*2)
            return RL_OK;
        split_position = height / 2;
    }

    rl_bsp_split(root, split_position, dir);

    /* continue recursion */
    left = root->left;
    right = root->right;

    if (left == NULL || right == NULL)
        return RL_ErrorMemory;

    ret = rl_bsp_recursive_split(left, min_width, min_height, max_recursion - 1);
    if (ret != RL_OK) {
        RL_FREE(left);
        RL_FREE(right);
        root->left = root->right = NULL;
        return ret;
    }

    ret = rl_bsp_recursive_split(right, min_width, min_height, max_recursion - 1);
    if (ret != RL_OK) {
        RL_FREE(left);
        RL_FREE(right);
        root->left = root->right = NULL;
        return ret;
    }

    return RL_OK;
}

bool rl_bsp_is_leaf(const RL_BSP *node)
{
    if (node == NULL) return 0;
    return (node->left == NULL && node->right == NULL);
}

RL_BSP *rl_bsp_sibling(const RL_BSP *node)
{
    if (node && node->parent) {
        if (node->parent->left == node)
            return node->parent->right;
        if (node->parent->right == node)
            return node->parent->left;

        RL_ASSERT("BSP structure is invalid" && 0); /* BSP structure is invalid */
    }

    return NULL;
}

RL_BSP *rl_bsp_next_node_recursive_down(RL_BSP *node, int depth)
{
    if (node == NULL)
        return NULL;
    if (depth == 0) /* found the node */
        return node;
    if (node->left == NULL)
        return NULL;
    return rl_bsp_next_node_recursive_down(node->left, depth + 1);
}
RL_BSP *rl_bsp_next_node_recursive(RL_BSP *node, int depth)
{
    if (node == NULL || node->parent == NULL)
        return NULL;
    if (node->parent->left == node) /* traverse back down */
        return rl_bsp_next_node_recursive_down(node->parent->right, depth);
    return rl_bsp_next_node_recursive(node->parent, depth - 1);
}
RL_BSP *rl_bsp_next_node(RL_BSP *node)
{
    if (node == NULL || node->parent == NULL)
        return NULL;

    /* LOOP up until we are on the left, then go back down */
    return rl_bsp_next_node_recursive(node, 0);
}

RL_BSP *rl_bsp_next_leaf_recursive_down(RL_BSP *node)
{
    if (node == NULL)
        return NULL;
    if (rl_bsp_is_leaf(node)) /* found the node */
        return node;
    if (node->left == NULL)
        return NULL;
    return rl_bsp_next_leaf_recursive_down(node->left);
}
RL_BSP *rl_bsp_next_leaf_recursive(const RL_BSP *node)
{
    if (node == NULL || node->parent == NULL)
        return NULL;
    if (node->parent->left == node) /* traverse back down */
        return rl_bsp_next_leaf_recursive_down(node->parent->right);
    return rl_bsp_next_leaf_recursive(node->parent);
}
RL_BSP *rl_bsp_next_leaf(const RL_BSP *node)
{
    if (node == NULL || node->parent == NULL)
        return NULL;
    RL_ASSERT(rl_bsp_is_leaf(node));

    /* LOOP up until we are on the left, then go back down */
    return rl_bsp_next_leaf_recursive(node);
}
RL_BSP* rl_bsp_random_leaf(const RL_BSP *root)
{
    const RL_BSP *node;

    if (root == NULL)
        return NULL;

    node = root;
    while (!rl_bsp_is_leaf(node)) {
        if (RL_RNG_F(0, 1)) {
            node = node->left;
        } else {
            node = node->right;
        }
    }

    return (RL_BSP*) node;
}

size_t rl_bsp_leaf_count(const RL_BSP *root)
{
    int count;
    const RL_BSP *node;
    if (root == NULL) return 0;
    RL_ASSERT(root->parent == NULL);
    /* find first leaf */
    node = root;
    while (node->left != NULL) {
        node = node->left;
    }
    /* count leaves */
    count = 1;
    while ((node = rl_bsp_next_leaf(node)) != NULL) {
        count++;
    }
    return count;
}

static void rl_map_bsp_generate_room(RL_Map *map, unsigned int room_width, unsigned int room_height, unsigned int room_x, unsigned int room_y)
{
    unsigned int x, y;
    RL_ASSERT(map && room_width + room_x <= map->width);
    RL_ASSERT(map && room_height + room_y <= map->height);
    if (map == NULL) return;
    for (x = room_x; x < room_x + room_width; ++x) {
        for (y = room_y; y < room_y + room_height; ++y) {
            if (x == room_x || x == room_x + room_width - 1 ||
                    y == room_y || y == room_y + room_height - 1
               ) {
                /* set sides of room to walls */
                map->tiles[y*map->width + x] = RL_TileRock;
            } else {
                map->tiles[y*map->width + x] = RL_TileRoom;
            }
        }
    }
}
static void rl_map_bsp_generate_rooms(RL_BSP *node, RL_Map *map, unsigned int room_min_width, unsigned int room_max_width, unsigned int room_min_height, unsigned int room_max_height, unsigned int room_padding)
{
    RL_ASSERT(map);
    RL_ASSERT(room_min_width < room_max_width);
    RL_ASSERT(room_min_height < room_max_height);
    RL_ASSERT(room_max_width + room_padding*2 < UINT_MAX);
    RL_ASSERT(room_max_height + room_padding*2 < UINT_MAX);
    RL_ASSERT(room_min_width > 2 && room_min_height > 2); /* width of 2 can end up having rooms made of nothing but walls */
    RL_ASSERT(node && room_min_width < node->width);
    RL_ASSERT(node && room_min_height < node->height);
    RL_ASSERT(node && room_max_width <= node->width);
    RL_ASSERT(node && room_max_height <= node->height);
    if (map == NULL) return;
    if (node && node->left) {
        if (rl_bsp_is_leaf(node->left)) {
            unsigned int room_width, room_height, room_x, room_y;
            RL_BSP *leaf = node->left;
            room_width = RL_RNG_F(room_min_width, room_max_width);
            if (room_width + room_padding*2 > leaf->width)
                room_width = leaf->width - room_padding*2;
            room_height = RL_RNG_F(room_min_height, room_max_height);
            if (room_height + room_padding*2 > leaf->height)
                room_height = leaf->height - room_padding*2;
#if(RL_MAPGEN_BSP_RANDOMISE_ROOM_LOC)
            room_x = RL_RNG_F(leaf->x + room_padding, leaf->x + leaf->width - room_width - room_padding);
            room_y = RL_RNG_F(leaf->y + room_padding, leaf->y + leaf->height - room_height - room_padding);
#else
            room_x = leaf->x + leaf->width/2 - room_width/2 - room_padding/2;
            room_y = leaf->y + leaf->height/2 - room_height/2 - room_padding/2;
#endif

            rl_map_bsp_generate_room(map, room_width, room_height, room_x, room_y);
        } else {
            rl_map_bsp_generate_rooms(node->left, map, room_min_width, room_max_width, room_min_height, room_max_height, room_padding);
        }
    }
    if (node && node->right) {
        if (rl_bsp_is_leaf(node->left)) {
            unsigned int room_width, room_height, room_x, room_y;
            RL_BSP *leaf = node->right;
            room_width = RL_RNG_F(room_min_width, room_max_width);
            if (room_width + room_padding*2 > leaf->width)
                room_width = leaf->width - room_padding*2;
            room_height = RL_RNG_F(room_min_height, room_max_height);
            if (room_height + room_padding*2 > leaf->height)
                room_height = leaf->height - room_padding*2;
#if(RL_MAPGEN_BSP_RANDOMISE_ROOM_LOC)
            room_x = RL_RNG_F(leaf->x + room_padding, leaf->x + leaf->width - room_width - room_padding);
            room_y = RL_RNG_F(leaf->y + room_padding, leaf->y + leaf->height - room_height - room_padding);
#else
            room_x = leaf->x + leaf->width/2 - room_width/2 - room_padding/2;
            room_y = leaf->y + leaf->height/2 - room_height/2 - room_padding/2;
#endif

            rl_map_bsp_generate_room(map, room_width, room_height, room_x, room_y);
        } else {
            rl_map_bsp_generate_rooms(node->right, map, room_min_width, room_max_width, room_min_height, room_max_height, room_padding);
        }
    }
}

RL_Status rl_mapgen_bsp(RL_Map *map, RL_MapgenConfigBSP config)
{
    RL_Status ret;
    RL_BSP *bsp;
    RL_ASSERT(map);
    if (map == NULL) return RL_ErrorMemory;
    bsp = rl_bsp_create(map->width, map->height);
    RL_ASSERT(bsp);
    if (bsp == NULL) return RL_ErrorMemory;
    memset(map->tiles, RL_TileRock, sizeof(*map->tiles)*map->width*map->height);
    ret = rl_mapgen_bsp_ex(map, bsp, &config);
    rl_bsp_destroy(bsp);

    return ret;
}

RL_Status rl_mapgen_bsp_ex(RL_Map *map, RL_BSP *root, const RL_MapgenConfigBSP *config)
{
    RL_Status ret;

    RL_ASSERT(map);
    RL_ASSERT(root);
    RL_ASSERT(root->width > 0 && root->height > 0);
    RL_ASSERT(root->x < root->width && root->y < root->height);
    RL_ASSERT(config);
    RL_ASSERT(config->room_min_width > 0 && config->room_max_width >= config->room_min_width && config->room_min_height > 0 && config->room_max_height >= config->room_min_height);
    RL_ASSERT(config->room_max_width <= map->width && config->room_max_height <= map->height);
    RL_ASSERT(config->max_splits > 0);

    if (map == NULL || root == NULL || config == NULL) {
        return RL_ErrorNullParameter;
    }

    ret = rl_bsp_recursive_split(root, config->room_max_width + config->room_padding, config->room_max_height + config->room_padding, config->max_splits);
    if (ret != RL_OK) return ret;
    rl_map_bsp_generate_rooms(root, map, config->room_min_width, config->room_max_width, config->room_min_height, config->room_max_height, config->room_padding);
    ret = rl_mapgen_connect_corridors(map, root, config->draw_doors, config->draw_corridors);
    if (ret != RL_OK) return ret;

    /* if (config->use_secret_passages) { */
        /* TODO connect secret passages */
    /* } */

    return RL_OK;
}

/* find the room tile within BSP */
void rl_bsp_find_room(RL_Map *map, RL_BSP *leaf, unsigned int *dx, unsigned int *dy)
{
    unsigned int x, y;
    unsigned int start_x, start_y, end_x, end_y;
    bool found_start = false;
    RL_ASSERT(dx && dy);
    RL_ASSERT(map);
    RL_ASSERT(leaf);
    for (x = leaf->x; x < leaf->width + leaf->x; ++x) {
        for (y = leaf->y; y < leaf->height + leaf->y; ++y) {
            if (!found_start) {
                if (rl_map_tile_is(map, x, y, RL_TileRoom)) {
                    start_x = x;
                    start_y = y;
                    end_x = x;
                    end_y = y;
                    found_start = true;
                }
            } else {
                if (rl_map_tile_is(map, x, y, RL_TileRoom)) {
                    end_x = x;
                    end_y = y;
                } else {
                    /* found end - return middle of room */
                    int diff_x = end_x - start_x;
                    int diff_y = end_y - start_y;
                    RL_ASSERT(diff_x >= 0 && diff_y >= 0);
                    *dx = start_x + diff_x/2;
                    *dy = start_y + diff_y/2;
                }
            }
        }
    }
}

bool rl_mapgen_automata_is_alive(const RL_Map *map, int x, int y)
{
    if (!rl_map_in_bounds(map, x, y)) return true;
    return rl_map_tile_is(map, x, y, RL_TileRock);
}
unsigned int rl_mapgen_automata_alive_neighbors(const RL_Map *map, int x, int y)
{
    return rl_mapgen_automata_is_alive(map, x + 1, y) +
           rl_mapgen_automata_is_alive(map, x - 1, y) +
           rl_mapgen_automata_is_alive(map, x,     y + 1) +
           rl_mapgen_automata_is_alive(map, x,     y - 1) +
           rl_mapgen_automata_is_alive(map, x + 1, y + 1) +
           rl_mapgen_automata_is_alive(map, x - 1, y + 1) +
           rl_mapgen_automata_is_alive(map, x + 1, y - 1) +
           rl_mapgen_automata_is_alive(map, x - 1, y - 1);
}

RL_Status rl_mapgen_automata(RL_Map *map, RL_MapgenConfigAutomata config)
{
    return rl_mapgen_automata_ex(map, 0, 0, map->width, map->height, &config);
}

#if RL_ENABLE_PATHFINDING
static inline float rl_mapgen_corridor_scorer(const RL_GraphNode *current, const RL_GraphNode *neighbor, void *context);
#endif
RL_Status rl_mapgen_automata_ex(RL_Map *map, unsigned int offset_x, unsigned int offset_y, unsigned int width, unsigned int height,  const RL_MapgenConfigAutomata *config)
{
    unsigned int i, x, y;

    RL_ASSERT(map && config);
    RL_ASSERT(width > 0 && height > 0);
    RL_ASSERT(offset_x < width && offset_y < height);
    RL_ASSERT(offset_x < map->width && offset_y < map->height);
    RL_ASSERT(offset_x + width <= map->width && offset_y + height <= map->height);
    RL_ASSERT(config->chance_cell_initialized > 0 && config->chance_cell_initialized <= 100);

    if (map == NULL || config == NULL) {
        return RL_ErrorNullParameter;
    }

    /* initialize map */
    for (x=offset_x; x<offset_x + width; ++x) {
        for (y=offset_y; y<offset_y + height; ++y) {
            unsigned int r = RL_RNG_F(1, 100);
            if (r <= config->chance_cell_initialized) {
                map->tiles[x + y*map->width] = RL_TileRock;
            } else {
                map->tiles[x + y*map->width] = RL_TileRoom;
            }
        }
    }

    /* cellular automata algorithm */
    for (i=config->max_iterations; i>0; i--) {
        for (x=offset_x; x<offset_x + width; ++x) {
            for (y=offset_y; y<offset_y + height; ++y) {
                unsigned int alive_neighbors = rl_mapgen_automata_alive_neighbors(map, x, y);
                if (!rl_mapgen_automata_is_alive(map, x, y) && alive_neighbors >= config->birth_threshold) {
                    /* cell isn't alive but has enough alive neighbors to be born */
                    map->tiles[x + y*map->width] = RL_TileRock;
                } else if (rl_mapgen_automata_is_alive(map, x, y) && alive_neighbors >= config->survival_threshold) {
                    /* cell is alive and has enough alive neighbors to survive */
                } else {
                    /* cell dies */
                    map->tiles[x + y*map->width] = RL_TileRoom;
                }
            }
        }
    }

    if (config->draw_corridors) {
#if RL_ENABLE_PATHFINDING
        /* A very crude algorithm for connecting corridors within the cellular automata. This creates a heap of Dijkstra
         * graphs, containing each floodfilled region of the map. Then, it goes through each of these regions and
         * connects it to another random region. This is pretty slow and can be optimized in the future, but works for
         * now. */

        RL_Heap *heap = rl_heap_create(1, NULL);
        /* fill floodfills array with a floodfill of each connected space */
        for (x=offset_x; x<offset_x + width; ++x) {
            for (y=offset_y; y<offset_y + height; ++y) {
                int i;
                bool is_scored = false;
                if (RL_PASSABLE_F(map, x, y)) {
                    for (i=0; i<heap->len; ++i) {
                        RL_Graph *floodfill = (RL_Graph*) heap->heap[i];
                        if (rl_graph_is_scored(floodfill, RL_XY(x, y))) {
                            is_scored = true;
                            break;
                        }
                    }
                    if (!is_scored) {
                        RL_Graph *floodfill = rl_dijkstra_create(map, RL_XY(x, y), NULL);
                        rl_heap_insert(heap, floodfill);
                    }
                }
            }
        }
        /* connect each floodfill with another random one */
        if (heap->len > 1) {
            int i;
            RL_Graph *graph = rl_graph_create_ex(map, NULL, 0);
            RL_ASSERT(graph);
            for (i=0; i<heap->len; ++i) {
                RL_Graph *floodfill_target;
                RL_Graph *floodfill;
                RL_Point dig_start, dig_end;
                size_t node_idx;
                int j = i;
                floodfill = (RL_Graph*) heap->heap[i];
                /* find a random target node to connect to */
                while (j == i) {
                    j = RL_RNG_F(0, heap->len - 1);
                }
                floodfill_target = (RL_Graph*) heap->heap[j];
                RL_ASSERT(floodfill && floodfill_target);
                /* find start & end point for corridor pathfinding */
                for (node_idx=0; node_idx<floodfill->length; ++node_idx) {
                    RL_GraphNode *n = &floodfill->nodes[node_idx];
                    RL_ASSERT(n);
                    if (n->score < FLT_MAX && RL_PASSABLE_F(map, n->point.x, n->point.y)) {
                        dig_start = n->point;
                        break;
                    }
                }
                RL_ASSERT(RL_PASSABLE_F(map, dig_start.x, dig_start.y));
                for (node_idx=0; node_idx<floodfill_target->length; ++node_idx) {
                    RL_GraphNode *n = &floodfill_target->nodes[node_idx];
                    RL_ASSERT(n);
                    if (n->score < FLT_MAX && RL_PASSABLE_F(map, n->point.x, n->point.y)) {
                        dig_end = n->point;
                        break;
                    }
                }
                RL_ASSERT(RL_PASSABLE_F(map, dig_end.x, dig_end.y));
                RL_ASSERT(!(dig_start.x == dig_end.x && dig_start.y == dig_end.y));
                /* carve out corridors */
                rl_dijkstra_score_ex(graph, dig_end, rl_mapgen_corridor_scorer, map);
                RL_Path *path = rl_path_create_from_graph(graph, dig_start);
                RL_ASSERT(path);
                while ((path = rl_path_walk(path))) {
                    if (rl_map_tile_is(map, path->point.x, path->point.y, RL_TileRock)) {
                        map->tiles[(size_t)floor(path->point.x) + (size_t)floor(path->point.y) * map->width] = RL_TileCorridor;
                    }
                }
            }
            rl_graph_destroy(graph);
        }
        /* cleanup */
        RL_Graph *floodfill = NULL;
        while ((floodfill = (RL_Graph*) rl_heap_pop(heap))) {
            rl_graph_destroy(floodfill);
        }
        rl_heap_destroy(heap);
#else
        return RL_ErrorMapgenInvalidConfig;
#endif
    }
    if (config->fill_border) {
        x = 0;
        for (y=offset_y; y<height; ++y) map->tiles[x + y*map->width] = RL_TileRock;
        x = width - 1;
        for (y=offset_y; y<height; ++y) map->tiles[x + y*map->width] = RL_TileRock;
        y = 0;
        for (x=offset_x; x<width; ++x) map->tiles[x + y*map->width] = RL_TileRock;
        y = height - 1;
        for (x=offset_x; x<width; ++x) map->tiles[x + y*map->width] = RL_TileRock;
    }
    if (config->cull_unconnected) {
#if RL_ENABLE_PATHFINDING
        RL_Graph *floodfill = rl_graph_floodfill_largest_area(map);
        if (floodfill) {
            for (x=offset_x; x<offset_x + width; ++x) {
                for (y=offset_y; y<offset_y + height; ++y) {
                    if (!rl_graph_is_scored(floodfill, RL_XY(x, y))) {
                        map->tiles[x + y*map->width] = RL_TileRock;
                    }
                }
            }
            rl_graph_destroy(floodfill);
        }
#else
        return RL_ErrorMapgenInvalidConfig;
#endif
    }

    return RL_OK;
}

typedef struct {
    int x, y;
} RL_MapPoint;

int rl_mapgen_maze_unvisited_neighbors(RL_MapPoint ps[4], const RL_Map *map, int x, int y, int sx, int mx, int sy, int my)
{
    RL_MapPoint neighbors[4];
    int i, count = 0;
    neighbors[0].x = x - 2;
    neighbors[0].y = y;
    neighbors[1].x = x + 2;
    neighbors[1].y = y;
    neighbors[2].x = x;
    neighbors[2].y = y - 2;
    neighbors[3].x = x;
    neighbors[3].y = y + 2;
    for (i = 0; i<4; ++i) {
        int x = neighbors[i].x;
        int y = neighbors[i].y;
        if (x < sx || x >= mx || y < sy || y >= my) continue;
        if (map->tiles[x + y*map->width] == RL_TileRock) {
            /* matching neighbor */
            ps[count].x = x;
            ps[count].y = y;
            count ++;
        }
    }

    return count;
}

RL_Status rl_mapgen_maze(RL_Map *map)
{
    RL_ASSERT(map);
    if (map == NULL) return RL_ErrorNullParameter;
    RL_ASSERT(map->width > 2 && map->height > 2);
    memset(map->tiles, RL_TileRock, sizeof(*map->tiles) * map->width * map->height);
    return rl_mapgen_maze_ex(map, 1, 1, map->width - 2, map->height - 2);
}

RL_Status rl_mapgen_maze_ex(RL_Map *map, unsigned int offset_x, unsigned int offset_y, unsigned int width, unsigned int height)
{
    int x, y;
    RL_MapPoint *ps;
    RL_MapPoint *p;
    RL_Heap *heap;

    RL_ASSERT(map);
    RL_ASSERT(width > 0 && height > 0);
    RL_ASSERT(offset_x < width && offset_y < height);
    RL_ASSERT(offset_x < map->width && offset_y < map->height);
    RL_ASSERT(offset_x + width <= map->width && offset_y + height <= map->height);
    RL_ASSERT(offset_x + width < INT_MAX);
    RL_ASSERT(offset_y + height < INT_MAX);

    if (map == NULL) {
        return RL_ErrorNullParameter;
    }

    /* reset all tiles within range to rock */
    for (x = (int)offset_x; x < (int)offset_x + (int)width; ++x) {
        for (y = (int)offset_y; y < (int)offset_y + (int)height; ++y) {
            map->tiles[x + y*map->width] = RL_TileRock;
        }
    }

    /* allocate memory for BFS */
    heap = rl_heap_create(width * height, NULL);
    ps = (RL_MapPoint*) RL_MALLOC(sizeof(*ps) * map->width * map->height);

    RL_ASSERT(ps && heap);
    if (ps == NULL || heap == NULL) {
        return RL_ErrorMemory;
    }

    /* choose random starting tile */
    x = RL_RNG_F(offset_x, offset_x + width - 1);
    y = RL_RNG_F(offset_y, offset_y + height - 1);
    map->tiles[x + y*map->width] = RL_TileCorridor;
    p = &ps[x + y*map->width];
    p->x = x;
    p->y = y;
    rl_heap_insert(heap, p);
    while ((p = (RL_MapPoint*) rl_heap_pop(heap)) != NULL) {
        /* check unvisited neighbors (+2 so we have enough space for walls) */
        RL_MapPoint neighbors[4];
        RL_MapPoint *p2;
        int wall_x, wall_y, i;
        int neighbors_count = rl_mapgen_maze_unvisited_neighbors(neighbors, map, p->x, p->y, offset_x, offset_x + width, offset_y, offset_y + height);
        if (neighbors_count == 0) continue;
        /* choose one unvisitied neighbor */
        i = RL_RNG_F(0, neighbors_count - 1);
        x = neighbors[i].x;
        y = neighbors[i].y;
        RL_ASSERT(rl_map_in_bounds(map, x, y));
        RL_ASSERT(map->tiles[x + y*map->width] == RL_TileRock);
        /* unvisited neighbor - remove wall and push to heap */
        wall_x = x;
        wall_y = y;
        if (x < p->x) wall_x = x + 1;
        if (x > p->x) wall_x = x - 1;
        if (y < p->y) wall_y = y + 1;
        if (y > p->y) wall_y = y - 1;
        map->tiles[wall_x + wall_y*map->width] = RL_TileCorridor;
        map->tiles[x + y*map->width] = RL_TileCorridor;
        p2 = &ps[x + y*map->width];
        p2->x = x;
        p2->y = y;
        rl_heap_insert(heap, p);
        rl_heap_insert(heap, p2);
    }

    /* free memory for BFS */
    rl_heap_destroy(heap);
    RL_FREE(ps);

    return RL_OK;
}

/* custom corridor connection to most efficiently connect leaves of the BSP tree */
void rl_mapgen_connect_corridors_simple(RL_Map *map, RL_BSP *root, bool draw_doors)
{
    /* unsigned int dig_start_x, dig_start_y, dig_end_x, dig_end_y, cur_x, cur_y; */
    unsigned int dig_start_x, dig_start_y, dig_end_x, dig_end_y, cur_x, cur_y;
    int direction, diff_y, diff_x;
    RL_BSP *node, *sibling, *left, *right;

    RL_ASSERT(map && root);
    if (!map || !root) return;

    /* connect siblings */
    node = root->left;
    sibling = root->right;
    if (node == NULL || sibling == NULL) return;

    /* find rooms in BSP */
    left = rl_bsp_random_leaf(node);
    right = rl_bsp_random_leaf(sibling);
#if RL_MAPGEN_BSP_RANDOMISE_ROOM_LOC
    rl_bsp_find_room(map, left, &dig_start_x, &dig_start_y);
#else
    dig_start_x = left->x + left->width / 2;
    dig_start_y = left->y + left->height / 2;
#endif
#if RL_MAPGEN_BSP_RANDOMISE_ROOM_LOC
    rl_bsp_find_room(map, right, &dig_end_x, &dig_end_y);
#else
    dig_end_x = right->x + right->width / 2;
    dig_end_y = right->y + right->height / 2;
#endif
    RL_ASSERT(RL_PASSABLE_F(map, dig_start_x, dig_start_y));
    RL_ASSERT(RL_PASSABLE_F(map, dig_end_x, dig_end_y));
    RL_ASSERT(!(dig_start_x == dig_end_x && dig_start_y == dig_end_y));

    /* carve out corridors */
    cur_x = dig_start_x;
    cur_y = dig_start_y;
    direction = 0;
    diff_y = cur_y - dig_end_y;
    if (diff_y < 0) diff_y *= -1;
    diff_x = cur_x - dig_end_x;
    if (diff_x < 0) diff_x *= -1;
    if (diff_y > diff_x) {
        direction = 1;
    }
    while (cur_x != dig_end_x || cur_y != dig_end_y) {
        /* prevent digging float wide corridors */
        unsigned int next_x, next_y; next_x = cur_x;
        next_y = cur_y;
        if (direction == 0) { /* digging left<->right */
            if (cur_x == dig_end_x) {
                direction = !direction;
            } else {
                next_x += dig_end_x < cur_x ? -1 : 1;
            }
        }
        if (direction == 1) { /* digging up<->down */
            if (cur_y == dig_end_y) {
                direction = !direction;
            } else {
                next_y += dig_end_y < cur_y ? -1 : 1;
            }
        }
        /* dig */
        if (map->tiles[cur_x + cur_y*map->width] == RL_TileRock) {
            if (draw_doors && rl_map_is_room_wall(map, cur_x, cur_y))
                map->tiles[cur_x + cur_y*map->width] = RL_TileDoor;
            else
                map->tiles[cur_x + cur_y*map->width] = RL_TileCorridor;
        }
        cur_x = next_x;
        cur_y = next_y;
    }

    /* connect siblings' children */
    rl_mapgen_connect_corridors_simple(map, node, draw_doors);
    rl_mapgen_connect_corridors_simple(map, sibling, draw_doors);
}


void rl_mapgen_connect_corridors_bsp(RL_Map *map, RL_BSP *root, bool draw_doors);
void rl_mapgen_connect_corridors_randomly(RL_Map *map, RL_BSP *root, bool draw_doors);
RL_Status rl_mapgen_connect_corridors(RL_Map *map, RL_BSP *root, bool draw_doors, RL_MapgenCorridorConnection connection_algorithm)
{
    switch (connection_algorithm) {
        case RL_ConnectNone:
            RL_UNUSED(map);
            RL_UNUSED(root);
            RL_UNUSED(draw_doors);
            break;
        case RL_ConnectSimple:
            rl_mapgen_connect_corridors_simple(map, root, draw_doors);
            break;
        case RL_ConnectRandomly:
#if RL_ENABLE_PATHFINDING
            rl_mapgen_connect_corridors_randomly(map, root, draw_doors);
            {
                /* cull non-connected tiles */
                RL_Graph *floodfill = rl_graph_floodfill_largest_area(map);
                RL_ASSERT(floodfill);
                if (floodfill) {
                    for (size_t x=0; x < map->width; ++x) {
                        for (size_t y=0; y < map->height; ++y) {
                            if (floodfill->nodes[x + y*map->width].score == FLT_MAX) {
                                /* set unreachable tiles to rock */
                                map->tiles[x + y*map->width] = RL_TileRock;
                            }
                        }
                    }
                    rl_graph_destroy(floodfill);
                }
            }
            break;
#endif
        case RL_ConnectBSP:
#if RL_ENABLE_PATHFINDING
            rl_mapgen_connect_corridors_bsp(map, root, draw_doors);
            break;
#endif
        default:
            return RL_ErrorMapgenInvalidConfig;
    }

    return RL_OK;
}

/**
 * Heap functions for pathfinding
 *
 * Ref: https://gist.github.com/skeeto/f012a207aff1753662b679917f706de6
 */

static int rl_heap_noop_comparison_f(const void *_a, const void *_b)
{
    RL_UNUSED(_a);
    RL_UNUSED(_b);
    return 1;
}

RL_Heap *rl_heap_create(int capacity, int (*comparison_f)(const void *heap_item_a, const void *heap_item_b))
{
    RL_Heap *heap;
    heap = (RL_Heap*) RL_MALLOC(sizeof(*heap));
    RL_ASSERT(heap);
    RL_ASSERT(capacity > 0);
    if (heap == NULL) {
        return NULL;
    }
    heap->heap = (void**) RL_MALLOC(sizeof(*heap->heap) * capacity);
    RL_ASSERT(heap->heap);
    if (heap->heap == NULL) {
        RL_FREE(heap);
        return NULL;
    }

    if (comparison_f == NULL) {
        comparison_f = rl_heap_noop_comparison_f;
    }

    heap->cap = capacity;
    heap->comparison_f = comparison_f;
    heap->len = 0;

    return heap;
}

void rl_heap_destroy(RL_Heap *h)
{
    if (h) {
        if (h->heap) {
            RL_FREE(h->heap);
        }
        RL_FREE(h);
    }
}

int rl_heap_length(const RL_Heap *h)
{
    if (h == NULL) return 0;
    return h->len;
}

bool rl_heap_insert(RL_Heap *h, void *item)
{
    int i;
    RL_ASSERT(h != NULL);
    if (h == NULL) return false;

    if (h->len == h->cap) {
        /* resize the heap */
        void **heap_items = (void**) RL_REALLOC(h->heap, sizeof(void*) * h->cap * 2);
        RL_ASSERT(heap_items);
        if (heap_items == NULL) {
            rl_heap_destroy(h);
            return false;
        }
        h->heap = heap_items;
        h->cap *= 2;
    }

    h->heap[h->len] = item;
    for (i = h->len++; i;) {
        void *tmp;
        int p = (i - 1) / 2;
        if (h->comparison_f(h->heap[p], h->heap[i])) {
            break;
        }
        tmp = h->heap[p];
        h->heap[p] = h->heap[i];
        h->heap[i] = tmp;
        i = p;
    }
    return true;
}

static void rl_heap_remove(RL_Heap *h, int index)
{
    int i;
    RL_ASSERT(h);
    if (h == NULL) {
        return;
    }

    h->heap[index] = h->heap[--h->len];
    for (i = index;;) {
        int a = 2*i + 1;
        int b = 2*i + 2;
        int j = i;
        void *tmp;
        if (a < h->len && h->comparison_f(h->heap[a], h->heap[j])) j = a;
        if (b < h->len && h->comparison_f(h->heap[b], h->heap[j])) j = b;
        if (i == j) break;
        tmp = h->heap[j];
        h->heap[j] = h->heap[i];
        h->heap[i] = tmp;
        i = j;
    }
}

void *rl_heap_pop(RL_Heap *h)
{
    void *r;
    if (h == NULL) {
        return NULL;
    }

    r = NULL;
    if (h->len) {
        RL_ASSERT(h->heap);
        r = h->heap[0];
        rl_heap_remove(h, 0);
    }
    return r;
}

void *rl_heap_peek(RL_Heap *h)
{
    if (h == NULL) {
        return NULL;
    }

    RL_ASSERT(h->heap);
    if (h->len) {
        return h->heap[0];
    } else {
        return NULL;
    }
}

#if RL_ENABLE_PATHFINDING
/* simplified distance for side by side nodes */
static float rl_distance_simple(RL_Point node, RL_Point end)
{
    if (node.x == end.x && node.y == end.y) return 0;
    if (node.x == end.x || node.y == end.y) return 1;
    return 1.4;
}

static int rl_scored_graph_heap_comparison(const void *heap_item_a, const void *heap_item_b)
{
    RL_GraphNode *node_a = (RL_GraphNode*) heap_item_a;
    RL_GraphNode *node_b = (RL_GraphNode*) heap_item_b;

    return node_a->score < node_b->score;
}

RL_Path *rl_path(RL_Point p)
{
    RL_Path *path = (RL_Path*) RL_MALLOC(sizeof(*path));
    RL_ASSERT(path);
    if (path == NULL) return NULL;
    path->next = NULL;
    path->point = p;

    return path;
}

float rl_distance_manhattan(RL_Point node, RL_Point end)
{
    return fabs(node.x - end.x) + fabs(node.y - end.y);
}

float rl_distance_euclidian(RL_Point node, RL_Point end)
{
    float distance_x = node.x - end.x;
    float distance_y = node.y - end.y;

    return sqrt(distance_x * distance_x + distance_y * distance_y);
}

float rl_distance_chebyshev(RL_Point node, RL_Point end)
{
    float distance_x = fabs(node.x - end.x);
    float distance_y = fabs(node.y - end.y);

    return distance_x > distance_y ? distance_x : distance_y;
}

/* custom Dijkstra scorer function to prevent carving double wide doors when carving corridors */
static inline float rl_mapgen_corridor_scorer(const RL_GraphNode *current, const RL_GraphNode *neighbor, void *context)
{
    RL_Map *map = (RL_Map*) context;
    RL_Point start = current->point;
    RL_Point end = neighbor->point;
    float r = current->score + rl_distance_manhattan(start, end);

    if (rl_map_tile_is(map, end.x, end.y, RL_TileDoor)) {
        return r; /* doors are passable but count as "walls" - encourage passing through them */
    }
    if (rl_map_is_corner_wall(map, end.x, end.y)) {
        return r + 99; /* discourage double wide corridors & double carving into walls */
    }
    if (RL_WALL_F(map, end.x, end.y)) {
        return r + 9; /* discourage double wide corridors & double carving into walls */
    }

    return r;
}

void rl_mapgen_connect_corridors_bsp_recursive(RL_Map *map, RL_BSP *root, bool draw_doors, RL_Graph *graph)
{
    RL_ASSERT(map && root && graph);
    if (map == NULL || root == NULL || graph == NULL) return;

    /* connect siblings */
    RL_BSP *node = root->left;
    RL_BSP *sibling = root->right;
    if (node == NULL || sibling == NULL) return;

    /* find rooms in BSP */
    unsigned int x, y;
    RL_BSP *leaf = rl_bsp_random_leaf(node);
    rl_bsp_find_room(map, leaf, &x, &y);
    RL_Point dig_start = {x, y};
    RL_ASSERT(RL_PASSABLE_F(map, dig_start.x, dig_start.y));
    leaf = rl_bsp_random_leaf(sibling);
    rl_bsp_find_room(map, leaf, &x, &y);
    RL_Point dig_end = {x, y};
    RL_ASSERT(RL_PASSABLE_F(map, dig_end.x, dig_end.y));
    RL_ASSERT(!(dig_start.x == dig_end.x && dig_start.y == dig_end.y));

    /* carve out corridors */
    rl_dijkstra_score_ex(graph, dig_end, rl_mapgen_corridor_scorer, map);
    RL_Path *path = rl_path_create_from_graph(graph, dig_start);
    RL_ASSERT(path);
    while ((path = rl_path_walk(path))) {
        if (rl_map_tile_is(map, path->point.x, path->point.y, RL_TileRock)) {
            if (rl_map_is_room_wall(map, path->point.x, path->point.y) && draw_doors) {
                map->tiles[(size_t)floor(path->point.x) + (size_t)floor(path->point.y) * map->width] = RL_TileDoor;
            } else {
                map->tiles[(size_t)floor(path->point.x) + (size_t)floor(path->point.y) * map->width] = RL_TileCorridor;
            }
        }
    }

    /* connect siblings' children */
    rl_mapgen_connect_corridors_bsp_recursive(map, node, draw_doors, graph);
    rl_mapgen_connect_corridors_bsp_recursive(map, sibling, draw_doors, graph);
}
void rl_mapgen_connect_corridors_bsp(RL_Map *map, RL_BSP *root, bool draw_doors)
{
    RL_Graph *graph = rl_graph_create_ex(map, NULL, 0);
    RL_ASSERT(graph);
    if (graph) {
        rl_mapgen_connect_corridors_bsp_recursive(map, root, draw_doors, graph);
        rl_graph_destroy(graph);
    }
}

void rl_mapgen_connect_corridors_randomly(RL_Map *map, RL_BSP *root, bool draw_doors)
{
    RL_ASSERT(map && root);
    if (!map || !root) return;

    /* find deepest left-most node */
    RL_BSP *leftmost_node = root;
    while (leftmost_node->left != NULL) {
        leftmost_node = leftmost_node->left;
    }
    RL_ASSERT(leftmost_node && rl_bsp_is_leaf(leftmost_node));
    RL_BSP *node = leftmost_node;
    RL_Graph *graph = rl_graph_create_ex(map, NULL, 0);
    RL_ASSERT(graph);
    if (graph == NULL) return;
    while (node) {
        RL_BSP *sibling;

        /* find random sibling */
        while ((sibling = rl_bsp_random_leaf(root)) == node) {}
        RL_ASSERT(sibling);

        /* TODO need to change this to find the *actual* room (e.g. what if the user provides a map filled with "."?) */
        unsigned int x, y;
        rl_bsp_find_room(map, node, &x, &y);
        RL_ASSERT(RL_PASSABLE_F(map, x, y));
        RL_Point dig_start = {x, y};
        RL_ASSERT(RL_PASSABLE_F(map, dig_start.x, dig_start.y));
        rl_bsp_find_room(map, sibling, &x, &y);
        RL_ASSERT(RL_PASSABLE_F(map, x, y));
        RL_Point dig_end = {x, y};
        RL_ASSERT(RL_PASSABLE_F(map, dig_end.x, dig_end.y));
        RL_ASSERT(!(dig_start.x == dig_end.x && dig_start.y == dig_end.y));

        /* carve out corridors */
        rl_dijkstra_score_ex(graph, dig_end, rl_mapgen_corridor_scorer, map);
        RL_Path *path = rl_path_create_from_graph(graph, dig_start);
        RL_ASSERT(path);
        while ((path = rl_path_walk(path))) {
            if (rl_map_tile_is(map, path->point.x, path->point.y, RL_TileRock)) {
                if (rl_map_is_room_wall(map, path->point.x, path->point.y) && draw_doors) {
                    map->tiles[(size_t)floor(path->point.x) + (size_t)floor(path->point.y) * map->width] = RL_TileDoor;
                } else {
                    map->tiles[(size_t)floor(path->point.x) + (size_t)floor(path->point.y) * map->width] = RL_TileCorridor;
                }
            }
        }

        /* find start node for next loop iteration */
        node = rl_bsp_next_leaf(node);
    }

    rl_graph_destroy(graph);
}


RL_Graph *rl_graph_floodfill_largest_area(const RL_Map *map)
{
    RL_ASSERT(map);
    if (map == NULL) return NULL;
    int *visited = (int*) RL_CALLOC(sizeof(*visited), map->width * map->height);
    RL_ASSERT(visited);
    if (visited == NULL) return NULL;
    RL_Graph *floodfill = NULL; /* largest floodfill */
    int floodfill_scored = 0;
    for (unsigned int x = 0; x < map->width; ++x) {
        for (unsigned int y = 0; y < map->height; ++y) {
            if (RL_PASSABLE_F(map, x, y) && !visited[x + y*map->width]) {
                RL_Graph *test = rl_dijkstra_create(map, RL_XY(x, y), NULL);
                RL_ASSERT(test);
                if (test == NULL) {
                    RL_FREE(visited);
                    if (floodfill) {
                        rl_graph_destroy(floodfill);
                    }
                    return NULL;
                }
                int test_scored = 0;
                for (size_t i = 0; i < test->length; i++) {
                    if (test->nodes[i].score != FLT_MAX) {
                        visited[i] = 1;
                        test_scored ++;
                    }
                }
                if (test_scored > floodfill_scored) {
                    floodfill_scored = test_scored;
                    if (floodfill) {
                        rl_graph_destroy(floodfill);
                    }
                    floodfill = test;
                } else {
                    rl_graph_destroy(test);
                }
            }
        }
    }

    RL_FREE(visited);

    return floodfill;
}


RL_Path *rl_line_create(RL_Point a, RL_Point b, float step)
{
    float delta_x = fabs(a.x - b.x);
    float x_increment = b.x > a.x ? step : -step;
    float delta_y = fabs(a.y - b.y);
    float y_increment = b.y > a.y ? step : -step;
    float error = 0.0;
    float slope = delta_x ? delta_y / delta_x : 0.0;

    RL_Path *head = rl_path(a);
    if (head == NULL) return NULL;
    RL_Path *path = head;
    while (path->point.x != b.x || path->point.y != b.y) {
        RL_Point point = path->point;

        if (delta_x > delta_y) {
            error += slope;
            if (error > 0.5 && point.y != b.y) {
                error -= 1.0;
                point.y += y_increment;
            }

            point.x += x_increment;
        } else {
            error += 1/slope;
            if (error > 0.5 && point.x != b.x) {
                error -= 1.0;
                point.x += x_increment;
            }

            point.y += y_increment;
        }

        /* add new member to linked list & advance */
        path->next = rl_path(point);
        path = path->next;
    }

    return head;
}

RL_Path *rl_path_create(const RL_Map *map, RL_Point start, RL_Point end, RL_DistanceFun distance_f)
{
    RL_Graph *graph = rl_dijkstra_create(map, end, distance_f);
    RL_ASSERT(graph);
    if (graph == NULL) return NULL;
    RL_Path *path = rl_path_create_from_graph(graph, start);
    RL_ASSERT(path);
    rl_graph_destroy(graph);

    return path;
}

RL_Path *rl_path_create_from_graph(const RL_Graph *graph, RL_Point start)
{
    RL_Path *path = rl_path(start);
    RL_Path *path_start = path;
    RL_GraphNode *node = NULL;
    RL_ASSERT(path);
    RL_ASSERT(graph && graph->nodes);
    if (path == NULL || graph == NULL || graph->nodes == NULL) return NULL;
    for (size_t i=0; i<graph->length; i++) {
        if (graph->nodes[i].point.x == start.x && graph->nodes[i].point.y == start.y) {
            node = &graph->nodes[i];
        }
    }
    if (node == NULL) {
        return path;
    }
    while (node != NULL && node->score > 0) {
        node = rl_graph_lowest_scored_neighbor(graph, node);
        if (node == NULL) break;
        path->next = rl_path(node->point);
        RL_ASSERT(path->next);
        if (path->next == NULL) {
            rl_path_destroy(path);
            return NULL;
        }
        path = path->next;
    }

    return path_start;
}

RL_Path *rl_path_walk(RL_Path *path)
{
    if (!path) return NULL;
    RL_Path *next = path->next;
    path->next = NULL;
    RL_FREE(path);

    return next;
}

void rl_path_destroy(RL_Path *path)
{
    if (path) {
        while ((path = rl_path_walk(path))) {}
    }
}

RL_Graph *rl_graph_create(const RL_Map *map)
{
    return rl_graph_create_ex(map, RL_PASSABLE_F, true);
}

RL_Graph *rl_graph_create_ex(const RL_Map *map, RL_PassableFun passable_f, bool allow_diagonal_neighbors)
{
    RL_Graph *graph = (RL_Graph*) RL_MALLOC(sizeof(*graph));
    RL_ASSERT(graph);
    if (graph == NULL) return NULL;
    size_t length = map->width * map->height;
    RL_GraphNode *nodes = (RL_GraphNode*) RL_CALLOC(sizeof(*nodes), length);
    RL_ASSERT(nodes != NULL);
    if (nodes == NULL) {
        RL_FREE(graph);
        return NULL;
    }
    for (unsigned int x=0; x<map->width; x++) {
        for (unsigned int y=0; y<map->height; y++) {
            size_t idx = x + y*map->width;
            RL_GraphNode *node = &nodes[idx];
            node->point.x = (float) x;
            node->point.y = (float) y;
            node->neighbors_length = 0;
            node->score = FLT_MAX;
            /* calculate neighbors */
            RL_Point neighbor_coords[8];
            neighbor_coords[0].x = (int)x + 1;
            neighbor_coords[0].y = (int)y;
            neighbor_coords[1].x = (int)x - 1;
            neighbor_coords[1].y = (int)y;
            neighbor_coords[2].x = (int)x;
            neighbor_coords[2].y = (int)y + 1;
            neighbor_coords[3].x = (int)x;
            neighbor_coords[3].y = (int)y - 1;
            neighbor_coords[4].x = (int)x + 1;
            neighbor_coords[4].y = (int)y + 1;
            neighbor_coords[5].x = (int)x + 1;
            neighbor_coords[5].y = (int)y - 1;
            neighbor_coords[6].x = (int)x - 1;
            neighbor_coords[6].y = (int)y + 1;
            neighbor_coords[7].x = (int)x - 1;
            neighbor_coords[7].y = (int)y - 1;
            for (int i=0; i<8; i++) {
                if (passable_f && !passable_f(map, neighbor_coords[i].x, neighbor_coords[i].y))
                    continue;
                if (!rl_map_in_bounds(map, neighbor_coords[i].x, neighbor_coords[i].y))
                    continue;
                if (!allow_diagonal_neighbors && i >= 4)
                    continue;

                size_t idx = neighbor_coords[i].x + neighbor_coords[i].y*map->width;
                node->neighbors[node->neighbors_length] = &nodes[idx];
                node->neighbors_length++;
            }
        }
    }

    graph->length = length;
    graph->nodes = nodes;

    return graph;
}

RL_Graph *rl_dijkstra_create(const RL_Map *map,
                            RL_Point start,
                            RL_DistanceFun distance_f)
{
    RL_Graph *graph = rl_graph_create(map);
    rl_dijkstra_score(graph, start, distance_f);

    return graph;
}

/* default scorer function for Dijkstra - this simply accepts a RL_DistanceFun as context and adds the current nodes */
/* score to the result of the distance function */
struct rl_score_context { RL_DistanceFun fun; };
float rl_dijkstra_default_score_f(const RL_GraphNode *current, const RL_GraphNode *neighbor, void *context)
{
    struct rl_score_context *distance_f = (struct rl_score_context*) context;

    return current->score + distance_f->fun(current->point, neighbor->point);
}

void rl_dijkstra_score(RL_Graph *graph, RL_Point start, RL_DistanceFun distance_f)
{
    struct { RL_DistanceFun fun; } scorer_context;
    scorer_context.fun = distance_f ? distance_f : rl_distance_simple; /* default to rl_distance_simple */
    rl_dijkstra_score_ex(graph, start, rl_dijkstra_default_score_f, &scorer_context);
}

void rl_dijkstra_score_ex(RL_Graph *graph, RL_Point start, RL_ScoreFun score_f, void *score_context)
{
    RL_ASSERT(graph);
    RL_ASSERT(score_f);
    if (graph == NULL) return;

    RL_GraphNode *current;
    RL_Heap *heap = rl_heap_create(graph->length, &rl_scored_graph_heap_comparison);

    /* reset scores of dijkstra map, setting the start point to 0 */
    for (size_t i=0; i < graph->length; i++) {
        RL_GraphNode *node = &graph->nodes[i];
        if (node->point.x == start.x && node->point.y == start.y) {
            node->score = 0;
            current = node;
        } else {
            node->score = FLT_MAX;
        }
    }

    rl_heap_insert(heap, (void*) current);
    current = (RL_GraphNode*) rl_heap_pop(heap);
    while (current) {
        for (size_t i=0; i<current->neighbors_length; i++) {
            RL_GraphNode *neighbor = current->neighbors[i];
            float distance = score_f(current, neighbor, score_context);
            if (distance < neighbor->score) {
                if (neighbor->score == FLT_MAX) {
                    rl_heap_insert(heap, neighbor);
                }
                neighbor->score = distance;
            }
        }

        current = (RL_GraphNode *) rl_heap_pop(heap);
    }

    rl_heap_destroy(heap);
}

void rl_graph_destroy(RL_Graph *graph)
{
    if (graph) {
        if (graph->nodes) {
            RL_FREE(graph->nodes);
        }
        RL_FREE(graph);
    }
}

bool rl_graph_is_scored(const RL_Graph *graph, RL_Point point)
{
    RL_GraphNode *n = rl_graph_node(graph, point);
    if (n) {
        return n->score < FLT_MAX;
    } else {
        return false;
    }
}

RL_GraphNode *rl_graph_node(const RL_Graph *graph, RL_Point point)
{
    RL_ASSERT(graph);
    if (graph == NULL) return NULL;
    for (unsigned int i=0; i<graph->length; ++i) {
        RL_GraphNode *n = &graph->nodes[i];
        RL_ASSERT(n);
        if (n && n->point.x == point.x && n->point.y == point.y) {
            return n;
        }
    }
    return NULL;
}

RL_GraphNode *rl_graph_lowest_scored_neighbor(const RL_Graph *graph, const RL_GraphNode *node)
{
    RL_ASSERT(graph);
    if (graph == NULL) return NULL;
    RL_GraphNode *lowest_neighbor = NULL;
    for (size_t i=0; i<node->neighbors_length; i++) {
        RL_GraphNode *neighbor = node->neighbors[i];
        if (!lowest_neighbor || neighbor->score < lowest_neighbor->score) {
            lowest_neighbor = neighbor;
        }
    }
    if (lowest_neighbor->score == FLT_MAX) return NULL;
    return lowest_neighbor;
}
#endif /* RL_ENABLE_PATHFINDING */

#if RL_ENABLE_FOV
RL_FOV *rl_fov_create(unsigned int width, unsigned int height)
{
    RL_FOV *fov;
    unsigned char *memory;
    RL_ASSERT(width > 0 && height > 0);
    RL_ASSERT(width != UINT_MAX && !(width > UINT_MAX / height)); /* check for overflow */
    fov = NULL;
    /* allocate all the memory we need at once */
    memory = (unsigned char*) RL_CALLOC(sizeof(*fov) + sizeof(*fov->visibility)*width*height, 1);
    RL_ASSERT(memory);
    if (memory == NULL) return NULL;
    fov = (RL_FOV*) memory;
    fov->width = width;
    fov->height = height;
    fov->visibility = (RL_Byte*) (memory + sizeof(*fov));
    RL_ASSERT(fov);
    RL_ASSERT(fov->visibility);

    return fov;
}

void rl_fov_destroy(RL_FOV *fov)
{
    if (fov) {
        RL_FREE(fov);
    }
}

typedef struct {
    int Y;
    int X;
} RL_Slope;

/* adapted from: https://www.adammil.net/blog/v125_Roguelike_Vision_Algorithms.html#shadowcode (public domain) */
/* also see: https://www.roguebasin.com/index.php/FOV_using_recursive_shadowcasting */
void rl_fov_calculate_recursive(void *map, unsigned int origin_x, unsigned int origin_y, RL_IsInRangeFun in_range_f, RL_IsOpaqueFun opaque_f, RL_MarkAsVisibleFun mark_visible_f, unsigned int octant, float original_x, RL_Slope top, RL_Slope bottom)
{
    int x;
    RL_ASSERT(in_range_f);
    RL_ASSERT(opaque_f);
    RL_ASSERT(mark_visible_f);
    for(x = original_x; x < RL_MAX_RECURSION; x++)
    {
        /* compute the Y coordinates where the top vector leaves the column (on the right) and where the bottom vector */
        /* enters the column (on the left). this equals (x+0.5)*top+0.5 and (x-0.5)*bottom+0.5 respectively, which can */
        /* be computed like (x+0.5)*top+0.5 = (2(x+0.5)*top+1)/2 = ((2x+1)*top+1)/2 to avoid floating point math */
        /* the rounding is a bit tricky, though */
        int topY = top.X == 1 ? x : ((x*2+1) * top.Y + top.X - 1) / (top.X*2); /* the rounding is a bit tricky, though */
        int bottomY = bottom.Y == 0 ? 0 : ((x*2-1) * bottom.Y + bottom.X) / (bottom.X*2);
        int wasOpaque = -1; /* 0:false, 1:true, -1:not applicable */
        int y;
        for(y=topY; y >= bottomY; y--)
        {
            float tx = origin_x, ty = origin_y;
            bool inRange, isOpaque;
            switch(octant) /* translate local coordinates to map coordinates */
            {
                case 0: tx += x; ty -= y; break;
                case 1: tx += y; ty -= x; break;
                case 2: tx -= y; ty -= x; break;
                case 3: tx -= x; ty -= y; break;
                case 4: tx -= x; ty += y; break;
                case 5: tx -= y; ty += x; break;
                case 6: tx += y; ty += x; break;
                case 7: tx += x; ty += y; break;
            }

            inRange = in_range_f(tx, ty, map);
            if(inRange) {
                if (RL_FOV_SYMMETRIC && (y != topY || top.Y*(int)x >= top.X*y) && (y != bottomY || bottom.Y*(int)x <= bottom.X*y)) {
                    mark_visible_f(tx, ty, map);
                } else {
                    mark_visible_f(tx, ty, map);
                }
            }

            if (x == original_x && !inRange) {
                return;
            }

            isOpaque = !inRange || opaque_f(tx, ty, map);
            if(isOpaque)
            {
                if(wasOpaque == 0) /* if we found a transition from clear to opaque, this sector is done in this column, so */
                {                  /* adjust the bottom vector upwards and continue processing it in the next column. */
                    RL_Slope newBottom;
                    newBottom.Y = y*2 + 1; /* (x*2-1, y*2+1) is a vector to the top-left of the opaque tile */
                    newBottom.X = x*2 - 1;
                    if(!inRange || y == bottomY) { bottom = newBottom; break; } /* don't recurse unless we have to */
                    else if (inRange) rl_fov_calculate_recursive(map, origin_x, origin_y, in_range_f, opaque_f, mark_visible_f, octant, x+1, top, newBottom);
                }
                wasOpaque = 1;
            }
            else /* adjust top vector downwards and continue if we found a transition from opaque to clear */
            {    /* (x*2+1, y*2+1) is the top-right corner of the clear tile (i.e. the bottom-right of the opaque tile) */
                if(wasOpaque > 0) {
                    top.Y = y*2 + 1;
                    top.X = x*2 + 1;
                }
                wasOpaque = 0;
            }
        }

        if(wasOpaque != 0) break; /* if the column ended in a clear tile, continue processing the current sector */
    }
}

struct RL_FOVMap {
    RL_FOV *fov;
    const RL_Map *map;
    unsigned int origin_x;
    unsigned int origin_y;
    int fov_radius;
};

void rl_fovmap_mark_visible_f(unsigned int x, unsigned int y, void *context)
{
    struct RL_FOVMap *map = (struct RL_FOVMap*) context;
    if (rl_map_in_bounds(map->map, x, y)) {
        map->fov->visibility[x + y*map->map->width] = RL_TileVisible;
    }
}

bool rl_fovmap_opaque_f(unsigned int x, unsigned int y, void *context)
{
    struct RL_FOVMap *map = (struct RL_FOVMap*) context;
    return RL_OPAQUE_F(map->map, x, y);
}

bool rl_fovmap_in_range_f(unsigned int x, unsigned int y, void *context)
{
    struct RL_FOVMap *map = (struct RL_FOVMap*) context;
#if RL_ENABLE_PATHFINDING
    RL_Point p1, p2;
    p1.x = map->origin_x;
    p1.y = map->origin_y;
    p2.x = x;
    p2.y = y;
    return map->fov_radius < 0 || RL_FOV_DISTANCE_F(p1, p2) <= (float)map->fov_radius;
#else
    /* simplistic manhattan distance distance */
    int diff_x = (int)map->origin_x - (int)x;
    int diff_y = (int)map->origin_y - (int)y;
    if (diff_x < 0) diff_x *= -1;
    if (diff_y < 0) diff_y *= -1;
    return map->fov_radius < 0 || diff_x + diff_y < map->fov_radius;
#endif
}

void rl_fov_calculate(RL_FOV *fov, const RL_Map *map, unsigned int x, unsigned int y, int fov_radius)
{
    struct RL_FOVMap fovmap;
    unsigned int cur_x, cur_y;
    if (!rl_map_in_bounds(map, x, y)) {
        return;
    }
    /* set previously visible tiles to seen */
    for (cur_x=0; cur_x<map->width; ++cur_x) {
        for (cur_y=0; cur_y<map->height; ++cur_y) {
            if (fov->visibility[cur_x + cur_y*map->width] == RL_TileVisible) {
                fov->visibility[cur_x + cur_y*map->width] = RL_TileSeen;
            }
        }
    }
    fovmap.map = map;
    fovmap.fov = fov;
    fovmap.origin_x = x;
    fovmap.origin_y = y;
    fovmap.fov_radius = fov_radius;
    rl_fov_calculate_ex(&fovmap, x, y, rl_fovmap_in_range_f, rl_fovmap_opaque_f, rl_fovmap_mark_visible_f);
}

void rl_fov_calculate_ex(void *context, unsigned int x, unsigned int y, RL_IsInRangeFun in_range_f, RL_IsOpaqueFun opaque_f, RL_MarkAsVisibleFun mark_visible_f)
{
    int octant;
    RL_Slope from = { 1, 1 };
    RL_Slope to = { 0, 1 };
    mark_visible_f(x, y, context);
    for (octant=0; octant<8; ++octant) {
        rl_fov_calculate_recursive(context, x, y, in_range_f, opaque_f, mark_visible_f, octant, 1, from, to);
    }
}

bool rl_fov_is_visible(const RL_FOV *map, unsigned int x, unsigned int y)
{
    RL_ASSERT(map);
    if (map == NULL) return false;
    if (!rl_map_in_bounds((const RL_Map*) map, x, y)) {
        return false;
    }
    return map->visibility[x + y*map->width] == RL_TileVisible;
}

bool rl_fov_is_seen(const RL_FOV *map, unsigned int x, unsigned int y)
{
    RL_ASSERT(map);
    if (map == NULL) return false;
    if (!rl_map_in_bounds((const RL_Map*) map, x, y)) {
        return false;
    }
    return map->visibility[x + y*map->width] == RL_TileSeen;
}
#endif /* if RL_ENABLE_FOV */


#endif /* RL_IMPLEMENTATION */

#ifdef __cplusplus
}
#endif
