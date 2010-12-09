/*
  mc__::World
    Store chunks comprising a voxel game world
  
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

//STL
#include <iostream>
using std::cerr;
using std::cout;
using std::endl;
using std::hex;
using std::dec;

//mc--
#include "World.hpp"
using mc__::World;
using mc__::Chunk;
using mc__::Block;

//using mc__::chunkIterator;

//Create empty world
World::World(): spawn_X(0), spawn_Y(0), spawn_Z(0), debugging(false)
{
}

//Destroy world
World::~World()
{

    //Delete all map chunks
    XZMapChunk_t::iterator iter_xz;  
    for (iter_xz = coordMapChunks.begin();
        iter_xz != coordMapChunks.end(); iter_xz++) {
        MapChunk* mc=iter_xz->second;
        if (mc != NULL) { delete mc; } else {
            cerr << "delete Null MapChunk" << endl;}
    }
    coordMapChunks.clear();

    //Delete all unused mini-chunks
    chunkSet_t::iterator iter_chunk;
    for (iter_chunk = chunkUpdates.begin(); iter_chunk != chunkUpdates.end();
        iter_chunk++)
    {
        Chunk* chunk=*iter_chunk;
        if ( chunk != NULL) { delete chunk; } else {
            cerr << "delete Null Chunk" << endl;}
    }
    chunkUpdates.clear();
}

//Add compressed chunk to list/map
//  Return false if chunk could not be added
bool World::addChunkZip(int32_t X, int8_t Y, int32_t Z,
    uint8_t size_X, uint8_t size_Y, uint8_t size_Z,
    uint32_t ziplength, uint8_t *zipped, bool unzip)
{
    bool result=false;
    
    Chunk* chunk = new Chunk(size_X, size_Y, size_Z, X, Y, Z);
    if (chunk) {
        chunk->copyZip(ziplength, zipped);
        addChunkUpdate(chunk);
        
        //byte_array and block_array are still NULL!
        if (unzip) {
            result = chunk->unzip();
            chunk->unpackBlocks();
        } else {
            result=true;
        }
        
    }
    return result;
}

/*
//Return absolute distance of X,Y,Z coordinates from (0,0,0):
uint64_t World::originDistance(int32_t X, int8_t Y, int32_t Z) {

    uint32_t Xsign  = (X>>31);
    uint32_t Xabs   = (X ^ Xsign - Xsign);
    uint32_t Zsign  = (Z>>31);
    uint32_t Zabs   = (Z ^ Zsign - Zsign);
    uint8_t Ysign   = (Y >> 7);
    uint8_t Yabs    = (Y ^ Ysign - Ysign);
    uint64_t distance = Xabs + Zabs + Y;

    return distance;
}
*/

//Return 64 bit key for X,Z coordinate
uint64_t World::getKey(const int32_t X, const int32_t Z) const
{
    uint64_t key = (((((uint64_t)X) << 32)& 0xFFFFFFFF00000000) |
                     (((uint64_t)Z) & 0x00000000FFFFFFFF));
    return key;
}

//Add chunk to mini-chunk list.  Still needs to be added to map!
bool World::addChunkUpdate( Chunk *chunk)
{
    //Don't add null chunks
    if (chunk == NULL) { return false; }
    
    //Add chunk to set.  Ignores duplicate pointer adds ;p
    chunkUpdates.insert(chunk);
    
    return true;

}

//Return MapChunk pointer at X,Y,Z if it exists, NULL otherwise
mc__::MapChunk* World::getChunk(int32_t X, int32_t Z)
{
    mc__::MapChunk* result=NULL;
    
    uint64_t key = getKey(X, Z);
    XZMapChunk_t::const_iterator iter_xz = coordMapChunks.find(key);
    if (iter_xz != coordMapChunks.end()) {
        result = iter_xz->second;
    }
    
    return result;
}

//Allocate new chunk on to-be-added list
mc__::Chunk* World::newChunk(int32_t X, int8_t Y, int32_t Z,
            uint8_t size_X, uint8_t size_Y, uint8_t size_Z, bool unzipped)
{
    //Create the new chunk, with no data in it
    mc__::Chunk* chunk = new Chunk(size_X, size_Y, size_Z, X, Y, Z, unzipped);
        
    //Add the chunk pointer to list of to-be-added chunks
    addChunkUpdate( chunk );
    
    //Caller must add data to the chunk!
    
    //Return chunk at location    
    return chunk;
}

