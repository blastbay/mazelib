/* Maze Generation Library
* Mazelib version 1.0 - 2021-02-18
*
* Philip Bennefall - philip@blastbay.com
*
* See the end of this file for licensing terms, references and further reading.
*
* This is a single file, public domain C library which generates mazes using the so called growing tree algorithm.
* The algorithm is highly configurable and generates nice mazes of varying styles.
*
* The library generates mazes portably across platforms and C standard library implementations.
* It comes bundled with a fast, high quality pseudo random number generator called xoshiro256++.
* This ensures that given the same seed and configuration settings, the exact same maze will always be generated.
*
* The library has no global state, which means it can be used safely across multiple threads with a modicum of care.
*
* The library does not allocate any dynamic memory.
*
* ALGORITHM DESCRIPTION
*
* The algorithm is flexible, but quite simple. Here's an overview:
* 1. Start with an empty list, and add a random cell to it.
* 2. While the list is not empty, select a cell from it.
* 3. Select an unvisited neighboring cell at random, and add it to the end of the list.
* 4. Carve a two way passage between the cell and the chosen neighbor, and mark both cells as visited.
* 5. If there are no unvisited neighbors in step 3, remove the cell from the list.
* 6. Return to step 2.
*
* The part that makes this algorithm especially interesting is how cells are chosen from the list in step 2.
* If you always choose the cell at the end of the list, you get a maze with a very high river factor (AKA long passages with few dead ends).
* If you instead choose a cell at random, you get a maze with a very low river factor (AKA short passages with many dead ends).
* You can also do something in between, such as selecting the last cell in the list 50% of the time and a random one the other 50%.
*
* USAGE
*
* This is a single-file library. To use it, do something like the following in one .c file.
* #define MAZELIB_IMPLEMENTATION
* #include "mazelib.h"
*
* You can then #include this file in other parts of the program as you would with any other header file.
*
* The library offers a high level API which allows you to get started with minimal effort,
* and a low level API with a slightly higher learning curve which offers more control by way of a callback function.
*
* The library can generate mazes in two formats, with an arbitrary width and height.
* 1. Compact: A grid of bitmask cells where the set bits indicate the directions in which it is possible to move. For example, if the mazelib_west bit is set, it is possible to walk to the west.
* 2. Blockwise: A so called blockwise grid where the walls take up actual space, and where each cell contains the number 0 for empty space and 1 for a wall.
*
* If generating a maze in the compact format, the final output will use width*height bytes.
*
* If generating a maze in the blockwise format, the final output will use ((width*2)+1)*((height*2)+1) bytes.
* A maze in the blockwise format will be entirely surrounded by walls.
*
* The usage pattern for the high level API goes something like this:
*
* 1. Call mazelib_get_required_buffer_size with your desired width and height, and decide whether you want a compact or a blockwise maze.
*
* 2. Allocate the required amount of space as indicated by the function.
*
* 3. Call mazelib_generate. This function returns the number of bytes from the beginning of your buffer where the generated maze has been stored.
* For both compact and blockwise mazes, the result is stored as a one dimensional array of bytes where each byte represents one cell.
*
* 4. Call mazelib_get_cell_index to get the specific cell index in the buffer that you want to examine.
*
* 5. Examine the given cell by checking the presence of certain bits if you have a compact maze,
* or by checking if the cell contains 0 or 1 for a blockwise maze.
*
* That's it! Refer to the API documentation below for more details.
*/

#ifndef MAZELIB_H
#define MAZELIB_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

    /* PUBLIC API */

    /* MACROS */

    /* Bitmask values for directions */
