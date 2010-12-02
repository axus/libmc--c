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
using std::set;
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
    }
    
    //Track adjacency to neighbor MapChunk
    bool adjA=false, adjB=false, adjE=false, adjF=false;
    
    uint16_t x_, y_, z_;             //internal chunk offsets
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
            
    //For X,Z...
    for(z_ = in_z; z_ <= max_z; z_++) {
        //Check Z neighbor adjacency
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
        updateVisFlags( x_, y_, z_, opaque, adjA, adjB, adjE, adjF, changes);

    }}}

    //Check updated blocks for visibility, update visibleIndices
    set<uint16_t>::const_iterator iter;
    for (iter = changes.begin(); iter != changes.end(); iter++) {
        index = *iter;

        //cerr << "0x" << hex << (int)visflags[index];
        
        //Is it visible?
        if ( (visflags[index]&0x2) != 0x2 && (visflags[index] & 0xFC) != 0xFC ) {
            visibleIndices.insert(index);
        } else {
            //Index is invisible or blocked from all sides, remove it
            visibleIndices.erase(index);
        }
    }
    //cerr << endl;
    
    return true;
}

//update local and neighbor visflags array for opacity at x,y,z
bool MapChunk::updateVisFlags(uint8_t x_, uint8_t y_, uint8_t z_, bool opaque,
    bool adjA, bool adjB, bool adjE, bool adjF,
    set<uint16_t>& changes)
{
    //Local index
    uint16_t index = (( (uint16_t)x_<<11)|( (uint16_t)z_<<7)| y_);
    
    //Adjacent block indices
    uint16_t indexA = index - (1<<11);  //X-1
    uint16_t indexB = index + (1<<11);  //X+1
    uint16_t indexC = index - 1;        //Y-1
    uint16_t indexD = index + 1;        //Y+1
    uint16_t indexE = index - (1<<7);   //Z-1
    uint16_t indexF = index + (1<<7);   //Z+1
    uint16_t n_index;

    uint8_t& my_flags = visflags[index];
    changes.insert(index);
    
    //Adjacent flags
    uint8_t* A_flags;
    uint8_t* B_flags;
    uint8_t* C_flags;
    uint8_t* D_flags;
    uint8_t* E_flags;
    uint8_t* F_flags;

    //Get flags pointers to adjacent block visflags
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
        my_flags |= 0x80;   //blocked!
    } else {
        my_flags &= ~0x80;   //unblocked!
    }

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
        my_flags |= 0x40;   //blocked!
    } else {
        my_flags &= ~0x40;   //unblocked!
    }


    if (y_ > 0) {
        C_flags = visflags + indexC;
        changes.insert(indexC);
    } else {
        C_flags = NULL;
    }
    if (C_flags == NULL || (*C_flags & 1)) {
        my_flags |= 0x20;   //blocked!
    } else {
        my_flags &= ~0x20;   //unblocked!
    }

    if (y_ < 127) {
        D_flags = visflags + indexD;
        changes.insert(indexD);
    } else {
        D_flags = NULL;
    }
    if (D_flags == NULL || (*D_flags & 1)) {
        my_flags |= 0x10;   //blocked!
    } else {
        my_flags &= ~0x10;   //unblocked!
    }

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
        my_flags |= 0x08;   //blocked!
    } else {
        my_flags &= ~0x08;   //unblocked!
    }

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
        my_flags |= 0x04;   //blocked!
    } else {
        my_flags &= ~0x04;   //unblocked!
    }

    //Set neighbor flags
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
    
