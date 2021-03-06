/*
  mc__::MapChunk
  16x128x16 complete Chunk, with visibility data
  
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

#ifndef MC__MAPCHUNK_H
#define MC__MAPCHUNK_H

//mc--
#include "Chunk.hpp"

//STL


namespace mc__ {

    //Fixed size Chunk
    class MapChunk : public Chunk {
        public:
            //When indexing block in MapChunk
            //index = y|(z << 7)|(x << 11)   Size_Y=127, Size_Z=15
            static const uint16_t mapChunkBlockMax = (1<<(4+7+4));  //32K blocks

            //Calculate space and set X,0,Z
            //  Actual size is 16x128x16
            //  ID, metadata, and lighting are set to 0
            MapChunk(int32_t x, int32_t z);
            
            //Deallocate chunk space
            //~MapChunk();

            //Update with (mini)-chunk
            bool addChunk( const mc__::Chunk *update);

            //Recalculate visibility for all blocks
            bool recalcVis();

            //Neighbors: Adjacent map chunks on -X, +X, -Y, +Y, -Z, +Z
            mc__::MapChunk *neighbors[6];
            
            //8 bits: [ A | B | C | D | E | F | invisible | self ] (1=opaque)
            // IF A BIT IS SET, THAT FACE IS NOT DRAWN
            uint8_t visflags[mapChunkBlockMax];
            
            //Ordered list of block indices to draw
            indexList_t visibleIndices;
            
            //flags used by Viewer:
            //  VISIBLE     = draw this chunk
            //  UPDATED     = recalculate visible faces, update adjacent, redraw
            //  LOADED      = received complete map chunk from server
            enum FLAGS { VISIBLE=0x1, UPDATED=0x2, LOADED=0x4, DRAWABLE=0x5,
                ADJ_UPDATED=0x8};
            uint32_t flags;
        protected:
            bool updateVisFlags(uint16_t i, bool adj[6], indexList_t& changes);
            bool updateVisRange(const mc__::Chunk *chunk,
                uint8_t off_x, uint8_t off_y, uint8_t off_z,
                uint8_t max_x, uint8_t max_y, uint8_t max_z);
    };
}

#endif