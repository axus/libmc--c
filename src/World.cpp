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

#include "World.hpp"

using std::vector;
using mc__::World;
using mc__::Chunk;
using mc__::Block;
using mc__::chunkIterator;

//Create empty world
World::World(): spawn_X(0), spawn_Y(0), spawn_Z(0)
{
}

//Destroy world
World::~World()
{
    //Delete all chunks and YchunkMaps
    XZChunksMap_t::iterator iter_xz;
    for (iter_xz = coordChunksMap.begin();
        iter_xz != coordChunksMap.end(); iter_xz++)
    {
        YChunkMap_t* Ychunks = iter_xz->second;
        if (Ychunks != NULL) {
            YChunkMap_t::iterator iter_y;
            for (iter_y = Ychunks->begin(); iter_y != Ychunks->end(); iter_y++)
            {
                Chunk* chunk=iter_y->second;
                if (chunk != NULL) { delete chunk; }
            }
        }
    }
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
        addChunk(chunk);
        
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

//Add chunk to world
bool World::addChunk( Chunk *chunk)
{
    //Can't add null chunks
    if (chunk == NULL) { return false; }
  
    bool result=true;
    const uint64_t key = getKey(chunk->X, chunk->Z);
    
    //Lookup stack of chunks by XZ
    XZChunksMap_t::const_iterator iter_xz = coordChunksMap.find( key );
    
    //Create a new stack if needed and set iter_xz
    if (iter_xz == coordChunksMap.end()) {
        coordChunksMap.insert( XZChunksMap_t::value_type(key, new YChunkMap_t));
        iter_xz = coordChunksMap.find(key);
    }
    
    //Get stack of chunks at XZ
    YChunkMap_t* Ychunks = iter_xz->second;
    if (Ychunks == NULL) {
        //This should never happen.
        Ychunks = new YChunkMap_t;
    }
    
    //Check for existing chunk, delete it if found
    YChunkMap_t::const_iterator iter_y = Ychunks->find(chunk->Y);
    if (iter_y != Ychunks->end()) {

        //Delete the old chunk
        Chunk* oldChunk = iter_y->second;
        if (oldChunk != NULL) {
            //Had to delete old chunk!
            result = false;
            delete oldChunk;
        }
    }
    
    //Assign new chunk
    //Ychunks->insert( YChunkMap_t::value_type( chunk->Y, chunk));
    (*Ychunks)[chunk->Y] = chunk;
    
    return true;
}

//Return chunk pointer at X,Y,Z if it exists, NULL otherwise
mc__::Chunk* World::getChunk(int32_t X, int8_t Y, int32_t Z)
{
    mc__::Chunk* result;
        
    //Lookup stack of chunks by XZ
    XZChunksMap_t::const_iterator iter_xz = coordChunksMap.find( getKey(X,Z));
    
    //Return NULL if XZ stack not found
    if (iter_xz == coordChunksMap.end()) { return NULL; }
    YChunkMap_t* yChunks = iter_xz->second;
    if (yChunks == NULL) { return NULL; }
    
    //Look for chunk at Y coordinate
    YChunkMap_t::const_iterator iter_y = yChunks->find( Y );
    if (iter_y == yChunks->end()) { return NULL; }
    result = iter_y->second;
    
    //Return chunk at location    
    return result;
}

//Return chunk pointer at X,Y,Z .  Erase old chunk if it exists
mc__::Chunk* World::newChunk(int32_t X, int8_t Y, int32_t Z,
                            uint8_t size_X, uint8_t size_Y, uint8_t size_Z)
{
    //Create the new chunk
    mc__::Chunk* chunk = new Chunk(size_X, size_Y, size_Z, X, Y, Z);
        
    //Add the chunk pointer to world
    addChunk( chunk );
    
    //Return chunk at location    
    return chunk;
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

    //Pack blocks in chunk and zip
    testChunk->packBlocks();
    testChunk->zip();

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

    //Pack blocks in chunk and zip
    flatChunk->packBlocks();
    flatChunk->zip();

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
    treeChunk->packBlocks();
    treeChunk->zip();

    //Map (X | Z | Y) -> Chunk*
    addChunk(treeChunk );
    
    return true;
}

//
//Minimal iterator object class for listing all world chunks
//

//        XZChunksMap_t::const_iterator iter_xz;
//        YChunkMap_t::const_iterator iter_y;
chunkIterator::chunkIterator( const World& w):
    world(w), chunks(NULL), chunk(NULL)
{
    //Assign internal iterators to start of world coordinate map
    iter_xz = world.coordChunksMap.begin();
    
    //Move iterator until a non-null chunk is found
    while (chunks == NULL && iter_xz != world.coordChunksMap.end()) {
        chunks = iter_xz->second;
        if (chunks == NULL) {
            iter_xz++;
        } else {
            //Search until non-null chunk found
            iter_y = chunks->begin();
            while (iter_y != chunks->end() && chunk == NULL) {
                chunk = iter_y->second;
                iter_y++;
            }

        }
    }
}

//Increment this iterator
chunkIterator& chunkIterator::operator++(int)
{
    //assume chunks and iter_y are valid at start        
    chunk = NULL;
    while (chunk == NULL && iter_xz != world.coordChunksMap.end()) {
        //Look for non-null chunk
        for (iter_y++; chunk == NULL && iter_y != chunks->end(); iter_y++) {
            chunk = iter_y->second;
        }
        
        //Move to next chunk list if needed
        if (iter_y == chunks->end()) {
          
            //Done with this stack, go to the next one
            iter_xz++;
            chunks = NULL;
            
            //Repeat until chunks and iter_y found, or end
            while (chunks == NULL && iter_xz != world.coordChunksMap.end()) {
                chunks = iter_xz->second;
                //Stil not found, keep looking
                if (chunks == NULL) {
                    iter_xz++;
                } else {
                    iter_y = chunks->begin();
                }
            }
        }
    }

    return *this;
}

//Derefence the iterator to get a chunk
Chunk* chunkIterator::operator*()
{
    //Return the currently pointed chunk
    return chunk;
}

//Check if we're at the end of the world
bool chunkIterator::end()
{
    return (iter_xz == world.coordChunksMap.end());
}

//Comparison operators
bool mc__::operator==(chunkIterator& left, chunkIterator& right)
{
    return (left.iter_xz == right.iter_xz && left.iter_y == right.iter_y);
}

bool mc__::operator!=(chunkIterator& left, chunkIterator& right)
{
    return !(left == right);
}
