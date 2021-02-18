# Introduction
This is a single file, cross platform, public domain C library which generates mazes using the so called growing tree algorithm.
The algorithm is highly configurable and generates nice mazes of varying styles.


The library generates mazes portably across platforms and C standard library implementations.
It comes bundled with a fast, high quality pseudo random number generator called xoshiro256++.
This ensures that given the same seed and configuration settings, the exact same maze will always be generated.


The library has no global state, which means it can be used safely across multiple threads with a modicum of care.


The library does not allocate any dynamic memory.


# Features
mazelib has the following notable features:
* Easy to integrate (the library is stored in a single file).
* Generates mazes in a reproducible fashion across platforms
* No dynamic memory allocations
* High level API which is easy to use out of the box
* Lower level API which gives maximum flexibility and control
* Generates mazes in two formats (compact and blockwise)
* Portable ANSI C89 code with no external dependencies.
* Easy to customize and configure
* No licensing restrictions (public domain or MIT licensed with no attribution requirements).

# Algorithm Description
The algorithm is flexible, but quite simple. Here's an overview:
1. Start with an empty list, and add a random cell to it.
2. While the list is not empty, select a cell from it.
3. Select an unvisited neighboring cell at random, and add it to the end of the list.
4. Carve a two way passage between the cell and the chosen neighbor, and mark both cells as visited.
5. If there are no unvisited neighbors in step 3, remove the cell from the list.
6. Return to step 2.


The part that makes this algorithm especially interesting is how cells are chosen from the list in step 2.
If you always choose the cell at the end of the list, you get a maze with a very high river factor (AKA long passages with few dead ends).
If you instead choose a cell at random, you get a maze with a very low river factor (AKA short passages with many dead ends).
You can also do something in between, such as selecting the last cell in the list 50% of the time and a random one the other 50%.


# References
The library was inspired by two blog posts by Jamis Buck.
Maze Generation: Growing Tree algorithm (https://weblog.jamisbuck.org/2011/1/27/maze-generation-growing-tree-algorithm)
Mazes with Blockwise Geometry (https://weblog.jamisbuck.org/2015/10/31/mazes-blockwise-geometry.html)

The library ships with a fast, high quality pseudo random number generator called xoshiro256++, as well as another generator called splitmix64 which is used for seeding from a 64 bit integer.
Both of these algorithms are in the public domain.
For more details see http://prng.di.unimi.it/

Got the idea for how to generate uniformly distributed integers in a given range without bias from https://github.com/camel-cdr/cauldron/


# License
This software is available under 2 licenses -- choose whichever you prefer.

## ALTERNATIVE A - MIT No Attribution License
Copyright (c) 2021 Philip Bennefall

Permission is hereby granted, free of charge, to any person obtaining a copy of
this software and associated documentation files (the "Software"), to deal in
the Software without restriction, including without limitation the rights to
use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies
of the Software, and to permit persons to whom the Software is furnished to do
so.


THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.

## ALTERNATIVE B - Public Domain (www.unlicense.org)
This is free and unencumbered software released into the public domain.
Anyone is free to copy, modify, publish, use, compile, sell, or distribute this
software, either in source code form or as a compiled binary, for any purpose,
commercial or non-commercial, and by any means.


In jurisdictions that recognize copyright laws, the author or authors of this
software dedicate any and all copyright interest in the software to the public
domain. We make this dedication for the benefit of the public at large and to
the detriment of our heirs and successors. We intend this dedication to be an
overt act of relinquishment in perpetuity of all present and future rights to
this software under copyright law.
THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