#define mazelib_west 1
#define mazelib_east 2
#define mazelib_north 4
#define mazelib_south 8

    /* COMMON FUNCTIONS */

    /* These functions are useful when working with both the low and the high level API. */

    /*
    * Get the required buffer size in bytes for a maze of the given width, height and format.
    * If any of the parameters are invalid, 0 is returned.
    */
    uint64_t mazelib_get_required_buffer_size ( uint32_t width, uint32_t height, uint8_t blockwise );

    /* Return the byte offset for a cell given a set of coordinates and the height of the maze. */
    uint64_t mazelib_get_cell_index ( uint32_t x, uint32_t y, uint32_t height );

    /* HIGH LEVEL API */

    /* Generate a maze using the high level API.
    *
    * This function generates a maze in either the compact or blockwise format.
    *
    * The space for the final output and the required temporary storage has to have been allocated by the user before invoking this function.
    * To determine how much space is needed, first call mazelib_get_required_buffer_size.
    *
    * The parameters are:
    * width - The width of the maze.
    *
    * height - The height of the maze.
    *
    * random_seed - The seed that should be used to initialize the internal pseudo random number generator.
    * Using the same seed and configuration settings will always produce the exact same result.
    *
    * random_threshold_percent - A number between 0 and 100 (inclusive), which determines how much randomness that should be used when selecting cells.
    * A value of 0 will always select the most recent cell, while a value of 100 will always select one at random.
    * If the value is less than 0, it will be randomized automatically to be between 0 and 100.
    *
    * blockwise - If set to nonzero (true), the function will generate a blockwise maze.
    * If set to 0 (false), the generated maze will be in the compact format.
    *
    * output - A buffer with at least the amount of space indicated by the mazelib_get_required_buffer_size function.
    *
    * output_size - The amount of space in output, in bytes.
    *
    * The function returns the number of bytes in the buffer that are used to store the final result.
    */
    uint64_t mazelib_generate ( uint32_t width, uint32_t height, uint64_t random_seed, int8_t random_threshold_percent, uint8_t blockwise, uint8_t* output, uint64_t output_size );

    /* LOW LEVEL API */

    /*
    * The low level API differs from the high level API in two ways:
    * 1. The user is responsible for managing the pseudo random number generator (prng).
    * 2. The user must supply a callback which selects a number within a specified range (this is how cells are chosen for backtracking).
    *
    * First, create a mazelib_prng structure and seed it by calling mazelib_prng_seed.
    * Then, implement your callback.
    * The callback receives a value called count, and should return a value between 0 and count-1 (see the callback description below for more details).
    * When you are ready to generate your maze, call mazelib_generate_extended.
    */

    /* PRNG */
    typedef struct mazelib_prng mazelib_prng;
    struct mazelib_prng
    {
        uint64_t s[4];
    };

    /* Seed the PRNG from a 64 bit unsigned integer. */
    void mazelib_prng_seed ( mazelib_prng* prng, uint64_t x );

    /* Generate the next unsigned 64 bit random number. */
    uint64_t mazelib_prng_next ( mazelib_prng* prng );

    /* Generate a random number in the range 0...range (exclusive). */
    uint64_t mazelib_prng_next_in_range ( mazelib_prng* prng, uint64_t range );

    /* Cell Selection Callback
    *
    * This callback is used to select a cell to which the algorithm should attempt to backtrack.
    * The function should return a value between 0 and count (exclusive).
    * The function receives a copy of the prng, as randomness is often an important component when selecting cells.
    * For example you might select the most recent cell (AKA count-1) 50% of the time, and a random value between 0 and count-1 the other 50%.
    *
    * The user pointer is the same as was given to mazelib_generate_extended, and is not modified or accessed by the library in any way.
    *
    * If the function returns a value equal to or greater than count, processing will be aborted and mazelib_generate_extended will return 0.
    */
    typedef uint64_t ( *mazelib_cell_selection_callback ) ( uint64_t count, mazelib_prng* prng, void* user );

    /* Generate a maze using the low level API.
    *
    * This function generates a maze in either the compact or blockwise format.
    *
    * The space for the final output and the required temporary storage has to have been allocated by the user before invoking this function.
    * To determine how much space is needed, first call mazelib_get_required_buffer_size.
    *
    * The parameters are:
    * width - The width of the maze.
    *
    * height - The height of the maze.
    *
    * prng - An already seeded prng instance.
    * If this parameter is NULL, the function aborts with a return value of 0.
    *
    * cell_selection_callback - The callback that is used to select a cell to which the algorithm should attempt to backtrack.
    * If this parameter is NULL, the function aborts with a return value of 0.
    *
    * user - A pointer which is passed to the cell selection callback.
    * The library does not modify or access this pointer in any way.
    *
    * blockwise - If set to nonzero (true), the function will generate a blockwise maze.
    * If set to 0 (false), the generated maze will be in the compact format.
    *
    * output - A buffer with at least the amount of space indicated by the mazelib_get_required_buffer_size function.
    *
    * output_size - The amount of space in output, in bytes.
    *
    * The function returns the number of bytes in the buffer that are used to store the final result.
    * If the cell selection callback returns a value equal to or greater than count, this function aborts and returns 0.
    */
    uint64_t mazelib_generate_extended ( uint32_t width, uint32_t height, mazelib_prng* prng, mazelib_cell_selection_callback cell_selection_callback, void* user, uint8_t blockwise, uint8_t* output, uint64_t output_size );

#ifdef __cplusplus
}
#endif

#endif  /* MAZELIB_H */

