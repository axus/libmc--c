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

#ifndef MC__WORLD_H
#define MC__WORLD_H


//mc__ classes
#include "MapChunk.hpp" //includes "Chunk.hpp"

//STL list
#include <unordered_map>      //map / unordered_map / hash_map

//Define class inside mc__ namespace
namespace mc__ {
    
    //Map Coordinates to MapChunk
    typedef std::unordered_map< uint64_t, MapChunk* > XZMapChunk_t;
    
    //List of mini-chunks to update
    typedef std::unordered_set< Chunk* > chunkSet_t;

    //World class ;)
    class World {
        
        public:
            //Genesis
            World();
            
            //Armageddon
            ~World();
            
            //Add compressed chunk to list/map
            bool addChunkZip(int32_t X, int8_t Y, int32_t Z,
                uint8_t size_X, uint8_t size_Y, uint8_t size_Z,
                uint32_t ziplength, uint8_t *zipped, bool unzip=true);
            
            //Return chunk at X,Y,Z
            mc__::MapChunk* getChunk(int32_t X, int32_t Z);
            
            //Return new chunk at X,Y,Z, erasing old chunk
            mc__::Chunk* newChunk(int32_t X, int8_t Y, int32_t Z,
                uint8_t size_X, uint8_t size_Y, uint8_t size_Z,
                bool unzipped=true);
            
            //Unzip all new chunks into MapChunks
            bool updateMapChunks(bool cleanup=true);

            //Add one mini-chunk to the map
            bool addMapChunk( mc__::Chunk *chunk);
            
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
            uint64_t getKey(const int32_t X, const int32_t Z) const;
            
            //Access this to see chunks in the world
            XZMapChunk_t coordMapChunks;    //X|Z -> MapChunk*
            
            //List of mini-chunks to apply to world
            chunkSet_t chunkUpdates;
            
            //World spawn point
            int32_t spawn_X;
            int8_t spawn_Y;
            int32_t spawn_Z;
            
            //TODO: list of warp points?
            
            bool debugging;

        protected:
           //Will be deleted when World ends
            bool addChunkUpdate( Chunk *chunk);
    };

}

#endif