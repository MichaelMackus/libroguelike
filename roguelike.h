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
 */

#ifdef __cplusplus
extern "C" {
#endif

#ifndef RL_ROGUELIKE_H
#define RL_ROGUELIKE_H

#include <stdbool.h>
#include <stddef.h>

/* This is a helper since MSVC & c89 don't support compound literals */
#ifndef RL_CLITERAL
#if _MSVC_LANG
#define RL_CLITERAL(type) type
#elif __STDC_VERSION__ < 199409L
#define RL_CLITERAL(type)
#else
#define RL_CLITERAL(type) (type)
#endif
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
    RL_TileDoor = '+'
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
 *
 * To generate a map, create the map via rl_map_create then call the function with the algorithm you wish to use for
 * mapgen. For example:
 *
 *   RL_Map *map = rl_map_create(80, 25);
 *   if (rl_mapgen_bsp(map, RL_MAPGEN_BSP_DEFAULTS) != RL_OK) {
 *     printf("Error occurred during mapgen!\n");
 *   }
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
    RL_ErrorMapgenInvalidCorridorAlgorithm
} RL_Status;

/* Generate map with recursive BSP split algorithm. This fills the map tiles with RL_TileRock before generation. */
RL_Status rl_mapgen_bsp(RL_Map *map, RL_MapgenConfigBSP config);

/* Generates map with recursive BSP split algorithm. This splits the BSP pointer passed. This allocates memory for the
 * BSP children - make sure to use rl_bsp_destroy or free them yourself. Note that this does not set the tiles to
 * RL_TileRock before generation. This way you can have separate regions of the map with different mapgen algorithms. */
RL_Status rl_mapgen_bsp_ex(RL_Map *map, RL_BSP *bsp, const RL_MapgenConfigBSP *config);

/* Connect map via corridors using the supplied BSP graph. */
RL_Status rl_mapgen_connect_corridors(RL_Map *map, RL_BSP *root, bool draw_doors, RL_MapgenCorridorConnection connection_algorithm);

/**
 * Generic map helper functions.
 */

/* Verifies a coordinates is within bounds of map. */
bool rl_map_in_bounds(const RL_Map *map, unsigned int x, unsigned int y);

/* Checks if a tile is passable. */
bool rl_map_is_passable(const RL_Map *map, unsigned int x, unsigned int y);

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
int rl_heap_length(RL_Heap *h);

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
bool rl_bsp_is_leaf(RL_BSP *node);

/* Return sibling node. Returns NULL if there is no parent (i.e. for the root */
/* node). */
RL_BSP *rl_bsp_sibling(RL_BSP *node);

/* Returns amount of leaves in tree. */
size_t rl_bsp_leaf_count(RL_BSP *root);

/* Return the next leaf node to the right if it exists. */
RL_BSP *rl_bsp_next_leaf(RL_BSP *node);

/* Returns a random leaf node beneath root */
RL_BSP* rl_bsp_random_leaf(RL_BSP *root);

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
typedef float (*RL_ScoreFun)(RL_GraphNode *current, RL_GraphNode *neighbor, void *context);

/* Generates a line starting at from ending at to. Each path in the line will be incremented by step. */
RL_Path *rl_line_create(RL_Point from, RL_Point to, float step);

/* Find a path between start and end via Dijkstra algorithm. Make sure to call rl_path_destroy when done with path.
 * Pass NULL to distance_f to use rough approximation for euclidian. */
RL_Path *rl_path_create(const RL_Map *map, RL_Point start, RL_Point end, RL_DistanceFun distance_f, RL_PassableFun passable_f);

/* Find a path between start and end via the scored Dijkstra graph. Make sure to call rl_path_destroy when done with path (or
 * use rl_path_walk). */
RL_Path *rl_path_create_from_graph(const RL_Graph *graph, RL_Point start);

/* Convenience function to "walk" the path. This will return the next path, freeing the current path. You do not need to
 * call rl_path_destroy if you walk the full path. */
RL_Path *rl_path_walk(RL_Path *path);

/* Frees the path & all linked nodes. */
void rl_path_destroy(RL_Path *path);