/* IMPLEMENTATION */

#ifdef MAZELIB_IMPLEMENTATION

#include <stddef.h>
#include <string.h>
#include <assert.h>

static uint8_t mazelib_get_cell_bytes_required_for_dimensions ( uint32_t width, uint32_t height )
{
    uint64_t temp = width;
    temp *= height;
    if ( temp < UINT8_MAX )
    {
        return 1;
    }
    if ( temp < UINT16_MAX )
    {
        return 2;
    }
    if ( temp < UINT32_MAX )
    {
        return 4;
    }
    return 8;
}

uint64_t mazelib_get_required_buffer_size ( uint32_t width, uint32_t height, uint8_t blockwise )
{
    uint8_t cell_bytes;
    uint64_t size;

    if ( width == 0 )
    {
        return 0;
    }
    else if ( height == 0 )
    {
        return 0;
    }
    cell_bytes = mazelib_get_cell_bytes_required_for_dimensions ( width, height );
    size = width;
    size *= height;
    size *= cell_bytes;
    if ( blockwise )
    {
        uint64_t new_width = width;
        uint64_t new_height = height;
        new_width *= 2;
        new_height *= 2;
        ++new_width;
        ++new_height;
        size += ( new_width * new_height );
    }
    else
    {
        size += ( size / cell_bytes );
    }
    return size;
}

void mazelib_prng_seed ( mazelib_prng* prng, uint64_t x )
{
    unsigned int i;

    assert ( prng );

    for ( i = 0; i < 4; ++i )
    {
        uint64_t z = ( x += 0x9e3779b97f4a7c15 );
        z = ( z ^ ( z >> 30 ) ) * 0xbf58476d1ce4e5b9;
        z = ( z ^ ( z >> 27 ) ) * 0x94d049bb133111eb;
        prng->s[i] = z ^ ( z >> 31 );
    }
}

static uint64_t mazelib_prng_rotl ( const uint64_t x, int k )
{
    return ( x << k ) | ( x >> ( 64 - k ) );
}

uint64_t mazelib_prng_next ( mazelib_prng* prng )
{
    uint64_t* s = prng->s;
    const uint64_t result = mazelib_prng_rotl ( s[0] + s[3], 23 ) + s[0];

    const uint64_t t = s[1] << 17;

    s[2] ^= s[0];
    s[3] ^= s[1];
    s[1] ^= s[2];
    s[0] ^= s[3];

    s[2] ^= t;

    s[3] = mazelib_prng_rotl ( s[3], 45 );

    return result;
}

uint64_t mazelib_prng_next_in_range ( mazelib_prng* prng, uint64_t range )
{
    uint64_t x, r;
    do
    {
        x = mazelib_prng_next ( prng );
        r = x % range;
    }
    while ( x - r > -range );
    return r;
}

uint64_t mazelib_get_cell_index ( uint32_t x, uint32_t y, uint32_t height )
{
    return ( ( uint64_t ) x * ( uint64_t ) height ) + ( uint64_t ) y;
}

static void mazelib_assign_cell ( uint8_t* mem, uint8_t cell_bytes, uint64_t cell_index, uint64_t value )
{
    switch ( cell_bytes )
    {
        case 1:
        {
            mem[cell_index] = ( uint8_t ) value;
        }
        break;
        case 2:
        {
            uint16_t* ptr = ( uint16_t* ) mem;
            ptr[cell_index] = ( uint16_t ) value;
        }
        break;
        case 4:
        {
            uint32_t* ptr = ( uint32_t* ) mem;
            ptr[cell_index] = ( uint32_t ) value;

        }
        break;
        default:
        {
            uint64_t* ptr = ( uint64_t* ) mem;
            ptr[cell_index] = value;
        }
    };
}

