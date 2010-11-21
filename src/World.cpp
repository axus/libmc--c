/*
  mc__::World
    Store chunks comprising Minecraft game world
  
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

#include "World.h"

using std::vector;
using mc__::World;
using mc__::Chunk;
using mc__::Block;

//Destroy world
World::~World()
{
    //Delete allocated chunks
    uint64Chunk0Map_t::iterator iter;
    Chunk* chunk0;
    for (iter = coordChunkMap.begin(); iter != coordChunkMap.end(); iter++)
    {
        chunk0 = iter->second;
        if (chunk0 != NULL) {
            delete chunk0;
        }
    }
    
}

//Copy uncompressed chunk data
bool World::addChunk(uint8_t* data,
    int32_t X, int8_t Y, int32_t Z,
    uint8_t size_X, uint8_t size_Y, uint8_t size_Z)
{
    //Allocate chunk
    Chunk *newChunk = new Chunk(size_X-1, size_Y-1, size_Z-1, X, Y, Z);
    addChunk(newChunk);

    return true;
}

//Will be deleted when World ends
bool World::addChunk( Chunk *chunk)
{
    if (chunk == NULL) return false;

    uint64_t key = ( (uint64_t)(chunk->X & 0x0FFFFFFF) << 40 )|
                   ( (uint64_t)(chunk->Z & 0x0FFFFFFF) << 8 )|
                   ( (uint64_t)(chunk->Y & 0xFF));
    coordChunkMap[key] = chunk;
    
    return true;
}


//Generate chunk representing block ID 0 - 95
bool World::genChunkTest(int32_t X, int8_t Y, int32_t Z) {
    
    const uint8_t size_X=16, size_Y=11, size_Z=1;

    //Allocate chunk
    Chunk *testChunk =
        new Chunk(size_X-1, size_Y-1, size_Z-1, X, Y, Z);
    
    //Allocate array of blocks
    Block *&firstBlockArray = testChunk->block_array;

    //Index variables
    size_t index=0, ID_x=0x00, ID_y=0x50;
    
    //Assign blocks from bottom right to top right.
    for (index=0; index < testChunk->array_length; index++ ) {

        //Every other row, create air.
        //This allows the viewer to see tops and bottoms.
        if ((index%size_Y)&1) {
            firstBlockArray[index].blockID = 0;
        } else {
            //blockID 0 starts in top left corner, correct for MC indexing
            ID_y = ((size_Y/2) - (index%size_Y)/2)<<4;
            ID_x = index/size_Y;
            firstBlockArray[index].blockID = (ID_y|ID_x);
        }
    }

    addChunk(testChunk );

    return true;
}

//Generate a flat chunk
bool World::genFlatGrass(int32_t X, int8_t Y, int32_t Z) {
    
    const uint8_t size_X=16, size_Y=2, size_Z=16;

    //Allocate chunk
    Chunk *flatChunk = new Chunk(size_X-1, size_Y-1, size_Z-1, X, Y, Z);
    
    //Allocate array of blocks
    Block *&firstBlockArray = flatChunk->block_array;

    //Index variables
    size_t index=0;

    //When indexing block in chunk array,
    //index = y + (z * (Size_Y+1)) + (x * (Size_Y+1) * (Size_Z+1))

    //Assign blocks from bottom right to top right.
    for (index=0; index < flatChunk->array_length; index++ ) {

        //Alternate between dirt and grass
        if (index&1) {
            firstBlockArray[index].blockID = 2; //Grass
        } else {
            firstBlockArray[index].blockID = 3; //Dirt
        }
    }

    //Map (X | Z | Y) -> Chunk*
    addChunk(flatChunk );
    
    return true;
}

//Generate a leafy tree with base at (X,Y,Z), chunk origin will be different
bool World::genTree(const int32_t X, const int8_t Y, const int32_t Z,
    uint8_t size_X, uint8_t size_Y, uint8_t size_Z, uint8_t leavesID)
{
    //Offsets
    uint8_t off_x, off_y, off_z;
    
    //Block ID for log
    const uint8_t logID=17;
    
    //Calculate chunk origin
    int32_t origin_X = X - size_X/2;
    int32_t origin_Z = Z - size_Z/2;    

    //Allocate chunk
    Chunk *treeChunk = new Chunk(size_X-1, size_Y-1, size_Z-1,
        origin_X, Y, origin_Z);
    
    //Allocate array of blocks
    Block *&firstBlockArray = treeChunk->block_array;

    //index = y + (z * (Size_Y+1)) + (x * (Size_Y+1) * (Size_Z+1))
    size_t index=0;
    
    //Block ID depends on distance to center of tree
    uint8_t ID, center_dist, center_x, center_y, center_z;

    //Write every block in chunk.  x,y,z determined by position in array.
    for (off_x=0; off_x < size_X; off_x++) {
    for (off_z=0; off_z < size_Z; off_z++) {
    for (off_y=0; off_y < size_Y; off_y++) {
        //Default is air
        ID=0;
      
        //Calculate distance from "Y-axus" of tree
        center_x = (size_X/2 < off_x ? off_x - size_X/2 : size_X/2 - off_x );
        center_z = (size_Z/2 < off_z ? off_z - size_Z/2 : size_Z/2 - off_z );
        center_y = (size_Y/2 < off_y ? off_y - size_Y/2 : size_Y/2 - off_y );
        center_dist = center_x + center_z;
                
        if (center_dist == 0 && off_y < size_Y-2) {
            //Draw trunk up to trunk height - 2
            ID = logID;
        } else if (off_y > 1) {
            //Leaves over 2 height
            if (center_dist < (size_X+size_Z)/4 + (size_Y - center_y - 1) - 5 ) {
                ID=leavesID;
            }
        }
        
        //Assign block ID and increment
        firstBlockArray[index].blockID = ID;
        index++;
    }}}

    //Map (X | Z | Y) -> Chunk*
    addChunk(treeChunk );
    
    return true;
}

