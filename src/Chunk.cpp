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
Chunk::Chunk(uint8_t size_x, uint8_t size_y, uint8_t size_z):
            size_X(size_x), size_Y(size_y), size_Z(size_z),
            block_array(NULL), byte_array(NULL),
            isUnzipped(true), zipped_length(0), zipped(NULL)
{
    //Calculate array lengths from sizes
    array_length = (size_X+1) * (size_Y+1) * (size_Z+1);
    byte_length = (array_length<<1) + ((array_length+1)>>1);

    //Allocate space for uncompressed arrays
    allocBlockArray();
    allocByteArray();
}

//Allocate space and set x,y,z
Chunk::Chunk(uint8_t size_x, uint8_t size_y, uint8_t size_z,
                int32_t x, int8_t y, int32_t z, bool allocate):
            size_X(size_x), size_Y(size_y), size_Z(size_z),
            X(x), Y(y), Z(z), block_array(NULL), byte_array(NULL),
            isUnzipped(allocate), zipped_length(0), zipped(NULL)
{
    //Calculate array lengths from sizes
    array_length = (size_X+1) * (size_Y+1) * (size_Z+1);
    
    //Number of bytes = array_length * 2.5
    byte_length = (array_length<<1) + ((array_length+1)>>1);

    if (isUnzipped) {
        allocBlockArray();
        allocByteArray();
    }
}

//Deallocate chunk space
Chunk::~Chunk()
{
    deleteByteArray();
    deleteBlockArray();
    deleteZipArray();
}

//Copy constructor: copy all pointed to memory
Chunk::Chunk( const Chunk& ch):
            size_X(ch.size_X), size_Y(ch.size_Y), size_Z(ch.size_Z),
            X(ch.X), Y(ch.Y), Z(ch.Z),
            array_length(ch.array_length), byte_length(ch.byte_length),
            block_array(NULL), byte_array(NULL),
            isUnzipped(ch.isUnzipped),
            zipped_length(ch.zipped_length), zipped(NULL)
{
    //Copy memory
    copyZip(zipped_length, zipped);
    
    if (byte_length > 0) {
        byte_array = new uint8_t[byte_length];
        memcpy(byte_array, ch.byte_array, byte_length);
    }

    if (array_length > 0) {
        block_array = new Block[array_length];
        memcpy(block_array, ch.block_array, array_length*sizeof(mc__::Block));
    }

}


//Assignment operator: copy all pointed to memory
Chunk& Chunk::operator=( const Chunk& ch)
{
    //Don't copy over myself
    if (this == &ch) { return *this; }
  
    //Copy values
    size_X = ch.size_X;
    size_Y = ch.size_Y;
    size_Z = ch.size_Z;
    X = ch.X; Y = ch.Y; Z = ch.Z;
    array_length = ch.array_length;
    byte_length = ch.byte_length;
    block_array = NULL;
    byte_array = NULL;
    isUnzipped = ch.isUnzipped;
    zipped_length = ch.zipped_length;
    zipped = NULL;
    
    
    //Copy memory
    copyZip(zipped_length, zipped);

    if (byte_length > 0) {
        byte_array = new uint8_t[byte_length];
        memcpy(byte_array, ch.byte_array, byte_length);
    }

    if (array_length > 0) {
        block_array = new Block[array_length];
        memcpy(block_array, ch.block_array, array_length*sizeof(mc__::Block));
    }

    return *this;
}

//Copy block_array to byte_array
void Chunk::packBlocks()
{
    //re-allocate byte array
    allocByteArray();
  
    //Vars
    uint32_t index;
    mc__::Block* block;
    bool half_byte=false;

    //Offsets to data in byte array
    uint32_t off_meta=array_length;
    uint32_t off_light= array_length + (array_length/2);
    uint32_t off_sky= array_length<<1;
    
    //Handle chunks where X,Y,Z are all odd
    bool odd_size = ((array_length & 0x1) == 0x1); 
    
    //For each block...
    for ( index=0; index < array_length; index++)
    {
        //Assign blockID
        block = block_array + index;
        byte_array[index] = block->blockID;
        
        //Pack the metadata and sky (don't care if odd number of blocks)
        if (!half_byte) {
            byte_array[off_meta] |= (block->metadata & 0x0F);
            byte_array[off_sky] |= (block->lighting & 0x0F);
        } else {
            byte_array[off_meta] |= (block->metadata << 4); off_meta++;
            byte_array[off_sky] = (block->lighting << 4); off_sky++;
        }
        
        //Pack block light (depends on odd number of blocks)
        if (half_byte ^ odd_size) {
            byte_array[off_light] |= (block->lighting & 0xF0); off_light++;
        } else {
            byte_array[off_light] |= (block->lighting >> 4);
        }
                
        //Toggle half-byte status, go to next block
        half_byte = !half_byte;
    }
}

