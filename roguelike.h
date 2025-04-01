#ifndef RL_ROGUELIKE_H
#define RL_ROGUELIKE_H

#include <stdlib.h>
#include <stdbool.h>

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

/**
 * Generic structs for library.
 */

typedef enum {
    RL_TileRock = 0,
    RL_TileRoom,
    RL_TileCorridor,
    RL_TileDoor,
} RL_Tile;

typedef enum {
    RL_TileCannotSee = 0,
    RL_TileVisible,
    RL_TileSeen,
} RL_TileVisibility;

// Generic dungeon map structure, supporting hex & square 2d maps
typedef struct RL_Map {
    int width;
    int height;
    RL_Tile *tiles; // 2d array of Width*Height for 2d square map - by default 0 is the impassable/rock tile
    RL_TileVisibility *visibility; // 2d array of Width*Height for checking tile visibility - must call rl_fov first to update visibility
} RL_Map;

typedef struct RL_Point {
    double x, y;
} RL_Point;
#define RL_XY(x, y) (RL_Point) { (double)x, (double)y }

// BSP tree
typedef struct RL_BSP {
    int width;
    int height;
    int x;
    int y;
    struct RL_BSP *parent;
    struct RL_BSP *left;  // left child
    struct RL_BSP *right; // right child
} RL_BSP;

// BSP split direction
typedef enum {
    RL_SplitHorizontally, // split the BSP node on the x axis (splits width)
    RL_SplitVertically,   // split the BSP node on the y axis (splits height)
} RL_SplitDirection;

// Type of wall - idea is they can be bitmasked together (e.g. for corners)
typedef enum {
    RL_WallToWest  = 1,
    RL_WallToEast  = 1 << 1,
    RL_WallToNorth = 1 << 2,
    RL_WallToSouth = 1 << 3,
    RL_WallOther          = 1 << 7, // e.g. a wall that has no connecting walls
} RL_Wall;

// Represents a graph of nodes that has been scored for pathfinding (e.g. with the Dijkstra algorithm).
#ifndef RL_MAX_NEIGHBOR_COUNT
#define RL_MAX_NEIGHBOR_COUNT 8
#endif
typedef struct RL_GraphNode {
    double score; // will be DBL_MAX for an unreachable/unscored node in Dijkstra
    RL_Point point;
    size_t neighbors_length;
    struct RL_GraphNode *neighbors[RL_MAX_NEIGHBOR_COUNT];
} RL_GraphNode;
typedef struct RL_Graph {
    size_t length; // length of nodes
    RL_GraphNode *nodes; // array of nodes - length will be the size of the map.width * map.height
} RL_Graph;

typedef struct RL_Path {
    RL_Point point;
    struct RL_Path *next;
} RL_Path;

/**
 * Random map generation
 */

// Creates empty map and fills it with impassable tiles (width & height
// must be positive). Make sure to call rl_map_destroy to clear memory.
RL_Map rl_map_create(int width, int height);

// Initialize map & allocates map->tiles if NULL. Called implicitly by rl_map_create.
void rl_map_init(RL_Map *map, int width, int height);

// Frees map tile memory.
void rl_map_destroy(RL_Map *map);

typedef struct {
    int room_min_width;
    int room_max_width;
    int room_min_height;
    int room_max_height;
    int room_padding;
    int draw_corridors;
    int draw_doors;
    int use_secret_passages;
} RL_MapgenConfigBSP;

// Generate map rooms with BSP split algorithm & connects corridors. Make sure to free the BSP with rl_bsp_destroy.
RL_BSP rl_mapgen_bsp(RL_Map *map, RL_MapgenConfigBSP config);

/**
 * Generic map helper functions.
 */

// Verifies a coordinates is within bounds of map.
int rl_map_in_bounds(const RL_Map *map, RL_Point point);

// Checks if a tile is passable. Define RL_PASSABLE_F to define a custom
// function definition for pathfinding.
int rl_map_is_passable(const RL_Map *map, RL_Point point);

// Get tile at point
RL_Tile *rl_map_tile(const RL_Map *map, RL_Point point);

// Returns 1 if tile at point matches given parameter.
int rl_map_tile_is(const RL_Map *map, RL_Point point, RL_Tile tile);

// A tile is considered a wall if it is touching a passable tile.
//
// Returns a bitmask of the RL_Wall enum. For example, a wall with a wall
// tile to the south, west, and east would have a bitmask of 0b1011.
int rl_map_wall(const RL_Map *map, RL_Point point);

// Is the wall a corner?
int rl_map_is_corner_wall(const RL_Map *map, RL_Point point);

// Is this a wall that is touching a room tile?
int rl_map_is_room_wall(const RL_Map *map, RL_Point point);

// A wall that is touching a room tile (e.g. to display it lit).
int rl_map_room_wall(const RL_Map *map, RL_Point point);

// Checks if a point is visible within FOV. Make sure to call rl_map_fov_calculate first.
int rl_map_is_visible(const RL_Map *map, RL_Point point);

// Checks if a point has been seen within FOV. Make sure to call rl_map_fov_calculate first.
int rl_map_is_seen(const RL_Map *map, RL_Point point);

// Returns a the largest connected area (of passable tiles) on the map.
RL_Graph rl_map_largest_connected_area(const RL_Map *map);

/**
 * Simple priority queue implementation
 */

typedef struct {
    void **heap;
    int cap;
    int len;
    int (*comparison_f)(const void *heap_item_a, const void *heap_item_b);
} RL_Heap;

