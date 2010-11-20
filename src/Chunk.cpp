/*
  mc__::Chunk
  Chunk of Minecraft blocks.  Also, Block and Item definitions.
  
  Copyright 2010 axus

    libmc--c is free software: you can redistribute it and/or modify
    it under the terms of the GNU Lesser General Public License as
    published by the Free Software Foundation, either version 3 of the
    License, or at your option) any later version.

    libmc--c is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with this program.  If not, see
    <http://www.gnu.org/licenses/>.
*/

#include <cstdlib>  //NULL

#include "Chunk.h"
using mc__::Chunk;

//Allocate space for chunk
Chunk::Chunk(uint8_t size_x, int8_t size_y, int32_t size_z):
            size_X(size_x), size_Y(size_y), size_Z(size_z), block_array(NULL)
{
    array_length = (size_X+1) * (size_Y+1) * (size_Z+1);
    block_array = new Block[array_length];
}

//Allocate space and set x,y,z
Chunk::Chunk(uint8_t size_x, int8_t size_y, int32_t size_z,
                int32_t x, int8_t y, int32_t z):
            size_X(size_x), size_Y(size_y), size_Z(size_z),
            X(x), Y(y), Z(z), block_array(NULL)
{
    array_length = (size_X+1) * (size_Y+1) * (size_Z+1);
    block_array = new Block[array_length];
}

//Deallocate chunk space
Chunk::~Chunk()
{
    if (block_array != NULL) {
        delete block_array;
        block_array = NULL;
    }
}

//Set world block coordinates
void Chunk::setCoord(int32_t x, int8_t y, int32_t z)
{
    X = x;
    Y = y;
    Z = z;
}