/* Dijkstra pathfinding algorithm. Pass NULL to distance_f to use rough approximation for euclidian.  Pass NULL to
 * passable_f to pass through impassable tiles, otherwise pass rl_map_is_passable for the default.
 *
 * You can use Dijkstra maps for pathfinding, simple AI, and much more. For example, by setting the player point to
 * "start" then you can pick the highest scored tile in the map and set that as the new "start" point. As with all
 * Dijkstra maps, you just walk the map by picking the lowest scored neighbor. This is a simplistic AI resembling a
 * wounded NPC fleeing from the player.
 *
 * Make sure to destroy the resulting RL_Graph with rl_graph_destroy. */
RL_Graph *rl_dijkstra_create(const RL_Map *map,
                            RL_Point start,
                            RL_DistanceFun distance_f,
                            RL_PassableFun passable_f);

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
RL_Graph *rl_graph_create(const RL_Map *map, RL_PassableFun passable_f, bool allow_diagonal_neighbors);

/* Frees the graph & internal memory. */
void rl_graph_destroy(RL_Graph *graph);

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

/* Function to determine if a tile is within the range of the FOV. */
typedef bool (*RL_IsInRangeFun)(unsigned int x, unsigned int y, void *context);
/* Function to determine if a tile is considered Opaque for FOV calculation. Make sure you do bounds checking that the point is within your map. */
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

/* Define RL_RNG_CUSTOM to provide your own function body for rl_rng_generate. */
unsigned int rl_rng_generate(unsigned int min, unsigned int max);

#ifdef RL_IMPLEMENTATION

#include <stdlib.h>
#include <time.h>
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

#if RL_ENABLE_PATHFINDING
#include <float.h>
#include <math.h>
#endif

#define RL_UNUSED(x) (void)x

#ifndef rl_assert
#include <assert.h>
#define rl_assert(expr)		(assert(expr));
#endif

#ifndef rl_malloc
#define rl_malloc malloc
#endif

#ifndef rl_calloc
#define rl_calloc calloc
#endif

#ifndef rl_realloc
#define rl_realloc realloc
#endif

#ifndef rl_free
#define rl_free free
#endif

RL_Map *rl_map_create(unsigned int width, unsigned int height)
{
    RL_Map *map;
    unsigned char *memory;
    rl_assert(width*height < UINT_MAX);
    rl_assert(width > 0 && height > 0);
    map = NULL;
    /* allocate all the memory we need at once */
    memory = (unsigned char*) rl_calloc(sizeof(*map) + sizeof(*map->tiles)*width*height, 1);
    rl_assert(memory);
    if (memory == NULL) return NULL;
    map = (RL_Map*) memory;
    rl_assert(map);
    map->width = width;
    map->height = height;
    map->tiles = (RL_Byte*) (memory + sizeof(*map));
    rl_assert(map->tiles);
    memset(map->tiles, RL_TileRock, sizeof(*map->tiles)*map->width*map->height);

    return map;
}

void rl_map_destroy(RL_Map *map)
{
    if (map) {
        rl_free(map);
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
               map->tiles[y * map->width + x] == RL_TileDoor;
    }

    return 0;
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
    if (!rl_map_is_passable(map, x, y) || rl_map_tile_is(map, x, y, RL_TileDoor)) {
        return rl_map_is_passable(map, x, y + 1) ||
               rl_map_is_passable(map, x, y - 1) ||
               rl_map_is_passable(map, x + 1, y) ||
               rl_map_is_passable(map, x - 1, y) ||
               rl_map_is_passable(map, x + 1, y - 1) ||
               rl_map_is_passable(map, x - 1, y - 1) ||
               rl_map_is_passable(map, x + 1, y + 1) ||
               rl_map_is_passable(map, x - 1, y + 1);
    }

    return 0;
}