// Allocates memory for the heap. Make sure to call rl_heap_destroy after you are done.
//
// capacity - initial capacity for the heap
// comparison_f - A comparison function that returns 1 if heap_item_a should be
//  popped from the queue before heap_item_b. If NULL the heap will still work
//  but order will be undefined.
RL_Heap rl_heap_create(int capacity, int (*comparison_f)(const void *heap_item_a, const void *heap_item_b));

// Free up the allocated heap memory.
void rl_heap_destroy(RL_Heap *h);

// Insert item into the heap. This will resize the heap if necessary.
int rl_heap_insert(RL_Heap *h, void *item);

// Returns & removes an item from the queue.
void *rl_heap_pop(RL_Heap *h);

/**
 * BSP Manipulation
 */

// Params width & height must be positive.
RL_BSP rl_bsp_create(int width, int height);
void rl_bsp_destroy(RL_BSP *root);

// Split the BSP by direction - this creates the left & right leaf and
// populates them in the BSP node. Position must be positive and within
// the BSP root node. Also node->left & node->right must be NULL
void rl_bsp_split(RL_BSP *node, int position, RL_SplitDirection direction);

// Recursively split the BSP. Used for map generation.
void rl_bsp_recursive_split(RL_BSP *root, int min_width, int min_height, int max_recursion);

// Returns 1 if the node is a leaf node.
int rl_bsp_is_leaf(RL_BSP *node);

// Return sibling node. Returns NULL if there is no parent (i.e. for the root
// node).
RL_BSP *rl_bsp_sibling(RL_BSP *node);

// Return the next node to the right (at the same depth) if it exists.
RL_BSP *rl_bsp_next_node(RL_BSP *node);

/**
 * Pathfinding
 */

// Useful distance functions for pathfinding.
double rl_distance_manhattan(RL_Point node, RL_Point end);
double rl_distance_euclidian(RL_Point node, RL_Point end);
double rl_distance_chebyshev(RL_Point node, RL_Point end);

// Custom distance function for pathfinding - calculates distance between map nodes
typedef double (*RL_DistanceFun)(RL_Point from, RL_Point to);

// Custom passable function for pathfinding. Return 0 to prevent neighbor from being included in graph.
typedef int (*RL_PassableFun)(const RL_Map *map, RL_Point point);

// Custom score function for pathfinding - most users won't need this, but it gives flexibility in weighting the
// Dijkstra graph. Note that Dijkstra expects you to add the current node's score to the newly calculated score.
typedef double (*RL_ScoreFun)(RL_GraphNode *current, RL_GraphNode *neighbor, void *context);

// Find a path between start and end via Dijkstra algorithm. Make sure to call rl_path_destroy when done with path.
// Pass NULL to distance_f to use rough approximation for euclidian.
RL_Path *rl_path_create(const RL_Map *map, RL_Point start, RL_Point end, RL_DistanceFun distance_f, RL_PassableFun passable_f);

// Find a path between start and end via the scored Dijkstra graph. Make sure to call rl_path_destroy when done with path (or
// use rl_path_walk).
RL_Path *rl_path_create_from_graph(const RL_Graph *graph, RL_Point start);

// Convenience function to "walk" the path. This will return the next path, freeing the current path. You do not need to
// call rl_path_destroy if you walk the full path.
RL_Path *rl_path_walk(RL_Path *path);

// Destroy & clean up all nodes from path onward.
void rl_path_destroy(RL_Path *path);

// Dijkstra pathfinding algorithm. Pass NULL to distance_f to use rough approximation for euclidian.  Pass NULL to
// passable_f to pass through impassable tiles, otherwise pass rl_map_is_passable for the default.
//
// You can use Dijkstra maps for pathfinding, simple AI, and much more. For example, by setting the player point to
// "start" then you can pick the highest scored tile in the map and set that as the new "start" point. As with all
// Dijkstra maps, you just walk the map by picking the lowest scored neighbor. This is a simplistic AI resembling a
// wounded NPC fleeing from the player.
//
// Make sure to destroy the resulting RL_Graph with rl_graph_destroy.
RL_Graph rl_dijkstra_create(const RL_Map *map,
                            RL_Point start,
                            RL_DistanceFun distance_f,
                            RL_PassableFun passable_f);

// Dijkstra pathfinding algorithm. Uses RL_Graph so that your code doesn't need to rely on RL_Map. Each node's
// distance should equal DBL_MAX in the resulting graph if it is impassable.
void rl_dijkstra_score(RL_Graph *graph, RL_Point start, RL_DistanceFun distance_f);

// Dijkstra pathfinding algorithm for advanced use cases such as weighting certain tiles higher than others. Uses
// RL_Graph so that your code doesn't need to rely on RL_Map. Each node's distance should equal DBL_MAX in the resulting
// graph if it is impassable. Most users should just use rl_dijkstra_score - only use this if you have a specific need.
void rl_dijkstra_score_ex(RL_Graph *graph, RL_Point start, RL_ScoreFun score_f, void *score_context);

// Create an unscored graph based on the 2d map. Make sure to call rl_graph_destroy when finished.
RL_Graph rl_graph_create(const RL_Map *map, RL_PassableFun passable_f, int allow_diagonal_neighbors);

// Free graph memory.
void rl_graph_destroy(RL_Graph *graph);

/**
 * FOV
 */

// Calculate FOV using simple shadowcasting algorithm. Set symmetric to 1 if you want the algorithm to be symmetrical
// Set symmetric to 1 if you want the algorithm to be symmetrical.
//
// Note that this sets previously visible tiles to RL_TileSeen.
void rl_map_fov_calculate(RL_Map *map, RL_Point start, int fov_radius, int symmetric, RL_DistanceFun distance_f);

