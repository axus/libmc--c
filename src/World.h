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

#ifndef MC__WORLD_H
#define MC__WORLD_H

//STL list
#include <vector>

/*
//Hash_map... faster than map?
#ifdef __GNUC__
    #if __GNUC__ <= 2
        #include <hash_map>
        using std::hash_map;
    #else
        #include <ext/hash_map>
        using __gnu_cxx::hash_map;
    #endif
#elif defined (_MSC_VER)
    #if _MSC_VER < 1500
        #include "stl_hash.h"
        using std::hash_map;
    #else
        #include <hash_map>
        using stdext::hash_map
    #endif
#endif

//Hash function for 64 bits (X&0x3FFF)<<36|(Z&0x3FFF)<<8|Y
struct hash_uint64 {
  size_t operator()(const uint64_t in)  const {
    uint64_t ret = (in >> 32L) ^ (in & 0xFFFFFFFF);
    return (size_t) ret;
}   //TODO: bucket_size and min_buckets for MSVC

//TODO: Replace with unordered_map
*/
#include <map>

//mc__ classes
#include "Chunk.h"

//Define class inside mc__ namespace
namespace mc__ {
    
    //Types
    typedef std::vector<Chunk> chunkVector;
    //typedef hash_map< uint64_t, Chunk*, hash_uint64 > uint64Chunk0Map_t;
    typedef std::map< uint64_t, Chunk*> uint64Chunk0Map_t;

    //World class ;)
    class World {
        
        public:
            //Armageddon
            ~World();
            
            //Add uncompressed chunk to list/map
            bool addChunk(uint8_t* data,
                int32_t X, int8_t Y, int32_t Z,
                uint8_t size_X, uint8_t size_Y, uint8_t size_Z);
                
            mc__::Chunk* getChunk(int32_t X, int8_t Y, int32_t Z);

            //Generate chunk 16x13x1 containing block ID 0 - 96
            bool genChunkTest(int32_t X, int8_t Y, int32_t Z);
            
            //Generate chunk 16x2x16 containing grass over dirt
            bool genFlatGrass(int32_t X, int8_t Y, int32_t Z);

            //Generate a tree with bottom at X,Y,Z
            //  optional: dimensions size_X, size_Y, size_Z
            bool genTree(const int32_t X, const int8_t Y, const int32_t Z,
                uint8_t size_X=5, uint8_t size_Y=8, uint8_t size_Z=5,
                uint8_t leavesID=18);
            
            //Check key for coordinates
            uint64_t getKey(const int32_t X, const int8_t Y, const int32_t Z) const;
            
            //Access this to see chunks in the world
            uint64Chunk0Map_t coordChunkMap;

        protected:
           //Will be deleted when World ends
            bool addChunk( Chunk *chunk);
    };
}

#endif