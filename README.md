# libroguelike

Simple single-header game library in ANSI C with zero dependencies. Most
useful for roguelike devs, but has functionality that should be useful to most
2d tile-based games. Everything but the pathfinding works with C89 -
pathfinding currently requires C99.

See the examples folder - [examples/simple.c](./examples/simple.c) contains a
simplified example starting with a predetermined map,
[examples/bsp.c](./examples/bsp.c) is an example with BSP dungeon generation,
and [examples/player_movement.c](./examples/player_movement.c) is a more
detailed example including FOV and basic player movement.

Note that the interface is considered unstable. This is in its very early
stages, but should be usable as-is. I'm currently using this to develop
roguelike-style games. There's a [commodore 64
POC](https://github.com/MichaelMackus/roguelike-c64-poc) (using LLVM-MOS &
cc65) and a [desktop POC](https://github.com/michaelmackus/simplerl). Once I
push a tagged release the code API should be more stable.
