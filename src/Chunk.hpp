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

/*
    //Block IDs and associated metadata
    Decimal     Hex       Name              Data        Range
    2           0x02      Grass             
    5           0x05      Wood
    6           0x06      Sapling           Growth      0-15
    8           0x08      Water (moving)    Spread      0-7
    9           0x09      Water
    10          0x0A      Lava (moving)     Spread      0-3
    11          0x0B      Lava
    12          0x0C      Sand
    18          0x12      Leaves
    46          0x2E      TNT
    50          0x32      Torch             Orientation 1-5
    53          0x35      Wood Stairs       Orientation 0-9
    59          0x3B      Crops             Growth      0-7
    60          0x3C      Dirt              Irrigation  0-7
    63          0x3F      SignPost          Orientation 0-15
    65          0x41      Ladder            Orientation 2-5
    66          0x42      Tracks            Orientation 0-9
    67          0x43      Rock Stairs       Orientation 0-3
    68          0x44      WallSign          Orientation 2-5
    78          0x4E      Snow
    79          0x4F      Ice
    81          0x51      Cactus            Growth      0-15
    83          0x53      Reed              Growth      0-15
    91          0x5B      Jack-O-Lantern
*/

namespace mc__ {

    //Block in the game world
    typedef struct {
        uint8_t blockID; uint8_t metadata; uint8_t lighting; uint8_t padding;
    } Block;
    
    //Item properties, on ground or in inventory
    typedef struct {
        uint16_t itemID; uint8_t  count; uint16_t health;
    } Item;

    class Chunk {

        //When indexing block in chunk array,
        //index = y + (z * (Size_Y+1)) + (x * (Size_Y+1) * (Size_Z+1))

        public:
            //Allocate space for chunk.  Actual size is size_x+1, size_y+1, size_z+1
            //  ID, metadata, and lighting are set to 0
            Chunk(uint8_t size_x, uint8_t size_y, uint8_t size_z);
            
            //Allocate space and set x,y,z
            //  ID, metadata, and lighting are set to 0
            Chunk(uint8_t size_x, uint8_t size_y, uint8_t size_z,
                    int32_t x, int8_t y, int32_t z);
            
            //Deallocate chunk space
            ~Chunk();                               

            //Set world block coordinates
            void setCoord(int32_t x, int8_t y, int32_t z);
            
            //Pack block_array to byte_array
            void packBlocks();
            
            //Unpack byte_array to block_array
            void unpackBlocks();

            //Allocate space for copying zipped data
            uint8_t* allocZip( uint32_t length);

            //Copy compressed data to chunk
            void copyZip( uint32_t length, uint8_t *data);
            
            //Compress the packed byte_array to *compressed
            bool zip();
                        
            //Uncompress *compressed to packed byte_array
            bool unzip();
                        
            //Dimension size - 1
            uint8_t size_X, size_Y, size_Z;
            
            //World block coordinates
            int32_t X;
            int8_t Y;
            int32_t Z;
            
            //Precompute size_X * size_Y * size_Z, number of bytes in uncompressed chunk
            uint32_t array_length, byte_length;
        
            //Point to storage for blocks in chunk (don't use!!!)
            Block *block_array;
            
            //Uncompressed block data storage. Size=array_length*2.5
            //  uint8_t item_ID[array_length];
            //  uint4_t metadata[array_length];
            //  uint4_t block_light[array_length];
            //  uint4_t sky_light[array_length];
            uint8_t *byte_array;
            
            //Also, keep compressed version
            uint32_t zipped_length;
            uint8_t *zipped;
    };
}

#endif