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

#ifndef MC__CHUNK_H
#define MC__CHUNK_H

//Compiler specific options
#ifdef _MSC_VER
    #include "ms_stdint.h"
#else
    #include <stdint.h>
#endif

namespace mc__ {

    //Block in the game world
    typedef struct {
        uint8_t blockID; uint8_t metadata; uint8_t lighting;
    } Block;
    
    //Item properties, on ground or in inventory
    typedef struct {
        uint16_t itemID; uint8_t  count; uint16_t health;
    } Item;

    class Chunk {

        public:
            //Allocate space for chunk.  Actual size is size_x+1, size_y+1, size_z+1
            Chunk(uint8_t size_x, int8_t size_y, int32_t size_z);
            
            //Allocate space and set x,y,z
            Chunk(uint8_t size_x, int8_t size_y, int32_t size_z,
                    int32_t x, int8_t y, int32_t z);
            
            //Deallocate chunk space
            ~Chunk();                               

            //Set world block coordinates
            void setCoord(int32_t x, int8_t y, int32_t z);
            
            //Dimension size - 1
            uint8_t size_X, size_Y, size_Z;
            
            //World block coordinates
            int32_t X;
            int8_t Y;
            int32_t Z;
            
            //Precompute size_X * size_Y * size_Z
            size_t array_length;
        
            //Point to storage for blocks in chunk
            Block *block_array;
    };
/*
    //Chunk of blocks (brick shaped, uncompressed)
    typedef struct {
        int32_t X, Y, Z;
        uint8_t size_X, size_Y, size_Z;
        uint32_t array_length; //array len = size_X * size_Y * size_Z, precomputed
        Block *block_array; 
    } Chunk;
*/

}

#endif