/*
    //Update visibility based on opaque/not-opaque
    if (opaque) {
        //This block
        my_flags |= 1;
      
        //Set X-1
        if (!adjA) {
            //blocking adj B-side
            visflags[indexA] |= 0x40;
            changes.insert(indexA);
            

        } else if ( neighbors[0] != NULL) {
            //Change neighboring y,z block
            n_index = (y_|(z_<<7)|(0xF<<11));
            uint8_t& n_flags = neighbors[0]->visflags[n_index];
            n_flags |= 0x40;            //blocking neighbor B-side
            if (n_flags & 1) { my_flags |= 0x80; }
            
            //Is it visible?
            if ((n_flags & 0xFC) == 0xFC) {
                neighbors[0]->visibleIndices.erase(n_index);
            }
        } else {
            my_flags |= 0x80;    //A-side blocked
        }
        
        //Set X+1
        if (!adjB) {
            visflags[indexB] |= 0x80;   //blocking A-side
            changes.insert(indexB);
        } else if (neighbors[1] != NULL) {
            //Change neighboring y,z block
            n_index = (y_|(z_<<7) );
            uint8_t& n_flags = neighbors[1]->visflags[n_index];
            n_flags |= 0x80;    //blocking neighbor A-side
            
            //Is it visible?
            if ((n_flags & 0xFC) == 0xFC) {
                neighbors[1]->visibleIndices.erase(n_index);
            }
        } else {
            my_flags |= 0x40;    //B-side blocked
        }
        
        //Set Y-1
        if (y_ > 0) {
            visflags[indexC] |= 0x10;   //blocking their D-side
            if ( visflags[indexC] & 1) { my_flags |= 0x20; }
            changes.insert(indexC);
        }
        
        //Set Y+1
        if (y_ < 127) {
            visflags[indexD] |= 0x20;   //blocking their C-side
            if ( visflags[indexD] & 1) { my_flags |= 0x10; }
            changes.insert(indexD);
        }

        //Set Z-1
        if (!adjE) {
            visflags[indexE] |= 0x04;   //blocking F-side
            changes.insert(indexE);
        } else if (neighbors[2] != NULL) {
            //Change neighboring y,z block
            n_index = (y_|(0xF<<7)|(x_<<11));
            uint8_t& n_flags = neighbors[2]->visflags[n_index];
            
            n_flags |= 0x04;    //Blocking neighbor F-side
            //Is it visible?
            if ((n_flags & 0xFC) == 0xFC) {
                neighbors[2]->visibleIndices.erase(n_index);
            }
        } else {
            my_flags |= 0x08;    //E-side blocked
        }
        
        //Set Z+1
        if (!adjF) {
            visflags[indexF] |= 0x08;   //blocking E-side
            changes.insert(indexF);
        } else if (neighbors[3] != NULL){
            //Change neighboring y,z block
            n_index = (y_|(x_<<11));
            uint8_t& n_flags = neighbors[3]->visflags[n_index];
            n_flags |= 0x08;    //blocking neighbor-E-side
            
            //Is it visible?
            if ((n_flags & 0xFC) == 0xFC) {
                neighbors[3]->visibleIndices.erase(n_index);
            }
        } else {
            my_flags |= 0x04;    //F-side blocked
        }
        

    } else {
        //This block is not opaque
        visflags[index] &= ~1;

        //Set X-1
        if (!adjA) {
            visflags[indexA] &= ~0x80;
            changes.insert(indexA);
        } else if (neighbors[0] != NULL){
            //Change neighboring y,z block
            n_index = (y_|(z_<<7)|(0xF<<11));
            uint8_t& n_flags = neighbors[0]->visflags[n_index];
            n_flags &= ~0x80;
            
            //Is it visible?
            if ( !(n_flags & 0x2) ) {
                neighbors[0]->visibleIndices.insert(n_index);
            }
        }
        
        //Set X+1
        if (!adjB) {
            visflags[indexB] &= ~0x40;
            changes.insert(indexB);
        } else if (neighbors[1] != NULL){
            //Change neighboring y,z block
            n_index = (y_|(z_<<7) );
            uint8_t& n_flags = neighbors[1]->visflags[n_index];
            n_flags &= ~0x40;
            
            //Is it visible?
            if ( !(n_flags & 0x2) ) {
                neighbors[1]->visibleIndices.insert(n_index);
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
        if (!adjE) {
            visflags[indexE] &= ~0x04;  //Free their F-side
            changes.insert(indexE);
        } else if (neighbors[2] != NULL){
            //Change neighboring y,z block
            n_index = (y_|(0xF<<7)|(x_<<11));
            uint8_t& n_flags = neighbors[2]->visflags[n_index];
            n_flags &= ~0x04;       //Free their F-side
            
            //Is it visible?
            if ( !(n_flags & 0x2) ) {
                neighbors[2]->visibleIndices.insert(n_index);
            }
        }
        
        //Set Z+1
        if (!adjF) {
            visflags[indexF] &= ~0x08;  //Free their E-side
            changes.insert(indexF);
        } else if (neighbors[3] != NULL){
            //Change neighboring y,z block
            n_index = (y_|(x_<<11));
            uint8_t& n_flags = neighbors[3]->visflags[n_index];
            n_flags &= ~0x08;   //Free their E-side
            
            //Is it visible?
            if ( !(n_flags & 0x2) ) {
                neighbors[3]->visibleIndices.insert(n_index);
            }
        }
    }
        */
    
    return true;
}

