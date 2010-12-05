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

//STL
#include <iostream>
#include <iomanip>
using std::cerr;
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
    
    //Default everything invisible and unblocked
    memset( visflags, 0x2, mapChunkBlockMax);
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
    
    //If chunk is still zipped...
    if (! chunk->isUnzipped) {

        //Unzip the chunk
        if (!chunk->unzip()) {
            return false;
        }
    }
    bool updateNeighbors=false;
    
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
    bool adjA=false, adjB=false, adjE=false, adjF=false;
    
    uint16_t x_, y_, z_;             //internal chunk offsets
    uint16_t index;                 //internal chunk index
    uint16_t c_index=0;                //external chunk index
    
    //Track changed blocks
    indexList_t changes;
    
    
    // 3D range: x_ to max_x, z_ to max_z, y_ to max_y
    //For X...
    for (x_ = in_x; x_ <= max_x; x_++) {
        //Check X neighbor chunk adjacency
        if (x_ == 0) { adjA=true; adjB=false; }
        else if (x_ < 15) { adjA=false; }
        else { adjB=true; }
            
    //For X,Z...
    for(z_ = in_z; z_ <= max_z; z_++) {
        //Check Z neighbor chunk adjacency
        if (z_ == 0) { adjE=true; adjF=false;}
        else if (z_ < 15) { adjE=false; }
        else { adjF=true; }
        
    //For X,Y,Z
    for(y_ = in_y; y_ <= max_y; y_++, c_index++) {
        index = (x_<<11)|(z_<<7)|y_;
        
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
        if (updateVisFlags(x_,y_,z_,opaque, adjA, adjB, adjE, adjF, changes)) {
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
    if (updateNeighbors) {
        if (neighbors[0] != NULL) {
            neighbors[0]->flags |= MapChunk::UPDATED;
        }
        if (neighbors[1] != NULL) {
            neighbors[1]->flags |= MapChunk::UPDATED;
        }
        if (neighbors[2] != NULL) {
            neighbors[2]->flags |= MapChunk::UPDATED;
        }
        if (neighbors[3] != NULL) {
            neighbors[3]->flags |= MapChunk::UPDATED;
        }
    }
    
    return true;
}

//update local and neighbor visflags array for opacity at x,y,z
//Return true if changes were made to neighbor outside of MapChunk
bool MapChunk::updateVisFlags(uint8_t x_, uint8_t y_, uint8_t z_, bool opaque,
    bool adjA, bool adjB, bool adjE, bool adjF,
    indexList_t& changes)
{
    bool result=false;
  
    //Local index
    const uint16_t index = (( (uint16_t)x_<<11)|( (uint16_t)z_<<7)| y_);
    
    //Adjacent block indices
    uint16_t indexA = index - (1<<11);  //X-1
    uint16_t indexB = index + (1<<11);  //X+1
    uint16_t indexC = index - 1;        //Y-1
    uint16_t indexD = index + 1;        //Y+1
    uint16_t indexE = index - (1<<7);   //Z-1
    uint16_t indexF = index + (1<<7);   //Z+1
    uint16_t n_index;

    //Copy visflags
    uint8_t my_flags = visflags[index];
    
    //Adjacent flags
    uint8_t* A_flags;
    uint8_t* B_flags;
    uint8_t* C_flags;
    uint8_t* D_flags;
    uint8_t* E_flags;
    uint8_t* F_flags;

    //Get flags pointers to adjacent block visflags
    //-X block face
    if (!adjA) {
        A_flags = visflags + indexA;
        changes.insert(indexA);
    } else if ( neighbors[0] != NULL) {
        n_index = (y_|(z_<<7)|(0xF<<11));
        A_flags = neighbors[0]->visflags + n_index;
    } else {
        A_flags = NULL;
    }
    if (A_flags == NULL || (*A_flags & 1)) {
        my_flags |= 0x80;   //A is blocked
    } else {
        my_flags &= ~0x80;   //A is unblocked
    }

    //+X block face
    if (!adjB) {
        B_flags = visflags + indexB;
        changes.insert(indexB);
    } else if ( neighbors[1] != NULL) {
        n_index = (y_|(z_<<7));
        B_flags = neighbors[1]->visflags + n_index;
    } else {
        B_flags = NULL;
    }
    if (B_flags == NULL || (*B_flags & 1)) {
        my_flags |= 0x40;   //B is blocked
    } else {
        my_flags &= ~0x40;   //B is unblocked
    }

    //-Y block face
    if (y_ > 0) {
        C_flags = visflags + indexC;
        changes.insert(indexC);
    } else {
        C_flags = NULL;
    }
    if (C_flags == NULL || (*C_flags & 1)) {
        my_flags |= 0x20;   //C is blocked
    } else {
        my_flags &= ~0x20;   //C is unblocked
    }

    //+Y block face
    if (y_ < 127) {
        D_flags = visflags + indexD;
        changes.insert(indexD);
    } else {
        D_flags = NULL;
    }
    if (D_flags == NULL || (*D_flags & 1)) {
        my_flags |= 0x10;   //D is blocked
    } else {
        my_flags &= ~0x10;   //D is unblocked
    }

    //-Z block face
    if (!adjE) {
        E_flags = visflags + indexE;
        changes.insert(indexE);
    } else if ( neighbors[2] != NULL) {
        n_index = (y_|(0xF<<7)|(x_<<11));
        E_flags = neighbors[2]->visflags + n_index;
    } else {
        E_flags = NULL;
    }
    if (E_flags == NULL || (*E_flags & 1)) {
        my_flags |= 0x08;   //E is blocked
    } else {
        my_flags &= ~0x08;   //E is unblocked
    }

    //+Z block face
    if (!adjF) {
        F_flags = visflags + indexF;
        changes.insert(indexF);
        
    } else if ( neighbors[3] != NULL) {
        n_index = (y_|(x_<<11));
        F_flags = neighbors[3]->visflags + n_index;
        
    } else {
        F_flags = NULL;
    }
    if (F_flags == NULL || (*F_flags & 1)) {
        my_flags |= 0x04;   //F is blocked
    } else {
        my_flags &= ~0x04;   //F is unblocked
    }

    //Set neighbor block flags
    if (opaque) {
        my_flags |= 1;
        if (A_flags) { *A_flags |= 0x40; }
        if (B_flags) { *B_flags |= 0x80; }
        if (C_flags) { *C_flags |= 0x10; }
        if (D_flags) { *D_flags |= 0x20; }
        if (E_flags) { *E_flags |= 0x04; }
        if (F_flags) { *F_flags |= 0x08; }
    } else {
        my_flags &= ~1;
        if (A_flags) { *A_flags &= ~0x40; }
        if (B_flags) { *B_flags &= ~0x80; }
        if (C_flags) { *C_flags &= ~0x10; }
        if (D_flags) { *D_flags &= ~0x20; }
        if (E_flags) { *E_flags &= ~0x04; }
        if (F_flags) { *F_flags &= ~0x08; }
    }

    //Insert myself into changes if I changed
    if (my_flags != visflags[index]) {
        changes.insert(index);
        visflags[index] = my_flags;
    }

    result=true;    //TODO: true only if external neighbor changed
    return result;
}