/**
 * Random number generation
 */

// Uses stdlib by default. Define RL_RAND_F to implement your own implementation
// (e.g. via another algorithm such as mtwister).
void rl_rng_seed(unsigned int seed);

// Free up internal RNG buffer(s) (for user-defined algorithms such as mtwister).
void rl_rng_destroy();

// Generate random number from min to max (inclusive).
unsigned long rl_rng_generate(unsigned long min, unsigned long max);

#ifdef RL_IMPLEMENTATION

#include <time.h>
#include <stdint.h>
#include <string.h>
#include <math.h>
#include <float.h>

#ifndef RL_MAPGEN_MAX_RECURSION
#define RL_MAPGEN_MAX_RECURSION 100
#endif

#ifndef RL_MAPGEN_BSP_DEVIATION
#define RL_MAPGEN_BSP_DEVIATION 0
#endif

#define RL_UNUSED(x) (void)x

#ifndef rl_assert
#include <assert.h>
#define rl_assert(expr)		(assert(expr));
#endif

RL_Map rl_map_create(int width, int height)
{
    RL_Map map = { 0 };
    rl_assert(width > 0 && height > 0);
    rl_map_init(&map, width, height);
    rl_assert(map.tiles);
    rl_assert(map.visibility);

    return map;
}

void rl_map_destroy(RL_Map *map)
{
    if (map) {
        if (map->tiles) {
            free(map->tiles);
            map->tiles = NULL;
        }
        if (map->visibility) {
            free(map->visibility);
            map->visibility = NULL;
        }
    }
}

void rl_map_init(RL_Map *map, int width, int height)
{
    rl_assert(map);
    map->width = width;
    map->height = height;
    if (map && map->tiles == NULL) {
        map->tiles = calloc(map->width * map->height, sizeof(*map->tiles));
    }
    if (map && map->visibility == NULL) {
        map->visibility = calloc(map->width * map->height, sizeof(*map->tiles));
    }
}

int rl_map_in_bounds(const RL_Map *map, RL_Point point)
{
    return point.x >= 0 && point.y >= 0 && point.x < map->width && point.y < map->height;
}

int rl_map_is_passable(const RL_Map *map, RL_Point point)
{
    if (rl_map_in_bounds(map, point)) {
        return map->tiles[(int)point.y * map->width + (int)point.x] != RL_TileRock;
    }

    return 0;
}

RL_Tile *rl_map_tile(const RL_Map *map, RL_Point point)
{
    if (rl_map_in_bounds(map, point)) {
        return &map->tiles[(int)point.x + (int)point.y*map->width];
    }

    return NULL;
}

int rl_map_is_wall(const RL_Map *map, RL_Point point)
{
    int y = point.y;
    int x = point.x;
    if (!rl_map_in_bounds(map, point))
        return 0;
    if (!rl_map_is_passable(map, point) || rl_map_tile_is(map, point, RL_TileDoor)) {
        return rl_map_is_passable(map, (RL_Point){ x, y + 1 }) ||
               rl_map_is_passable(map, (RL_Point){ x, y - 1 }) ||
               rl_map_is_passable(map, (RL_Point){ x + 1, y }) ||
               rl_map_is_passable(map, (RL_Point){ x - 1, y }) ||
               rl_map_is_passable(map, (RL_Point){ x + 1, y - 1 }) ||
               rl_map_is_passable(map, (RL_Point){ x - 1, y - 1 }) ||
               rl_map_is_passable(map, (RL_Point){ x + 1, y + 1 }) ||
               rl_map_is_passable(map, (RL_Point){ x - 1, y + 1 });
    }

    return 0;
}

// TODO check for doors
int rl_map_wall(const RL_Map *map, RL_Point point)
{
    int mask = 0;
    if (!rl_map_is_wall(map, point))
        return mask;
    if (rl_map_is_wall(map, RL_XY(point.x + 1, point.y)))
        mask |= RL_WallToEast;
    if (rl_map_is_wall(map, RL_XY(point.x - 1, point.y)))
        mask |= RL_WallToWest;
    if (rl_map_is_wall(map, RL_XY(point.x,     point.y - 1)))
        mask |= RL_WallToNorth;
    if (rl_map_is_wall(map, RL_XY(point.x,     point.y + 1)))
        mask |= RL_WallToSouth;
    return mask ? mask : RL_WallOther;
}

int rl_map_is_corner_wall(const RL_Map *map, RL_Point point)
{
    int wall = rl_map_wall(map, point);
    if (!wall) return 0;
    return (wall & RL_WallToWest && wall & RL_WallToNorth) ||
           (wall & RL_WallToWest && wall & RL_WallToSouth) ||
           (wall & RL_WallToEast && wall & RL_WallToNorth) ||
           (wall & RL_WallToEast && wall & RL_WallToSouth);
}

int rl_map_tile_is(const RL_Map *map, RL_Point point, RL_Tile tile)
{
    if (!rl_map_in_bounds(map, point)) return 0;
    return map->tiles[(int)point.x + (int)point.y*map->width] == tile;
}

