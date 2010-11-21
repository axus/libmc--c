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

//Destroy world
World::~World()
{
    //Delete allocated chunks
    uint64Chunk0Map_t::iterator iter;
    mc__::Chunk* chunk0;
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
    mc__::Chunk *newChunk = new mc__::Chunk(size_X-1, size_Y-1, size_Z-1, X, Y, Z);
    addChunk(newChunk);

    return true;
}

//Will be deleted when World ends
bool World::addChunk( Chunk *chunk)
{
    if (chunk == NULL) return false;
  
    //Map (X | Z | Y) -> Chunk*
    uint64_t key =
        (((uint64_t)chunk->X<<40)&0xFFFFFFFF00000000)|
        (((uint64_t)chunk->Z<<8)&0xFFFFFFFF00)|
        (chunk->Y&0xFF);
    coordChunkMap[key] = chunk;
    
    return true;
}


//Generate chunk representing block ID 0 - 95
bool World::genChunkTest(int32_t X, int8_t Y, int32_t Z) {
    
    const uint8_t size_X=16, size_Y=11, size_Z=1;

    //Allocate chunk
    mc__::Chunk *testChunk = new mc__::Chunk(size_X-1, size_Y-1, size_Z-1, X, Y, Z);
    
    //Allocate array of blocks
    mc__::Block *&firstBlockArray = testChunk->block_array;

    //Index variables
    size_t index=0, ID_x=0x00, ID_y=0x50;
    
    //Assign blocks from bottom right to top right.
    for (index=0; index < testChunk->array_length; index++ ) {

        //Every other row, create air.  This allows the viewer to see tops and bottoms.

        if ((index%size_Y)&1) {
            firstBlockArray[index].blockID = 0;
        } else {
            ID_y = (5 - (index%size_Y)/2)<<4;
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
    mc__::Chunk *flatChunk = new mc__::Chunk(size_X-1, size_Y-1, size_Z-1, X, Y, Z);
    
    //Allocate array of blocks
    mc__::Block *&firstBlockArray = flatChunk->block_array;

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
    uint8_t treeType)
{
    
    uint8_t size_X, size_Y, size_Z, off_x, off_y, off_z;
    int32_t origin_X, origin_Z;
    
    //Tree properties determined by treeType
    switch( treeType ) {
        case 0:
        default:
            size_X=5; size_Z=5; size_Y=7;
            origin_X= X-3; origin_Z= Z-3;
            break;
    }

    //Allocate chunk
    mc__::Chunk *treeChunk = new mc__::Chunk(size_X-1, size_Y-1, size_Z-1,
        origin_X, Y, origin_Z);
    
    //Allocate array of blocks
    mc__::Block *&firstBlockArray = treeChunk->block_array;

    //Index variables
    size_t index=0;
    uint8_t ID;

    //Write every block in chunk.  x,y,z determined by position in array.
    for (off_x=0; off_x <= size_X; off_x++) {
    for (off_z=0; off_z <= size_Z; off_z++) {
    for (off_y=0; off_y <= size_Y; off_y++) {
        //Default is air
        ID=0;
      
        if (off_y < 5 && off_x == X && off_z == Z) {
            //Draw trunk up to trunk height
            ID = 17;
        } else {
            //Drawing depends on current height
            switch(off_y) {
                case 0:
                case 1:
                case 2:
                    //Air at levels 0-2
                    break;
                case 3:
                    //Leaves in a pattern around trunk
                    if (off_x + off_z > 1 && off_x + off_z < (size_X + size_Z - 3)) {
                        ID=19;
                    }
                    break;
                case 4:
                    //Leaves in a bigger pattern around trunk
                    if (off_x + off_z > 0 && off_x + off_z < (size_X + size_Z - 2)) {
                        ID=19;
                    }
                    break;                
                case 5:
                    //Leaves in a square, no trunk
                    if (off_x + off_z > 0 && off_x + off_z < 8) {
                        ID=19;
                    }
                    break;                
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

