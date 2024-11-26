#ifndef RL_ROGUELIKE_H
#define RL_ROGUELIKE_H

#include <stdlib.h>

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

// Generic dungeon map structure, supporting hex & square 2d maps
typedef struct RL_2DMap {
    int width;
    int height;
    RL_Tile *tiles; // 2d array of Width*Height for 2d square map - by default 0 is the impassable/rock tile
} RL_2DMap;
typedef RL_2DMap RL_Map;

typedef struct RL_Point {
    int x, y;
} RL_Point;

#define RL_XY(x, y) (RL_Point) { x, y }

typedef struct RL_Path {
    RL_Point point;
    struct RL_Path *next;
    struct RL_Path *start;
} RL_Path;

typedef struct {
    RL_Point point;
    double distance; // will be DBL_MAX for an unreachable/unscored node
} RL_PathNode;

// Represents a RL_Map that has been scored for path finding (e.g. a Dijkstra
// map).
typedef struct RL_PathMap {
    int scored_count; // length of scored nodes
    RL_PathNode *nodes; // array of nodes - length will be the size of the map.width * map.height
} RL_PathMap;

// BSP tree
typedef struct RL_BSP {
    int width;
    int height;
    RL_Point point;
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
    RL_WallWest  = 1,
    RL_WallEast  = 1 << 1,
    RL_WallNorth = 1 << 2,
    RL_WallSouth = 1 << 3,
    RL_WallNE    = 1 << 4,
    RL_WallNW    = 1 << 5,
    RL_WallSE    = 1 << 6,
    RL_WallSW    = 1 << 7,
} RL_Wall;

/**
 * Random map generation
 */

// Creates empty map and fills it with impassable tiles (width & height
// must be positive). Make sure to call rl_map_destroy to clear memory.
RL_Map rl_map_create(int width, int height);

// Frees map tile memory.
void rl_map_destroy(RL_Map map);

// Allocate map->tiles and zero out the memory. Note: map->tiles must be NULL.
void rl_map_populate(RL_Map *map);

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
RL_BSP *rl_mapgen_bsp(RL_Map *map, RL_MapgenConfigBSP config);

/**
 * Generic map helper functions.
 */

// Verifies a coordinates is within bounds of map.
int rl_map_in_bounds(RL_Map map, RL_Point point);

// Checks if a tile is passable. Define RL_PASSABLE_F to define a custom
// function definition.
int rl_map_is_passable(RL_Map map, RL_Point point);

// Returns 1 if tile at point matches given parameter.
int rl_map_tile_is(RL_Map map, RL_Point point, RL_Tile tile);

// A tile is considered a wall if it is touching a passable tile.
//
// Returns a bitmask of the RL_Wall enum. For example, a wall with a wall
// tile to the south, west, and east would have a bitmask of 0b1011.
int rl_map_wall(RL_Map map, RL_Point point);

// Is this a wall that is touching a room tile?
int rl_map_is_room_wall(RL_Map map, RL_Point point);

// A wall that is touching a room tile (e.g. to display it lit).
int rl_map_room_wall(RL_Map map, RL_Point point);

// Returns a the largest connected area (of passable tiles) on the map.
RL_PathMap rl_map_largest_connected_area(RL_Map *map);

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
RL_BSP *rl_bsp_create(int width, int height);
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

// Useful heuristic functions for pathfinding.
double rl_distance_manhattan(RL_Point node, RL_Point end);
double rl_distance_euclidian(RL_Point node, RL_Point end);
// TODO Chebyshev distance ("chessboard distance")

// Custom heuristic function for pathfinding - calculates distance between map nodes
typedef double (*RL_DistanceF)(RL_Point from, RL_Point to);

// Custom passable function for pathfinding. If not sure, pass rl_map_is_passable.
typedef int (*RL_PassableF)(RL_Map map, RL_Point point);

// Find a path between start and end via Dijkstra algorithm. Make sure to call rl_path_destroy when done with path.
// Pass NULL to distance_f to use rough approximation for euclidian.
RL_Path *rl_path_create(RL_Map map, RL_Point start, RL_Point end, RL_DistanceF distance_f, RL_PassableF passable_f, int allow_diagonals);