uint64_t mazelib_generate_extended ( uint32_t width, uint32_t height, mazelib_prng* prng, mazelib_cell_selection_callback cell_selection_callback, void* user, uint8_t blockwise, uint8_t* output, uint64_t output_size )
{
    uint64_t result, temp, i;
    const uint64_t required_size = mazelib_get_required_buffer_size ( width, height, blockwise );
    const uint8_t cell_bytes = mazelib_get_cell_bytes_required_for_dimensions ( width, height );
    uint8_t* cells;
    uint8_t* grid;
    uint64_t cells_size = 1;
    uint8_t directions[4];

    if ( output == NULL )
    {
        return 0;
    }
    if ( output_size < required_size )
    {
        return 0;
    }
    if ( prng == NULL )
    {
        return 0;
    }
    if ( cell_selection_callback == NULL )
    {
        return 0;
    }

    output_size = required_size;

    result = width;
    result *= height;

    if ( blockwise )
    {

        /* If we are generating a blockwise maze, we put the temporary storage at the beginning so that we can then override it with the final result at the end. */
        grid = output + output_size;
        grid -= result;
        cells = output;
    }
    else
    {

        /* If we are not generating a blockwise maze, we put the grid at the beginning since that will be our final result. */
        grid = output;
        cells = output + result;
    }

    directions[0] = mazelib_west;
    directions[1] = mazelib_east;
    directions[2] = mazelib_north;
    directions[3] = mazelib_south;

    /* Clear the grid initially. */
    for ( i = 0; i < result; ++i )
    {
        grid[i] = 0;
    }

    /* Start by inserting a random cell. */
    temp = mazelib_get_cell_index ( ( uint32_t ) mazelib_prng_next_in_range ( prng, width ), ( uint32_t ) mazelib_prng_next_in_range ( prng, height ), height );
    mazelib_assign_cell ( cells, cell_bytes, 0, temp );

    while ( cells_size )
    {
        uint32_t x, y, new_x, new_y;
        uint64_t new_cell_index, current_cell;
        uint64_t cell_index = 0;
        uint8_t found_new_neighbor = 0;
        uint8_t opposite_direction = 0;

        if ( cells_size > 1 )
        {
            cell_index = cell_selection_callback ( cells_size, prng, user );
            if ( cell_index >= cells_size )
            {
                return 0;    /* The callback returned a value outside the allowed range, so we abort. */
            }
        }

        switch ( cell_bytes )
        {
            case 1:
            {
                current_cell = cells[cell_index];
            }
            break;
            case 2:
            {
                uint16_t* cell_ptr = ( uint16_t* ) cells;
                current_cell = cell_ptr[cell_index];
            }
            break;
            case 4:
            {
                uint32_t* cell_ptr = ( uint32_t* ) cells;
                current_cell = cell_ptr[cell_index];
            }
            break;
            default:
            {
                uint64_t* cell_ptr = ( uint64_t* ) cells;
                current_cell = cell_ptr[cell_index];
            }
            break;
        };

        x = ( uint32_t ) ( current_cell / height );
        y = ( uint32_t ) ( current_cell % height );

        /* Shuffle the directions. */
        for ( i = 3; i; --i )
        {
            uint8_t swap_index = ( uint8_t ) mazelib_prng_next_in_range ( prng, i + 1 );
            temp = directions[i];
            directions[i] = directions[swap_index];
            directions[swap_index] = temp;
        }

        for ( i = 0; i < 4; ++i )
        {
            switch ( directions[i] )
            {
                case mazelib_west:
                    if ( x == 0 )
                    {
                        continue;
                    }
                    new_x = x - 1;
                    new_y = y;
                    opposite_direction = mazelib_east;
                    break;
                case mazelib_east:
                    if ( x == width - 1 )
                    {
                        continue;
                    }
                    new_x = x + 1;
                    new_y = y;
                    opposite_direction = mazelib_west;
                    break;
                case mazelib_north:
                    if ( y == 0 )
                    {
                        continue;
                    }
                    new_x = x;
                    new_y = y - 1;
                    opposite_direction = mazelib_south;
                    break;
                default: /* South */
                    if ( y == height - 1 )
                    {
                        continue;
                    }
                    new_x = x;
                    new_y = y + 1;
                    opposite_direction = mazelib_north;
                    break;
            };
            new_cell_index = mazelib_get_cell_index ( new_x, new_y, height );
            assert ( new_cell_index < ( width * height ) );

            /* If we have already visited the given cell, we don't consider it again. */
            if ( grid[new_cell_index] )
            {
                continue;
            }

            found_new_neighbor = 1;

            /* Carve a two way path between the current and the new cell. */
            grid[current_cell] |= directions[i];
            grid[new_cell_index] |= opposite_direction;

            /* Add the new cell to our list. */
            switch ( cell_bytes )
            {
                case 1:
                    cells[cells_size] = ( uint8_t ) new_cell_index;
                    break;
                case 2:
                {
                    uint16_t* cell_ptr = ( uint16_t* ) cells;
                    cell_ptr[cells_size] = ( uint16_t ) new_cell_index;
                }
                break;
                case 4:
                {
                    uint32_t* cell_ptr = ( uint32_t* ) cells;
                    cell_ptr[cells_size] = ( uint32_t ) new_cell_index;
                }
                break;
                default:
                {
                    uint64_t* cell_ptr = ( uint64_t* ) cells;
                    cell_ptr[cells_size] = new_cell_index;
                }
            };

            ++cells_size;
            break;
        }
        if ( found_new_neighbor == 0 )
        {

            /* The current cell has no unvisited neighbors, so we remove it from our list. */
            if ( cell_index < cells_size - 1 )
            {
                memmove ( ( void* ) &cells[cell_index * cell_bytes], ( void* ) &cells[ ( cell_index + 1 ) *cell_bytes], ( cells_size - ( cell_index + 1 ) ) *cell_bytes );
            }
            --cells_size;
        }
    }

    if ( blockwise )
    {
        uint32_t old_x, old_y, new_x, new_y;
        uint64_t new_width = width;
        uint64_t new_height = height;
        new_width *= 2;
        new_height *= 2;
        ++new_width;
        ++new_height;
        result = ( new_width * new_height );

        for ( i = 0; i < result; ++i )
        {
            output[i] = 1;
        }

        for ( old_x = 0, new_x = 1; old_x < width; ++old_x, new_x += 2 )
        {
            for ( old_y = 0, new_y = 1; old_y < height; ++old_y, new_y += 2 )
            {
                uint64_t old_cell_index = mazelib_get_cell_index ( old_x, old_y, height );
                uint64_t new_cell_index = mazelib_get_cell_index ( new_x, new_y, new_height );
                assert ( new_cell_index < result );
                assert ( new_x != new_width - 1 );
                assert ( new_y != new_height - 1 );
                output[new_cell_index] = 0;
                if ( ( grid[old_cell_index]&mazelib_south ) )
                {
                    assert ( new_y + 1 != new_height - 1 );
                    new_cell_index = mazelib_get_cell_index ( new_x, new_y + 1, new_height );
                    assert ( new_cell_index < result );
                    output[new_cell_index] = 0;
                }
                if ( ( grid[old_cell_index]&mazelib_east ) )
                {
                    assert ( new_x + 1 != new_width - 1 );
                    new_cell_index = mazelib_get_cell_index ( new_x + 1, new_y, new_height );
                    assert ( new_cell_index < result );
                    output[new_cell_index] = 0;
                }
            }
        }

    }

    return result;
}