//Unzip/copy one mini-chunk to appropriate map chunk
bool World::addMapChunk( const Chunk* chunk)
{
    //Validate pointer
    if (chunk == NULL) {
        return false;
    }
    bool result;
    
    //Get X/Z of MapChunk from chunk
    int32_t X = chunk->X & 0xFFFFFFF0;
    int32_t Z = chunk->Z & 0xFFFFFFF0;
    uint64_t key = getKey(X, Z);
    
    MapChunk *mapchunk;

    //Look for existing MapChunk
    XZMapChunk_t::const_iterator iter = coordMapChunks.find(key);    
    if (iter == coordMapChunks.end()) {
      
        //Create a new MapChunk in coordMapChunks if needed
        mapchunk = new MapChunk(X, Z);
        coordMapChunks.insert( XZMapChunk_t::value_type(key, mapchunk));
        mapChunks.push_back( mapchunk );
        
        MapChunk* neighbor;
        //Check neighbor A (-X)
        iter = coordMapChunks.find( getKey(X-16, Z) );
        if (iter != coordMapChunks.end()) {
            //Double-link neighbors
            neighbor = iter->second;
            mapchunk->neighbors[0] = neighbor;
            neighbor->neighbors[1] = mapchunk;
        }

        //Check neighbor B (+X)
        iter = coordMapChunks.find( getKey(X+16, Z) );
        if (iter != coordMapChunks.end()) {
            //Double-link neighbors
            neighbor = iter->second;
            mapchunk->neighbors[1] = neighbor;
            neighbor->neighbors[0] = mapchunk;
        }

        //Check neighbor E (-Z)
        iter = coordMapChunks.find( getKey(X, Z-16) );
        if (iter != coordMapChunks.end()) {
            //Double-link neighbors
            neighbor = iter->second;
            mapchunk->neighbors[4] = neighbor;
            neighbor->neighbors[5] = mapchunk;
        }

        //Check neighbor F (+Z)
        iter = coordMapChunks.find( getKey(X, Z+16) );
        if (iter != coordMapChunks.end()) {
            //Double-link neighbors
            neighbor = iter->second;
            mapchunk->neighbors[5] = neighbor;
            neighbor->neighbors[4] = mapchunk;
        }

    } else {
        //MapChunk already exists
        mapchunk = iter->second;
    }
    
    //Finally, add the mini-chunk to the MapChunk
    result = mapchunk->addChunk(chunk);
    
    return result;
}


//Unzip all mini-chunks into MapChunks
bool World::updateMapChunks(bool cleanup)
{

    //Apply all unused mini-chunks to map then delete them
    chunkSet_t::const_iterator iter_chunk;
    for (iter_chunk = chunkUpdates.begin(); iter_chunk != chunkUpdates.end();
        iter_chunk++)
    {
        Chunk* chunk=*iter_chunk;
        if ( chunk != NULL)
        {
            //Get the next chunk pointer
            Chunk* chunk = *iter_chunk;
            
            //Unzip chunk if needed
            if (! chunk->isUnzipped ) {
                chunk->unzip();
            }
            
            //Add chunk to map (uncompresses if needed)
            if (addMapChunk(chunk)) {
              //DEBUG
              cout << "Updated chunk to map @ " << chunk->X
                << "," << (int)chunk->Y << "," << chunk->Z << endl;
            } else {
                cerr << "Error updating chunk to map @ X=" << chunk->X
                << " Y=" << (int)chunk->Y << "Z=" << chunk->Z << endl;
            }
            
            //Delete the mini-chunk
            if (cleanup) { delete chunk; chunk = NULL;}
        } else {
            cerr << "update Null Chunk" << endl;
        }
        
    }

    //Clear the mini-chunk list
    if (cleanup) { chunkUpdates.clear(); }

    return true;

}

//Generate chunk representing block ID 0 - 95
bool World::genChunkTest(int32_t X, int8_t Y, int32_t Z) {
    
    const uint8_t size_X=16, size_Y=11, size_Z=1;

    //Align to MapChunk grid
    int32_t chunkX = X & 0xFFFFFFF0;

    //Allocate chunk
    Chunk *testChunk =
        new Chunk(size_X-1, size_Y-1, size_Z-1, chunkX, Y, Z);
    
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

    //Pack blocks in chunk and zip
    //testChunk->packBlocks();
    //testChunk->zip();

    bool result=addMapChunk(testChunk );

    return result;
}

