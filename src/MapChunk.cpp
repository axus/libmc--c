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

//mc--
#include "MapChunk.hpp"
using mc__::MapChunk;
using mc__::Chunk;
using mc__::Block;

//C
#include <cstring>  //memset
#include <assert.h>

//STL
#include <iostream>
#include <iomanip>
using std::cerr;
using std::cout;
using std::endl;
using std::hex;
using std::dec;

//Constructor
MapChunk::MapChunk( int32_t X, int32_t Z):
    Chunk(15, 127, 15, X, 0, Z), flags(0)
{
    //Someone must set neighbors later
    neighbors[0] = NULL;
    neighbors[1] = NULL;
    neighbors[2] = NULL;
    neighbors[3] = NULL;
    neighbors[4] = NULL;
    neighbors[5] = NULL;
    
    //Default everything invisible and unblocked
    memset( visflags, 0x2, mapChunkBlockMax);
}

//Add chunk, update visibility
bool MapChunk::addChunk( const Chunk *chunk)
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
    
    //Can't add a chunk with NULL block_array
    if (chunk->block_array == NULL) {
        return false;
    }
    bool updateNeighbors=false;
    
    //DEBUG
    //cout << "addChunk " << chunk->X << "," << (int)chunk->Y << ","
    //    << chunk->Z << endl;
    
    //References to chunk data structures
    uint8_t in_x = (chunk->X & 0x0F);
    uint8_t in_y = (chunk->Y & 0x7F);
    uint8_t in_z = (chunk->Z & 0x0F);
    uint8_t max_x = chunk->size_X + in_x;
    uint8_t max_y = chunk->size_Y + in_y;
    uint8_t max_z = chunk->size_Z + in_z;
    
    if ((max_x > 15) || (max_y > 127) || (max_z > 15)) {
        cerr << "ERROR! chunk too big for megachunk: "
            << (int)max_x << "," << (int)max_y << "," << (int)max_z << endl;
        //The person adding chunk should have dissected it
    }
    
    //Track adjacency to neighbor MapChunk
    bool adj_N[6] = { false, false, false, false, false, false};      //A, B, C, D, E, F
    
    uint16_t x_, y_, z_;             //internal chunk offsets
    uint16_t index;                 //internal chunk index
    uint16_t c_index=0;                //external chunk index
    
    //Track changed blocks
    indexList_t changes;
    
    // 3D range: x_ to max_x, z_ to max_z, y_ to max_y
    //For X...
    for (x_ = in_x; x_ <= max_x; x_++) {
        //Check X neighbor chunk adjacency
        if (x_ == 0) { adj_N[0]=true; adj_N[1]=false; }
        else if (x_ < 15) { adj_N[0]=false; }
        else { adj_N[1]=true; }
            
    //For X,Z...
    for(z_ = in_z; z_ <= max_z; z_++) {
        //Check Z neighbor chunk adjacency
        if (z_ == 0) { adj_N[4]=true; adj_N[5]=false;}
        else if (z_ < 15) { adj_N[4]=false; }
        else { adj_N[5]=true; }
        
    //For X,Y,Z
    adj_N[2] = false;
    adj_N[3] = false;
    for(y_ = in_y; y_ <= max_y; y_++, c_index++) {
        index = (x_<<11)|(z_<<7)|y_;
        
        //Copy the block
        Block& block=chunk->block_array[c_index];
        block_array[index] = block;

        //Determine Y-adjacency (to mapchunks that don't exist!)
        switch (y_) {
            case 0: adj_N[2] = true; adj_N[3] = false; break;
            case 1: adj_N[2] = false; break;
            case 127: adj_N[3] = true; break;
            default: adj_N[2] = false; adj_N[3] = false;
        }
                
        //Update visflags, get list of updated block indices
        if (updateVisFlags(index, adj_N, changes)) {
            updateNeighbors=true;
        }
    }}}

    //Check updated blocks for visibility, update visibleIndices
    indexList_t::const_iterator iter;
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
    if (changes.size() > 0) {
        flags |= MapChunk::UPDATED;
    }
    
    //Mark neighor chunks as UPDATED, if needed
    if (updateNeighbors && false) {
        if (neighbors[0] != NULL) {
            neighbors[0]->flags |= MapChunk::UPDATED;
        }
        if (neighbors[1] != NULL) {
            neighbors[1]->flags |= MapChunk::UPDATED;
        }
        if (neighbors[4] != NULL) {
            neighbors[4]->flags |= MapChunk::UPDATED;
        }
        if (neighbors[5] != NULL) {
            neighbors[5]->flags |= MapChunk::UPDATED;
        }
    }
    
    return true;
}