int rl_map_is_room_wall(const RL_Map *map, RL_Point point)
{
    int y = point.y;
    int x = point.x;
    if (!rl_map_is_wall(map, point))
        return 0;

    return rl_map_tile_is(map, (RL_Point){ x, y + 1 }, RL_TileRoom) ||
           rl_map_tile_is(map, (RL_Point){ x, y - 1 }, RL_TileRoom) ||
           rl_map_tile_is(map, (RL_Point){ x + 1, y }, RL_TileRoom) ||
           rl_map_tile_is(map, (RL_Point){ x - 1, y }, RL_TileRoom) ||
           rl_map_tile_is(map, (RL_Point){ x + 1, y - 1 }, RL_TileRoom) ||
           rl_map_tile_is(map, (RL_Point){ x - 1, y - 1 }, RL_TileRoom) ||
           rl_map_tile_is(map, (RL_Point){ x + 1, y + 1 }, RL_TileRoom) ||
           rl_map_tile_is(map, (RL_Point){ x - 1, y + 1 }, RL_TileRoom);
}

int rl_map_room_wall(const RL_Map *map, RL_Point point)
{
    int mask = 0;
    if (!rl_map_is_room_wall(map, point))
        return mask;
    if (rl_map_is_room_wall(map, RL_XY(point.x + 1, point.y)))
        mask |= RL_WallToEast;
    if (rl_map_is_room_wall(map, RL_XY(point.x - 1, point.y)))
        mask |= RL_WallToWest;
    if (rl_map_is_room_wall(map, RL_XY(point.x,     point.y - 1)))
        mask |= RL_WallToNorth;
    if (rl_map_is_room_wall(map, RL_XY(point.x,     point.y + 1)))
        mask |= RL_WallToSouth;
    return mask ? mask : RL_WallOther;
}

#ifndef RL_RAND_F
void rl_rng_seed(unsigned int seed)
{
    if (seed) srand(seed);
    else      srand(time(0));
}

unsigned long rl_rng_generate(unsigned long min, unsigned long max)
{
    rl_assert(max >= min);
    rl_assert(max < RAND_MAX);

    if (max < min || max >= RAND_MAX)
        return min;

    return min + rand() / (RAND_MAX / (max - min + 1) + 1);
}

void rl_rng_destroy()
{
}
#endif

RL_BSP rl_bsp_create(int width, int height)
{
    rl_assert(width > 0 && height > 0);

    return (RL_BSP) { .width = width, .height = height };
}
void rl_bsp_destroy(RL_BSP* root)
{
    if (root) {
        if (root->left) {
            rl_bsp_destroy(root->left);
            free(root->left);
            root->left = NULL;
        }
        if (root->right) {
            rl_bsp_destroy(root->right);
            free(root->right);
            root->right = NULL;
        }
    }
}

void rl_bsp_split(RL_BSP *node, int position, RL_SplitDirection direction)
{
    // can't split something already split
    rl_assert(node->left == NULL && node->right == NULL);

    if (node->left || node->right)
        return;

    if (direction == RL_SplitVertically && position >= node->height)
        return;
    if (direction == RL_SplitHorizontally && position >= node->width)
        return;

    RL_BSP *left = calloc(1, sizeof(RL_BSP));
    if (left == NULL)
        return;
    RL_BSP *right = calloc(1, sizeof(RL_BSP));
    if (right == NULL) {
        free(left);
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
        right->y = node->y;
        right->y += position;
    } else {
        left->width = position;
        left->height = node->height;
        left->x = node->x;
        left->y = node->y;
        right->width = node->width - position;
        right->height = node->height;
        right->x = node->x;
        right->y = node->y;
        right->x += position;
    }

    left->parent = right->parent = node;
    node->left = left;
    node->right = right;
}

void rl_bsp_recursive_split(RL_BSP *root, int min_width, int min_height, int max_recursion)
{
    rl_assert(min_width > 0 && min_height > 0 && root != NULL);

    if (max_recursion <= 0)
        return;

    rl_assert(RL_MAPGEN_BSP_DEVIATION >= 0.0 && RL_MAPGEN_BSP_DEVIATION <= 1.0);

    int width = root->width;
    int height = root->height;
    int max_width = width - min_width;
    int max_height = height - min_height;

    // determine split dir & split
    RL_SplitDirection dir;
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

    int split_position;
    if (dir == RL_SplitHorizontally) {
        // cannot split if current node size is too small
        if (width < min_width*2)
            return;

        int center = width / 2;
        int from = center - (center * RL_MAPGEN_BSP_DEVIATION);
        int to = center + (center * RL_MAPGEN_BSP_DEVIATION);
        if (from < min_width)
            from = min_width;
        if (to > max_width)
            to = max_width;

        split_position = rl_rng_generate(from, to);
    } else {
        // cannot split if current node size is too small
        if (height < min_height*2)
            return;

        int center = height / 2;
        int from = center - (center * RL_MAPGEN_BSP_DEVIATION);
        int to = center + (center * RL_MAPGEN_BSP_DEVIATION);
        if (from < min_height)
            from = min_height;
        if (to > max_height)
            to = max_height;

        split_position = rl_rng_generate(from, to);
    }

    rl_bsp_split(root, split_position, dir);

    // continue recursion
    RL_BSP *left = root->left;
    RL_BSP *right = root->right;

    if (left == NULL || right == NULL)
        return;

    rl_bsp_recursive_split(left, min_width, min_height, max_recursion - 1);
    rl_bsp_recursive_split(right, min_width, min_height, max_recursion - 1);
}

int rl_bsp_is_leaf(RL_BSP *node)
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

        rl_assert(1 != 1); // BSP structure is invalid
    }

    return NULL;
}