// Convenience function to "walk" the path. This will return the next path, freeing the current path. Note that this
// invalidates the "path->start" member and sets it to NULL. You do not need to call rl_path_destroy if you walk the
// full path.
RL_Path *rl_path_walk(RL_Path *path);

// Destroy & clean up all nodes from path onward.
void rl_path_destroy(RL_Path *path);

// Dijkstra pathfinding algorithm. Pass NULL to distance_f to use rough approximation for euclidian. Make sure to
// destroy it with rl_pathmap_destroy. Pass NULL to passable_f to pass through impassable tiles.
//
// You can use Dijkstra maps for pathfinding, simple AI, and much more. For example, by setting the player point to
// "start" then you can pick the highest scored tile in the map and set that as the new "start" point. As with all
// Dijkstra maps, you just walk the map by picking the highest scored neighbor. This is a simplistic AI resembling a
// wounded NPC fleeing from the player.
RL_PathMap rl_pathmap_create(RL_Map map,
                             RL_Point start,
                             RL_DistanceF distance_f,
                             RL_PassableF passable_f);

// Free path map memory.
void rl_pathmap_destroy(RL_PathMap path_map);

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

#ifndef rl_assert
#include <assert.h>
#define rl_assert(expr)		(assert(expr));
#endif

RL_Map rl_map_create(int width, int height)
{
    rl_assert(width > 0 && height > 0);

    RL_Map map = {
        .width = width,
        .height = height,
        .tiles = NULL
    };

    rl_map_populate(&map);

    return map;
}

void rl_map_destroy(RL_Map map)
{
    if (map.tiles)
        free(map.tiles);
}

void rl_map_populate(RL_Map *map)
{
    if (map && map->tiles == NULL) {
        map->tiles = calloc(map->width * map->height, sizeof(*map->tiles));
    }
}

int rl_map_in_bounds(RL_Map map, RL_Point point)
{
    return point.x >= 0 && point.y >= 0 && point.x < map.width && point.y < map.height;
}

#ifndef RL_PASSABLE_F
#define RL_PASSABLE_F
int rl_map_is_passable(RL_Map map, RL_Point point)
{
    if (rl_map_in_bounds(map, point)) {
        return map.tiles[point.y * map.width + point.x] != RL_TileRock;
    }

    return 0;
}
#endif