//Load byte_array to block_array
bool Chunk::unpackBlocks(bool free_packed)
{
    if (byte_array == NULL) {
        return false;
    }
  
    uint32_t index;
    mc__::Block* block;
    bool half_byte=false;

    //Handle chunks where X,Y,Z are all odd
    bool odd_size = ((array_length & 0x1) == 0x1); 

    //Reallocate block_array if needed
    allocBlockArray();

    //Offsets to data in byte array
    uint32_t off_meta=array_length;
    uint32_t off_light= array_length + (array_length/2);
    uint32_t off_sky= array_length<<1;
    
    //For each block...
    for ( index=0; index < array_length; index++)
    {
        //Assign blockID from start of byte array
        block = block_array + index;
        block->blockID = byte_array[index];

        //Unpack half-bytes: low 4 bits first, high 4 bits second
        //Unpack the metadata, light, and sky, according to half-byte
        if (!half_byte) {
            block->metadata = (byte_array[off_meta] & 0x0F);
            block->lighting = (byte_array[off_sky] & 0x0F);
        } else {
            block->metadata = (byte_array[off_meta] >> 4);
            block->lighting = (byte_array[off_sky] >> 4);
            off_meta++;
            off_sky++;
        }

        //Unpack block light depending on odd_size and half-byte
        if (half_byte ^ odd_size) {
            block->lighting |= (byte_array[off_light] & 0xF0);
            off_light++;
        } else {
            block->lighting |= ((byte_array[off_light] & 0x0F) << 4);
        }

        //Toggle half-byte status, go to next block
        half_byte = !half_byte;
    }
    
    //Reclaim memory if asked to
    if (free_packed) {
        deleteByteArray();
    }
    
    return true;
}


//Set world block coordinates
void Chunk::setCoord(int32_t x, int8_t y, int32_t z)
{
    X = x;
    Y = y;
    Z = z;
}

//Allocate space for mc__::Block[array_length]
Block* Chunk::allocBlockArray()
{
    //No leaks
    deleteBlockArray();
    
    //Assign array and clear to 0
    block_array = new Block[array_length];
    memset(block_array, 0, array_length*sizeof(mc__::Block));
    return block_array;
}

//Free memory
void Chunk::deleteBlockArray() {
    if (block_array != NULL) {
        delete[] block_array;
        block_array = NULL;
    }
}

//Allocate space for byte_array[byte_length]
uint8_t* Chunk::allocByteArray()
{
    //No leaks
    deleteByteArray();

    byte_array = new uint8_t[ byte_length];
    memset(byte_array, 0, byte_length);
    return byte_array;
}

//Free memory
void Chunk::deleteByteArray() {

    if (byte_array != NULL) {
        delete[] byte_array;
        byte_array = NULL;
    }
}

//Allocate space for zipped data
uint8_t* Chunk::allocZip( uint32_t size)
{
    deleteZipArray();
    zipped_length = size;
    zipped = new uint8_t[zipped_length];
    
    return zipped;
}

//Free memory
void Chunk::deleteZipArray() {
    if (zipped != NULL) {
        delete[] zipped;
        zipped = NULL;
    }
}

//Copy compressed data to chunk
void Chunk::copyZip(uint32_t size, const uint8_t *data)
{
    //Allocate space and copy data
    if (allocZip(size) != NULL) {
        memcpy(zipped, data, size);
    }
}

//Compress the packed byte_array to *compressed, set compressed_length
bool Chunk::zip()
{

    //Calculate max zipped bytes    
    zipped_length = compressBound(byte_length);

    //Re-allocate zipped bytes
    allocZip(zipped_length);

    //Use Zlib to compress the byte_array
    int result = compress2( zipped, (uLongf*)&zipped_length,
        (const Bytef*)byte_array, (uLong)byte_length, Z_BEST_SPEED);

    //Problem?
    if (result != Z_OK)
    {
        if (zipped != NULL) { deleteZipArray(); }
        zipped = NULL;
        zipped_length = 0;
        return false;
    }
    
    return true;
}

//Uncompress *compressed to packed byte_array
//  byte_length must be preset, and will be updated after unzip
bool Chunk::unzip(bool free_zip)
{
    //Re-allocate byte array
    allocByteArray();

    //Use Zlib to uncompress the zipped data to byte_array
    int result = uncompress( byte_array, (uLongf*)&byte_length,
        zipped, zipped_length );
    
    //Problem?
    if (result != Z_OK)
    {
        if (byte_array != NULL) { deleteByteArray(); }
        byte_array = NULL;
        byte_length = 0;
        return false;
    }
    
    //Now, copy the byte array to the block array
    unpackBlocks(free_zip);
    if (free_zip) {
        deleteZipArray();
    }
    
    isUnzipped=true;
    
    return true;
}