RL_BSP *rl_bsp_next_node_recursive_down(RL_BSP *node, int depth)
{
    if (node == NULL)
        return NULL;
    if (depth == 0) // found the node
        return node;
    if (node->left == NULL)
        return NULL;
    return rl_bsp_next_node_recursive_down(node->left, depth + 1);
}
RL_BSP *rl_bsp_next_node_recursive(RL_BSP *node, int depth)
{
    if (node == NULL || node->parent == NULL)
        return NULL;
    if (node->parent->left == node) // traverse back down
        return rl_bsp_next_node_recursive_down(node->parent->right, depth);
    return rl_bsp_next_node_recursive(node->parent, depth - 1);
}
RL_BSP *rl_bsp_next_node(RL_BSP *node)
{
    if (node == NULL || node->parent == NULL)
        return NULL;

    // LOOP up until we are on the left, then go back down
    return rl_bsp_next_node_recursive(node, 0);
}

static void rl_map_bsp_generate_room(RL_Map *map, int room_width, int room_height, RL_Point room_loc)
{
    for (int x = room_loc.x; x < room_loc.x + room_width; ++x) {
        for (int y = room_loc.y; y < room_loc.y + room_height; ++y) {
            if (x == room_loc.x || x == room_loc.x + room_width - 1 ||
                    y == room_loc.y || y == room_loc.y + room_height - 1
               ) {
                // set sides of room to walls
                map->tiles[y*map->width + x] = 0;
            } else {
                map->tiles[y*map->width + x] = RL_TileRoom;
            }
        }
    }
}
static void rl_map_bsp_generate_rooms(RL_BSP *node, RL_Map *map, int room_min_width, int room_max_width, int room_min_height, int room_max_height, int room_padding)
{
    if (node && node->left) {
        if (rl_bsp_is_leaf(node->left)) {
            int room_width, room_height;
            RL_Point room_loc;
            RL_BSP *leaf = node->left;
            room_width = rl_rng_generate(room_min_width, room_max_width);
            if (room_width + room_padding*2 > leaf->width)
                room_width = leaf->width - room_padding*2;
            room_height = rl_rng_generate(room_min_height, room_max_height);
            if (room_height + room_padding*2 > leaf->height)
                room_height = leaf->height - room_padding*2;
            room_loc.x = rl_rng_generate(leaf->x + room_padding, leaf->x + leaf->width - room_width - room_padding);
            room_loc.y = rl_rng_generate(leaf->y + room_padding, leaf->y + leaf->height - room_height - room_padding);

            rl_map_bsp_generate_room(map, room_width, room_height, room_loc);
        } else {
            rl_map_bsp_generate_rooms(node->left, map, room_min_width, room_max_width, room_min_height, room_max_height, room_padding);
        }
    }
    if (node && node->right) {
        if (rl_bsp_is_leaf(node->left)) {
            int room_width, room_height;
            RL_Point room_loc;
            RL_BSP *leaf = node->right;
            room_width = rl_rng_generate(room_min_width, room_max_width);
            if (room_width + room_padding*2 > leaf->width)
                room_width = leaf->width - room_padding*2;
            room_height = rl_rng_generate(room_min_height, room_max_height);
            if (room_height + room_padding*2 > leaf->height)
                room_height = leaf->height - room_padding*2;
            room_loc.x = rl_rng_generate(leaf->x + room_padding, leaf->x + leaf->width - room_width - room_padding);
            room_loc.y = rl_rng_generate(leaf->y + room_padding, leaf->y + leaf->height - room_height - room_padding);

            rl_map_bsp_generate_room(map, room_width, room_height, room_loc);
        } else {
            rl_map_bsp_generate_rooms(node->right, map, room_min_width, room_max_width, room_min_height, room_max_height, room_padding);
        }
    }
}

static void rl_mapgen_bsp_connect_corridors(RL_Map *map, RL_BSP *root, int draw_doors);
RL_BSP rl_mapgen_bsp(RL_Map *map, RL_MapgenConfigBSP config)
{
    rl_assert(map);
    rl_assert(config.room_min_width > 0 && config.room_max_width >= config.room_min_width && config.room_min_height > 0 && config.room_max_height >= config.room_min_height && config.room_padding >= 0);

    RL_BSP root = rl_bsp_create(map->width, map->height);

    rl_bsp_recursive_split(&root, config.room_max_width + config.room_padding, config.room_max_height + config.room_padding, RL_MAPGEN_MAX_RECURSION);
    rl_map_bsp_generate_rooms(&root, map, config.room_min_width, config.room_max_width, config.room_min_height, config.room_max_height, config.room_padding);

    if (config.draw_corridors) {
        rl_mapgen_bsp_connect_corridors(map, &root, config.draw_doors);
    }

    // if (config.use_secret_passages) {
        // TODO connect secret passages
    // }

    return root;
}

// custom Dijkstra scorer function to prevent carving double wide doors when carving corridors
static inline double rl_mapgen_corridor_scorer(RL_GraphNode *current, RL_GraphNode *neighbor, void *context)
{
    RL_Map *map = context;
    RL_Point start = current->point;
    RL_Point end = neighbor->point;
    double r = current->score + rl_distance_manhattan(start, end);

    if (rl_map_tile_is(map, end, RL_TileDoor)) {
        return r; // doors are passable but count as "walls" - encourage passing through them
    }
    if (rl_map_is_corner_wall(map, end)) {
        return r + 99; // discourage double wide corridors & double carving into walls
    }
    if (rl_map_is_wall(map, end)) {
        return r + 9; // discourage double wide corridors & double carving into walls
    }

    return r;
}

