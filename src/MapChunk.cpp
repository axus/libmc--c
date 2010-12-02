/*
  mc__::MapChunk
  16x128x16 complete Chunk, with visibility data
  
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

//C
#include <cstring>  //memset

//mc--
#include "MapChunk.hpp"
using mc__::MapChunk;
using mc__::Chunk;
using mc__::Block;

//STL
#include <iostream>
using std::set;
using std::cerr;
using std::endl;

//Constructor
MapChunk::MapChunk( int32_t X, int32_t Z):
    Chunk(15, 127, 15, X, 0, Z), flags(0)
{
    //Someone must set neighbors later
    neighbors[0] = NULL;
    neighbors[1] = NULL;
    neighbors[2] = NULL;
    neighbors[3] = NULL;
    
    //Default everything visible and blocked
    memset( visflags, 0xFD, mapChunkBlockMax);
}

//Add chunk, update visibility
bool MapChunk::addChunk( Chunk *chunk)
{
    //Validate
    if (chunk == NULL) {
        return false;
    }
    if ((chunk->X < X)||(chunk->X > X+15)||(chunk->Z < Z)||(chunk->Z > Z+15))
    {
        cerr << "addChunk out of range!" << endl;
        return false;
    }
    
    //References to chunk data structures
    uint8_t in_x = chunk->X & 0x0F;
    uint8_t in_y = chunk->Y & 0x7F;
    uint8_t in_z = chunk->Z & 0x0F;
    uint8_t max_x = chunk->size_X + in_x;
    uint8_t max_y = chunk->size_Y + in_y;
    uint8_t max_z = chunk->size_Z + in_z;
    
    //Track adjacency to neighbor MapChunk
    bool adjA=false, adjB=false, adjE=false, adjF=false;
    
    uint8_t x_, y_, z_;             //internal chunk offsets
    uint16_t index;                 //internal chunk index
    uint16_t c_index=0;                //external chunk index
    
    //Track changed blocks
    set<uint16_t> changes;
    
    //For X...
    for (x_ = in_x; x_ <= max_x; x_++) {
        //Check X neighbor adjacency
        if (x_ == 0) { adjA=true; adjB=false; }
        else if (x_ < 15) { adjA=false; }
        else { adjB=true; }
        
        //Start index at X,0,0
        index = (x_ << 11);
    
    //For X,Z...
    for(z_ = in_z; z_ <= max_z; z_++) {
        //Check Z neighbor adjacency
        if (z_ == 0) { adjE=true; adjF=false;}
        else if (z_ < 15) { adjE=false; }
        else { adjF=true; }
        
        //Continue index at X,Z,0
        index |= (z_ << 7);
    
    //For X,Y,Z
    for(y_ = in_y, index|= in_y; y_ <= max_y; y_++, index++, c_index++) {
        //index = (X<<11)|(Z<<7)|y
        
        //Copy the block
        Block& block=chunk->block_array[c_index];
        block_array[index] = block;
        
        //Update visibility flags (for building visible set later)
        bool opaque = Chunk::isOpaque[block.blockID];
        
        //Air is invisible        
        if (block.blockID == 0) {
            visflags[index] |= 0x2;
        } else {
            //Everything else is visible
            visflags[index] &= ~0x2;
        }
        
        //Update visflags, get list of updated block indices
        updateVisFlags( x_, y_, z_, opaque, adjA, adjB, adjE, adjF, changes);

    }}}

    //Check updated blocks for visibility, update visibleIndices
    set<uint16_t>::const_iterator iter;
    for (iter = changes.begin(); iter != changes.end(); iter++) {
        index = *iter;
        
        //Is it visible?
        if ( (visflags[index]&0x2) != 0x2 && (visflags[index] & 0xFC) != 0xFC ) {
            visibleIndices.insert(index);
        } else {
            //Index is invisible or blocked from all sides, remove it
            visibleIndices.erase(index);
        }
    }
    
    return true;
}

//update local and neighbor visflags array for opacity at x,y,z
bool MapChunk::updateVisFlags(uint8_t x_, uint8_t y_, uint8_t z_, bool opaque,
    bool adjA, bool adjB, bool adjE, bool adjF,
    set<uint16_t>& changes)
{
    //Local index
    uint16_t index = ((x_<<11)|(z_<<7)|y_);
    
    //Adjacent block indices
    uint16_t indexA = index - (1<<11);
    uint16_t indexB = index + (1<<11);
    uint16_t indexC = index - 1;
    uint16_t indexD = index + 1;
    uint16_t indexE = index - (1<<7);
    uint16_t indexF = index + (1<<7);
    uint16_t n_index;

    //Update visibility based on opaque/not-opaque
    if (opaque) {
        //This block
        visflags[index] |= 1;
        changes.insert(index);
      
        //Set X-1
        if (!adjA || neighbors[0] == NULL) {
            visflags[indexA] |= 0x80;
            changes.insert(indexA);
        } else {
            //Change neighboring y,z block
            n_index = (y_|(z_<<7)|(0xF<<11));
            uint8_t& n_flags = neighbors[0]->visflags[n_index];
            n_flags |= 0x80;
            //Is it visible?
            if ((n_flags&0x2)!=0x2 && (n_flags & 0xFC) != 0xFC) {
                neighbors[0]->visibleIndices.insert(n_index);
            }
        }
        
        //Set X+1
        if (!adjB || neighbors[1] == NULL) {
            visflags[indexB] |= 0x40;
            changes.insert(indexB);
        } else {
            //Change neighboring y,z block
            n_index = (y_|(z_<<7) );
            uint8_t& n_flags = neighbors[1]->visflags[n_index];
            n_flags |= 0x40;
            //Is it visible?
            if ((n_flags&0x2)!=0x2 && (n_flags & 0xFC) != 0xFC) {
                neighbors[1]->visibleIndices.insert(n_index);
            }
        }
        
        //Set Y-1
        if (y_ > 0) {
            visflags[indexC] |= 0x20;
            changes.insert(indexC);
        }
        
        //Set Y+1
        if (y_ < 127) {
            visflags[indexD] |= 0x10;
            changes.insert(indexD);
        }

        //Set Z-1
        if (!adjE || neighbors[2] == NULL) {
            visflags[indexE] |= 0x08;
            changes.insert(indexE);
        } else {
            //Change neighboring y,z block
            n_index = (y_|(0xF<<7)|(x_<<11));
            uint8_t& n_flags = neighbors[2]->visflags[n_index];
            n_flags |= 0x08;
            //Is it visible?
            if ((n_flags&0x2)!=0x2 && (n_flags & 0xFC) != 0xFC) {
                neighbors[2]->visibleIndices.insert(n_index);
            }
        }
        
        //Set Z+1
        if (!adjF || neighbors[3] == NULL) {
            visflags[indexF] |= 0x04;
            changes.insert(indexF);
        } else {
            //Change neighboring y,z block
            n_index = (y_|(x_<<11));
            uint8_t& n_flags = neighbors[3]->visflags[n_index];
            n_flags |= 0x04;
            //Is it visible?
            if ((n_flags&0x2)!=0x2 && (n_flags & 0xFC) != 0xFC) {
                neighbors[3]->visibleIndices.insert(n_index);
            }
        }
        

    } else {
        //This block
        visflags[index] &= ~1;
        changes.insert(index);

        //Set X-1
        if (!adjA || neighbors[0] == NULL) {
            visflags[indexA] &= ~0x80;
            changes.insert(indexA);
        } else {
            //Change neighboring y,z block
            n_index = (y_|(z_<<7)|(0xF<<11));
            uint8_t& n_flags = neighbors[0]->visflags[n_index];
            n_flags &= ~0x80;
            //Is it invisible?
            if ((n_flags & 0x2) ||(n_flags & 0xFC) == 0xFC) {
                neighbors[0]->visibleIndices.erase(n_index);
            }
        }
        
        //Set X+1
        if (!adjB || neighbors[1] == NULL) {
            visflags[indexB] &= ~0x40;
            changes.insert(indexB);
        } else {
            //Change neighboring y,z block
            n_index = (y_|(z_<<7) );
            uint8_t& n_flags = neighbors[1]->visflags[n_index];
            n_flags &= ~0x40;
            //Is it invisible?
            if ((n_flags & 0x2) ||(n_flags & 0xFC) == 0xFC) {
                neighbors[1]->visibleIndices.erase(n_index);
            }
        }
        
        //Set Y-1
        if (y_ > 0) {
            visflags[indexC] &= ~0x20;
            changes.insert(indexC);
        }
        
        //Set Y+1
        if (y_ < 127) {
            visflags[indexD] &= ~0x10;
            changes.insert(indexD);
        }

        //Set Z-1
        if (!adjE || neighbors[2] == NULL) {
            visflags[indexE] &= ~0x08;
            changes.insert(indexE);
        } else {
            //Change neighboring y,z block
            n_index = (y_|(0xF<<7)|(x_<<11));
            uint8_t& n_flags = neighbors[2]->visflags[n_index];
            n_flags &= ~0x08;
            //Is it invisible?
            if ((n_flags & 0x2) ||(n_flags & 0xFC) == 0xFC) {
                neighbors[2]->visibleIndices.erase(n_index);
            }
        }
        
        //Set Z+1
        if (!adjF || neighbors[3] == NULL) {
            visflags[indexF] &= ~0x04;
            changes.insert(indexF);
        } else {
            //Change neighboring y,z block
            n_index = (y_|(x_<<11));
            uint8_t& n_flags = neighbors[3]->visflags[n_index];
            n_flags &= ~0x04;
            //Is it invisible?
            if ((n_flags & 0x2) ||(n_flags & 0xFC) == 0xFC) {
                neighbors[3]->visibleIndices.erase(n_index);
            }
        }
        
    }
    
    return true;
}

