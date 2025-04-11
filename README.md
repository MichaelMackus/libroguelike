# libroguelike

Simple single-header game library in ANSI C with zero dependencies. Most
useful for roguelike devs, but has functionality that should be useful to most
2d tile-based games.

See the examples folder - [examples/simple.c](./examples/simple.c) contains a
simplified example with no dynamic memory starting with a predetermined map +
FOV, [examples/bsp.c](./examples/bsp.c) is a minimal example for dungeon
generation, and [examples/player_movement.c](./exapmles/player_movement.c) is a
more detailed example including FOV and basic player movement.

Note that the interface is considered unstable. This is in its very early
stages, but should be usable as-is. I'm currently using this to develop a
commodore 64 based roguelike (using LLVM-MOS) and a [simple
POC](https://github.com/michaelmackus/simplerl). Once I push a tagged release
the code API should be more stable.