static void rl_mapgen_bsp_connect_corridors(RL_Map *map, RL_BSP *root, int draw_doors)
{
    rl_assert(map && root);

    // find deepest left-most node
    RL_BSP *leftmost_node = root;
    while (leftmost_node->left != NULL) {
        leftmost_node = leftmost_node->left;
    }
    RL_BSP *node = leftmost_node;
    RL_Graph graph = rl_graph_create(map, NULL, 0);
    while (node) {
        RL_BSP *sibling = rl_bsp_next_node(node);

        if (sibling == NULL) {
            if (node->parent) {
                rl_assert(node->parent->right == node);
            }
            // find leftmost node at higher depth
            node = leftmost_node->parent;
            leftmost_node = node;
            continue;
        }

        // floodfill the rooms to find the center
        // TODO find a random point on a wall that isn't a corner
        RL_Point dig_start, dig_end;
        for (int x = node->x; x < node->width + node->x; ++x) {
            for (int y = node->y; y < node->height + node->y; ++y) {
                if (rl_map_tile_is(map, RL_XY(x, y), RL_TileRoom)) {
                    dig_start = RL_XY(x, y);
                }
            }
        }
        rl_assert(rl_map_is_passable(map, dig_start));
        for (int x = sibling->x; x < sibling->width + sibling->x; ++x) {
            for (int y = sibling->y; y < sibling->height + sibling->y; ++y) {
                if (rl_map_tile_is(map, RL_XY(x, y), RL_TileRoom)) {
                    dig_end = RL_XY(x, y);
                }
            }
        }
        rl_assert(rl_map_is_passable(map, dig_end));
        rl_assert(!(dig_start.x == dig_end.x && dig_start.y == dig_end.y));

        // carve out corridors
        rl_dijkstra_score_ex(&graph, dig_end, rl_mapgen_corridor_scorer, map);
        RL_Path *path = rl_path_create_from_graph(&graph, dig_start);
        rl_assert(path);
        while ((path = rl_path_walk(path))) {
            if (rl_map_tile_is(map, path->point, RL_TileRock)) {
                if (rl_map_is_room_wall(map, path->point) && draw_doors) {
                    map->tiles[(int)path->point.x + (int)path->point.y * map->width] = RL_TileDoor;
                } else {
                    map->tiles[(int)path->point.x + (int)path->point.y * map->width] = RL_TileCorridor;
                }
            }
        }

        // find start node for next loop iteration
        node = rl_bsp_next_node(sibling);
        if (node == NULL) {
            // if we're at the last node in this depth, connect parents
            node = leftmost_node->parent;
            leftmost_node = leftmost_node->parent;
        }
    }

    rl_graph_destroy(&graph);
}

int rl_map_is_visible(const RL_Map *map, RL_Point point)
{
    if (!rl_map_in_bounds(map, point)) {
        return 0;
    }
    return map->visibility[(int)point.x + (int)point.y*map->width] == RL_TileVisible;
}

int rl_map_is_seen(const RL_Map *map, RL_Point point)
{
    if (!rl_map_in_bounds(map, point)) {
        return 0;
    }
    return map->visibility[(int)point.x + (int)point.y*map->width] == RL_TileSeen;
}

RL_Graph rl_map_largest_connected_area(const RL_Map *map)
{
    int *visited = calloc(sizeof(*visited), map->width * map->height);
    rl_assert(visited);
    RL_Graph floodfill = { 0 }; // largest floodfill
    int floodfill_scored = 0;
    for (int x = 0; x < map->width; ++x) {
        for (int y = 0; y < map->height; ++y) {
            if (rl_map_is_passable(map, RL_XY(x, y)) && !visited[x + y*map->width]) {
                RL_Graph test = rl_dijkstra_create(map, RL_XY(x, y), NULL, rl_map_is_passable);
                int test_scored = 0;
                for (size_t i = 0; i < test.length; i++) {
                    if (test.nodes[i].score != DBL_MAX) {
                        visited[i] = 1;
                        test_scored ++;
                    }
                }
                if (test_scored > floodfill_scored) {
                    floodfill_scored = test_scored;
                    rl_graph_destroy(&floodfill);
                    floodfill = test;
                } else {
                    rl_graph_destroy(&test);
                }
            }
        }
    }

    free(visited);

    return floodfill;
}

// TODO method to connect corridors "randomly" (e.g. to make the map more circular)

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

RL_Heap rl_heap_create(int capacity, int (*comparison_f)(const void *heap_item_a, const void *heap_item_b))
{
    void **heap_items = calloc(sizeof(void*), capacity);
    rl_assert(heap_items);

    if (comparison_f == NULL) {
        comparison_f = rl_heap_noop_comparison_f;
    }

    return (RL_Heap) { heap_items, capacity, 0, comparison_f };
}

void rl_heap_destroy(RL_Heap *h)
{
    if (h && h->heap) {
        free(h->heap);
        h->heap = NULL;
    }
}

int rl_heap_insert(RL_Heap *h, void *item)
{
    rl_assert(h != NULL);

    if (h->len == h->cap) {
        // resize the heap
        void **heap_items = realloc(h->heap, sizeof(void*) * h->cap * 2);
        rl_assert(heap_items);
        h->heap = heap_items;
        h->cap *= 2;
    }

    h->heap[h->len] = item;
    for (int i = h->len++; i;) {
        int p = (i - 1) / 2;
        if (h->comparison_f(h->heap[p], h->heap[i])) {
            break;
        }
        void *tmp = h->heap[p];
        h->heap[p] = h->heap[i];
        h->heap[i] = tmp;
        i = p;
    }
    return 1;
}

static void rl_heap_remove(RL_Heap *h, int index)
{
    if (h == NULL) {
        rl_assert(1 != 1);
        return;
    }

    h->heap[index] = h->heap[--h->len];
    for (int i = index;;) {
        int a = 2*i + 1;
        int b = 2*i + 2;
        int j = i;
        if (a < h->len && h->comparison_f(h->heap[a], h->heap[j])) j = a;
        if (b < h->len && h->comparison_f(h->heap[b], h->heap[j])) j = b;
        if (i == j) break;
        void *tmp = h->heap[j];
        h->heap[j] = h->heap[i];
        h->heap[i] = tmp;
        i = j;
    }
}

