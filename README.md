# libroguelike

[![libroguelike](https://github.com/MichaelMackus/libroguelike/actions/workflows/build.yml/badge.svg)](https://github.com/MichaelMackus/libroguelike/actions/workflows/build.yml)

Simple single-header game library in ANSI C with zero dependencies. Pathfinding
does require linking with math (`-lm` with gcc). Most useful for roguelike
devs, but has functionality that should be useful to most 2d tile-based games.
Everything but the pathfinding works with C89 - pathfinding currently requires
C99.

## Quickstart

[examples/minimal.c](./examples/minimal.c)

```c
#include <stdio.h>

#define RL_IMPLEMENTATION
#include "roguelike.h"

int main()
{
    srand(time(0)); /* seed the RNG */
    RL_Map *map = rl_map_create(80, 25); /* generate tiles memory */
    if (rl_mapgen_bsp(map, RL_MAPGEN_BSP_DEFAULTS) != RL_OK) { /* generate random map */
        printf("Error occurred during mapgen!\n");
        return 1;
    }
    /* print map */
    for (unsigned int y=0; y<map->height; ++y) {
        for (unsigned int x=0; x<map->width; ++x) {
            printf("%c", map->tiles[x + y*map->width]);
        }
        printf("\n");
    }
    rl_map_destroy(map);

    return 0;
}
```

## Examples

See the examples folder - [maze.c](./examples/maze.c),
[bsp.c](./examples/bsp.c), and [automata.c](./examples/automata.c) all contain
basic dungeon generation. [dungeon.c](./examples/dungeon.c) and
[town_sewers.c](./examples/town_sewers.c) are more detailed examples including
FOV and basic player movement.

Note that the interface is considered unstable. This is in its very early
stages, but should be usable as-is. I'm currently using this to develop
roguelike-style games. There's a [commodore 64
POC](https://github.com/MichaelMackus/roguelike-c64-poc) (using LLVM-MOS &
cc65) and a [desktop POC](https://github.com/michaelmackus/simplerl). Once I
push a tagged release the code API should be more stable.
