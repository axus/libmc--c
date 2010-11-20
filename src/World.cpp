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

//Load a compressed chunk
bool World::addCompressedChunk( )
{
    return true;
}

//Generate chunk representing block ID 0 - 96
bool World::genChunkTest(int32_t X, int8_t Y, int32_t Z) {
    
    const uint8_t size_X=16, size_Y=13, size_Z=1;

    //Allocate chunk
    mc__::Chunk *firstChunk = new mc__::Chunk(size_X-1, size_Y-1, size_Z-1, X, Y, Z);
    
    //Allocate array of blocks
    mc__::Block *&firstBlockArray = firstChunk->block_array;

    //Index variables
    size_t index=0, ID_x=0x00, ID_y=((size_Y/2)<<4);
    
    //Assign blocks from bottom right to top right.
    for (index=0; index < firstChunk->array_length; index++ ) {

        //Every other row, create air.  This allows the viewer to see tops and bottoms.
        if (index&0x10) {
            firstBlockArray[index].blockID = 0;
        } else {
            firstBlockArray[index].blockID = (ID_y|ID_x);
            ID_x++;
            //Check for next row
            if (!(ID_x&0xF)) {
                ID_x=0;
                ID_y -= 0x10;
            }
        }
    }

    //Map (X | Z | Y) -> Chunk*
    uint64_t key = (((uint64_t)X<<40)&0xFFFFFFFF00000000)|(((uint64_t)Z<<8)&0xFFFFFFFF00)|(Y&0xFF);
    coordChunkMap[key] = firstChunk;
    
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
    
    //Assign blocks from bottom right to top right.
    for (index=0; index < flatChunk->array_length; index++ ) {

        //Alternate between dirt and grass
        if (index&0x10) {
            firstBlockArray[index].blockID = 2; //Grass
        } else {
            firstBlockArray[index].blockID = 3; //Dirt
        }
    }

    //Map (X | Z | Y) -> Chunk*
    uint64_t key = (((uint64_t)X<<40)&0xFFFFFFFF00000000)|(((uint64_t)Z<<8)&0xFFFFFFFF00)|(Y&0xFF);
    coordChunkMap[key] = flatChunk;
    
    return true;
}

