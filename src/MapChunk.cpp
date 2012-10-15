/*
  mc__::MapChunk
  16x128x16 complete Chunk, with visibility data
  
  Copyright 2010 - 2012 axus

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
        cerr << "ERROR! chunk @ " << (int)(chunk->X) << "," << (int)(chunk->Y)
            << "," << (int)(chunk->Z) << " too big, edge @ "
            << (int)max_x << "," << (int)max_y << "," << (int)max_z << endl;
        //The person adding chunk should have dissected it
    }

    //Get changes in the chunk range, and flag neighbors as updated
    updateVisRange(chunk, in_x, in_y, in_z, max_x, max_y, max_z);

    return true;
}

//update the visflags without a new chunk
bool MapChunk::recalcVis()
{
    return updateVisRange(NULL, 0, 0, 0, 15, 127, 15);
    
}

//Update the visflags for a range of blocks in the mapchunk
bool MapChunk::updateVisRange(const Chunk *chunk,
    uint8_t off_x, uint8_t off_y, uint8_t off_z,
    uint8_t max_x, uint8_t max_y, uint8_t max_z)
{
    //internal chunk offsets
    uint16_t x_, y_, z_;

    //Track adjacency to neighbor MapChunk
    bool adj_N[6] = { false, false, false, false, false, false};
    bool update_N[6] = { false, false, false, false, false, false};

    //external chunk index
    uint16_t c_index=0;
    
    //internal chunk index
    uint16_t index;
    
    //List of changes
    indexList_t changes;
    
    // 3D range: x_ to max_x, z_ to max_z, y_ to max_y
    //For X...
    for (x_ = off_x; x_ <= max_x; x_++) {
        //Check X neighbor chunk adjacency
        if (x_ == 0) { adj_N[0]=true; adj_N[1]=false; }
        else if (x_ < 15) { adj_N[0]=false; }
        else { adj_N[1]=true; }
            
    //For X,Z...
    for(z_ = off_z; z_ <= max_z; z_++) {
        //Check Z neighbor chunk adjacency
        if (z_ == 0) { adj_N[4]=true; adj_N[5]=false;}
        else if (z_ < 15) { adj_N[4]=false; }
        else { adj_N[5]=true; }
        
    //For X,Y,Z
    adj_N[2] = false;
    adj_N[3] = false;
    for(y_ = off_y; y_ <= max_y; y_++, c_index++) {
        index = (x_<<11)|(z_<<7)|y_;
        
        //Copy the block (IF THERE IS ONE!)
        if (chunk) {
            Block& block=chunk->block_array[c_index];
            block_array[index] = block;
        }

        //Determine Y-adjacency (to mapchunks that don't exist!)
        if (y_ == 0) {
            adj_N[2] = true; adj_N[3] = false;
        } else if (y_ == 127) {
            adj_N[2] = false; adj_N[3] = true;
        } else {
            adj_N[2] = false; adj_N[3] = false;
        }

        //Update visflags, get list of updated block indices
        if (updateVisFlags(index, adj_N, changes)) {
            if (adj_N[0]) { update_N[0] = true; }
            if (adj_N[1]) { update_N[1] = true; }
            if (adj_N[4]) { update_N[4] = true; }
            if (adj_N[5]) { update_N[5] = true; }
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
    bool opaque = Blk::isOpaque[blockID];
    bool cube = Blk::isCube[blockID];

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
                //index_n = index - (1<<11);
                index_n = (index & 0x07FF)|((index - (1<<11))&0x7800);
                break;
            case 1:  //X+1
                neighborMask = 0x80;
                //index_n = index + (1<<11);
                index_n = (index & 0x07FF)|((index + (1<<11))&0x7800);
                break;
            case 2:        //Y-1
                neighborMask = 0x10;
                //index_n = index - 1;
                index_n = (index & 0x7F80)|((index - 1)&0x007F);
                break;
            case 3:   //Y+1
                neighborMask = 0x20;
                //index_n = index + 1;
                index_n = (index & 0x7F80)|((index + 1)&0x007F);
                break;
            case 4:   //Z-1
                neighborMask = 0x04;
                //index_n = index - (1<<7);
                index_n = (index & 0x787F)|((index - (1<<7))&0x0780);
                break;
            case 5:   //Z+1
                neighborMask = 0x08;
                //index_n = index + (1<<7);
                index_n = (index & 0x787F)|((index + (1<<7))&0x0780);
                break;
            default:  //me
                neighborMask = 0x01;
                index_n = index;
        }
        
        //index_n % 32768
        index_n &= ((1<<15) - 1);

        //Copy from this MapChunk, or neighbor, or NULL
        MapChunk *neighbor = neighbors[i];
        if (!adj_N[i]) {
            //Index inside this MapChunk
            flags_p = &(visflags[index_n]);
            flags_v = *flags_p;
            blockid_n = block_array[index_n].blockID;
        } else if ( neighbor != NULL && (neighbor->flags & DRAWABLE)==DRAWABLE) {
            //Index inside neighbor
            flags_p = (neighbor->visflags + index_n);
            flags_v = *flags_p;
            blockid_n = neighbor->block_array[index_n].blockID;
        } else {
            //Index inside unloaded MapChunk
            flags_p = NULL;
            flags_v = ( i != 3 ? 0xFD : 0x02);
            
            //Bedrock block except on top of world (air up there)
            blockid_n = ( i != 3 ? 7 : 0);
        }
        
        //vismask for this face depends on which face
        thisMask = (0x80 >> i);
        
        //Get neigbhor opacity/cubity        
        bool n_opaque = Blk::isOpaque[blockid_n];
        bool n_cube = Blk::isCube[blockid_n];

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
        
        //Update adjacent block flags
        if ((flags_p != NULL) && (flags_v != *flags_p)) {
            *flags_p = flags_v;
            
            //Mark change
            if (adj_N[i]) {
              
                //Change outside this mapchunk
                result=true;
                if ( neighbor != NULL) {
                    //Update neighbor visible index list in neighbor
                    if ( (flags_v & 0x02) != 0x02 &&
                         (flags_v & 0xFC) != 0xFC )
                    {
                        neighbor->visibleIndices.insert(index_n);
                    } else {
                        //Index is unseen, remove it
                        neighbor->visibleIndices.erase(index_n);
                    }
                    neighbor->flags |= UPDATED;
                }
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