static uint64_t mazelib_high_level_cell_selection_callback ( uint64_t count, mazelib_prng* prng, void* user )
{
    int8_t* random_threshold_percent = ( int8_t* ) user;
    if ( *random_threshold_percent > 0 && ( int8_t ) mazelib_prng_next_in_range ( prng, 101 ) < *random_threshold_percent )
    {
        return mazelib_prng_next_in_range ( prng, count );
    }
    return count - 1;
}

uint64_t mazelib_generate ( uint32_t width, uint32_t height, uint64_t random_seed, int8_t random_threshold_percent, uint8_t blockwise, uint8_t* output, uint64_t output_size )
{
    mazelib_prng prng;

    mazelib_prng_seed ( &prng, random_seed );

    if ( random_threshold_percent < 0 )
    {
        random_threshold_percent = ( int8_t ) mazelib_prng_next_in_range ( &prng, 101 );
    }
    else if ( random_threshold_percent > 100 )
    {
        random_threshold_percent = 100;
    }

    return mazelib_generate_extended ( width, height, &prng, mazelib_high_level_cell_selection_callback, ( void* ) &random_threshold_percent, blockwise, output, output_size );
}

#endif /* MAZELIB_IMPLEMENTATION */

/* REFERENCES
*
* The library was inspired by two blog posts by Jamis Buck.
* Maze Generation: Growing Tree algorithm (https://weblog.jamisbuck.org/2011/1/27/maze-generation-growing-tree-algorithm)
* Mazes with Blockwise Geometry (https://weblog.jamisbuck.org/2015/10/31/mazes-blockwise-geometry.html)
*
* The library ships with a fast, high quality pseudo random number generator called xoshiro256++,
* as well as another generator called splitmix64 which is used for seeding from a 64 bit integer.
* Both of these algorithms are in the public domain.
* For more details see http://prng.di.unimi.it/
*
* Got the idea for how to generate uniformly distributed integers in a given range without bias from https://github.com/camel-cdr/cauldron/
*
* REVISION HISTORY
*
* Version 1.0 - 2021-02-18
* Initial release.
*/

/* LICENSE

This software is available under 2 licenses -- choose whichever you prefer.
------------------------------------------------------------------------------
ALTERNATIVE A - MIT No Attribution License
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
------------------------------------------------------------------------------
ALTERNATIVE B - Public Domain (www.unlicense.org)
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
------------------------------------------------------------------------------
*/