//update local and neighbor visflags array for opacity at x,y,z
//Return true if changes were made to neighbor outside of MapChunk
bool MapChunk::updateVisFlags( uint16_t index, bool adj_N[6],
                                indexList_t& changes)
{
    bool result=false;

    //Copy my block info
    uint8_t blockID = block_array[index].blockID;
    uint8_t my_flags = visflags[index];
    
    //Determine opacity and cubicity from blockID
    bool opaque = Chunk::isOpaque[blockID];
    bool cube = Chunk::isCube[blockID];

    //Air is invisible
    if (blockID == 0) {
        my_flags = 0x2;
    } else {
        //Everything else is visible
        my_flags &= ~0x2;
    }
     
    //Block index of adjacent block
    uint16_t index_n=0;
    
    //adjacent visibility flags (pointer and copy)
    uint8_t *flags_p;
    uint8_t flags_v;
    
    //Block ID of adjacent block
    uint8_t blockid_n;
    
    //Visibility flags mask for me and neighbors
    uint8_t thisMask, neighborMask;

    //Update flags for each adjacent block
    for (int i = 0; i < 6; i++) {
        //Calculate hard to predict values from face index
        switch (i) {
            case 0:  //X-1
                neighborMask = 0x40;
                index_n = index - (1<<11);
                break;
            case 1:  //X+1
                neighborMask = 0x80;
                index_n = index + (1<<11);
                break;
            case 2:        //Y-1
                neighborMask = 0x10;
                index_n = index - 1;
                break;
            case 3:   //Y+1
                neighborMask = 0x20;
                index_n = index + 1;
                break;
            case 4:   //Z-1
                neighborMask = 0x04;
                index_n = index - (1<<7);
                break;
            case 5:   //Z+1
                neighborMask = 0x08;
                index_n = index + (1<<7);
                break;
            default:  //me
                neighborMask = 0x01;
                index_n = index;
        }
        index_n = index_n % uint16_t(1<<15);
        //assert( index_n < 32768);

        //Copy from this MapChunk, or neighbor, or NULL
        MapChunk *neighbor = neighbors[i];
        if (!adj_N[i]) {
            //Index inside this MapChunk
            flags_p = &(visflags[index_n]);
            flags_v = *flags_p;
            blockid_n = block_array[index_n].blockID;
        } else if ( neighbor != NULL) {
            //Index inside neighbor
            flags_p = &(neighbor->visflags[index_n]);
            flags_v = *flags_p;
            blockid_n = neighbor->block_array[index_n].blockID;
        } else {
            //Index inside unloaded MapChunk
            flags_p = NULL;
            flags_v = 0xFD;
            blockid_n = 1;
        }
        
        //vismask for this face depends on which face
        thisMask = (0x80 >> i);
        
        //Get neigbhor opacity/cubity        
        bool n_opaque = Chunk::isOpaque[blockid_n];
        bool n_cube = Chunk::isCube[blockid_n];

        //My default flags depend on if neighbor is opaque        
        if (n_opaque) {
            my_flags |= thisMask;
        } else {
            my_flags &= ~thisMask;
        }
        
        //Update neighbor face and my face depending on combination
        if (opaque) {
            //I am opaque...
            flags_v |= neighborMask;            
        } else if (cube) {
            //I am translucent cube
            if (!n_opaque && n_cube) {
                //Neighbor is translucent cube
                flags_v |= neighborMask;
                my_flags |= thisMask;
            } else if (!n_cube) {
                //Neighbor is item
                flags_v &= ~neighborMask;
                my_flags &= ~thisMask;
            } else {
                //Neighbor is opaque
                flags_v &= ~neighborMask;
            }
        } else {
            //I am item
            flags_v &= ~neighborMask;
        }
        
        //Update neighbor block flags
        if ((flags_p != NULL) && (flags_v != *flags_p)) {
            *flags_p = flags_v;
            
            //Mark change
            if (adj_N[i]) {
                //Change outside this mapchunk
                result=true;
            } else {
                //Change inside this mapchunk
                changes.insert(index_n);
            }
        }
    }

    //Mark change if I changed
    if (my_flags != visflags[index] ) {
        changes.insert(index);
        visflags[index] = my_flags;
    }

    return result;
}

