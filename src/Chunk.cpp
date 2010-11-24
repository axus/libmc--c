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

//Standard lib
#include <cstdlib>  //NULL
#include <cstring>   //memcpy, memset

//Zlib
#include <zlib.h>

//libmc--
#include "Chunk.hpp"
using mc__::Chunk;
using mc__::Block;

//Allocate space for chunk
Chunk::Chunk(uint8_t size_x, int8_t size_y, int32_t size_z):
            size_X(size_x), size_Y(size_y), size_Z(size_z),
            block_array(NULL), byte_array(NULL),
            zipped_length(0), zipped(NULL)
{
    //Calculate array lengths from sizes
    array_length = (size_X+1) * (size_Y+1) * (size_Z+1);
    byte_length = (array_length<<1) + ((array_length+1)>>1);
    
    //Assign array and clear to 0
    block_array = new Block[array_length];
    byte_array = new uint8_t[ byte_length ];
    memset(block_array, 0, array_length*sizeof(mc__::Block));
    memset(byte_array, 0, byte_length);
}

//Allocate space and set x,y,z
Chunk::Chunk(uint8_t size_x, int8_t size_y, int32_t size_z,
                int32_t x, int8_t y, int32_t z):
            size_X(size_x), size_Y(size_y), size_Z(size_z),
            X(x), Y(y), Z(z), block_array(NULL), byte_array(NULL),
            zipped_length(0), zipped(NULL)
{
    //Calculate array lengths from sizes
    array_length = (size_X+1) * (size_Y+1) * (size_Z+1);
    byte_length = (array_length<<1) + ((array_length+1)>>1);

    //Assign array and clear to 0
    block_array = new Block[array_length];
    byte_array = new uint8_t[ byte_length ];
    memset(block_array, 0, array_length*sizeof(mc__::Block));
    memset(byte_array, 0, byte_length);
}

//Deallocate chunk space
Chunk::~Chunk()
{
    if (byte_array != NULL) {
        delete byte_array;
        byte_array = NULL;
    }
}

//Copy block_array to byte_array
void  Chunk::packBlocks()
{
    size_t index;
    mc__::Block* block;
    bool half_byte=false;

    //Offsets to data in byte array
    size_t off_meta=array_length;
    
    size_t off_light= array_length + (array_length/2);
    
    size_t off_sky= array_length<<1;
    
    //For each block...
    for ( index=0; index < array_length; index++)
    {
        //Assign blockID
        block = block_array + index;
        byte_array[index] = block->blockID;
        
        //Pack the metadata, light, and sky, according to half-byte
        if (!half_byte) {
            byte_array[off_meta] = (block->metadata << 4);
            byte_array[off_light] |= (block->lighting >> 4); off_light++;
            byte_array[off_sky] = (block->lighting << 4);
        } else {
            byte_array[off_meta] |= (block->metadata & 0x0F); off_meta++;
            byte_array[off_light] = (block->lighting & 0xF0);
            byte_array[off_sky] |= (block->lighting & 0x0F); off_sky++;
        }
        
        //Toggle half-byte status, go to next block
        half_byte = !half_byte;
    }
}

//Load byte_array to block_array
void  Chunk::unpackBlocks()
{
    size_t index;
    mc__::Block* block;
    bool half_byte=false;

    //Offsets to data in byte array
    size_t off_meta=array_length;
    size_t off_light= array_length + (array_length/2);
    size_t off_sky= array_length<<1;
    
    //For each block...
    for ( index=0; index < array_length; index++)
    {
        //Assign blockID from start of byte array
        block = block_array + index;
        block->blockID = byte_array[index];
        
        //Unpack the metadata, light, and sky, according to half-byte
        if (!half_byte) {
            block->metadata = (byte_array[off_meta] >> 4);
            block->lighting = (byte_array[off_light] << 4) |
                              (byte_array[off_sky] >> 4);
                              off_light++;
        } else {
            block->metadata = (byte_array[off_meta] & 0x0F);
            block->lighting = (byte_array[off_light] & 0xF0) |
                              (byte_array[off_sky] & 0x0F);
                              off_meta++; off_sky++;
        }
        
        //Toggle half-byte status, go to next block
        half_byte = !half_byte;
    }
}


//Set world block coordinates
void Chunk::setCoord(int32_t x, int8_t y, int32_t z)
{
    X = x;
    Y = y;
    Z = z;
}

//Copy compressed data to chunk
void Chunk::setZipped(size_t size, uint8_t *data)
{
    //Erase old data if needed
    if (zipped != NULL) {
        delete zipped;
    }
  
    zipped_length = size;
    memcpy(zipped, data, size);
}

//Compress the packed byte_array to *compressed, set compressed_length
bool Chunk::zip()
{
    //Delete old zip
    if (zipped != NULL) { delete zipped; }

    //Allocate more space for compressed than uncompressed, for header bytes
    zipped_length = compressBound(byte_length);
    zipped = new Bytef[ zipped_length ];

    //Use Zlib to compress the byte_array
    int result = compress2( zipped, (uLongf*)&zipped_length,
        (const Bytef*)byte_array, (uLong)byte_length, Z_BEST_SPEED);
    
    //Problem?
    if (result != Z_OK)
    {
        if (zipped != NULL) { delete zipped; }
        zipped = NULL;
        zipped_length = 0;
        return false;
    }
    
    return true;
}

//Uncompress *compressed to packed byte_array
//  byte_length must be preset, and will be updated after unzip
bool Chunk::unzip()
{
    //Delete old byte array
    if (byte_array != NULL) { delete byte_array; }

    //Allocate space for byte_array
    byte_array = new uint8_t[byte_length];

    //Use Zlib to uncompress the zipped data to byte_array
    int result = uncompress( byte_array, (uLongf*)&byte_length,
        zipped, zipped_length );
    
    //Problem?
    if (result != Z_OK)
    {
        if (byte_array != NULL) { delete byte_array; }
        byte_array = NULL;
        byte_length = 0;
        return false;
    }
    
    return true;
}
            