void *rl_heap_pop(RL_Heap *h)
{
    if (h == NULL) {
        rl_assert(1 != 1);
        return NULL;
    }

    void *r = 0;
    if (h->len) {
        r = h->heap[0];
        rl_heap_remove(h, 0);
    }
    return r;
}

// simplified distance for side by side nodes
static double rl_distance_simple(RL_Point node, RL_Point end)
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
    RL_Path *path = malloc(sizeof(*path));
    rl_assert(path);
    path->next = NULL;
    path->point = p;

    return path;
}

double rl_distance_manhattan(RL_Point node, RL_Point end)
{
    return fabs(node.x - end.x) + fabs(node.y - end.y);
}

double rl_distance_euclidian(RL_Point node, RL_Point end)
{
    double distance_x = node.x - end.x;
    double distance_y = node.y - end.y;

    return sqrt(distance_x * distance_x + distance_y * distance_y);
}

double rl_distance_chebyshev(RL_Point node, RL_Point end)
{
    double distance_x = fabs(node.x - end.x);
    double distance_y = fabs(node.y - end.y);

    return distance_x > distance_y ? distance_x : distance_y;
}

RL_Path *rl_path_create(const RL_Map *map, RL_Point start, RL_Point end, RL_DistanceFun distance_f, RL_PassableFun passable_f)
{
    RL_Graph graph = rl_dijkstra_create(map, end, distance_f, passable_f);
    RL_Path *path = rl_path_create_from_graph(&graph, start);
    rl_assert(path);
    rl_graph_destroy(&graph);

    return path;
}

RL_Path *rl_path_create_from_graph(const RL_Graph *graph, RL_Point start)
{
    RL_Path *path = rl_path(start);
    RL_Path *path_start = path;
    RL_GraphNode *node = NULL;
    rl_assert(path);
    rl_assert(graph->nodes);
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
        if (!lowest_neighbor || lowest_neighbor->score == DBL_MAX || node == lowest_neighbor) {
            break; // no path found
        }
        node = lowest_neighbor;
        path->next = rl_path(node->point);
        rl_assert(path->next);
        path = path->next;
    }

    return path_start;
}

RL_Path *rl_path_walk(RL_Path *path)
{
    if (!path) return NULL;
    RL_Path *next = path->next;
    free(path);

    return next;
}

void rl_path_destroy(RL_Path *path)
{
    if (path) {
        while ((path = rl_path_walk(path))) {}
    }
}

RL_Graph rl_graph_create(const RL_Map *map, RL_PassableFun passable_f, int allow_diagonal_neighbors)
{
    int length = map->width * map->height;
    RL_GraphNode *nodes = calloc(sizeof(*nodes), length);
    rl_assert(nodes != NULL);
    for (int x=0; x<map->width; x++) {
        for (int y=0; y<map->height; y++) {
            int idx = x + y*map->width;
            RL_GraphNode *node = &nodes[idx];
            node->point.x = (double) x;
            node->point.y = (double) y;
            node->neighbors_length = 0;
            node->score = DBL_MAX;
            // calculate neighbors
            RL_Point neighbor_coords[8] = {
                (RL_Point) { x + 1, y },
                (RL_Point) { x - 1, y },
                (RL_Point) { x,     y + 1 },
                (RL_Point) { x,     y - 1 },
                (RL_Point) { x + 1, y + 1 },
                (RL_Point) { x + 1, y - 1 },
                (RL_Point) { x - 1, y + 1 },
                (RL_Point) { x - 1, y - 1 },
            };
            for (int i=0; i<8; i++) {
                if (passable_f && !passable_f(map, RL_XY(neighbor_coords[i].x, neighbor_coords[i].y)))
                    continue;
                if (!rl_map_in_bounds(map, neighbor_coords[i]))
                    continue;
                if (!allow_diagonal_neighbors && i >= 4)
                    continue;

                int idx = neighbor_coords[i].x + neighbor_coords[i].y*map->width;
                node->neighbors[node->neighbors_length] = &nodes[idx];
                node->neighbors_length++;
            }
        }
    }

    return (RL_Graph) { length, nodes };
}

RL_Graph rl_dijkstra_create(const RL_Map *map,
                            RL_Point start,
                            RL_DistanceFun distance_f,
                            RL_PassableFun passable_f)
{
    RL_Graph graph = rl_graph_create(map, passable_f, 1);
    rl_dijkstra_score(&graph, start, distance_f);

    return graph;
}

// default scorer function for Dijkstra - this simply accepts a RL_DistanceFun as context and adds the current nodes
// score to the result of the distance function
double rl_dijkstra_default_score_f(RL_GraphNode *current, RL_GraphNode *neighbor, void *context)
{
    struct { RL_DistanceFun fun; } *distance_f = context;

    return current->score + distance_f->fun(current->point, neighbor->point);
}

void rl_dijkstra_score(RL_Graph *graph, RL_Point start, RL_DistanceFun distance_f)
{
    struct { RL_DistanceFun fun; } scorer_context;
    scorer_context.fun = distance_f ? distance_f : rl_distance_simple; // default to rl_distance_simple
    rl_dijkstra_score_ex(graph, start, rl_dijkstra_default_score_f, &scorer_context);
}