int rl_map_is_wall(RL_Map map, RL_Point point)
{
    int y = point.y;
    int x = point.x;
    if (!rl_map_in_bounds(map, point))
        return 0;
    if (!rl_map_is_passable(map, point)) {
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
int rl_map_wall(RL_Map map, RL_Point point)
{
    int mask = 0;
    if (!rl_map_is_wall(map, point))
        return mask;
    if (rl_map_is_wall(map, RL_XY(point.x + 1, point.y)))
        mask |= RL_WallEast;
    if (rl_map_is_wall(map, RL_XY(point.x - 1, point.y)))
        mask |= RL_WallWest;
    if (rl_map_is_wall(map, RL_XY(point.x,     point.y - 1)))
        mask |= RL_WallNorth;
    if (rl_map_is_wall(map, RL_XY(point.x,     point.y + 1)))
        mask |= RL_WallSouth;
    if (rl_map_is_wall(map, RL_XY(point.x + 1, point.y - 1)))
        mask |= RL_WallNE;
    if (rl_map_is_wall(map, RL_XY(point.x - 1, point.y - 1)))
        mask |= RL_WallNW;
    if (rl_map_is_wall(map, RL_XY(point.x + 1, point.y + 1)))
        mask |= RL_WallSE;
    if (rl_map_is_wall(map, RL_XY(point.x - 1, point.y + 1)))
        mask |= RL_WallSW;
    return mask;
}

int rl_map_tile_is(RL_Map map, RL_Point point, RL_Tile tile)
{
    if (!rl_map_in_bounds(map, point)) return 0;
    return map.tiles[point.x + point.y*map.width] == tile;
}

int rl_map_is_room_wall(RL_Map map, RL_Point point)
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

int rl_map_room_wall(RL_Map map, RL_Point point)
{
    int mask = 0;
    if (!rl_map_is_room_wall(map, point))
        return mask;
    if (rl_map_is_room_wall(map, RL_XY(point.x + 1, point.y)))
        mask |= RL_WallEast;
    if (rl_map_is_room_wall(map, RL_XY(point.x - 1, point.y)))
        mask |= RL_WallWest;
    if (rl_map_is_room_wall(map, RL_XY(point.x,     point.y - 1)))
        mask |= RL_WallNorth;
    if (rl_map_is_room_wall(map, RL_XY(point.x,     point.y + 1)))
        mask |= RL_WallSouth;
    if (rl_map_is_room_wall(map, RL_XY(point.x + 1, point.y - 1)))
        mask |= RL_WallNE;
    if (rl_map_is_room_wall(map, RL_XY(point.x - 1, point.y - 1)))
        mask |= RL_WallNW;
    if (rl_map_is_room_wall(map, RL_XY(point.x + 1, point.y + 1)))
        mask |= RL_WallSE;
    if (rl_map_is_room_wall(map, RL_XY(point.x - 1, point.y + 1)))
        mask |= RL_WallSW;
    return mask;
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

RL_BSP *rl_bsp_create(int width, int height)
{
    rl_assert(width > 0 && height > 0);

    if (width <= 0 || height <= 0)
        return NULL;

    RL_BSP *bsp = calloc(1, sizeof(RL_BSP));
    if (bsp == NULL)
        return NULL;

    bsp->width = width;
    bsp->height = height;

    return bsp;
}
void rl_bsp_destroy(RL_BSP* root)
{
    if (root->left) {
        rl_bsp_destroy(root->left);
        root->left = NULL;
    }
    if (root->right) {
        rl_bsp_destroy(root->right);
        root->right = NULL;
    }
    if (!root->left && !root->right)
        free(root);
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
        left->point = node->point;
        right->width = node->width;
        right->height = node->height - position;
        right->point = node->point;
        right->point.y += position;
    } else {
        left->width = position;
        left->height = node->height;
        left->point = node->point;
        right->width = node->width - position;
        right->height = node->height;
        right->point = node->point;
        right->point.x += position;
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
            room_loc.x = rl_rng_generate(leaf->point.x + room_padding, leaf->point.x + leaf->width - room_width - room_padding);
            room_loc.y = rl_rng_generate(leaf->point.y + room_padding, leaf->point.y + leaf->height - room_height - room_padding);

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
            room_loc.x = rl_rng_generate(leaf->point.x + room_padding, leaf->point.x + leaf->width - room_width - room_padding);
            room_loc.y = rl_rng_generate(leaf->point.y + room_padding, leaf->point.y + leaf->height - room_height - room_padding);

            rl_map_bsp_generate_room(map, room_width, room_height, room_loc);
        } else {
            rl_map_bsp_generate_rooms(node->right, map, room_min_width, room_max_width, room_min_height, room_max_height, room_padding);
        }
    }
}

static void rl_mapgen_bsp_connect_corridors(RL_Map *map, RL_BSP *root);
RL_BSP *rl_mapgen_bsp(RL_Map *map, RL_MapgenConfigBSP config)
{
    rl_assert(map);
    rl_assert(config.room_min_width > 0 && config.room_max_width >= config.room_min_width && config.room_min_height > 0 && config.room_max_height >= config.room_min_height && config.room_padding >= 0);

    if (map) {
        RL_BSP *root = rl_bsp_create(map->width, map->height);

        rl_bsp_recursive_split(root, config.room_max_width + config.room_padding, config.room_max_height + config.room_padding, RL_MAPGEN_MAX_RECURSION);
        rl_map_bsp_generate_rooms(root, map, config.room_min_width, config.room_max_width, config.room_min_height, config.room_max_height, config.room_padding);

        if (config.draw_corridors) {
            rl_mapgen_bsp_connect_corridors(map, root);
        }

        // if (config.use_secret_passages) {
            // TODO connect secret passages
        // }

        // if (config.door_tile) {
            // TODO connect doors
        // }

        return root;
    }

    return NULL;
}

static void rl_mapgen_bsp_connect_corridors(RL_Map *map, RL_BSP *root)
{
    rl_assert(map && root);

    // find deepest left-most node
    RL_BSP *leftmost_node = root;
    while (leftmost_node->left != NULL) {
        leftmost_node = leftmost_node->left;
    }
    RL_BSP *node = leftmost_node;
    while (node->parent) {
        RL_BSP *sibling = rl_bsp_next_node(node);

        if (sibling == NULL) {
            // if we're at the last node in this depth, connect parents
            node = leftmost_node->parent;
            leftmost_node = node;
            continue;
        }

        // floodfill the rooms to find the center
        // TODO find a random point on a wall that isn't a corner
        RL_Point dig_start;
        for (int x = node->point.x; x < node->width + node->point.x; ++x) {
            for (int y = node->point.y; y < node->height + node->point.y; ++y) {
                if (rl_map_is_passable(*map, RL_XY(x, y))) {
                    dig_start = RL_XY(x, y);
                }
            }
        }
        rl_assert(rl_map_is_passable(*map, dig_start));
        RL_Point dig_end;
        for (int x = sibling->point.x; x < sibling->width + sibling->point.x; ++x) {
            for (int y = sibling->point.y; y < sibling->height + sibling->point.y; ++y) {
                if (rl_map_is_passable(*map, RL_XY(x, y))) {
                    dig_end = RL_XY(x, y);
                }
            }
        }
        rl_assert(rl_map_is_passable(*map, dig_end));
        rl_assert(!(dig_start.x == dig_end.x && dig_start.y == dig_end.y));

        // carve out corridors
        RL_Path *path = rl_path_create(*map, dig_start, dig_end, rl_distance_manhattan, NULL, 0);
        while ((path = rl_path_walk(path))) {
            if (map->tiles[path->point.x + path->point.y * map->width] == RL_TileRock)
                map->tiles[path->point.x + path->point.y * map->width] = RL_TileCorridor;

            // prevent digging double wide corridors
            if (path->next) {
                RL_Path *neighbor = path->next;
                if (neighbor->point.x != path->point.x) { // digging left<->right, check for consecutive paths up<->down
                    RL_Point up_point    = RL_XY(path    ->point.x, path->point.y - 1);
                    RL_Point up_point2   = RL_XY(neighbor->point.x, neighbor->point.y - 1);
                    RL_Point down_point  = RL_XY(path    ->point.x, path->point.y + 1);
                    RL_Point down_point2 = RL_XY(neighbor->point.x, neighbor->point.y + 1);

                    if (rl_map_in_bounds(*map, up_point) && rl_map_in_bounds(*map, up_point2) &&
                        map->tiles[up_point.x + up_point.y * map->width] == RL_TileCorridor &&
                        map->tiles[up_point2.x + up_point2.y * map->width] == RL_TileCorridor
                    ) {
                        rl_path_destroy(path);
                        break;
                    }
                    if (rl_map_in_bounds(*map, down_point) && rl_map_in_bounds(*map, down_point2) &&
                        map->tiles[down_point.x + down_point.y * map->width] == RL_TileCorridor &&
                        map->tiles[down_point2.x + down_point2.y * map->width] == RL_TileCorridor
                    ) {
                        rl_path_destroy(path);
                        break;
                    }
                } else { // digging up<->down, check for consecutive paths left<->right
                    RL_Point left_point   = RL_XY(path    ->point.x - 1, path->point.y);
                    RL_Point left_point2  = RL_XY(neighbor->point.x - 1, neighbor->point.y);
                    RL_Point right_point  = RL_XY(path    ->point.x + 1, path->point.y);
                    RL_Point right_point2 = RL_XY(neighbor->point.x + 1, neighbor->point.y);

                    if (rl_map_in_bounds(*map, left_point) && rl_map_in_bounds(*map, left_point2) &&
                        map->tiles[left_point.x + left_point.y * map->width] == RL_TileCorridor &&
                        map->tiles[left_point2.x + left_point2.y * map->width] == RL_TileCorridor
                    ) {
                        rl_path_destroy(path);
                        break;
                    }
                    if (rl_map_in_bounds(*map, right_point) && rl_map_in_bounds(*map, right_point2) &&
                        map->tiles[right_point.x + right_point.y * map->width] == RL_TileCorridor &&
                        map->tiles[right_point2.x + right_point2.y * map->width] == RL_TileCorridor
                    ) {
                        rl_path_destroy(path);
                        break;
                    }
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
}

// TODO can use a "room index" to assign each tile a room index based on how connected it is
RL_PathMap rl_map_largest_connected_area(RL_Map *map)
{
    // int *room_indices = calloc(sizeof(*room_indices), map->width * map->height);
    RL_PathMap floodfill = { 0 };
    for (int x = 0; x < map->width; ++x) {
        for (int y = 0; y < map->height; ++y) {
            if (map->tiles[x + y*map->width] == RL_TileCorridor &&
                (!floodfill.scored_count || floodfill.nodes[x + y*map->width].distance == DBL_MAX)
            ) {
                RL_PathMap cur_floodfill = rl_pathmap_create(*map, RL_XY(x, y), NULL, rl_map_is_passable);
                if (cur_floodfill.scored_count > floodfill.scored_count) {
                    rl_pathmap_destroy(floodfill);
                    floodfill = cur_floodfill;
                } else {
                    rl_pathmap_destroy(cur_floodfill);
                }
            }
        }
    }

    return floodfill;
}

// TODO method to connect corridors "randomly" (e.g. to make the map more circular)

double manhattan_distance(RL_Point node, RL_Point end)
{
    return abs(node.x - end.x) + abs(node.y - end.y);
}

double euclidian_distance(RL_Point node, RL_Point end)
{
    return sqrt(pow(node.x - end.x, 2) + pow(node.y - end.y, 2));
}

/**
 * Heap functions for pathfinding
 *
 * Ref: https://gist.github.com/skeeto/f012a207aff1753662b679917f706de6
 */

#define RL_UNUSED(x) (void)x
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
    if (h->heap) {
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

#define RL_NODE_STATE_VISITED    1
#define RL_NODE_STATE_PROCESSING 1 << 1

// simplified distance for side by side nodes
static double rl_distance_simple(RL_Point node, RL_Point end)
{
    if (node.x == end.x && node.y == end.y) return 0;
    if (node.x == end.x || node.y == end.y) return 1;
    return 1.4;
}

static int rl_pathmap_heap_comparison(const void *heap_item_a, const void *heap_item_b)
{
    RL_PathNode *node_a = (RL_PathNode*) heap_item_a;
    RL_PathNode *node_b = (RL_PathNode*) heap_item_b;

    return node_a->distance < node_b->distance;
}

RL_Path *rl_path(RL_Path *prev, RL_Point p)
{
    RL_Path *path = malloc(sizeof(*path));
    rl_assert(path);
    if (prev)
        path->start = prev->start;
    else
        path->start = path;
    path->next = NULL;
    path->point = p;

    return path;
}

double rl_distance_manhattan(RL_Point node, RL_Point end)
{
    return abs(node.x - end.x) + abs(node.y - end.y);
}

double rl_distance_euclidian(RL_Point node, RL_Point end)
{
    double distance_x = node.x - end.x;
    double distance_y = node.y - end.y;

    return sqrt(distance_x * distance_x + distance_y * distance_y);
}

RL_Path *rl_path_create(RL_Map map, RL_Point start, RL_Point end, RL_DistanceF distance_f, RL_PassableF passable_f, int allow_diagonals)
{
    RL_PathMap pathmap = rl_pathmap_create(map, end, distance_f, passable_f);
    RL_Path *path = rl_path(NULL, start);
    RL_PathNode node = pathmap.nodes[start.x + start.y*map.width];
    while (node.point.x != end.x || node.point.y != end.y) {
        int neighbors_length;
        RL_Point neighbor_coords[8];
        if (allow_diagonals) {
            neighbors_length = 8;
            neighbor_coords[0] = (RL_Point) { node.point.x + 1, node.point.y };
            neighbor_coords[1] = (RL_Point) { node.point.x - 1, node.point.y };
            neighbor_coords[2] = (RL_Point) { node.point.x,     node.point.y + 1 };
            neighbor_coords[3] = (RL_Point) { node.point.x,     node.point.y - 1 };
            neighbor_coords[4] = (RL_Point) { node.point.x + 1, node.point.y + 1 };
            neighbor_coords[5] = (RL_Point) { node.point.x + 1, node.point.y - 1 };
            neighbor_coords[6] = (RL_Point) { node.point.x - 1, node.point.y + 1 };
            neighbor_coords[7] = (RL_Point) { node.point.x - 1, node.point.y - 1 };
        } else {
            neighbors_length = 4;
            neighbor_coords[0] = (RL_Point) { node.point.x + 1, node.point.y };
            neighbor_coords[1] = (RL_Point) { node.point.x - 1, node.point.y };
            neighbor_coords[2] = (RL_Point) { node.point.x,     node.point.y + 1 };
            neighbor_coords[3] = (RL_Point) { node.point.x,     node.point.y - 1 };
        }
        for (int i=0; i<neighbors_length; i++) {
            RL_Point p = neighbor_coords[i];
            if (rl_map_in_bounds(map, p) && pathmap.nodes[p.x + p.y * map.width].distance < node.distance) {
                node = pathmap.nodes[p.x + p.y * map.width];
            }
        }

        if (node.distance == DBL_MAX)
            break; // no path found

        path->next = rl_path(path, node.point);
        rl_assert(path->next);
        path = path->next;
    }

    rl_pathmap_destroy(pathmap);

    return path->start;
}

RL_Path *rl_path_walk(RL_Path *path)
{
    rl_assert(path);
    if (!path) return NULL;

    RL_Path *next = path->next;
    rl_assert(!path->start || path->start == path);
    if (next) {
        next->start = NULL;
    }
    free(path);

    return next;
}

void rl_path_destroy(RL_Path *path)
{
    rl_assert(path);
    while ((path = rl_path_walk(path))) {}
}

RL_PathMap rl_pathmap_create(RL_Map map,
                             RL_Point start,
                             RL_DistanceF distance_f,
                             RL_PassableF passable_f)
{
    if (distance_f == NULL) {
        distance_f = rl_distance_simple;
    }

    // concrete items to go into the queue
    RL_PathNode *nodes = calloc(sizeof(*nodes), map.width * map.height);

    // distance to max for each node except start node
    for (int x=0; x < map.width; x++) {
        for (int y=0; y < map.height; y++) {
            RL_PathNode *node = &nodes[y * map.width + x];
            node->point = (RL_Point) { x, y };
            if (x == start.x && y == start.y) {
                node->distance = 0;
            } else {
                node->distance = DBL_MAX;
            }
        }
    }

    RL_Heap heap = rl_heap_create(map.width * map.height, &rl_pathmap_heap_comparison);
    rl_heap_insert(&heap, &nodes[start.y * map.width + start.x]);
    RL_PathNode *current = (RL_PathNode *) rl_heap_pop(&heap);
    int scored_count = 1;
    while (current) {
        const RL_Point neighbor_coords[8] = {
            (RL_Point) { current->point.x + 1, current->point.y },
            (RL_Point) { current->point.x - 1, current->point.y },
            (RL_Point) { current->point.x,     current->point.y + 1 },
            (RL_Point) { current->point.x,     current->point.y - 1 },
            (RL_Point) { current->point.x + 1, current->point.y + 1 },
            (RL_Point) { current->point.x + 1, current->point.y - 1 },
            (RL_Point) { current->point.x - 1, current->point.y + 1 },
            (RL_Point) { current->point.x - 1, current->point.y - 1 },
        };
        for (int i=0; i<8; i++) {
            if (passable_f && !passable_f(map, neighbor_coords[i]))
                continue;
            if (!rl_map_in_bounds(map, neighbor_coords[i]))
                continue;

            int idx = neighbor_coords[i].y * map.width + neighbor_coords[i].x;
            double distance = current->distance + distance_f(current->point, neighbor_coords[i]);
            if (distance < nodes[idx].distance) {
                if (nodes[idx].distance == DBL_MAX) {
                    scored_count++;
                    rl_heap_insert(&heap, &nodes[idx]);
                }
                nodes[idx].distance = distance;
            }
        }

        current = (RL_PathNode *) rl_heap_pop(&heap);
    }

    rl_heap_destroy(&heap);

    return (RL_PathMap) { scored_count, nodes };
}

void rl_pathmap_destroy(RL_PathMap path_map)
{
    if (path_map.nodes) {
        free(path_map.nodes);
    }
}
#endif

#endif