//Generate a flat chunk and insert to MapChunks
bool World::genFlatGrass(int32_t X, int8_t Y, int32_t Z, uint8_t height) {
    
    const uint8_t size_X=16, size_Y=height, size_Z=16;
    
    //Align grass to MapChunk grid
    int32_t chunkX = X & 0xFFFFFFF0;
    int32_t chunkZ = Z & 0xFFFFFFF0;

    //Allocate mini-chunk
    Chunk *flatChunk = new Chunk(size_X-1, size_Y-1, size_Z-1, chunkX, Y, chunkZ);
    
    //Allocate array of blocks
    Block *&firstBlockArray = flatChunk->block_array;

    //Index variables
    size_t index=0;

    //When indexing block in chunk array,
    //index = y + (z * (Size_Y+1)) + (x * (Size_Y+1) * (Size_Z+1))

    //Assign blocks from bottom right to top right.
    for (index=0; index < flatChunk->array_length; index++ ) {

        //Bedrock, stone, dirt, or grass
        uint8_t gen_y = (index & ((1<<7)-1) );
        if (gen_y < 2) {
            firstBlockArray[index].blockID = 7; //Bedrock
        } else if (gen_y < height - 4) {
            firstBlockArray[index].blockID = 1; //Stone
        } else if (gen_y < height - 1) {
            firstBlockArray[index].blockID = 3; //Dirt
        } else {
            firstBlockArray[index].blockID = 2; //Grass
        }
    }

    //Pack blocks in chunk and zip
    //flatChunk->packBlocks();
    //flatChunk->zip();

    //Map (X | Z | Y) -> Chunk*
    bool result=addMapChunk(flatChunk );
    
    return result;
}

//Generate brick of block ID @ x,y,z size_X*size_Y*size_Z
bool World::genWall(int32_t X, int8_t Y, int32_t Z,
    int32_t size_X, int8_t size_Y, int32_t size_Z, uint8_t ID)
{
    //Allocate mini-chunk
    Chunk *brickChunk = new Chunk(size_X-1, size_Y-1, size_Z-1, X, Y, Z);
    
    //Point at array of blocks
    Block *&blockArray = brickChunk->block_array;

    //Index variables
    size_t index=0;

    //When indexing block in chunk array,
    //index = y + (z * (Size_Y+1)) + (x * (Size_Y+1) * (Size_Z+1))

    //Assign blocks from bottom right to top right.
    for (index=0; index < brickChunk->array_length; index++ ) {
        //All one type
        blockArray[index].blockID = ID;
    }

    //Pack blocks in chunk and zip
    //brickChunk->packBlocks();
    //brickChunk->zip();

    //Map (X | Z | Y) -> Chunk*
    bool result=addMapChunk(brickChunk );   //TODO: split it up!
    
    return result;
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
    
    //Move origin if tree is on chunk boundary
    if ((origin_X & 0xFFFFFFF0) != (X & 0xFFFFFFF0)) {
        origin_X = X;
    }
    if ((origin_Z & 0xFFFFFFF0) != (Z & 0xFFFFFFF0)) {
        origin_Z = Z;
    }

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
        center_x = (size_X/2 < off_x ? off_x - size_X/2 : size_X/2 - off_x);
        
        for (off_z=0; off_z < size_Z; off_z++) {  
            center_z = (size_Z/2 < off_z ?off_z - size_Z/2: size_Z/2 - off_z);
            
            //Calculate distance from "Y-axus" of tree
            center_dist = center_x + center_z;

            for (off_y=0; off_y < size_Y; off_y++) {
                center_y=(size_Y/2<off_y? off_y - size_Y/2: size_Y/2 - off_y);
                
                //Default is air
                ID=0;
      
                if (center_dist == 0 && off_y < size_Y-2) {
                    //Draw trunk up to trunk height - 2
                    ID = logID;
                } else if (off_y > 1) {
                    //Leaves over 2 height
                    if (center_dist <
                        ((size_X+size_Z)/4 + (size_Y - center_y - 1) - 5))
                    {
                        ID=leavesID;
                    }
                }
        
                //Assign block ID and increment
                firstBlockArray[index].blockID = ID;
                index++;
            }
        }
    }

    //Pack blocks in chunk, and zip
    //treeChunk->packBlocks();
    //treeChunk->zip();

    //Map (X | Z | Y) -> Chunk*
    bool result=addMapChunk(treeChunk );
    
    return result;
}