/* checks if target tile is connecting from source (e.g. they can reach it) */
bool rl_map_is_connecting(const RL_Map *map, unsigned int from_x, unsigned int from_y, unsigned int target_x, unsigned int target_y)
{
    /* check that from passable neighbors can connect to target */
    unsigned int x, y, x2, y2;
    for (x = from_x - 1; x <= from_x + 1; ++x) {
        for (y = from_y - 1; y <= from_y + 1; ++y) {
            if (!rl_map_in_bounds(map, x, y) || !rl_map_is_passable(map, x, y))
                continue;
            if (rl_map_tile_is(map, x, y, RL_TileDoor))
                continue;
            /* this is a passable neighbor - check its neighbors to see if it can reach target */
            for (x2 = x - 1; x2 <= x + 1; ++x2) {
                for (y2 = y - 1; y2 <= y + 1; ++y2) {
                    if (!rl_map_in_bounds(map, x2, y2)) continue;
                    if (x2 == target_x && y2 == target_y)
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
    if (!rl_map_is_wall(map, x, y))
        return mask;
    if (rl_map_is_wall(map, x + 1, y    ) && rl_map_is_connecting(map, x, y, x + 1, y))
        mask |= RL_WallToEast;
    if (rl_map_is_wall(map, x - 1, y    ) && rl_map_is_connecting(map, x, y, x - 1, y))
        mask |= RL_WallToWest;
    if (rl_map_is_wall(map, x,     y - 1) && rl_map_is_connecting(map, x, y, x,     y - 1))
        mask |= RL_WallToNorth;
    if (rl_map_is_wall(map, x,     y + 1) && rl_map_is_connecting(map, x, y, x,     y + 1))
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
    if (!rl_map_is_wall(map, x, y))
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

#ifndef RL_RNG_CUSTOM
unsigned int rl_rng_generate(unsigned int min, unsigned int max)
{
    int rnd;

    rl_assert(max >= min);
    rl_assert(max < RAND_MAX);
    rl_assert(max < UINT_MAX);

    if (max < min || max >= RAND_MAX || max >= UINT_MAX)
        return min;
    if (min == max)
        return min;

    rnd = rand();
    if (rnd < 0) rnd = abs(rnd); /* fixes issue on LLVM MOS */

    /* produces more uniformity than using mod */
    return min + rnd / (RAND_MAX / (max - min + 1) + 1);
}
#endif

RL_BSP *rl_bsp_create(unsigned int width, unsigned int height)
{
    RL_BSP *bsp;

    rl_assert(width > 0 && height > 0);
    bsp = (RL_BSP*) rl_calloc(sizeof(*bsp), 1);
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
        rl_free(root);
    }
}

void rl_bsp_split(RL_BSP *node, unsigned int position, RL_SplitDirection direction)
{
    RL_BSP *left, *right;

    /* can't split something already split */
    rl_assert(node->left == NULL && node->right == NULL);

    if (node->left || node->right)
        return;

    if (direction == RL_SplitVertically && position >= node->height)
        return;
    if (direction == RL_SplitHorizontally && position >= node->width)
        return;

    left = (RL_BSP*) rl_calloc(1, sizeof(RL_BSP));
    if (left == NULL)
        return;
    right = (RL_BSP*) rl_calloc(1, sizeof(RL_BSP));
    if (right == NULL) {
        rl_free(left);
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

    rl_assert(root);
    rl_assert(min_width > 0 && min_height > 0 && root != NULL);
    rl_assert(min_width <= root->width && min_height <= root->height);

    if (root == NULL)
        return RL_ErrorNullParameter;
    if (max_recursion <= 0)
        return RL_OK;

    width = root->width;
    height = root->height;

    /* determine split dir & split */
    if (rl_rng_generate(0, 1)) {
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
        rl_free(left);
        rl_free(right);
        root->left = root->right = NULL;
        return ret;
    }

    ret = rl_bsp_recursive_split(right, min_width, min_height, max_recursion - 1);
    if (ret != RL_OK) {
        rl_free(left);
        rl_free(right);
        root->left = root->right = NULL;
        return ret;
    }

    return RL_OK;
}

bool rl_bsp_is_leaf(RL_BSP *node)
{
    if (node == NULL) return 0;
    return (node->left == NULL && node->right == NULL);
}

RL_BSP *rl_bsp_sibling(RL_BSP *node)
{
    if (node && node->parent) {
        if (node->parent->left == node)
            return node->parent->right;
        if (node->parent->right == node)
            return node->parent->left;

        rl_assert("BSP structure is invalid" && 0); /* BSP structure is invalid */
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
RL_BSP *rl_bsp_next_leaf_recursive(RL_BSP *node)
{
    if (node == NULL || node->parent == NULL)
        return NULL;
    if (node->parent->left == node) /* traverse back down */
        return rl_bsp_next_leaf_recursive_down(node->parent->right);
    return rl_bsp_next_leaf_recursive(node->parent);
}
RL_BSP *rl_bsp_next_leaf(RL_BSP *node)
{
    if (node == NULL || node->parent == NULL)
        return NULL;
    rl_assert(rl_bsp_is_leaf(node));

    /* LOOP up until we are on the left, then go back down */
    return rl_bsp_next_leaf_recursive(node);
}
RL_BSP* rl_bsp_random_leaf(RL_BSP *root)
{
    RL_BSP *node;

    if (root == NULL)
        return NULL;

    node = root;
    while (!rl_bsp_is_leaf(node)) {
        if (rl_rng_generate(0, 1)) {
            node = node->left;
        } else {
            node = node->right;
        }
    }

    return node;
}

size_t rl_bsp_leaf_count(RL_BSP *root)
{
    int count;
    RL_BSP *node;
    if (root == NULL) return 0;
    rl_assert(root->parent == NULL);
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
    rl_assert(map && room_width + room_x <= map->width);
    rl_assert(map && room_height + room_y <= map->height);
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
    rl_assert(map);
    rl_assert(room_min_width < room_max_width);
    rl_assert(room_min_height < room_max_height);
    rl_assert(room_max_width + room_padding*2 < UINT_MAX);
    rl_assert(room_max_height + room_padding*2 < UINT_MAX);
    rl_assert(room_min_width > 2 && room_min_height > 2); /* width of 2 can end up having rooms made of nothing but walls */
    rl_assert(node && room_min_width < node->width);
    rl_assert(node && room_min_height < node->height);
    rl_assert(node && room_max_width <= node->width);
    rl_assert(node && room_max_height <= node->height);
    if (map == NULL) return;
    if (node && node->left) {
        if (rl_bsp_is_leaf(node->left)) {
            unsigned int room_width, room_height, room_x, room_y;
            RL_BSP *leaf = node->left;
            room_width = rl_rng_generate(room_min_width, room_max_width);
            if (room_width + room_padding*2 > leaf->width)
                room_width = leaf->width - room_padding*2;
            room_height = rl_rng_generate(room_min_height, room_max_height);
            if (room_height + room_padding*2 > leaf->height)
                room_height = leaf->height - room_padding*2;
#if(RL_MAPGEN_BSP_RANDOMISE_ROOM_LOC)
            room_x = rl_rng_generate(leaf->x + room_padding, leaf->x + leaf->width - room_width - room_padding);
            room_y = rl_rng_generate(leaf->y + room_padding, leaf->y + leaf->height - room_height - room_padding);
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
            room_width = rl_rng_generate(room_min_width, room_max_width);
            if (room_width + room_padding*2 > leaf->width)
                room_width = leaf->width - room_padding*2;
            room_height = rl_rng_generate(room_min_height, room_max_height);
            if (room_height + room_padding*2 > leaf->height)
                room_height = leaf->height - room_padding*2;
#if(RL_MAPGEN_BSP_RANDOMISE_ROOM_LOC)
            room_x = rl_rng_generate(leaf->x + room_padding, leaf->x + leaf->width - room_width - room_padding);
            room_y = rl_rng_generate(leaf->y + room_padding, leaf->y + leaf->height - room_height - room_padding);
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
    rl_assert(map);
    if (map == NULL) return RL_ErrorMemory;
    bsp = rl_bsp_create(map->width, map->height);
    rl_assert(bsp);
    if (bsp == NULL) return RL_ErrorMemory;
    memset(map->tiles, RL_TileRock, sizeof(*map->tiles)*map->width*map->height);
    ret = rl_mapgen_bsp_ex(map, bsp, &config);
    rl_bsp_destroy(bsp);

    return ret;
}

RL_Status rl_mapgen_bsp_ex(RL_Map *map, RL_BSP *root, const RL_MapgenConfigBSP *config)
{
    RL_Status ret;

    rl_assert(map);
    rl_assert(root);
    rl_assert(config->room_min_width > 0 && config->room_max_width >= config->room_min_width && config->room_min_height > 0 && config->room_max_height >= config->room_min_height);
    rl_assert(config->room_max_width <= map->width && config->room_max_height <= map->height);
    rl_assert(config->max_splits > 0);

    if (map == NULL || root == NULL) {
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
    rl_assert(dx && dy);
    rl_assert(map);
    rl_assert(leaf);
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
                    rl_assert(diff_x >= 0 && diff_y >= 0);
                    *dx = start_x + diff_x/2;
                    *dy = start_y + diff_y/2;
                }
            }
        }
    }
}

/* custom corridor connection to most efficiently connect leaves of the BSP tree */
void rl_mapgen_connect_corridors_simple(RL_Map *map, RL_BSP *root, bool draw_doors)
{
    /* unsigned int dig_start_x, dig_start_y, dig_end_x, dig_end_y, cur_x, cur_y; */
    unsigned int dig_start_x, dig_start_y, dig_end_x, dig_end_y, cur_x, cur_y;
    int direction, diff_y, diff_x;
    RL_BSP *node, *sibling, *left, *right;

    rl_assert(map && root);
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
    rl_assert(rl_map_is_passable(map, dig_start_x, dig_start_y));
    rl_assert(rl_map_is_passable(map, dig_end_x, dig_end_y));
    rl_assert(!(dig_start_x == dig_end_x && dig_start_y == dig_end_y));

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
        unsigned int next_x, next_y;
        next_x = cur_x;
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
                rl_assert(floodfill);
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
            return RL_ErrorMapgenInvalidCorridorAlgorithm;
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
    heap = (RL_Heap*) rl_malloc(sizeof(*heap));
    rl_assert(heap);
    if (heap == NULL) {
        return NULL;
    }
    heap->heap = (void**) rl_malloc(sizeof(*heap->heap) * capacity);
    rl_assert(heap->heap);
    if (heap->heap == NULL) {
        rl_free(heap);
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
            rl_free(h->heap);
        }
        rl_free(h);
    }
}

int rl_heap_length(RL_Heap *h)
{
    if (h == NULL) return 0;
    return h->len;
}

bool rl_heap_insert(RL_Heap *h, void *item)
{
    int i;
    rl_assert(h != NULL);
    if (h == NULL) return false;

    if (h->len == h->cap) {
        /* resize the heap */
        void **heap_items = (void**) rl_realloc(h->heap, sizeof(void*) * h->cap * 2);
        rl_assert(heap_items);
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
    rl_assert(h);
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
        rl_assert(h->heap);
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

    rl_assert(h->heap);
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
    RL_Path *path = (RL_Path*) rl_malloc(sizeof(*path));
    rl_assert(path);
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
static inline float rl_mapgen_corridor_scorer(RL_GraphNode *current, RL_GraphNode *neighbor, void *context)
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
    if (rl_map_is_wall(map, end.x, end.y)) {
        return r + 9; /* discourage double wide corridors & double carving into walls */
    }

    return r;
}

void rl_mapgen_connect_corridors_bsp_recursive(RL_Map *map, RL_BSP *root, bool draw_doors, RL_Graph *graph)
{
    rl_assert(map && root && graph);
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
    rl_assert(rl_map_is_passable(map, dig_start.x, dig_start.y));
    leaf = rl_bsp_random_leaf(sibling);
    rl_bsp_find_room(map, leaf, &x, &y);
    RL_Point dig_end = {x, y};
    rl_assert(rl_map_is_passable(map, dig_end.x, dig_end.y));
    rl_assert(!(dig_start.x == dig_end.x && dig_start.y == dig_end.y));

    /* carve out corridors */
    rl_dijkstra_score_ex(graph, dig_end, rl_mapgen_corridor_scorer, map);
    RL_Path *path = rl_path_create_from_graph(graph, dig_start);
    rl_assert(path);
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
    RL_Graph *graph = rl_graph_create(map, NULL, 0);
    rl_assert(graph);
    if (graph) {
        rl_mapgen_connect_corridors_bsp_recursive(map, root, draw_doors, graph);
        rl_graph_destroy(graph);
    }
}

void rl_mapgen_connect_corridors_randomly(RL_Map *map, RL_BSP *root, bool draw_doors)
{
    rl_assert(map && root);
    if (!map || !root) return;

    /* find deepest left-most node */
    RL_BSP *leftmost_node = root;
    while (leftmost_node->left != NULL) {
        leftmost_node = leftmost_node->left;
    }
    rl_assert(leftmost_node && rl_bsp_is_leaf(leftmost_node));
    RL_BSP *node = leftmost_node;
    RL_Graph *graph = rl_graph_create(map, NULL, 0);
    rl_assert(graph);
    if (graph == NULL) return;
    while (node) {
        RL_BSP *sibling;

        /* find random sibling */
        while ((sibling = rl_bsp_random_leaf(root)) == node) {}
        rl_assert(sibling);

        unsigned int x, y;
        rl_bsp_find_room(map, node, &x, &y);
        rl_assert(rl_map_is_passable(map, x, y));
        RL_Point dig_start = {x, y};
        rl_assert(rl_map_is_passable(map, dig_start.x, dig_start.y));
        rl_bsp_find_room(map, sibling, &x, &y);
        rl_assert(rl_map_is_passable(map, x, y));
        RL_Point dig_end = {x, y};
        rl_assert(rl_map_is_passable(map, dig_end.x, dig_end.y));
        rl_assert(!(dig_start.x == dig_end.x && dig_start.y == dig_end.y));

        /* carve out corridors */
        rl_dijkstra_score_ex(graph, dig_end, rl_mapgen_corridor_scorer, map);
        RL_Path *path = rl_path_create_from_graph(graph, dig_start);
        rl_assert(path);
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
    rl_assert(map);
    if (map == NULL) return NULL;
    int *visited = (int*) rl_calloc(sizeof(*visited), map->width * map->height);
    rl_assert(visited);
    if (visited == NULL) return NULL;
    RL_Graph *floodfill = NULL; /* largest floodfill */
    int floodfill_scored = 0;
    for (unsigned int x = 0; x < map->width; ++x) {
        for (unsigned int y = 0; y < map->height; ++y) {
            if (rl_map_is_passable(map, x, y) && !visited[x + y*map->width]) {
                RL_Graph *test = rl_dijkstra_create(map, RL_XY(x, y), NULL, rl_map_is_passable);
                rl_assert(test);
                if (test == NULL) {
                    rl_free(visited);
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

    rl_free(visited);

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

RL_Path *rl_path_create(const RL_Map *map, RL_Point start, RL_Point end, RL_DistanceFun distance_f, RL_PassableFun passable_f)
{
    RL_Graph *graph = rl_dijkstra_create(map, end, distance_f, passable_f);
    rl_assert(graph);
    if (graph == NULL) return NULL;
    RL_Path *path = rl_path_create_from_graph(graph, start);
    rl_assert(path);
    rl_graph_destroy(graph);

    return path;
}

RL_Path *rl_path_create_from_graph(const RL_Graph *graph, RL_Point start)
{
    RL_Path *path = rl_path(start);
    RL_Path *path_start = path;
    RL_GraphNode *node = NULL;
    rl_assert(path);
    rl_assert(graph && graph->nodes);
    if (path == NULL || graph == NULL || graph->nodes == NULL) return NULL;
    for (size_t i=0; i<graph->length; i++) {
        if (graph->nodes[i].point.x == start.x && graph->nodes[i].point.y == start.y) {
            node = &graph->nodes[i];
        }
    }
    if (node == NULL) {
        return path;
    }
    while (node->score > 0) {
        RL_GraphNode *lowest_neighbor = NULL;
        for (size_t i=0; i<node->neighbors_length; i++) {
            RL_GraphNode *neighbor = node->neighbors[i];
            if (!lowest_neighbor || neighbor->score < lowest_neighbor->score) {
                lowest_neighbor = neighbor;
            }
        }
        if (!lowest_neighbor || lowest_neighbor->score == FLT_MAX || node == lowest_neighbor) {
            break; /* no path found */
        }
        node = lowest_neighbor;
        path->next = rl_path(node->point);
        rl_assert(path->next);
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
    rl_free(path);

    return next;
}

void rl_path_destroy(RL_Path *path)
{
    if (path) {
        while ((path = rl_path_walk(path))) {}
    }
}

RL_Graph *rl_graph_create(const RL_Map *map, RL_PassableFun passable_f, bool allow_diagonal_neighbors)
{
    RL_Graph *graph = (RL_Graph*) rl_malloc(sizeof(*graph));
    rl_assert(graph);
    if (graph == NULL) return NULL;
    size_t length = map->width * map->height;
    RL_GraphNode *nodes = (RL_GraphNode*) rl_calloc(sizeof(*nodes), length);
    rl_assert(nodes != NULL);
    if (nodes == NULL) {
        rl_free(graph);
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
            RL_Point neighbor_coords[8] = {
                (RL_Point) { (int)x + 1, (int)y },
                (RL_Point) { (int)x - 1, (int)y },
                (RL_Point) { (int)x,     (int)y + 1 },
                (RL_Point) { (int)x,     (int)y - 1 },
                (RL_Point) { (int)x + 1, (int)y + 1 },
                (RL_Point) { (int)x + 1, (int)y - 1 },
                (RL_Point) { (int)x - 1, (int)y + 1 },
                (RL_Point) { (int)x - 1, (int)y - 1 },
            };
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
                            RL_DistanceFun distance_f,
                            RL_PassableFun passable_f)
{
    RL_Graph *graph = rl_graph_create(map, passable_f, 1);
    rl_dijkstra_score(graph, start, distance_f);

    return graph;
}

/* default scorer function for Dijkstra - this simply accepts a RL_DistanceFun as context and adds the current nodes */
/* score to the result of the distance function */
struct rl_score_context { RL_DistanceFun fun; };
float rl_dijkstra_default_score_f(RL_GraphNode *current, RL_GraphNode *neighbor, void *context)
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
    rl_assert(graph);
    rl_assert(score_f);
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
            rl_free(graph->nodes);
        }
        rl_free(graph);
    }
}
#endif /* RL_ENABLE_PATHFINDING */

#if RL_ENABLE_FOV
RL_FOV *rl_fov_create(unsigned int width, unsigned int height)
{
    RL_FOV *fov;
    unsigned char *memory;
    rl_assert(width > 0 && height > 0);
    rl_assert(width != UINT_MAX && !(width > UINT_MAX / height)); /* check for overflow */
    fov = NULL;
    /* allocate all the memory we need at once */
    memory = (unsigned char*) rl_calloc(sizeof(*fov) + sizeof(*fov->visibility)*width*height, 1);
    rl_assert(memory);
    if (memory == NULL) return NULL;
    fov = (RL_FOV*) memory;
    fov->width = width;
    fov->height = height;
    fov->visibility = (RL_Byte*) (memory + sizeof(*fov));
    rl_assert(fov);
    rl_assert(fov->visibility);

    return fov;
}

void rl_fov_destroy(RL_FOV *fov)
{
    if (fov) {
        rl_free(fov);
    }
}

typedef struct {
    int Y;
    int X;
} RL_Slope;

/* adapted from: https://www.adammil.net/blog/v125_Roguelike_Vision_Algorithms.html#shadowcode (public domain) */
/* also see: https://www.roguebasin.com/index.php/FOV_using_recursive_shadowcasting */
void rl_fov_calculate_recursive(void *map, unsigned int origin_x, unsigned int origin_y, RL_IsInRangeFun in_range_f, RL_IsOpaqueFun opaque_f, RL_MarkAsVisibleFun mark_visible_f, unsigned int octant, float x, RL_Slope top, RL_Slope bottom)
{
    rl_assert(in_range_f);
    rl_assert(opaque_f);
    rl_assert(mark_visible_f);
    for(; x < RL_MAX_RECURSION; x++)
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

            isOpaque = !inRange || !opaque_f(tx, ty, map);
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
    if (!rl_map_in_bounds(map->map, x, y)) {
        return true;
    }
    return rl_map_is_passable(map->map, x, y);
}

#ifndef RL_FOV_DISTANCE_F
#define RL_FOV_DISTANCE_F rl_distance_euclidian
#endif
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
    rl_assert(map);
    if (map == NULL) return false;
    if (!rl_map_in_bounds((const RL_Map*) map, x, y)) {
        return false;
    }
    return map->visibility[x + y*map->width] == RL_TileVisible;
}

bool rl_fov_is_seen(const RL_FOV *map, unsigned int x, unsigned int y)
{
    rl_assert(map);
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