void rl_dijkstra_score_ex(RL_Graph *graph, RL_Point start, RL_ScoreFun score_f, void *score_context)
{
    rl_assert(graph);
    rl_assert(score_f);

    RL_GraphNode *current;
    RL_Heap heap = rl_heap_create(graph->length, &rl_scored_graph_heap_comparison);

    // reset scores of dijkstra map, setting the start point to 0
    for (size_t i=0; i < graph->length; i++) {
        RL_GraphNode *node = &graph->nodes[i];
        if (node->point.x == start.x && node->point.y == start.y) {
            node->score = 0;
            current = node;
        } else {
            node->score = DBL_MAX;
        }
    }

    rl_heap_insert(&heap, (void*) current);
    current = (RL_GraphNode*) rl_heap_pop(&heap);
    while (current) {
        for (size_t i=0; i<current->neighbors_length; i++) {
            RL_GraphNode *neighbor = current->neighbors[i];
            double distance = score_f(current, neighbor, score_context);
            if (distance < neighbor->score) {
                if (neighbor->score == DBL_MAX) {
                    rl_heap_insert(&heap, neighbor);
                }
                neighbor->score = distance;
            }
        }

        current = (RL_GraphNode *) rl_heap_pop(&heap);
    }

    rl_heap_destroy(&heap);
}

void rl_graph_destroy(RL_Graph *graph)
{
    if (graph) {
        if (graph->nodes) {
            free(graph->nodes);
        }
    }
}

typedef struct {
    int Y;
    int X;
} RL_Slope;

// adapted from: https://www.adammil.net/blog/v125_Roguelike_Vision_Algorithms.html#shadowcode (public domain)
// also see: https://www.roguebasin.com/index.php/FOV_using_recursive_shadowcasting
void rl_map_fov_calculate_recursive(RL_Map *map, RL_Point origin, int fov_radius, int symmetric, RL_DistanceFun distance_f, int octant, int x, RL_Slope top, RL_Slope bottom)
{
    for(; (unsigned int)x <= (unsigned int)fov_radius; x++)
    {
        // compute the Y coordinates where the top vector leaves the column (on the right) and where the bottom vector
        // enters the column (on the left). this equals (x+0.5)*top+0.5 and (x-0.5)*bottom+0.5 respectively, which can
        // be computed like (x+0.5)*top+0.5 = (2(x+0.5)*top+1)/2 = ((2x+1)*top+1)/2 to avoid floating point math
        // the rounding is a bit tricky, though
        int topY = top.X == 1 ? x : ((x*2+1) * top.Y + top.X - 1) / (top.X*2); // the rounding is a bit tricky, though
        int bottomY = bottom.Y == 0 ? 0 : ((x*2-1) * bottom.Y + bottom.X) / (bottom.X*2);
        int wasOpaque = -1; // 0:false, 1:true, -1:not applicable
        for(int y=topY; y >= bottomY; y--)
        {
            int tx = origin.x, ty = origin.y;
            switch(octant) // translate local coordinates to map coordinates
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

            bool inRange = fov_radius < 0 || distance_f(origin, RL_XY(tx, ty)) <= (double)fov_radius;
            if(inRange) {
                if (symmetric && (y != topY || top.Y*x >= top.X*y) && (y != bottomY || bottom.Y*x <= bottom.X*y)) {
                    map->visibility[tx + ty*map->width] = RL_TileVisible;
                } else {
                    map->visibility[tx + ty*map->width] = RL_TileVisible;
                }
            }

            bool isOpaque = !inRange || !rl_map_is_passable(map, RL_XY(tx, ty));
            if((int)x != fov_radius)
            {
                if(isOpaque)
                {
                    if(wasOpaque == 0) // if we found a transition from clear to opaque, this sector is done in this column, so
                    {                  // adjust the bottom vector upwards and continue processing it in the next column.
                        RL_Slope newBottom = { (y*2+1), (x*2-1) }; // (x*2-1, y*2+1) is a vector to the top-left of the opaque tile
                        if(!inRange || y == bottomY) { bottom = newBottom; break; } // don't recurse unless we have to
                        else rl_map_fov_calculate_recursive(map, origin, fov_radius, symmetric, distance_f, octant, x+1, top, newBottom);
                    }
                    wasOpaque = 1;
                }
                else // adjust top vector downwards and continue if we found a transition from opaque to clear
                {    // (x*2+1, y*2+1) is the top-right corner of the clear tile (i.e. the bottom-right of the opaque tile)
                    if(wasOpaque > 0) top = (RL_Slope) { (y*2+1), (x*2+1) };
                    wasOpaque = 0;
                }
            }
        }

        if(wasOpaque != 0) break; // if the column ended in a clear tile, continue processing the current sector
    }
}

// TODO need passable_f
void rl_map_fov_calculate(RL_Map *map, RL_Point origin, int fov_radius, int symmetric, RL_DistanceFun distance_f)
{
    if (!rl_map_in_bounds(map, origin)) {
        return;
    }
    // set previously visible tiles to seen
    for (int x=0; x<map->width; ++x) {
        for (int y=0; y<map->height; ++y) {
            if (map->visibility[x + y*map->width] == RL_TileVisible) {
                map->visibility[x + y*map->width] = RL_TileSeen;
            }
        }
    }
    map->visibility[(int)origin.x + (int)origin.y*map->width] = RL_TileVisible;
    for (int octant=0; octant<8; ++octant) {
        rl_map_fov_calculate_recursive(map, origin, fov_radius, symmetric, distance_f, octant, 1, (RL_Slope) { 1, 1 }, (RL_Slope) { 0, 1 });
    }
}

#endif
#endif
