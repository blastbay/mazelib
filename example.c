#define MAZELIB_IMPLEMENTATION
#include "mazelib.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

int main()
{
    uint8_t blockwise = 1;
    uint32_t width = 30;
    uint32_t height = 10;
    uint32_t column, row;
    uint64_t result;
    uint64_t buffer_size = mazelib_get_required_buffer_size ( width, height, blockwise );
    uint8_t* buffer = ( uint8_t* ) malloc ( buffer_size );
    if ( buffer == NULL )
    {
        printf ( "Failed to allocate memory.\n" );
        return 0;
    }

    /*
    * We pass 25 for the randomness threshold, giving us fairly long passages but with some random elements thrown in.
    * We use the time function to seed the pseudo random number generator.
    * Note: This is not the best way to seed a prng, it is done for demonstration purposes only.
    */
    result = mazelib_generate ( width, height, ( uint64_t ) time ( NULL ), 25, blockwise, buffer, buffer_size );
    if ( result == 0 )
    {
        printf ( "Generation failed.\n" );
        free ( buffer );
        return 0;
    }

    /* For a blockwise maze, the size of each dimension will be multiplied by 2 plus 1. */
    width *= 2;
    ++width;
    height *= 2;
    ++height;

    for ( row = 0; row < height; ++row )
    {
        for ( column = 0; column < width; ++column )
        {
            if ( buffer[mazelib_get_cell_index ( column, row, height )] )
            {
                printf ( "#" );
            }
            else
            {
                printf ( "_" );
            }
        }
        printf ( "\n" );
    }
    free ( buffer );
    return 0;
}
