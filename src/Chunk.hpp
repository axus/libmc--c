/*
  mc__::Chunk
  Chunk of voxel blocks.  Also, Block and Item definitions.
  
  Copyright 2010 - 2011 axus

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

//mc__
#include "Block.hpp"

//STL
#include <unordered_set>



namespace mc__ {

    typedef std::unordered_set<uint16_t> indexList_t;

    class Chunk {

        //When indexing block in chunk array,
        //index = y + (z * (Size_Y+1)) + (x * (Size_Y+1) * (Size_Z+1))

        public:
            //Allocate space for chunk.
            //  Actual size is size_x+1, size_y+1, size_z+1
            //  ID, metadata, and lighting are set to 0
            Chunk(uint8_t size_x, uint8_t size_y, uint8_t size_z);
            
            //Calculate space and set x,y,z
            //  Actual size is size_x+1, size_y+1, size_z+1
            //  If allocate==true, ID, metadata, and lighting are set to 0
            Chunk(uint8_t size_x, uint8_t size_y, uint8_t size_z,
                    int32_t x, int8_t y, int32_t z, bool allocate=true);
            
            //Deallocate chunk space
            ~Chunk();                               

            //Copy constructor: copy all pointed to memory
            Chunk( const Chunk& ch);
            Chunk& operator=( const Chunk& ch);

            //Set world block coordinates
            void setCoord(int32_t x, int8_t y, int32_t z);
            
            //Pack block_array to byte_array
            void packBlocks();
            
            //Unpack byte_array to block_array
            bool unpackBlocks(bool free_packed=false);

            //Allocate space for copying zipped data
            uint8_t* allocZip( uint32_t length);
            
            //Allocate space for mc__::Block[array_length]
            Block* allocBlockArray();
            
            //Allocate space for byte_array[byte_length]
            uint8_t* allocByteArray();
            
            //Free allocated memory
            void deleteZipArray();
            void deleteBlockArray();
            void deleteByteArray();

            //Copy compressed data to chunk
            void copyZip( uint32_t length, const uint8_t *data);
            
            //Compress the packed byte_array to *compressed
            bool zip();
                        
            //Uncompress *compressed to packed byte_array
            bool unzip(bool free_zip=false);


            //Dimension size - 1
            uint8_t size_X, size_Y, size_Z;

            //World block coordinates
            int32_t X;
            int8_t Y;
            int32_t Z;

            //Precompute size_X * size_Y * size_Z,
            //  number of bytes in uncompressed chunk
            uint32_t array_length, byte_length;

            //Point to storage for blocks in chunk (NULL if data is zipped)
            Block *block_array;

            //Uncompressed block data storage. Size=array_length*2.5
            //  uint8_t item_ID[array_length];
            //  uint4_t metadata[array_length];
            //  uint4_t block_light[array_length];
            //  uint4_t sky_light[array_length];
            uint8_t *byte_array;

            //Also, keep compressed version
            bool isUnzipped;
            uint32_t zipped_length;
            uint8_t *zipped;
            
    };
}


#endif