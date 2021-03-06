/*
  mc__::BlockDrawer
  How to draw block 0-255 in OpenGL, for mc__::Viewer

  Copyright 2011 axus

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


/*
    OpenGL has specific meaning for x, y, and z axes.
    
    z-axis is distance from the initial viewpoint (facing forward). As Z
    increases, the location gets closer to the initial viewpoint. Negative is
    farther.
    
    y-axis is "up" relative to the initial viewpoint.  As Y increases,
    the location moves farther "up".
    
    x-axis is "right" relative to the intial viewpoint.  As X increases,
    the location moves farther "right".

    Block faces are always aligned with major axes. So, we can say that each
    face is on an axis, and is "greater" or "less" compared to the opposing face.
    
    Face direction:
    On the x axis: A = left,    B = right
    On the y axis: C = down,    D = up
    On the z axis: E = farther, F = closer

    Game directions (North, South, East, West) correspond to axes:
    A = West (-X)
    B = East (+X)
    E = North (-Z)
    F = South (+Z)
    
    We can refer to the corners of the cube by which faces they are part of.
    Each of those 3 faces will describe a different axis.  For example:
        ADF is  x=left, y=up, z=close.
    
    You can visualize the cube with those 8 points:
           ADE ---- BDE
           /.       /|
          / .      / |
        ADF ---- BDF |
         | ACE . .| BCE
         | .      | /
         |.       |/
        ACF ---- BCF

    A face will have four points containing its letter.
    
    
    vflags option is a bitmask, indicating which faces to hide (for performance).
    0x80 = +X
    0x40 = -X
    0x20 = +Y
    0x10 = -Y
    0x08 = +Z
    0x04 = -Z
    
    vflags for each block were calculated when its chunk changes.
*/

//libmc--c
#include "BlockDrawer.hpp"
using mc__::BlockDrawer;
using mc__::face_ID;
using mc__::World;
using mc__::TextureInfo;

//C
#include <cmath>    //fmod

//More STL
#include <iostream>
#include <iomanip>
#include <fstream>
#include <sstream>
#include <set>

using std::cout;
using std::cerr;
using std::endl;
using std::hex;
using std::dec;
using std::setw;
using std::setfill;
using std::ofstream;
using std::ios;
using std::flush;

using std::string;
using std::stringstream;

//Macros for multiple flag testing
#define HAS_FLAGS(VAL,FLAGS) ((VAL&FLAGS)==FLAGS)

BlockDrawer::BlockDrawer( mc__::World* w, GLuint tex_array[mc__::TEX_MAX] ):
    world(w)
{
    //Copy textures... it was crashing when I used a pointer.
    //  So, I probably have memory corruption elsewhere, will track it later
    for (int i=mc__::TEX_TERRAIN; i < mc__::TEX_MAX; i++) {
        textures[i] = tex_array[i];
    }

    //Load the texture info for possible face textures
    loadTexInfo();
  
    //Load the block info for all known block types
    loadBlockInfo();


    //Default biome colors
    
    //Dark green tree leaves
    leaf_color[0] = 0x00;    //Red
    leaf_color[1] = 0xFF;    //Green
    leaf_color[2] = 0x00;    //Blue
    leaf_color[3] = 0xFF;    //Opaque

    //Sort of a yellow/green grass
    grass_color[0] = 0x7F;    //Red
    grass_color[1] = 0xCF;    //Green
    grass_color[2] = 0x1F;    //Blue
    grass_color[3] = 0xFF;    //Opaque


}

//change back to texture if needed
void BlockDrawer::bindTexture( tex_t index) const
{
    //glBind texture before assigning it
    
    glEnd();
    //Some video cards don't need to do this outside of glBegin/glEnd, some do
    glBindTexture(GL_TEXTURE_2D, textures[index]);
    glBegin(GL_QUADS);
}


//Set glColor if needed by block type and face
void BlockDrawer::setBlockColor(uint16_t blockID, face_ID face) const
{
    //return; Uncomment to disable block coloring
    
    GLubyte red, green, blue;
    switch (blockID) {
        case Blk::Air:
            red=0xFF; green=0xFF; blue=0xFF;
            break;
        case Blk::Grass:     //Grass
            if (face == mc__::TOP) {
                red=grass_color[0]; green=grass_color[1]; blue=grass_color[2];
            } else {
                red=0xFF; green=0xFF; blue=0xFF;
            }
            break;
        case Blk::Leaves:    //Leaves, tall grass
        case Blk::TallGrass:
        case (256+Blk::Leaves):
        case (512+Blk::Leaves):
            red=leaf_color[0]; green=leaf_color[1]; blue=leaf_color[2];
            break;
        case (256 + Blk::Wool + 0): //White wool
            red=0xFF; green=0xFF; blue=0xFF;
            break;
        case (256 + Blk::Wool + 1): //Orange wool
            red=0xFF; green=0x7F; blue=0x3F;
            break;
        case (256 + Blk::Wool + 2): //Magenta wool
            red=0xFF; green=0; blue=0xFF;
            break;
        case (256 + Blk::Wool + 3): //Light Blue wool
            red=0x5F; green=0x7F; blue=0xFF;
            break;
        case (256 + Blk::Wool + 4): //Yellow wool
            red=0xFF; green=0xFF; blue=0;
            break;
        case (256 + Blk::Wool + 5): //Lime wool
            red=0;  green=0xFF; blue=0;
            break;
        case (256 + Blk::Wool + 6): //Pink wool
            red=0xFF; green=0xCF; blue=0xCF;
            break;
        case (256 + Blk::Wool + 7): //Gray wool
            red=0x5F; green=0x5F; blue=0x5F;
            break;
        case (256 + Blk::Wool + 8): //Light Gray wool
            red=0xCF; green=0xCF; blue=0xCF;
            break;
        case (256 + Blk::Wool + 9): //Cyan wool
            red=0;  green=0xFF; blue=0xFF;
            break;
        case (256 + Blk::Wool + 10)://Purple wool
            red=0x9F; green=0x2F; blue=0xFF;
            break;
        case (256 + Blk::Wool + 11)://Blue wool
            red=0;  green=0;    blue=0xFF;
            break;
        case (256 + Blk::Wool + 12)://Brown wool
            red=0xAF; green=0x5F; blue=0x3F;
            break;
        case (256 + Blk::Wool + 13)://Dark Green wool
            red=0;  green=0x5F; blue=0;
            break;
        case (256 + Blk::Wool + 14)://Red wool
            red=0xFF; green=0; blue=0;
            break;
        case (256 + Blk::Wool + 15)://Black wool
            red=0x1F; green=0x1F; blue=0x1F;
            break;
        case Blk::Wire:    //Redstone wire
            red=255; green=127; blue=127;
            break;
        default:
            red=255; green=255; blue=255;
            break;
    }
    glColor3ub( red, green, blue);
}

//Draw me, using function pointer assigned for block ID
void BlockDrawer::draw( uint8_t blockID, uint8_t meta,
    GLint x, GLint y, GLint z, uint8_t visflags) const
{
    //Drawing function for this block type
    drawBlock_f f = drawFunction[blockID];
    
    //Metadata hackery to conform special cases (not needed yet)
    
    //Draw!
    if (f != NULL) {
        //Member function pointer syntax is hard
        (this->*f)(blockID, meta, x, y, z, visflags);
    }
}

//Use OpenGL to draw a solid cube with appropriate textures for blockID
void BlockDrawer::drawCube( uint8_t blockID, uint8_t meta,
    GLint x, GLint y, GLint z, uint8_t vflags) const
{
    //Use the "16 bit blockID" cube drawing function
    drawCubeMeta(blockID, meta, x, y, z, vflags);
}

//Use OpenGL to draw a solid cube with appropriate textures for blockID
// It will normally only be called by other functions that use metadata
// This function does not match drawBlock_f type
void BlockDrawer::drawCubeMeta( uint16_t blockID, uint8_t /*meta*/,
    GLint x, GLint y, GLint z, uint8_t vflags) const
{
    
    //Face coordinates (in pixels)
    GLint A = (x << 4) + 0;
    GLint B = (x << 4) + texmap_TILE_LENGTH;
    GLint C = (y << 4) + 0;
    GLint D = (y << 4) + texmap_TILE_LENGTH;
    GLint E = (z << 4) + 0;
    GLint F = (z << 4) + texmap_TILE_LENGTH;

    //Texture map coordinates (0.0 - 1.0)
    GLfloat tx_0, tx_1, ty_0, ty_1;

    //For each face, use the appropriate texture offsets for the block ID
    //       ADE ---- BDE
    //       /.       /|
    //      / .      / |
    //    ADF ---- BDF |
    //     | ACE . .| BCE
    //     | .      | /
    //     |.       |/
    //    ACF ---- BCF

    //   Texture map was loaded upside down...
    // 0.0 -------------> 1.0 (X)
    // |
    // |
    // |
    // |
    // |
    // |
    // v
    // 1.0
    // (Y)
    
    //
    // (tx_0, ty_1)      (tx_1, ty_1)
    //
    // (tx_0, ty_0)      (tx_1, ty_0)

    //A
    if (!(vflags & 0x80)) {
        tx_0 = blockInfo[blockID].tx[LEFT];
        tx_1 = tx_0 + tmr;
        ty_1 = blockInfo[blockID].ty[LEFT];
        ty_0 = ty_1 + tmr;    //flip y
        setBlockColor(blockID, LEFT);  //Set leaf/grass color if needed
        
        glTexCoord2f(tx_0,ty_0); glVertex3i( A, C, E);  //Lower left:  ACE
        glTexCoord2f(tx_1,ty_0); glVertex3i( A, C, F);  //Lower right: ACF
        glTexCoord2f(tx_1,ty_1); glVertex3i( A, D, F);  //Top right:   ADF
        glTexCoord2f(tx_0,ty_1); glVertex3i( A, D, E);  //Top left:    ADE
    }

    //B
    if (!(vflags & 0x40)) {
        tx_0 = blockInfo[blockID].tx[RIGHT];
        tx_1 = tx_0 + tmr;
        ty_1 = blockInfo[blockID].ty[RIGHT];
        ty_0 = ty_1 + tmr;    //flip y
        setBlockColor(blockID, RIGHT);  //Set leaf/grass color if needed
        
        glTexCoord2f(tx_0,ty_0); glVertex3i( B, C, F);  //Lower left:  BCF
        glTexCoord2f(tx_1,ty_0); glVertex3i( B, C, E);  //Lower right: BCE
        glTexCoord2f(tx_1,ty_1); glVertex3i( B, D, E);  //Top right:   BDE
        glTexCoord2f(tx_0,ty_1); glVertex3i( B, D, F);  //Top left:    BDF
    }
    
    //C
    if (!(vflags & 0x20)) {
        tx_0 = blockInfo[blockID].tx[BOTTOM];
        tx_1 = tx_0 + tmr;
        ty_1 = blockInfo[blockID].ty[BOTTOM];
        ty_0 = ty_1 + tmr;    //flip y
        setBlockColor(blockID, BOTTOM);  //Set leaf/grass color if needed
        
        glTexCoord2f(tx_0,ty_0); glVertex3i( A, C, E);  //Lower left:  ACE
        glTexCoord2f(tx_1,ty_0); glVertex3i( B, C, E);  //Lower right: BCE
        glTexCoord2f(tx_1,ty_1); glVertex3i( B, C, F);  //Top right:   BCF
        glTexCoord2f(tx_0,ty_1); glVertex3i( A, C, F);  //Top left:    ACF
    }
    
    //D
    if (!(vflags & 0x10)) {
        tx_0 = blockInfo[blockID].tx[TOP];
        tx_1 = tx_0 + tmr;
        ty_1 = blockInfo[blockID].ty[TOP];
        ty_0 = ty_1 + tmr;    //flip y
        setBlockColor(blockID, TOP);  //Set leaf/grass color if needed
    
        glTexCoord2f(tx_0,ty_0); glVertex3i( A, D, F);  //Lower left:  ADF
        glTexCoord2f(tx_1,ty_0); glVertex3i( B, D, F);  //Lower right: BDF
        glTexCoord2f(tx_1,ty_1); glVertex3i( B, D, E);  //Top right:   BDE
        glTexCoord2f(tx_0,ty_1); glVertex3i( A, D, E);  //Top left:    ADE
    }
    
    //E
    if (!(vflags & 0x08)) {
        tx_0 = blockInfo[blockID].tx[BACK];
        tx_1 = tx_0 + tmr;
        ty_1 = blockInfo[blockID].ty[BACK];
        ty_0 = ty_1 + tmr;    //flip y
        setBlockColor(blockID, BACK);  //Set leaf/grass color if needed
        
        glTexCoord2f(tx_0,ty_0); glVertex3i( B, C, E);  //Lower left:  BCE
        glTexCoord2f(tx_1,ty_0); glVertex3i( A, C, E);  //Lower right: ACE
        glTexCoord2f(tx_1,ty_1); glVertex3i( A, D, E);  //Top right:   ADE
        glTexCoord2f(tx_0,ty_1); glVertex3i( B, D, E);  //Top left:    BDE
    }
    
    //F
    if (!(vflags & 0x04)) {
        tx_0 = blockInfo[blockID].tx[FRONT];
        tx_1 = tx_0 + tmr;
        ty_1 = blockInfo[blockID].ty[FRONT];
        ty_0 = ty_1 + tmr;    //flip y
        setBlockColor(blockID, FRONT);  //Set leaf/grass color if needed
        
        glTexCoord2f(tx_0,ty_0); glVertex3i( A, C, F);  //Lower left:  ACF
        glTexCoord2f(tx_1,ty_0); glVertex3i( B, C, F);  //Lower right: BCF
        glTexCoord2f(tx_1,ty_1); glVertex3i( B, D, F);  //Top right:   BDF
        glTexCoord2f(tx_0,ty_1); glVertex3i( A, D, F);  //Top left:    ADF
    }
    
    //Return color to normal
    setBlockColor( 0, (face_ID)0);
}


//variation of solid cube drawing, with one "face"
// METADATA: 2 = east, 3 = west, 4 = north, 5 = south
void BlockDrawer::drawFaceCube( uint8_t blockID, uint8_t meta,
    GLint x, GLint y, GLint z, uint8_t vflags) const
{
    
    //Face coordinates (in pixels)
    GLint A = (x << 4) + 0;
    GLint B = (x << 4) + texmap_TILE_LENGTH;
    GLint C = (y << 4) + 0;
    GLint D = (y << 4) + texmap_TILE_LENGTH;
    GLint E = (z << 4) + 0;
    GLint F = (z << 4) + texmap_TILE_LENGTH;

    //Texture map coordinates (0.0 - 1.0)
    GLfloat tx_0, tx_1, ty_0, ty_1;

    //For each face, use the appropriate texture offsets for the block ID
    //       ADE ---- BDE
    //       /.       /|
    //      / .      / |
    //    ADF ---- BDF |
    //     | ACE . .| BCE
    //     | .      | /
    //     |.       |/
    //    ACF ---- BCF

    //Use "LEFT" as "side", "FRONT" as "face" setBlock
    mc__::face_ID west=LEFT, east=LEFT, north=LEFT, south=LEFT;
    
    //Choose face depending on metadata
    switch (meta) {
        case 2:
            east = FRONT; break;
        case 3:
            west = FRONT; break;
        case 4:
            north = FRONT; break;
        case 5:
            south = FRONT; break;
        default:
            south = FRONT; break;
    }

    //A
    if (!(vflags & 0x80)) {
        tx_0 = blockInfo[blockID].tx[west];
        tx_1 = blockInfo[blockID].tx[west] + tmr;
        ty_0 = blockInfo[blockID].ty[west] + tmr;    //flip y
        ty_1 = blockInfo[blockID].ty[west];
        
        glTexCoord2f(tx_0,ty_0); glVertex3i( A, C, E);  //Lower left:  ACE
        glTexCoord2f(tx_1,ty_0); glVertex3i( A, C, F);  //Lower right: ACF
        glTexCoord2f(tx_1,ty_1); glVertex3i( A, D, F);  //Top right:   ADF
        glTexCoord2f(tx_0,ty_1); glVertex3i( A, D, E);  //Top left:    ADE
    }

    //B
    if (!(vflags & 0x40)) {
        tx_0 = blockInfo[blockID].tx[east];
        tx_1 = blockInfo[blockID].tx[east] + tmr;
        ty_0 = blockInfo[blockID].ty[east] + tmr;
        ty_1 = blockInfo[blockID].ty[east];
        
        glTexCoord2f(tx_0,ty_0); glVertex3i( B, C, F);  //Lower left:  BCF
        glTexCoord2f(tx_1,ty_0); glVertex3i( B, C, E);  //Lower right: BCE
        glTexCoord2f(tx_1,ty_1); glVertex3i( B, D, E);  //Top right:   BDE
        glTexCoord2f(tx_0,ty_1); glVertex3i( B, D, F);  //Top left:    BDF
    }
    
    //C
    if (!(vflags & 0x20)) {
        tx_0 = blockInfo[blockID].tx[BOTTOM];
        tx_1 = blockInfo[blockID].tx[BOTTOM] + tmr;
        ty_0 = blockInfo[blockID].ty[BOTTOM] + tmr;
        ty_1 = blockInfo[blockID].ty[BOTTOM];
        
        glTexCoord2f(tx_0,ty_0); glVertex3i( A, C, E);  //Lower left:  ACE
        glTexCoord2f(tx_1,ty_0); glVertex3i( B, C, E);  //Lower right: BCE
        glTexCoord2f(tx_1,ty_1); glVertex3i( B, C, F);  //Top right:   BCF
        glTexCoord2f(tx_0,ty_1); glVertex3i( A, C, F);  //Top left:    ACF
    }
    
    //D
    if (!(vflags & 0x10)) {
        tx_0 = blockInfo[blockID].tx[TOP];
        tx_1 = blockInfo[blockID].tx[TOP] + tmr;
        ty_0 = blockInfo[blockID].ty[TOP] + tmr;
        ty_1 = blockInfo[blockID].ty[TOP];
    
        glTexCoord2f(tx_0,ty_0); glVertex3i( A, D, F);  //Lower left:  ADF
        glTexCoord2f(tx_1,ty_0); glVertex3i( B, D, F);  //Lower right: BDF
        glTexCoord2f(tx_1,ty_1); glVertex3i( B, D, E);  //Top right:   BDE
        glTexCoord2f(tx_0,ty_1); glVertex3i( A, D, E);  //Top left:    ADE
    }
    
    //E
    if (!(vflags & 0x08)) {
        tx_0 = blockInfo[blockID].tx[north];
        tx_1 = blockInfo[blockID].tx[north] + tmr;
        ty_0 = blockInfo[blockID].ty[north] + tmr;
        ty_1 = blockInfo[blockID].ty[north];
        
        glTexCoord2f(tx_0,ty_0); glVertex3i( B, C, E);  //Lower left:  BCE
        glTexCoord2f(tx_1,ty_0); glVertex3i( A, C, E);  //Lower right: ACE
        glTexCoord2f(tx_1,ty_1); glVertex3i( A, D, E);  //Top right:   ADE
        glTexCoord2f(tx_0,ty_1); glVertex3i( B, D, E);  //Top left:    BDE
    }
    
    //F
    if (!(vflags & 0x04)) {
        tx_0 = blockInfo[blockID].tx[south];
        tx_1 = blockInfo[blockID].tx[south] + tmr;
        ty_0 = blockInfo[blockID].ty[south] + tmr;
        ty_1 = blockInfo[blockID].ty[south];
        
        glTexCoord2f(tx_0,ty_0); glVertex3i( A, C, F);  //Lower left:  ACF
        glTexCoord2f(tx_1,ty_0); glVertex3i( B, C, F);  //Lower right: BCF
        glTexCoord2f(tx_1,ty_1); glVertex3i( B, D, F);  //Top right:   BDF
        glTexCoord2f(tx_0,ty_1); glVertex3i( A, D, F);  //Top left:    ADF
    }
    
}

//Alternate face cube type for pumpkins
void BlockDrawer::drawFaceCube2( uint8_t blockID, uint8_t meta,
    GLint x, GLint y, GLint z, uint8_t vflags) const
{
    //Convert to "standard" face cube function
    uint8_t newmeta=0;
    switch (meta) {
        case 0:
            newmeta = 2; break;
        case 1:
            newmeta = 5; break;
        case 2:
            newmeta = 3; break;
        case 3:
            newmeta = 4; break;
        default:
            newmeta = 5; break;
    }
    
    //Draw with the converted metadata
    drawFaceCube( blockID, newmeta, x, y, z, vflags);

}

//Draw silverfish hidden block, depending on metadata
void BlockDrawer::drawEggBlock( uint8_t /*blockID*/, uint8_t meta,
    GLint x, GLint y, GLint z, uint8_t vflags) const
{
    //Block looks like Stone, Cobble, or BrickStone
    switch(meta) {
      case 0:
        drawCube(Blk::Stone, 0, x, y, z, vflags); break;
      case 1:
        drawCube(Blk::Cobble, 0, x, y, z, vflags); break;
      case 2:
        drawCube(Blk::StoneBrick, 0, x, y, z, vflags); break;
      default:
        drawCube(Blk::Stone, 0, x, y, z, vflags); break;
    }
}


//Draw "treasure" chest (large chest if adjacent block is chest)
void BlockDrawer::drawChest( uint8_t blockID, uint8_t meta,
    GLint x, GLint y, GLint z, uint8_t vflags) const
{

    //Examine neighbors  
    //Compare to adjacent blocks
    uint8_t mask=0;
    if (world != NULL) {
        //A neighbor
        Block block = world->getBlock(x - 1, y, z);
        if (block.blockID == blockID) {
            mask |= 1;
        }
        //B neighbor
        block = world->getBlock(x + 1, y, z);
        if (block.blockID == blockID) {
            mask |= 2;
        }
        //E neighbor
        block = world->getBlock(x, y, z - 1);
        if (block.blockID == blockID) {
            mask |= 4;
        }
        //F neighbor
        block = world->getBlock(x, y, z + 1);
        if (block.blockID == blockID) {
            mask |= 8;
        }
    }
    uint16_t ID=blockID;
    switch (mask) {
        case 1:
            ID = 256 + blockID + 1;
            break;
        case 2:
            ID = 256 + blockID + 0;
            break;
        case 4:
            ID = 256 + blockID + 2;
            break;
        case 8:
            ID = 256 + blockID + 3;
            break;
        default:
            ID = blockID;   //Don't combine blocks
    }

    //Chest (54=normal, 256 + 54=left, 55=right, 56=left X, 57=right X)    
    drawCubeMeta(ID, meta, x, y, z, vflags);

}

//Draw cactus... almost like a cube, but A, B, E, F are inset 1 pixel
//  metadata is used for draw height :)
void BlockDrawer::drawCactus( uint8_t blockID, uint8_t meta,
    GLint x, GLint y, GLint z, uint8_t vflags) const
{
    
    //Cactus height is a function of metadata
    GLint cactusHeight = 1 + (meta + 15)%16;
    
    //Face coordinates (in pixels)
    GLint A = (x << 4) + 0;
    GLint B = (x << 4) + texmap_TILE_LENGTH;
    GLint C = (y << 4) + 0;
    //Adjust cactus height based on metadata
    GLint D = (y << 4) + cactusHeight;
    GLint E = (z << 4) + 0;
    GLint F = (z << 4) + texmap_TILE_LENGTH;

    //Texture map coordinates (0.0 - 1.0)
    GLfloat tx_0, tx_1, ty_0, ty_1;

    //For each face, use the appropriate texture offsets for the block ID
    //       ADE ---- BDE
    //       /.       /|
    //      / .      / |
    //    ADF ---- BDF |
    //     | ACE . .| BCE
    //     | .      | /
    //     |.       |/
    //    ACF ---- BCF

    //   Texture map was loaded upside down...
    // 0.0 -------------> 1.0 (X)
    // |
    // |
    // |
    // |
    // |
    // |
    // v
    // 1.0
    // (Y)
    
    //
    // (tx_0, ty_1)      (tx_1, ty_1)
    //
    // (tx_0, ty_0)      (tx_1, ty_0)

    //Scale texture to adjusted cactus height
    GLfloat tmr_y = tmr*(cactusHeight)/16.0;

    //A always visible
        tx_0 = blockInfo[blockID].tx[LEFT];
        tx_1 = blockInfo[blockID].tx[LEFT] + tmr;
        ty_0 = blockInfo[blockID].ty[LEFT] + tmr_y;    //flip y
        ty_1 = blockInfo[blockID].ty[LEFT];
        setBlockColor(blockID, LEFT);  //Set leaf/grass color if needed
        
    // For cactus, the face coordinates are inset from the face. A+1
        glTexCoord2f(tx_0,ty_0); glVertex3i( A+1, C, E);
        glTexCoord2f(tx_1,ty_0); glVertex3i( A+1, C, F);
        glTexCoord2f(tx_1,ty_1); glVertex3i( A+1, D, F);
        glTexCoord2f(tx_0,ty_1); glVertex3i( A+1, D, E);

    //B
        tx_0 = blockInfo[blockID].tx[RIGHT];
        tx_1 = blockInfo[blockID].tx[RIGHT] + tmr;
        ty_0 = blockInfo[blockID].ty[RIGHT] + tmr_y;
        ty_1 = blockInfo[blockID].ty[RIGHT];
        setBlockColor(blockID, RIGHT);  //Set leaf/grass color if needed
        
        glTexCoord2f(tx_0,ty_0); glVertex3i( B-1, C, F);  //Lower left:  BCF
        glTexCoord2f(tx_1,ty_0); glVertex3i( B-1, C, E);  //Lower right: BCE
        glTexCoord2f(tx_1,ty_1); glVertex3i( B-1, D, E);  //Top right:   BDE
        glTexCoord2f(tx_0,ty_1); glVertex3i( B-1, D, F);  //Top left:    BDF
    
    //C not always visible
    if (!(vflags & 0x20)) {
        tx_0 = blockInfo[blockID].tx[BOTTOM];
        tx_1 = blockInfo[blockID].tx[BOTTOM] + tmr;
        ty_0 = blockInfo[blockID].ty[BOTTOM] + tmr;
        ty_1 = blockInfo[blockID].ty[BOTTOM];
        setBlockColor(blockID, BOTTOM);  //Set leaf/grass color if needed
        
        glTexCoord2f(tx_0,ty_0); glVertex3i( A, C, E);  //Lower left:  ACE
        glTexCoord2f(tx_1,ty_0); glVertex3i( B, C, E);  //Lower right: BCE
        glTexCoord2f(tx_1,ty_1); glVertex3i( B, C, F);  //Top right:   BCF
        glTexCoord2f(tx_0,ty_1); glVertex3i( A, C, F);  //Top left:    ACF
    }
    
    //D
    if (!(vflags & 0x10)) {
        tx_0 = blockInfo[blockID].tx[TOP];
        tx_1 = blockInfo[blockID].tx[TOP] + tmr;
        ty_0 = blockInfo[blockID].ty[TOP] + tmr;
        ty_1 = blockInfo[blockID].ty[TOP];
        setBlockColor(blockID, TOP);  //Set leaf/grass color if needed
    
        glTexCoord2f(tx_0,ty_0); glVertex3i( A, D, F);  //Lower left:  ADF
        glTexCoord2f(tx_1,ty_0); glVertex3i( B, D, F);  //Lower right: BDF
        glTexCoord2f(tx_1,ty_1); glVertex3i( B, D, E);  //Top right:   BDE
        glTexCoord2f(tx_0,ty_1); glVertex3i( A, D, E);  //Top left:    ADE
    }
    
    //E always visible
        tx_0 = blockInfo[blockID].tx[BACK];
        tx_1 = blockInfo[blockID].tx[BACK] + tmr;
        ty_0 = blockInfo[blockID].ty[BACK] + tmr_y;
        ty_1 = blockInfo[blockID].ty[BACK];
        setBlockColor(blockID, BACK);  //Set leaf/grass color if needed
        
        glTexCoord2f(tx_0,ty_0); glVertex3i( B, C, E+1);  //Lower left:  BCE
        glTexCoord2f(tx_1,ty_0); glVertex3i( A, C, E+1);  //Lower right: ACE
        glTexCoord2f(tx_1,ty_1); glVertex3i( A, D, E+1);  //Top right:   ADE
        glTexCoord2f(tx_0,ty_1); glVertex3i( B, D, E+1);  //Top left:    BDE
    
    //F
        tx_0 = blockInfo[blockID].tx[FRONT];
        tx_1 = blockInfo[blockID].tx[FRONT] + tmr;
        ty_0 = blockInfo[blockID].ty[FRONT] + tmr_y;
        ty_1 = blockInfo[blockID].ty[FRONT];
        setBlockColor(blockID, FRONT);  //Set leaf/grass color if needed
        
        glTexCoord2f(tx_0,ty_0); glVertex3i( A, C, F-1);  //Lower left:  ACF
        glTexCoord2f(tx_1,ty_0); glVertex3i( B, C, F-1);  //Lower right: BCF
        glTexCoord2f(tx_1,ty_1); glVertex3i( B, D, F-1);  //Top right:   BDF
        glTexCoord2f(tx_0,ty_1); glVertex3i( A, D, F-1);  //Top left:    ADF
    
    //Return color to normal
    setBlockColor( 0, (face_ID)0);
}

//Draw a placed cake block (use metadata to determine how much is eaten)
void BlockDrawer::drawCake( uint8_t blockID, uint8_t meta,
    GLint x, GLint y, GLint z, uint8_t vflags) const
{
    // For cake, the face coordinates are inset from the face
    const size_t offset = texmap_TILE_LENGTH/16;
    const size_t half = texmap_TILE_LENGTH/2;
    
    //Number of "pixels" eaten depends on metadata
    const size_t eaten = (meta == 0 ? 0 : offset * (1 + (meta<<1)));
    float tmr_eat = tmr * eaten/16;
    
    //Face coordinates (in pixels)
    GLint A = (x << 4) + eaten;
    GLint B = (x << 4) + texmap_TILE_LENGTH;
    GLint C = (y << 4) + 0;
    GLint D = (y << 4) + texmap_TILE_LENGTH;
    GLint E = (z << 4) + 0;
    GLint F = (z << 4) + texmap_TILE_LENGTH;

    //Texture map coordinates (0.0 - 1.0)
    GLfloat tx_0, tx_1, ty_0, ty_1;

    //For each face, use the appropriate texture offsets for the block ID
    //       ADE ---- BDE
    //       /.       /|
    //      / .      / |
    //    ADF ---- BDF |
    //     | ACE . .| BCE
    //     | .      | /
    //     |.       |/
    //    ACF ---- BCF

    //   Texture map was loaded upside down...
    // 0.0 -------------> 1.0 (X)
    // |
    // |
    // |
    // |
    // |
    // |
    // v
    // 1.0
    // (Y)
    
    //
    // (tx_0, ty_1)      (tx_1, ty_1)
    //
    // (tx_0, ty_0)      (tx_1, ty_0)

    //A
    mc__::face_ID leftTex = RIGHT;       //"unsliced" side texture
    GLint A_offset = A + offset;    //Offset by 1 for transparent texture
    if (meta > 0) {
        leftTex = LEFT; //Use "sliced" texture on left side, if eaten
        A_offset = A;   //No offset for transparent texture
    } 
    
    //Texture coordinates
    tx_0 = blockInfo[blockID].tx[leftTex];
    tx_1 = blockInfo[blockID].tx[leftTex] + tmr;
    ty_0 = blockInfo[blockID].ty[leftTex] + tmr;    //flip y
    ty_1 = blockInfo[blockID].ty[leftTex];
    
    glTexCoord2f(tx_0,ty_0); glVertex3i( A_offset, C, E);
    glTexCoord2f(tx_1,ty_0); glVertex3i( A_offset, C, F);
    glTexCoord2f(tx_1,ty_1); glVertex3i( A_offset, D, F);
    glTexCoord2f(tx_0,ty_1); glVertex3i( A_offset, D, E);

    //B
    tx_0 = blockInfo[blockID].tx[RIGHT];
    tx_1 = blockInfo[blockID].tx[RIGHT] + tmr;
    ty_0 = blockInfo[blockID].ty[RIGHT] + tmr;
    ty_1 = blockInfo[blockID].ty[RIGHT];
    
    glTexCoord2f(tx_0,ty_0); glVertex3i( B - offset, C , F);
    glTexCoord2f(tx_1,ty_0); glVertex3i( B - offset, C , E);
    glTexCoord2f(tx_1,ty_1); glVertex3i( B - offset, D, E);
    glTexCoord2f(tx_0,ty_1); glVertex3i( B - offset, D, F);

    //C (might be blocked from below)
    if (!(vflags & 0x20)) {
        tx_0 = blockInfo[blockID].tx[BOTTOM] + tmr_eat;
        tx_1 = blockInfo[blockID].tx[BOTTOM] + tmr;
        ty_0 = blockInfo[blockID].ty[BOTTOM] + tmr;
        ty_1 = blockInfo[blockID].ty[BOTTOM];
        
        glTexCoord2f(tx_0,ty_0); glVertex3i( A, C, E);
        glTexCoord2f(tx_1,ty_0); glVertex3i( B, C, E);
        glTexCoord2f(tx_1,ty_1); glVertex3i( B, C, F);
        glTexCoord2f(tx_0,ty_1); glVertex3i( A, C, F);
    }
    
    //D
    tx_0 = blockInfo[blockID].tx[TOP] + tmr_eat;
    tx_1 = blockInfo[blockID].tx[TOP] + tmr;
    ty_0 = blockInfo[blockID].ty[TOP] + tmr;
    ty_1 = blockInfo[blockID].ty[TOP];

    glTexCoord2f(tx_0,ty_0); glVertex3i( A, D - half, F);
    glTexCoord2f(tx_1,ty_0); glVertex3i( B, D - half, F);
    glTexCoord2f(tx_1,ty_1); glVertex3i( B, D - half, E);
    glTexCoord2f(tx_0,ty_1); glVertex3i( A, D - half, E);

    //E always visible
    tx_0 = blockInfo[blockID].tx[BACK];
    tx_1 = blockInfo[blockID].tx[BACK] + tmr - tmr_eat;
    ty_0 = blockInfo[blockID].ty[BACK] + tmr;
    ty_1 = blockInfo[blockID].ty[BACK];
    
    glTexCoord2f(tx_0,ty_0); glVertex3i( B, C , E + offset);
    glTexCoord2f(tx_1,ty_0); glVertex3i( A, C , E + offset);
    glTexCoord2f(tx_1,ty_1); glVertex3i( A, D, E + offset);
    glTexCoord2f(tx_0,ty_1); glVertex3i( B, D, E + offset);

    //F
    tx_0 = blockInfo[blockID].tx[FRONT] + tmr_eat;
    tx_1 = blockInfo[blockID].tx[FRONT] + tmr;
    ty_0 = blockInfo[blockID].ty[FRONT] + tmr;
    ty_1 = blockInfo[blockID].ty[FRONT];
    
    glTexCoord2f(tx_0,ty_0); glVertex3i( A, C , F - offset);
    glTexCoord2f(tx_1,ty_0); glVertex3i( B, C , F - offset);
    glTexCoord2f(tx_1,ty_1); glVertex3i( B, D , F - offset);
    glTexCoord2f(tx_0,ty_1); glVertex3i( A, D , F - offset);
    
}

//Draw a placed bed block (use metadata to determine which half)
void BlockDrawer::drawBed( uint8_t blockID, uint8_t meta,
    GLint x, GLint y, GLint z, uint8_t vflags) const
{

    //Top or bottom half
    bool top_half = ((meta & 0x8) != 0);
    
    //Direction of block depends on metadata.
    //  drawVertexBlock rotates the block depending on facing,
    //  and the vflags rotate with it!
    //
    //Due to the orientation of textures for the top of the bed,
    //  we have to "turn" the block orientation clockwise
    face_ID facing;
    switch (meta & 0xB) {
        //Direction
        case 0x0: facing = LEFT;  vflags |= 0x04; break;    //South foot
        case 0x1: facing = BACK;  vflags |= 0x80; break;    //West foot
        case 0x2: facing = RIGHT; vflags |= 0x08; break;    //North foot
        case 0x3: facing = FRONT; vflags |= 0x40; break;    //East foot
        case 0x8: facing = LEFT;  vflags |= 0x08; break;    //South head
        case 0x9: facing = BACK;  vflags |= 0x40; break;    //West head
        case 0xA: facing = RIGHT; vflags |= 0x04; break;    //East head
        case 0xB: facing = FRONT; vflags |= 0x80; break;    //South head
        default:  facing = FRONT;
    }
    // facing points to right side of pillow
    
    //Use block ID offset for top or bottom half of bed
    uint16_t blockOffset = blockID + 256;
    if ( top_half ) {
        blockOffset++;
    }
    //This block info has properly adjusted texture coordinates
    const BlockInfo& binfo = blockInfo[blockOffset];

    //Create vertices for bed half
    GLint vX[8], vY[8], vZ[8];
    //Bed block is 16x9x16 pixels
    makeCuboidVertex(x, y, z, 16, 9, 16, vX, vY, vZ, facing);
    
    //Draw bed half.  Always remove bottom, it will be drawn next.
    drawVertexBlock( vX, vY, vZ, binfo.tx, binfo.tx_1, binfo.ty, binfo.ty_1,
        vflags |= 0x20, facing);
        
    //Draw the wood bottom, raised by 3/16
    glTexCoord2f(binfo.tx[2],binfo.ty_1[2]);
    glVertex3i( vX[0], vY[0]+3, vZ[0]);
    
    glTexCoord2f(binfo.tx_1[2],binfo.ty_1[2]);
    glVertex3i( vX[4], vY[4]+3, vZ[4]);
    
    glTexCoord2f(binfo.tx_1[2],binfo.ty[2]);
    glVertex3i( vX[6], vY[6]+3, vZ[6]);
    
    glTexCoord2f(binfo.tx[2],binfo.ty[2]);
    glVertex3i( vX[2], vY[2]+3, vZ[2]);


}

//  adjustTexture: Reduce texture coordinates to match cuboid
//
//  blockID = normal or "special" ID for this block
//  off_x =   The cuboid X offset, used to calculate the texture adjustment
//  off_y =   The cuboid Y offset
//  off_z =   The cuboid Z offset
//  width =   Cuboid X size
//  height =  Cuboid Y size
//  depth =   Cuboid Z size
    //  Before
    //       ADE ---- BDE
    //       /.       /|
    //      / .      / |
    //    ADF ---- BDF |
    //     | ACE . .| BCE
    //     | .      | /
    //     |.       |/
    //    ACF ---- BCF

    //  After     off_x
    //           /
    //       ADE ---- BDE
    //       / +------+--- off_z  
    //      / /      / |
    //    ADF ---- BDF |
    //     | ACE . .| BCE
    //     | .      | /
    //     |------------ off_y
    //    ACF ---- BCF
void BlockDrawer::adjustTexture(uint16_t blockID,
    GLint off_x, GLint off_y, GLint off_z,
    GLsizei width, GLsizei height, GLsizei depth)
{
    //Scaled texture map ratios (1/block length)
    GLfloat tmr_x, tmr_y, tmr_z, tmr_off_x, tmr_off_y, tmr_off_z;

    tmr_x  = (width/TILE_LENGTH) * tmr;
    tmr_y = (height/TILE_LENGTH) * tmr;
    tmr_z  = (depth/TILE_LENGTH) * tmr;
    tmr_off_x = fabs(tmr*off_x/TILE_LENGTH);
    tmr_off_y = fabs(tmr*off_y/TILE_LENGTH);
    tmr_off_z = fabs(tmr*off_z/TILE_LENGTH);
    
    //Sup3r s3(R37 c++ array reference technique
    GLfloat (&tx_0)[6] = blockInfo[blockID].tx;
    GLfloat (&ty_0)[6] = blockInfo[blockID].ty;
    GLfloat (&tx_1)[6] = blockInfo[blockID].tx_1;
    GLfloat (&ty_1)[6] = blockInfo[blockID].ty_1;

    //  Remember, texture coordinates tx and ty have nothing
    //  to do with vertex offsets off_x, off_y, off_z
    
    //Careful, adjust tx_1 and ty_1 before tx_0 and ty_0
    tx_1[LEFT] = tx_0[LEFT] + tmr_off_z + tmr_z;
    tx_0[LEFT] = tx_0[LEFT] + tmr_off_z;
    ty_1[LEFT] = ty_0[LEFT] + tmr_off_y + tmr_y;
    ty_0[LEFT] = ty_0[LEFT] + tmr_off_y;    //Set ty_0 after ty_1
    
    tx_1[RIGHT] = tx_0[RIGHT] + tmr_off_z;
    tx_0[RIGHT] = tx_0[RIGHT] + tmr_off_z + tmr_z;
    ty_1[RIGHT] = ty_0[RIGHT] + tmr_off_y + tmr_y;
    ty_0[RIGHT] = ty_0[RIGHT] + tmr_off_y;
    
    tx_1[BOTTOM] = tx_0[BOTTOM] + tmr_off_x + tmr_x;
    tx_0[BOTTOM] = tx_0[BOTTOM] + tmr_off_x;
    ty_1[BOTTOM] = ty_0[BOTTOM] + tmr_off_z + tmr_z;
    ty_0[BOTTOM] = ty_0[BOTTOM] + tmr_off_z;

    tx_1[TOP] = tx_0[TOP] + tmr_off_x + tmr_x;
    tx_0[TOP] = tx_0[TOP] + tmr_off_x;
    ty_1[TOP] = ty_0[TOP] + tmr_off_z + tmr_z;
    ty_0[TOP] = ty_0[TOP] + tmr_off_z;

    tx_1[BACK] = tx_0[BACK] + tmr_off_x;
    tx_0[BACK] = tx_0[BACK] + tmr_off_x + tmr_x;
    ty_1[BACK] = ty_0[BACK] + tmr_off_y + tmr_y;
    ty_0[BACK] = ty_0[BACK] + tmr_off_y;

    tx_1[FRONT] = tx_0[FRONT] + tmr_off_x + tmr_x;
    tx_0[FRONT] = tx_0[FRONT] + tmr_off_x;
    ty_1[FRONT] = ty_0[FRONT] + tmr_off_y + tmr_y;
    ty_0[FRONT] = ty_0[FRONT] + tmr_off_y;

}

//Use OpenGL to draw partial solid cube, with offsets, scale, mirroring
//
//  For example, to draw 
//
void BlockDrawer::drawScaledBlock( uint16_t blockID, uint8_t /*meta*/,
    GLint x, GLint y, GLint z, uint8_t vflags,
    GLfloat scale_x, GLfloat scale_y, GLfloat scale_z,
    bool scale_texture,
    GLint off_x, GLint off_y, GLint off_z, uint8_t mirror) const
{
    GLint width, height, depth;
    
    width  = texmap_TILE_LENGTH * scale_x;
    height = texmap_TILE_LENGTH * scale_y;
    depth  = texmap_TILE_LENGTH * scale_z;
    
    //Face coordinates (GL coordinates are 16*block coords)
    GLint A = (x << 4) + off_x;
    GLint B = (x << 4) + off_x + width;
    GLint C = (y << 4) + off_y;
    GLint D = (y << 4) + off_y + height;
    GLint E = (z << 4) + off_z;
    GLint F = (z << 4) + off_z + depth;

    //Texture map coordinates (0.0 - 1.0), and mirror copying coords
    GLfloat tx_0, tx_1, ty_0, ty_1;
    
    //Scaled texture map ratios (1/block length)
    GLfloat tmr_x, tmr_y, tmr_z, tmr_off_x, tmr_off_y, tmr_off_z;
    if (scale_texture) {
        tmr_x  = scale_x * tmr;
        tmr_y = scale_y * tmr;
        tmr_z  = scale_z * tmr;
        tmr_off_x = fabs(tmr*off_x/float(texmap_TILE_LENGTH));
        tmr_off_y = fabs(tmr*off_y/float(texmap_TILE_LENGTH));
        tmr_off_z = fabs(tmr*off_z/float(texmap_TILE_LENGTH));
    } else {
        tmr_x = tmr_y = tmr_z = tmr;
        tmr_off_x =  tmr_off_y = tmr_off_z = 0;
    }

    //For each face, use the appropriate texture offsets for the block ID
    //       ADE ---- BDE
    //       /|       /|
    //      /ACE . ./BCE
    //    ADF ---- BDF/
    //     |.       |/
    //    ACF ---- BCF

    //   Texture map was loaded upside down...
    // 0.0 -------------> 1.0 (X)
    // |
    // |
    // |
    // |
    // |
    // |
    // v
    // 1.0
    // (Y)
    
    //
    // (tx_0, ty_1)      (tx_1, ty_1)
    //
    // (tx_0, ty_0)      (tx_1, ty_0)

    //A
    if (!(vflags & 0x80) && (scale_y != 0.0 && scale_z != 0.0) ) {
        //Use offsets and scaling for texture coordinates
        tx_0 = blockInfo[blockID].tx[LEFT] + tmr_off_z;
        tx_1 = blockInfo[blockID].tx[LEFT] + tmr_off_z + tmr_z;
        ty_0 = blockInfo[blockID].ty[LEFT] + tmr_off_y + tmr_y;
        ty_1 = blockInfo[blockID].ty[LEFT] + tmr_off_y;
        
        //Swap coordinates if needed
        if (mirror & 0x80) { mirrorCoords(tx_0, tx_1, ty_0, ty_1); }
        
        glTexCoord2f(tx_0,ty_0); glVertex3i( A, C, E);  //Lower left:  ACE
        glTexCoord2f(tx_1,ty_0); glVertex3i( A, C, F);  //Lower right: ACF
        glTexCoord2f(tx_1,ty_1); glVertex3i( A, D, F);  //Top right:   ADF
        glTexCoord2f(tx_0,ty_1); glVertex3i( A, D, E);  //Top left:    ADE
    }

    //B
    if (!(vflags & 0x40) && (scale_y != 0.0 && scale_z != 0.0) ) {
        //Use offsets and scaling for texture coordinates
        tx_0 = blockInfo[blockID].tx[RIGHT] + tmr_off_z + tmr_z;
        tx_1 = blockInfo[blockID].tx[RIGHT] + tmr_off_z;
        ty_0 = blockInfo[blockID].ty[RIGHT] + tmr_off_y + tmr_y;
        ty_1 = blockInfo[blockID].ty[RIGHT] + tmr_off_y;
        
        //Swap coordinates if needed
        if (mirror & 0x40) { mirrorCoords(tx_0, tx_1, ty_0, ty_1); }
        
        glTexCoord2f(tx_0,ty_0); glVertex3i( B, C, F);  //Lower left:  BCF
        glTexCoord2f(tx_1,ty_0); glVertex3i( B, C, E);  //Lower right: BCE
        glTexCoord2f(tx_1,ty_1); glVertex3i( B, D, E);  //Top right:   BDE
        glTexCoord2f(tx_0,ty_1); glVertex3i( B, D, F);  //Top left:    BDF
    }
    
    //C
    if (!(vflags & 0x20) && (scale_x != 0.0 && scale_z != 0.0) ) {
        //Use offsets and scaling for texture coordinates
        tx_0 = blockInfo[blockID].tx[BOTTOM] + tmr_off_x;
        tx_1 = blockInfo[blockID].tx[BOTTOM] + tmr_off_x + tmr_x;
        ty_0 = blockInfo[blockID].ty[BOTTOM] + tmr_off_z + tmr_z;
        ty_1 = blockInfo[blockID].ty[BOTTOM] + tmr_off_z;
        
        //Swap coordinates if needed
        if (mirror & 0x20) { mirrorCoords(tx_0, tx_1, ty_0, ty_1); }
        
        glTexCoord2f(tx_0,ty_0); glVertex3i( A, C, E);  //Lower left:  ACE
        glTexCoord2f(tx_1,ty_0); glVertex3i( B, C, E);  //Lower right: BCE
        glTexCoord2f(tx_1,ty_1); glVertex3i( B, C, F);  //Top right:   BCF
        glTexCoord2f(tx_0,ty_1); glVertex3i( A, C, F);  //Top left:    ACF
    }
    
    //D
    if (!(vflags & 0x10) && (scale_x != 0.0 && scale_z != 0.0) ) {
        //Use offsets and scaling for texture coordinates
        tx_0 = blockInfo[blockID].tx[TOP] + tmr_off_x;
        tx_1 = blockInfo[blockID].tx[TOP] + tmr_off_x + tmr_x;
        ty_0 = blockInfo[blockID].ty[TOP] + tmr_off_z + tmr_z;
        ty_1 = blockInfo[blockID].ty[TOP] + tmr_off_z;
        
        //Swap coordinates if needed
        if (mirror & 0x10) { mirrorCoords(tx_0, tx_1, ty_0, ty_1); }
    
        glTexCoord2f(tx_0,ty_0); glVertex3i( A, D, F);  //Lower left:  ADF
        glTexCoord2f(tx_1,ty_0); glVertex3i( B, D, F);  //Lower right: BDF
        glTexCoord2f(tx_1,ty_1); glVertex3i( B, D, E);  //Top right:   BDE
        glTexCoord2f(tx_0,ty_1); glVertex3i( A, D, E);  //Top left:    ADE
    }
    
    //E
    if (!(vflags & 0x08) && (scale_x != 0.0 && scale_y != 0.0) ) {
        //Use offsets and scaling for texture coordinates
        tx_0 = blockInfo[blockID].tx[BACK] + tmr_off_x + tmr_x;
        tx_1 = blockInfo[blockID].tx[BACK] + tmr_off_x;
        ty_0 = blockInfo[blockID].ty[BACK] + tmr_off_y + tmr_y;
        ty_1 = blockInfo[blockID].ty[BACK] + tmr_off_y;
        
        //Swap coordinates if needed
        if (mirror & 0x08) { mirrorCoords(tx_0, tx_1, ty_0, ty_1); }
        
        glTexCoord2f(tx_0,ty_0); glVertex3i( B, C, E);  //Lower left:  BCE
        glTexCoord2f(tx_1,ty_0); glVertex3i( A, C, E);  //Lower right: ACE
        glTexCoord2f(tx_1,ty_1); glVertex3i( A, D, E);  //Top right:   ADE
        glTexCoord2f(tx_0,ty_1); glVertex3i( B, D, E);  //Top left:    BDE
    }
    
    //F
    if (!(vflags & 0x04) && (scale_x != 0.0 && scale_y != 0.0) ) {
        //Use offsets and scaling for texture coordinates
        tx_0 = blockInfo[blockID].tx[FRONT] + tmr_off_x;
        tx_1 = blockInfo[blockID].tx[FRONT] + tmr_off_x + tmr_x;
        ty_0 = blockInfo[blockID].ty[FRONT] + tmr_off_y + tmr_y;
        ty_1 = blockInfo[blockID].ty[FRONT] + tmr_off_y;
        
        //Swap coordinates if needed
        if (mirror & 0x04) { mirrorCoords(tx_0, tx_1, ty_0, ty_1); }
        
        glTexCoord2f(tx_0,ty_0); glVertex3i( A, C, F);  //Lower left:  ACF
        glTexCoord2f(tx_1,ty_0); glVertex3i( B, C, F);  //Lower right: BCF
        glTexCoord2f(tx_1,ty_1); glVertex3i( B, D, F);  //Top right:   BDF
        glTexCoord2f(tx_0,ty_1); glVertex3i( A, D, F);  //Top left:    ADF
    }
}

//Draw half a block
void BlockDrawer::drawSlab( uint8_t blockID, uint8_t meta,
    GLint x, GLint y, GLint z, uint8_t vflags) const
{
    
    //Use metadata to determine which texture
    uint16_t material=blockID;
    switch (meta) {
        default:
        case 0: material = blockID; break;  //Stone
        case 1: material = 24; break;       //Sandstone
        case 2: material = 5; break;        //Wood
        case 3: material = 4; break;        //Cobble
    }
    //Draw bottom half
    drawScaledBlock( material, meta, x, y, z, vflags&0xEF, 1, 0.5, 1);

}

//Draw two stacked half blocks
void BlockDrawer::drawDoubleSlab( uint8_t blockID, uint8_t meta,
    GLint x, GLint y, GLint z, uint8_t vflags) const
{
    
    //Use metadata to determine which texture
    uint16_t material=blockID;
    switch (meta) {
        default:
        case 0: material = blockID; break;  //Stone
        case 1: material = 24; break;       //Sandstone
        case 2: material = 5; break;        //Wood
        case 3: material = 4; break;        //Cobble
    }
    //Draw top half
    drawScaledBlock( material, meta, x, y, z, vflags|0x20, 1, 0.5, 1,
        true, 0, 8, 0);
    //Draw bottom half
    drawScaledBlock( material, meta, x, y, z, vflags|0x10, 1, 0.5, 1);

}


//Draw minecart track (meta affects angle, direction, intersection)
void BlockDrawer::drawTrack( uint8_t blockID, uint8_t meta,
    GLint x, GLint y, GLint z, uint8_t /*vflags*/) const
{
    //Object boundaries... flat square 1 pixel off the ground
    GLint A = (x << 4) + 0;
    GLint B = (x << 4) + texmap_TILE_LENGTH;
    GLint C = (y << 4) + 1;
    GLint D = (y << 4) + 17;
    GLint E = (z << 4) + 0;
    GLint F = (z << 4) + texmap_TILE_LENGTH;

    //3D coords for Bottom-left, bottom-right, top-right, top-left
    GLint X[4], Y[4], Z[4];
    //Defaults
    X[0] = X[3] = A; X[1] = X[2] = B;
    Y[0] = Y[1] = Y[2] = Y[3] = C;
    Z[0] = Z[1] = F; Z[2] = Z[3] = E;
    
    //Texture index: 0 is straight, 1 is turn
    face_ID t_index=LEFT;

    //Metadata determines track orientation, shape
    switch (meta) {
        case 1: //East-West
            X[1] = A; X[3] = B; Z[0] = E; Z[2] = F; break;
        case 2: //Ascend South
            X[1] = A; X[3] = B; Z[0] = E; Z[2] = F; //rotate
            Y[2] = Y[3] = D; break;
        case 3: //Ascend North
            X[1] = A; X[3] = B; Z[0] = E; Z[2] = F; //rotate
                Y[0] = Y[1] = D; break;
        case 4: //Ascend East
            Y[2] = Y[3] = D; break;  
        case 5: //Ascend West
            Y[0] = Y[1] = D; break;
        case 6: //NorthEast corner
            t_index = RIGHT; break;
        case 7: //SouthEast corner
            t_index = RIGHT;
            X[1] = A; X[3] = B; Z[0] = E; Z[2] = F; break;
        case 8: //SouthWest corner
            t_index = RIGHT; X[0] = X[3] = B; X[1] = X[2] = A;
            Z[0] = Z[1] = E; Z[2] = Z[3] = F; break;
        case 9: //NorthWest corner
            t_index = RIGHT; X[0] = B; X[2] = A;
            Z[1] = E; Z[3] = F; break;
        default: break; //default, flat track on ground
    }

    //Texture coordinates
    const BlockInfo& binfo = blockInfo[blockID];
    GLfloat tx_0 = binfo.tx[t_index];
    GLfloat tx_1 = binfo.tx[t_index] + tmr;
    GLfloat ty_0 = binfo.ty[t_index] + tmr;
    GLfloat ty_1 = binfo.ty[t_index];
    
    //Top side?
    glTexCoord2f(tx_0,ty_0); glVertex3i( X[0], Y[0], Z[0]);  //Lower left:  ACE
    glTexCoord2f(tx_1,ty_0); glVertex3i( X[1], Y[1], Z[1]);  //Lower right: BCE
    glTexCoord2f(tx_1,ty_1); glVertex3i( X[2], Y[2], Z[2]);  //Top right:   BCF
    glTexCoord2f(tx_0,ty_1); glVertex3i( X[3], Y[3], Z[3]);  //Top left:    ACF

    //Reverse side
    mirrorCoords(tx_0, tx_1, ty_0, ty_1);
    glTexCoord2f(tx_0,ty_0); glVertex3i( X[1], Y[1], Z[1]);  //Lower left:  ADF
    glTexCoord2f(tx_1,ty_0); glVertex3i( X[0], Y[0], Z[0]);  //Lower right: BDF
    glTexCoord2f(tx_1,ty_1); glVertex3i( X[3], Y[3], Z[3]);  //Top right:   BDE
    glTexCoord2f(tx_0,ty_1); glVertex3i( X[2], Y[2], Z[2]);  //Top left:    ADE

}


//Draw powered minecart track (meta affects angle, direction, texture)
void BlockDrawer::drawTrack2( uint8_t blockID, uint8_t meta,
    GLint x, GLint y, GLint z, uint8_t /*vflags*/) const
{
    //Object boundaries... flat square 1 pixel off the ground
    GLint A = (x << 4) + 0;
    GLint B = (x << 4) + texmap_TILE_LENGTH;
    GLint C = (y << 4) + 1;
    GLint D = (y << 4) + 17;
    GLint E = (z << 4) + 0;
    GLint F = (z << 4) + texmap_TILE_LENGTH;

    //3D coords for Bottom-left, bottom-right, top-right, top-left
    GLint X[4], Y[4], Z[4];
    //Defaults
    X[0] = X[3] = A; X[1] = X[2] = B;
    Y[0] = Y[1] = Y[2] = Y[3] = C;
    Z[0] = Z[1] = F; Z[2] = Z[3] = E;
    
    //Texture index: 0 is unpowered, 1 is powered
    face_ID t_index=LEFT;
    if (meta & 0x8) {
        t_index = RIGHT;    //"Powered" texture
    }

    //Metadata determines track orientation, shape
    switch (meta & 0x7) {
        case 1: //East-West
            X[1] = A; X[3] = B; Z[0] = E; Z[2] = F; break;
        case 2: //Ascend South
            X[1] = A; X[3] = B; Z[0] = E; Z[2] = F; //rotate
            Y[2] = Y[3] = D; break;
        case 3: //Ascend North
            X[1] = A; X[3] = B; Z[0] = E; Z[2] = F; //rotate
                Y[0] = Y[1] = D; break;
        case 4: //Ascend East
            Y[2] = Y[3] = D; break;  
        case 5: //Ascend West
            Y[0] = Y[1] = D; break;
        case 6: //NorthEast corner
            t_index = RIGHT; break;
        case 7: //SouthEast corner
            t_index = RIGHT;
            X[1] = A; X[3] = B; Z[0] = E; Z[2] = F; break;
        default: break; //default, flat track on ground
    }

    //Texture coordinates
    const BlockInfo& binfo = blockInfo[blockID];
    GLfloat tx_0 = binfo.tx[t_index];
    GLfloat tx_1 = binfo.tx[t_index] + tmr;
    GLfloat ty_0 = binfo.ty[t_index] + tmr;
    GLfloat ty_1 = binfo.ty[t_index];
    
    //Top side?
    glTexCoord2f(tx_0,ty_0); glVertex3i( X[0], Y[0], Z[0]);  //Lower left:  ACE
    glTexCoord2f(tx_1,ty_0); glVertex3i( X[1], Y[1], Z[1]);  //Lower right: BCE
    glTexCoord2f(tx_1,ty_1); glVertex3i( X[2], Y[2], Z[2]);  //Top right:   BCF
    glTexCoord2f(tx_0,ty_1); glVertex3i( X[3], Y[3], Z[3]);  //Top left:    ACF

    //Reverse side
    mirrorCoords(tx_0, tx_1, ty_0, ty_1);
    glTexCoord2f(tx_0,ty_0); glVertex3i( X[1], Y[1], Z[1]);  //Lower left:  ADF
    glTexCoord2f(tx_1,ty_0); glVertex3i( X[0], Y[0], Z[0]);  //Lower right: BDF
    glTexCoord2f(tx_1,ty_1); glVertex3i( X[3], Y[3], Z[3]);  //Top right:   BDE
    glTexCoord2f(tx_0,ty_1); glVertex3i( X[2], Y[2], Z[2]);  //Top left:    ADE

}



//Draw item blockID which is placed flat on the wall
void BlockDrawer::drawWallItem( uint8_t blockID, uint8_t meta,
    GLint x, GLint y, GLint z, uint8_t /*vflags*/) const
{
    //Texture coords
    GLfloat tx_0 = blockInfo[blockID].tx[LEFT];
    GLfloat tx_1 = blockInfo[blockID].tx[LEFT] + tmr;
    GLfloat ty_0 = blockInfo[blockID].ty[LEFT] + tmr;
    GLfloat ty_1 = blockInfo[blockID].ty[LEFT];

    //Block face coordinates (in pixels)
    GLint A = (x << 4) + 0;
    GLint B = (x << 4) + texmap_TILE_LENGTH;
    GLint C = (y << 4) + 0;
    GLint D = (y << 4) + texmap_TILE_LENGTH;
    GLint E = (z << 4) + 0;
    GLint F = (z << 4) + texmap_TILE_LENGTH;

    //Use metadata for vertex coordinates
    GLint x0, x1, x2, x3, z0, z1, z2, z3;
    switch (meta) {
        case 5:
            x0 = x1 = A; x2 = x3 = A + 1; z0 = z2 = E; z1 = z3 = F;
            break;
        case 4:
            x0 = x1 = B; x2 = x3 = B - 1; z0 = z2 = F; z1 = z3 = E;
            break;
        case 3:
            x0 = x2 = B; x1 = x3 = A; z0 = z1 = E; z2 = z3 = E + 1;
            break;
        case 2:
        default:
            x0 = x2 = A; x1 = x3 = B; z0 = z1 = F; z2 = z3 = F - 1;
            break;
    }
    
    //outer face (not seen except through glass)
    glTexCoord2f(tx_0,ty_0); glVertex3i( x0, C, z0);  //Lower left:  ACF
    glTexCoord2f(tx_1,ty_0); glVertex3i( x1, C, z1);  //Lower right: BCF
    glTexCoord2f(tx_1,ty_1); glVertex3i( x1, D, z1);  //Top right:   BDF
    glTexCoord2f(tx_0,ty_1); glVertex3i( x0, D, z0);  //Top left:    ADF
    //inner face
    glTexCoord2f(tx_0,ty_1); glVertex3i( x2, D, z2);  //Top left:    ADF
    glTexCoord2f(tx_1,ty_1); glVertex3i( x3, D, z3);  //Top right:   BDF
    glTexCoord2f(tx_1,ty_0); glVertex3i( x3, C, z3);  //Lower right: BCF
    glTexCoord2f(tx_0,ty_0); glVertex3i( x2, C, z2);  //Lower left:  ACF

}

void BlockDrawer::drawBiomeItem( uint8_t blockID, uint8_t meta,
    GLint x, GLint y, GLint z, uint8_t vflags) const
{
    setBlockColor(blockID, (face_ID)0);
    
    //Draw the item
    drawItem( blockID, meta, x, y, z, vflags);
    
    //Return to normal
    setBlockColor( 0, (face_ID)0);

}


//Draw item blockID which is placed as a block
void BlockDrawer::drawItem( uint8_t blockID, uint8_t /*meta*/,
    GLint x, GLint y, GLint z, uint8_t /*vflags*/) const
{
    //Texture map coordinates (0.0 - 1.0)
    GLfloat tx_0, tx_1, ty_0, ty_1;

    //Object boundaries... 2 crossed squares inside a clear cube
    GLint A = (x << 4) + 0;
    GLint B = (x << 4) + texmap_TILE_LENGTH;
    GLint C = (y << 4) + 0;
    GLint D = (y << 4) + texmap_TILE_LENGTH;
    GLint E = (z << 4) + 0;
    GLint F = (z << 4) + texmap_TILE_LENGTH;
    GLint G = (z << 4) + (texmap_TILE_LENGTH/2);    //half-way through z 
    GLint H = (x << 4) + (texmap_TILE_LENGTH/2);    //half-way through x

    //Look up texture coordinates for the item
    tx_0 = blockInfo[blockID].tx[LEFT];
    tx_1 = blockInfo[blockID].tx[LEFT] + tmr;
    ty_0 = blockInfo[blockID].ty[LEFT] + tmr;    //flip y
    ty_1 = blockInfo[blockID].ty[LEFT];
    glTexCoord2f(tx_0,ty_0); glVertex3i( A, C, G);  //Lower left:  ACG
    glTexCoord2f(tx_1,ty_0); glVertex3i( B, C, G);  //Lower right: BCG
    glTexCoord2f(tx_1,ty_1); glVertex3i( B, D, G);  //Top right:   BDG
    glTexCoord2f(tx_0,ty_1); glVertex3i( A, D, G);  //Top left:    ADG

    //Back face
    glTexCoord2f(tx_0,ty_0); glVertex3i( A, C, G);  //Lower left:  ACG
    glTexCoord2f(tx_0,ty_1); glVertex3i( A, D, G);  //Top left:    ADG
    glTexCoord2f(tx_1,ty_1); glVertex3i( B, D, G);  //Top right:   BDG
    glTexCoord2f(tx_1,ty_0); glVertex3i( B, C, G);  //Lower right: BCG

    //Intersecting plane
    tx_0 = blockInfo[blockID].tx[RIGHT];
    tx_1 = blockInfo[blockID].tx[RIGHT] + tmr;
    ty_0 = blockInfo[blockID].ty[RIGHT] + tmr;    //flip y
    ty_1 = blockInfo[blockID].ty[RIGHT];
    glTexCoord2f(tx_0,ty_0); glVertex3i( H, C, F);  //Lower left:  HCF
    glTexCoord2f(tx_1,ty_0); glVertex3i( H, C, E);  //Lower right: HCE
    glTexCoord2f(tx_1,ty_1); glVertex3i( H, D, E);  //Top right:   HDE
    glTexCoord2f(tx_0,ty_1); glVertex3i( H, D, F);  //Top left:    HDF

    //Back face
    glTexCoord2f(tx_0,ty_0); glVertex3i( H, C, F);  //Lower left:  HCF
    glTexCoord2f(tx_0,ty_1); glVertex3i( H, D, F);  //Top left:    HDF
    glTexCoord2f(tx_1,ty_1); glVertex3i( H, D, E);  //Top right:   HDE
    glTexCoord2f(tx_1,ty_0); glVertex3i( H, C, E);  //Lower right: HCE

}

//Draw item blockID which is placed as a block
void BlockDrawer::drawSapling( uint8_t blockID, uint8_t meta,
    GLint x, GLint y, GLint z, uint8_t /*vflags*/) const
{

    //Texture map coordinates (0.0 - 1.0)
    GLfloat tx_0, tx_1, ty_0, ty_1;
    
    //Sapling type depends on metadata
    size_t saptex = (meta & 0x3);

    //Object boundaries... 2 crossed squares inside a clear cube
    GLint A = (x << 4) + 0;
    GLint B = (x << 4) + texmap_TILE_LENGTH;
    GLint C = (y << 4) + 0;
    GLint D = (y << 4) + texmap_TILE_LENGTH;
    GLint E = (z << 4) + 0;
    GLint F = (z << 4) + texmap_TILE_LENGTH;
    GLint G = (z << 4) + (texmap_TILE_LENGTH/2);    //half-way through z 
    GLint H = (x << 4) + (texmap_TILE_LENGTH/2);    //half-way through x

    //Look up texture coordinates for the sapling
    tx_0 = blockInfo[blockID].tx[saptex];
    tx_1 = blockInfo[blockID].tx[saptex] + tmr;
    ty_0 = blockInfo[blockID].ty[saptex] + tmr;    //flip y
    ty_1 = blockInfo[blockID].ty[saptex];
    glTexCoord2f(tx_0,ty_0); glVertex3i( A, C, G);  //Lower left:  ACG
    glTexCoord2f(tx_1,ty_0); glVertex3i( B, C, G);  //Lower right: BCG
    glTexCoord2f(tx_1,ty_1); glVertex3i( B, D, G);  //Top right:   BDG
    glTexCoord2f(tx_0,ty_1); glVertex3i( A, D, G);  //Top left:    ADG

    //Back face
    glTexCoord2f(tx_0,ty_0); glVertex3i( A, C, G);  //Lower left:  ACG
    glTexCoord2f(tx_0,ty_1); glVertex3i( A, D, G);  //Top left:    ADG
    glTexCoord2f(tx_1,ty_1); glVertex3i( B, D, G);  //Top right:   BDG
    glTexCoord2f(tx_1,ty_0); glVertex3i( B, C, G);  //Lower right: BCG

    //Intersecting plane
    tx_0 = blockInfo[blockID].tx[saptex];
    tx_1 = blockInfo[blockID].tx[saptex] + tmr;
    ty_0 = blockInfo[blockID].ty[saptex] + tmr;    //flip y
    ty_1 = blockInfo[blockID].ty[saptex];
    glTexCoord2f(tx_0,ty_0); glVertex3i( H, C, F);  //Lower left:  HCF
    glTexCoord2f(tx_1,ty_0); glVertex3i( H, C, E);  //Lower right: HCE
    glTexCoord2f(tx_1,ty_1); glVertex3i( H, D, E);  //Top right:   HDE
    glTexCoord2f(tx_0,ty_1); glVertex3i( H, D, F);  //Top left:    HDF

    //Back face
    glTexCoord2f(tx_0,ty_0); glVertex3i( H, C, F);  //Lower left:  HCF
    glTexCoord2f(tx_0,ty_1); glVertex3i( H, D, F);  //Top left:    HDF
    glTexCoord2f(tx_1,ty_1); glVertex3i( H, D, E);  //Top right:   HDE
    glTexCoord2f(tx_1,ty_0); glVertex3i( H, C, E);  //Lower right: HCE

}

//Draw torch, on ground or wall
void BlockDrawer::drawTorch( uint8_t blockID, uint8_t meta,
    GLint x, GLint y, GLint z, uint8_t /*vflags*/) const
{

    //Look up texture coordinates for the item
    GLfloat tx_0 = blockInfo[blockID].tx[LEFT];
    GLfloat tx_1 = blockInfo[blockID].tx[LEFT] + tmr;
    GLfloat ty_0 = blockInfo[blockID].ty[LEFT] + tmr;    //flip y
    GLfloat ty_1 = blockInfo[blockID].ty[LEFT];
    
    //pieces of torch texture (used for top and bottom)
    GLfloat tx_m1 = blockInfo[blockID].tx[LEFT] + tmr*7.0/16.0;
    GLfloat tx_m2 = blockInfo[blockID].tx[LEFT] + tmr*9.0/16.0;
    GLfloat ty_m1 = blockInfo[blockID].ty[LEFT] + tmr*8.0/16.0;
    GLfloat ty_m2 = blockInfo[blockID].ty[LEFT] + tmr*6.0/16.0;
    GLfloat ty_b1 = blockInfo[blockID].ty[LEFT] + tmr*14.0/16.0;
    GLfloat ty_b2 = blockInfo[blockID].ty[LEFT] + tmr;

    //Cube boundaries
    GLint A = (x << 4) + 0;
    GLint B = (x << 4) + texmap_TILE_LENGTH;
    GLint C = (y << 4) + 0;
    GLint D = (y << 4) + texmap_TILE_LENGTH;
    GLint E = (z << 4) + 0;
    GLint F = (z << 4) + texmap_TILE_LENGTH;
    
    //torch top (10/16 block over bottom)
    GLint H = C + 10;

    //Vertices of "parallelocube"
    GLint vX[8], vY[8], vZ[8];
    
    //A: 0, 2, 3, 1
    vX[0] = A; vX[1] = A; vX[2] = A; vX[3] = A;
    vY[0] = C; vY[1] = D; vY[2] = C; vY[3] = D;
    vZ[0] = E; vZ[1] = E; vZ[2] = F; vZ[3] = F;

    //B: 6, 4, 5, 7
    vX[6] = B; vX[4] = B; vX[5] = B; vX[7] = B;
    vY[6] = C; vY[4] = C; vY[5] = D; vY[7] = D;
    vZ[6] = F; vZ[4] = E; vZ[5] = E; vZ[7] = F;

    //Edge offsets for faces of torch
    GLint AC = 7, AD = 7, BC = -7, BD = -7, EC = 7, ED = 7, FC = -7, FD = -7;
    
    //Wall torch offsets
    GLint dY = 0, dXC = 0, dXD = 0, dZC = 0, dZD = 0;

    //Torch location depends on metadata
    switch(meta) {
        //Metadata to determine which wall torch is on
        case 1: //North side (-X)
            dXC = -8; dXD = -4; dY = 3;
            break;
        case 2: //South side (+X)
            dXC = 8; dXD = 4; dY = 3;
            break;
        case 3: //East side (-Z)
            dZC = -8; dZD = -4; dY = 3;
            break;
        case 4: //West side (+Z)
            dZC = 8; dZD = 4; dY = 3;
            break;
        default:    //Placed on floor
            break;
    }

    //Vertex order: Lower left, lower right, top right, top left
    //A side: 0, 2, 3, 1
    glTexCoord2f(tx_0,ty_0); glVertex3i( dXC+vX[0]+AC, vY[0]+dY, dZC+vZ[0]);
    glTexCoord2f(tx_1,ty_0); glVertex3i( dXC+vX[2]+AC, vY[2]+dY, dZC+vZ[2]);
    glTexCoord2f(tx_1,ty_1); glVertex3i( dXD+vX[3]+AD, vY[3]+dY, dZD+vZ[3]);
    glTexCoord2f(tx_0,ty_1); glVertex3i( dXD+vX[1]+AD, vY[1]+dY, dZD+vZ[1]);
    //B side: 6, 4, 5, 7
    glTexCoord2f(tx_0,ty_0); glVertex3i( dXC+vX[6]+BC, vY[6]+dY, dZC+vZ[6]);
    glTexCoord2f(tx_1,ty_0); glVertex3i( dXC+vX[4]+BC, vY[4]+dY, dZC+vZ[4]);
    glTexCoord2f(tx_1,ty_1); glVertex3i( dXD+vX[5]+BD, vY[5]+dY, dZD+vZ[5]);
    glTexCoord2f(tx_0,ty_1); glVertex3i( dXD+vX[7]+BD, vY[7]+dY, dZD+vZ[7]);
    //E side: 4, 0, 1, 5
    glTexCoord2f(tx_0,ty_0); glVertex3i( dXC+vX[4], vY[4]+dY, dZC+vZ[4]+EC);
    glTexCoord2f(tx_1,ty_0); glVertex3i( dXC+vX[0], vY[0]+dY, dZC+vZ[0]+EC);
    glTexCoord2f(tx_1,ty_1); glVertex3i( dXD+vX[1], vY[1]+dY, dZD+vZ[1]+ED);
    glTexCoord2f(tx_0,ty_1); glVertex3i( dXD+vX[5], vY[5]+dY, dZD+vZ[5]+ED);
    //F side: 2, 6, 7, 3
    glTexCoord2f(tx_0,ty_0); glVertex3i( dXC+vX[2], vY[2]+dY, dZC+vZ[2]+FC);
    glTexCoord2f(tx_1,ty_0); glVertex3i( dXC+vX[6], vY[6]+dY, dZC+vZ[6]+FC);
    glTexCoord2f(tx_1,ty_1); glVertex3i( dXD+vX[7], vY[7]+dY, dZD+vZ[7]+FD);
    glTexCoord2f(tx_0,ty_1); glVertex3i( dXD+vX[3], vY[3]+dY, dZD+vZ[3]+FD);
    
    //Bottom side (C): 0, 4, 6, 2
    glTexCoord2f(tx_m1,ty_b1); glVertex3i( dXC+vX[0]+AC, C+dY, dZC+vZ[0]+EC);
    glTexCoord2f(tx_m2,ty_b1); glVertex3i( dXC+vX[4]+BC, C+dY, dZC+vZ[4]+EC);
    glTexCoord2f(tx_m2,ty_b2); glVertex3i( dXC+vX[6]+BC, C+dY, dZC+vZ[6]+FC);
    glTexCoord2f(tx_m1,ty_b2); glVertex3i( dXC+vX[2]+AC, C+dY, dZC+vZ[2]+FC);
    
    //Calculate offsets of torch top (linear interpolation of XC<->XD, ZC<->ZD)
    GLfloat dX = (10*dXD + 6*dXC)/16.0;
    GLfloat dZ = (10*dZD + 6*dZC)/16.0;
    //Top side (D): 3, 7, 5, 1
    glTexCoord2f(tx_m1,ty_m1); glVertex3f( dX+vX[3]+AD, H+dY, dZ+vZ[3]+FD);
    glTexCoord2f(tx_m2,ty_m1); glVertex3f( dX+vX[7]+BD, H+dY, dZ+vZ[7]+FD);
    glTexCoord2f(tx_m2,ty_m2); glVertex3f( dX+vX[5]+BD, H+dY, dZ+vZ[5]+ED);
    glTexCoord2f(tx_m1,ty_m2); glVertex3f( dX+vX[1]+AD, H+dY, dZ+vZ[1]+ED);

}

//Draw fire burning (use item texture)
void BlockDrawer::drawFire( uint8_t blockID, uint8_t meta,
    GLint x, GLint y, GLint z, uint8_t vflags) const
{
    //Metadata indicates age of fire
    
    //Draw fire on each flammable face
    drawCube(blockID, meta, x, y, z, vflags);
}


//Draw dyed block (metadata block, offset by 256)
void BlockDrawer::drawDyed( uint8_t blockID, uint8_t meta,
    GLint x, GLint y, GLint z, uint8_t vflags) const
{
    //Change texture depending on meta
    uint16_t ID = 256 + blockID + meta;
    drawCubeMeta(ID, 0, x, y, z, vflags);
}


//Draw redstone wire (active or inactive)
void BlockDrawer::drawWire( uint8_t blockID, uint8_t meta,
    GLint x, GLint y, GLint z, uint8_t /*vflags*/) const
{
    //Compare to adjacent blocks
    uint8_t mask=0;
    if (world != NULL) {
        //A neighbor
        Block block = world->getBlock(x - 1, y, z);
        if (Blk::isLogic[block.blockID]) {
            mask |= 1;
        }
        //B neighbor
        block = world->getBlock(x + 1, y, z);
        if (Blk::isLogic[block.blockID]) {
            mask |= 2;
        }
        //E neighbor
        block = world->getBlock(x, y, z - 1);
        if (Blk::isLogic[block.blockID]) {
            mask |= 4;
        }
        //F neighbor
        block = world->getBlock(x, y, z + 1);
        if (Blk::isLogic[block.blockID]) {
            mask |= 8;
        }
    }
        
    //Mask for adjacent "y+1" blocks
    uint8_t up_mask=0;
    if (world != NULL) {
        //A neighbor
        Block block = world->getBlock(x - 1, y + 1, z);
        if (Blk::isLogic[block.blockID]) {
            up_mask |= 1;
            mask |= 1;
        }
        //B neighbor
        block = world->getBlock(x + 1, y + 1, z);
        if (Blk::isLogic[block.blockID]) {
            up_mask |= 2;
            mask |= 2;
        }
        //E neighbor
        block = world->getBlock(x, y + 1, z - 1);
        if (Blk::isLogic[block.blockID]) {
            up_mask |= 4;
            mask |= 4;
        }
        //F neighbor
        block = world->getBlock(x, y + 1, z + 1);
        if (Blk::isLogic[block.blockID]) {
            up_mask |= 8;
            mask |= 8;
        }
    }
    
    //If metadata > 0, wire is active
    uint8_t wireFace = (meta == 0 ? 0 : 2 );   //'+' shape, lit or unlit

    const GLfloat scale = 11.0/16.0;    //Ratio for truncated wires
    const GLfloat offset = 5.0/16.0;    //Offset for truncated wires
    GLfloat scale_x = 1.0, scale_y = 1.0, off_x=0.0, off_y=0.0;
    uint8_t rotate = 0;                 //Number of texture rotations
    
    //Wire drawing depends on mask
    switch (mask) {
        case 1:
        case 2:
        case 3:
            //'-' shape
            wireFace++;
            break;
        case 4:
        case 8:
        case 12:
            //'|' shape
            wireFace++;
            rotate = 1;
            break;
        case 5:  //Top-left angle
            scale_x = scale_y = scale;
            break;
        case 6: //Top-right angle
            off_x = offset;
            scale_x = scale_y = scale;
            break;
        case 7:  //Top T-shape
            scale_y = scale;
            break;
        case 9:  //Bottom-left angle
            off_y = offset;
            scale_x = scale_y = scale;
            break;
        case 10: //Bottom-right angle
            off_x = off_y = offset;
            scale_x = scale_y = scale;
            break;
        case 11: //Bottom T-shape
            off_y = offset;
            scale_y = scale;
            break;
        case 13: //Left T-shape
            scale_x = scale;
            break;
        case 14: //Right T-shape
            off_x = offset;
            scale_x = scale;
            break;
        case 0:
        case 15:
        default:
            break;  //Default '+' shape
    }
    
    //Texture map coordinates of terrain.png (0.0 - 1.0)
    GLfloat tx[5], ty[5];

    tx[0] = blockInfo[blockID].tx[wireFace] + off_x*tmr;
    tx[1] = tx[0] + scale_x*tmr;
    tx[2] = tx[1];
    tx[3] = tx[0];
    ty[0] = blockInfo[blockID].ty[wireFace] + (off_y + scale_y)*tmr;
    ty[1] = ty[0];
    ty[2] = blockInfo[blockID].ty[wireFace] + off_y*tmr;
    ty[3] = ty[2];
    
    //Rotate texture as many times as needed
    for (; rotate > 0; rotate--) {
        tx[4] = tx[0]; ty[4] = ty[0];
        tx[0] = tx[1]; ty[0] = ty[1];
        tx[1] = tx[2]; ty[1] = ty[2];
        tx[2] = tx[3]; ty[2] = ty[3];
        tx[3] = tx[4]; ty[3] = ty[4];
    }
    
    //Object boundaries... flat square 1 pixel off the ground
    GLint A = (x << 4) + (off_x * texmap_TILE_LENGTH);
    GLint B = A + (scale_x * texmap_TILE_LENGTH);
    GLint C = (y << 4) + 0;
    GLint D = (y << 4) + 1;
    GLint E = (z << 4) + (off_y * texmap_TILE_LENGTH);
    GLint F = E + (scale_y * texmap_TILE_LENGTH);

    //Set color, depending on state
    if (meta != 0) {
        glColor3ub( 255, 63, 63);
    } else {
        glColor3ub( 127, 0, 0);
    }

    //Top face (seen by player)
    glTexCoord2f(tx[0],ty[0]); glVertex3i( A, D, F);  //Lower left:  ADF
    glTexCoord2f(tx[1],ty[1]); glVertex3i( B, D, F);  //Lower right: BDF
    glTexCoord2f(tx[2],ty[2]); glVertex3i( B, D, E);  //Top right:   BDE
    glTexCoord2f(tx[3],ty[3]); glVertex3i( A, D, E);  //Top left:    ADE

    //Bottom face (only seen from below a glass floor)
    glTexCoord2f(tx[3],ty[3]); glVertex3i( A, C, E);  //Lower left:  ACE
    glTexCoord2f(tx[2],ty[2]); glVertex3i( B, C, E);  //Lower right: BCE
    glTexCoord2f(tx[1],ty[1]); glVertex3i( B, C, F);  //Top right:   BCF
    glTexCoord2f(tx[0],ty[0]); glVertex3i( A, C, F);  //Top left:    ACF


    //Vertical wires if adjacent "y+1" blocks are logic types
    wireFace=( meta == 0 ? 1 : 3);
    tx[0] = blockInfo[blockID].tx[wireFace];
    tx[1] = tx[0]; 
    tx[2] = tx[1] + tmr;
    tx[3] = tx[2];
    ty[0] = blockInfo[blockID].ty[wireFace];
    ty[1] = ty[0] + tmr;
    ty[2] = ty[1];
    ty[3] = ty[0];


    //Face coordinates (in pixels)
    A = (x << 4) + 0;
    B = (x << 4) + texmap_TILE_LENGTH;
    C = (y << 4) + 0;
    D = (y << 4) + texmap_TILE_LENGTH;
    E = (z << 4) + 0;
    F = (z << 4) + texmap_TILE_LENGTH;


    //A
    if (up_mask & 1) {
        //outer face
        glTexCoord2f(tx[0],ty[0]); glVertex3i( A, C, E);  //Lower left:  ACE
        glTexCoord2f(tx[1],ty[1]); glVertex3i( A, C, F);  //Lower right: ACF
        glTexCoord2f(tx[2],ty[2]); glVertex3i( A, D, F);  //Top right:   ADF
        glTexCoord2f(tx[3],ty[3]); glVertex3i( A, D, E);  //Top left:    ADE
        //inner face
        glTexCoord2f(tx[3],ty[3]); glVertex3i( A, D, E);  //Top left:    ADE
        glTexCoord2f(tx[2],ty[2]); glVertex3i( A, D, F);  //Top right:   ADF
        glTexCoord2f(tx[1],ty[1]); glVertex3i( A, C, F);  //Lower right: ACF
        glTexCoord2f(tx[0],ty[0]); glVertex3i( A, C, E);  //Lower left:  ACE
    }

    //B
    if (up_mask & 2) {
        //outer face
        glTexCoord2f(tx[0],ty[0]); glVertex3i( B, C, F);  //Lower left:  BCF
        glTexCoord2f(tx[1],ty[1]); glVertex3i( B, C, E);  //Lower right: BCE
        glTexCoord2f(tx[2],ty[2]); glVertex3i( B, D, E);  //Top right:   BDE
        glTexCoord2f(tx[3],ty[3]); glVertex3i( B, D, F);  //Top left:    BDF
        //inner face
        glTexCoord2f(tx[3],ty[3]); glVertex3i( B, D, F);  //Top left:    BDF
        glTexCoord2f(tx[2],ty[2]); glVertex3i( B, D, E);  //Top right:   BDE
        glTexCoord2f(tx[1],ty[1]); glVertex3i( B, C, E);  //Lower right: BCE
        glTexCoord2f(tx[0],ty[0]); glVertex3i( B, C, F);  //Lower left:  BCF
    }


    if (up_mask & 4) {
        //E (outer face)
        glTexCoord2f(tx[0],ty[0]); glVertex3i( B, C, E);  //Lower left:  BCE
        glTexCoord2f(tx[1],ty[1]); glVertex3i( A, C, E);  //Lower right: ACE
        glTexCoord2f(tx[2],ty[2]); glVertex3i( A, D, E);  //Top right:   ADE
        glTexCoord2f(tx[3],ty[3]); glVertex3i( B, D, E);  //Top left:    BDE
        //E (inner face)
        glTexCoord2f(tx[3],ty[3]); glVertex3i( B, D, E);  //Top left:    BDE
        glTexCoord2f(tx[2],ty[2]); glVertex3i( A, D, E);  //Top right:   ADE
        glTexCoord2f(tx[1],ty[1]); glVertex3i( A, C, E);  //Lower right: ACE
        glTexCoord2f(tx[0],ty[0]); glVertex3i( B, C, E);  //Lower left:  BCE
    }
    
    if (up_mask & 8) {
        //F (outer face)
        glTexCoord2f(tx[0],ty[0]); glVertex3i( A, C, F);  //Lower left:  ACF
        glTexCoord2f(tx[1],ty[1]); glVertex3i( B, C, F);  //Lower right: BCF
        glTexCoord2f(tx[2],ty[2]); glVertex3i( B, D, F);  //Top right:   BDF
        glTexCoord2f(tx[3],ty[3]); glVertex3i( A, D, F);  //Top left:    ADF
        //F (inner face)
        glTexCoord2f(tx[3],ty[3]); glVertex3i( A, D, F);  //Top left:    ADF
        glTexCoord2f(tx[2],ty[2]); glVertex3i( B, D, F);  //Top right:   BDF
        glTexCoord2f(tx[1],ty[1]); glVertex3i( B, C, F);  //Lower right: BCF
        glTexCoord2f(tx[0],ty[0]); glVertex3i( A, C, F);  //Lower left:  ACF
    }

    //Return color to normal
    glColor3ub( 255, 255, 255);

}

//Draw planted crops (meta affects texture)
void BlockDrawer::drawCrops( uint8_t blockID, uint8_t meta,
    GLint x, GLint y, GLint z, uint8_t /*vflags*/) const
{
    //Crop textures in terrain.png are 86 - 93
    uint8_t cropTexture = blockInfo[blockID].textureID[0] + (meta & 0x7);
    
    //Texture map coordinates in terrain.png (0.0 - 1.0)
    GLfloat tx_0 = float(cropTexture & (texmap_TILES-1))/((float)texmap_TILES);
    GLfloat tx_1 = tx_0 + tmr;
    GLfloat ty_1 = float(cropTexture/texmap_TILES)/((float)texmap_TILES);
    GLfloat ty_0 = ty_1 + tmr;

    //Object boundaries... 2 crossed squares inside a clear cube
    GLint A = (x << 4) + 0;
    GLint B = (x << 4) + texmap_TILE_LENGTH;
    GLint C = (y << 4) + 0;
    GLint D = (y << 4) + texmap_TILE_LENGTH;
    GLint E = (z << 4) + 0;
    GLint F = (z << 4) + texmap_TILE_LENGTH;
    GLint G = (z << 4) + (texmap_TILE_LENGTH/2);    //half-way through z 
    GLint H = (x << 4) + (texmap_TILE_LENGTH/2);    //half-way through x


    //Apply texture to planted item face
    glTexCoord2f(tx_0,ty_0); glVertex3i( A, C, G);  //Lower left:  ACG
    glTexCoord2f(tx_1,ty_0); glVertex3i( B, C, G);  //Lower right: BCG
    glTexCoord2f(tx_1,ty_1); glVertex3i( B, D, G);  //Top right:   BDG
    glTexCoord2f(tx_0,ty_1); glVertex3i( A, D, G);  //Top left:    ADG

    //Back face
    glTexCoord2f(tx_0,ty_0); glVertex3i( A, C, G);  //Lower left:  ACG
    glTexCoord2f(tx_0,ty_1); glVertex3i( A, D, G);  //Top left:    ADG
    glTexCoord2f(tx_1,ty_1); glVertex3i( B, D, G);  //Top right:   BDG
    glTexCoord2f(tx_1,ty_0); glVertex3i( B, C, G);  //Lower right: BCG

    //Intersecting plane
    glTexCoord2f(tx_0,ty_0); glVertex3i( H, C, F);  //Lower left:  HCF
    glTexCoord2f(tx_1,ty_0); glVertex3i( H, C, E);  //Lower right: HCE
    glTexCoord2f(tx_1,ty_1); glVertex3i( H, D, E);  //Top right:   HDE
    glTexCoord2f(tx_0,ty_1); glVertex3i( H, D, F);  //Top left:    HDF

    //Back face
    glTexCoord2f(tx_0,ty_0); glVertex3i( H, C, F);  //Lower left:  HCF
    glTexCoord2f(tx_0,ty_1); glVertex3i( H, D, F);  //Top left:    HDF
    glTexCoord2f(tx_1,ty_1); glVertex3i( H, D, E);  //Top right:   HDE
    glTexCoord2f(tx_1,ty_0); glVertex3i( H, C, E);  //Lower right: HCE
}

//Draw melon/pumpkin stem, height and color depends on metadata
void BlockDrawer::drawMelonStem( uint8_t blockID, uint8_t meta,
    GLint x, GLint y, GLint z, uint8_t /* vflags */) const
{

    //Drawing properties which depend on meta
    uint8_t melonFace = FACE_MAX;   //No adjacent melon
    uint8_t melonType = Blk::Melon;
    const uint8_t green=255;
    const uint8_t blue=63;
    const uint8_t red=63 + (meta*24);
    uint8_t height = (1+meta)*2;

    //Get terrain.png texture coords for drawing stem texture
    GLfloat tx_0, tx_1, ty_0, ty_1;

    //Object boundaries... 2 crossed squares inside a clear cube
    const GLint A = (x << 4) + 0;
    const GLint B = (x << 4) + texmap_TILE_LENGTH;
    const GLint C = (y << 4) + 0;
    const GLint D = (y << 4) + height;
    const GLint E = (z << 4) + 0;
    const GLint F = (z << 4) + texmap_TILE_LENGTH;
    const GLint G = (z << 4) + (texmap_TILE_LENGTH/2);    //half-way through z 
    const GLint H = (x << 4) + (texmap_TILE_LENGTH/2);    //half-way through x
    GLint I = D;

    //Use modified color for stem
    glColor3ub( red, green, blue);

    //Look for adjacent melon type, to curve if needed
    if (meta == 0x7 && world != NULL) {
        switch ( blockID ) {
            case Blk::PumpkinStem:
                melonType = Blk::Pumpkin; break;
            case Blk::MelonStem:
                melonType = Blk::Melon; break;
            default:
                melonType = Blk::Melon; break;
        }
        
        //Set melonFace to point to adjacent melon
        GLint left, right, back, front;
        if (world->getBlock(x - 1, y, z).blockID == melonType) {
            melonFace = LEFT;   //A
            left = A;
            right = B;
            back = front = G;
        } else if (world->getBlock(x + 1, y, z).blockID == melonType) {
            melonFace = RIGHT;  //B
            left = B;
            right = A;
            back = front = G;
        } else if (world->getBlock(x, y, z - 1).blockID == melonType) {
            melonFace = BACK;   //C
            left = right = H;
            back = F;
            front = E;
        } else if (world->getBlock(x, y, z + 1).blockID == melonType) {
            melonFace = FRONT;  //D
            left = right = H;
            back = E;
            front = F;
        } else {
            melonFace = FACE_MAX;
        }
        
        //Draw a bent stem towards the adjacent melon
        if (melonFace != FACE_MAX) {
            //Use bent stem texture
            getTexCoords( blockID, FRONT, tx_0, tx_1, ty_0, ty_1);
            
            //Front face
            glTexCoord2f(tx_0,ty_0); glVertex3i( left, C, front);//Low left: HCF
            glTexCoord2f(tx_1,ty_0); glVertex3i( right, C, back);//Low right:HCE
            glTexCoord2f(tx_1,ty_1); glVertex3i( right, D, back);//Top right:HDE
            glTexCoord2f(tx_0,ty_1); glVertex3i( left, D, front);//Top left: HDF
        
            //Back face
            glTexCoord2f(tx_0,ty_0); glVertex3i( left, C, front);//Low left: HCF
            glTexCoord2f(tx_0,ty_1); glVertex3i( left, D, front);//Top left: HDF
            glTexCoord2f(tx_1,ty_1); glVertex3i( right, D, back);//Top right:HDE
            glTexCoord2f(tx_1,ty_0); glVertex3i( right, C, back);//Low right:HCE
            
            //Set height to half, for the straight stems drawn after
            height = texmap_TILE_LENGTH/2;
            I = (y << 4) + height;  //Instead of D
        }
    }

    //Draw straight stems
    getTexCoords( blockID, BACK, tx_0, tx_1, ty_0, ty_1);
    //Adjust bottom texture coord to (TOP - height)
    ty_0 = ty_1 + tmr*( height/TILE_LENGTH);

    //Apply texture to planted item face
    glTexCoord2f(tx_0,ty_0); glVertex3i( A, C, F);  //Lower left:  
    glTexCoord2f(tx_1,ty_0); glVertex3i( B, C, E);  //Lower right: 
    glTexCoord2f(tx_1,ty_1); glVertex3i( B, I, E);  //Top right:   
    glTexCoord2f(tx_0,ty_1); glVertex3i( A, I, F);  //Top left:    

    //Back face
    glTexCoord2f(tx_0,ty_0); glVertex3i( A, C, F);  //Lower left:  
    glTexCoord2f(tx_0,ty_1); glVertex3i( A, I, F);  //Top left:    
    glTexCoord2f(tx_1,ty_1); glVertex3i( B, I, E);  //Top right:   
    glTexCoord2f(tx_1,ty_0); glVertex3i( B, C, E);  //Lower right: 

    //Intersecting plane
    glTexCoord2f(tx_0,ty_0); glVertex3i( B, C, F);  //Lower left:  
    glTexCoord2f(tx_1,ty_0); glVertex3i( A, C, E);  //Lower right: 
    glTexCoord2f(tx_1,ty_1); glVertex3i( A, I, E);  //Top right:   
    glTexCoord2f(tx_0,ty_1); glVertex3i( B, I, F);  //Top left:    

    //Back face
    glTexCoord2f(tx_0,ty_0); glVertex3i( B, C, F);  //Lower left:  
    glTexCoord2f(tx_0,ty_1); glVertex3i( B, I, F);  //Top left:    
    glTexCoord2f(tx_1,ty_1); glVertex3i( A, I, E);  //Top right:   
    glTexCoord2f(tx_1,ty_0); glVertex3i( A, C, E);  //Lower right: 

    //Resume normal color drawing
    glColor3ub( 255, 255, 255);

}

//Draw part of door (meta affects top/bottom, side of block)
void BlockDrawer::drawDoor( uint8_t blockID, uint8_t meta,
    GLint x, GLint y, GLint z, uint8_t vflags) const
{
    //Change texture and wall depending on meta
    uint16_t ID = 256 + blockID;
    
    //Check top bit
    if (meta & 0x8) {
        ID++;
        vflags |= 0x20; //don't draw bottom face
    } else {
        vflags |= 0x10; //don't draw top face
    }
    
    //Door position and side depends on metadata
    switch (meta & 0x7) {
        case 0:     //Closed (hinge NE, door on A face)
            drawScaledBlock( ID, meta, x, y, z, vflags&0xBC,
                3.0/16.0, 1.0, 1.0, true, 0, 0, 0, 0xC0);   //mirror A+B
            break;
        case 1:     //Closed (hinge SE, door on E face)
            drawScaledBlock( ID, meta, x, y, z, vflags&0xF8,
                1.0, 1.0, 3.0/16.0, true, 0, 0, 0, 0x00);
            break;
        case 2:     //Closed (hinge SW , door on B face)
            drawScaledBlock( ID, meta, x, y, z, vflags&0x7C,
                3.0/16.0, 1.0, 1.0, true, 13, 0, 0, 0x00);
            break;
        case 3:     //Closed (hinge NW, door on F face)
            drawScaledBlock( ID, meta, x, y, z, vflags&0xF4,
                1.0, 1.0, 3.0/16.0, true, 0, 0, 13, 0x0C);   //mirror E+F
            break;
        case 4:     //Open (hinge NE, door on E face)
            drawScaledBlock( ID, meta, x, y, z, vflags&0xF8,
                1.0, 1.0, 3.0/16.0, true, 0, 0, 0, 0x0C);   //mirror E+F
            break;
        case 5:     //Open (hinge SE, door on B face)
            drawScaledBlock( ID, meta, x, y, z, vflags&0x7C,
                3.0/16.0, 1.0, 1.0, true, 13, 0, 0, 0xC0);   //mirror A+B
            break;
        case 6:     //Open (hinge SW, door on F face)
            drawScaledBlock( ID, meta, x, y, z, vflags&0xF4,
                1.0, 1.0, 3.0/16.0, true, 0, 0, 13, 0x00);
            break;
        case 7:     //Open (hinge NW, door on A face)
            drawScaledBlock( ID, meta, x, y, z, vflags&0xBC,
                3.0/16.0, 1.0, 1.0, true, 0, 0, 0, 0x00);
            break;
        default:
            break;
    }

}


//Draw stairs (meta affects orientation)
void BlockDrawer::drawStairs( uint8_t blockID, uint8_t meta,
    GLint x, GLint y, GLint z, uint8_t vflags) const
{
    //Top step: depends on meta
    switch (meta & 0x3) {
        case 0: //Ascend south
            drawScaledBlock(blockID, meta, x, y, z, vflags,
                0.5, 0.5, 1.0, true, 8, 8, 0);
            break;
        case 1: //Ascend north
            drawScaledBlock(blockID, meta, x, y, z, vflags,
                0.5, 0.5, 1.0, true, 0, 8, 0);
            break;
        case 2: //Ascend west
            drawScaledBlock(blockID, meta, x, y, z, vflags,
                1, 0.5, 0.5, true, 0, 8, 8);
            break;
        case 3:
            drawScaledBlock(blockID, meta, x, y, z, vflags,
                1, 0.5, 0.5, true, 0, 8, 0);
            break;
    }
    
    //Bottom step
    drawSlab( blockID, 0/*meta, affects material*/, x, y, z, vflags);

}

//Draw floor or wall lever (meta affects position)
void BlockDrawer::drawLever( uint8_t blockID, uint8_t /*TODO: meta*/,
    GLint x, GLint y, GLint z, uint8_t /*vflags*/) const
{
    //TODO: handle angle and base location depend on meta
    
    //Handle
    drawItem( blockID, 0, x, y, z);
    
    //Cobblestone base
    drawScaledBlock(4, 0, x, y, z, 0,
        0.25, 0.25, 0.5, true, 6, 0, 4);

}


//Draw torch on ground, offset by small amount from center (used for diode)
void BlockDrawer::drawTorchOffset( uint8_t blockID, GLint x, GLint y, GLint z,
    GLint off_x, GLint off_y, GLint off_z) const
{

    //Look up texture coordinates for the item
    GLfloat tx_0 = blockInfo[blockID].tx[LEFT];
    GLfloat tx_1 = blockInfo[blockID].tx[LEFT] + tmr;
    GLfloat ty_0 = blockInfo[blockID].ty[LEFT] + tmr;    //flip y
    GLfloat ty_1 = blockInfo[blockID].ty[LEFT];
    
    //pieces of torch texture (used for top and bottom)
    GLfloat tx_m1 = blockInfo[blockID].tx[LEFT] + tmr*7.0/16.0;
    GLfloat tx_m2 = blockInfo[blockID].tx[LEFT] + tmr*9.0/16.0;
    GLfloat ty_m1 = blockInfo[blockID].ty[LEFT] + tmr*8.0/16.0;
    GLfloat ty_m2 = blockInfo[blockID].ty[LEFT] + tmr*6.0/16.0;
    GLfloat ty_b1 = blockInfo[blockID].ty[LEFT] + tmr*14.0/16.0;
    GLfloat ty_b2 = blockInfo[blockID].ty[LEFT] + tmr;

    //Cube boundaries
    GLint A = (x << 4) + 0;
    GLint B = (x << 4) + texmap_TILE_LENGTH;
    GLint C = (y << 4) + 0;
    GLint D = (y << 4) + texmap_TILE_LENGTH;
    GLint E = (z << 4) + 0;
    GLint F = (z << 4) + texmap_TILE_LENGTH;
    
    //torch top (10/16 block over bottom)
    GLint H = C + 10;

    //Vertices of "parallelocube"
    GLint vX[8], vY[8], vZ[8];
    
    //A: 0, 2, 3, 1
    vX[0] = A; vX[1] = A; vX[2] = A; vX[3] = A;
    vY[0] = C; vY[1] = D; vY[2] = C; vY[3] = D;
    vZ[0] = E; vZ[1] = E; vZ[2] = F; vZ[3] = F;

    //B: 6, 4, 5, 7
    vX[6] = B; vX[4] = B; vX[5] = B; vX[7] = B;
    vY[6] = C; vY[4] = C; vY[5] = D; vY[7] = D;
    vZ[6] = F; vZ[4] = E; vZ[5] = E; vZ[7] = F;

    //Edge offsets for faces of torch
    GLint AC = 7, AD = 7, BC = -7, BD = -7, EC = 7, ED = 7, FC = -7, FD = -7;
    
    //Torch offsets
    GLint dY = off_y, dXC = off_x, dXD = off_x, dZC = off_z, dZD = off_z;

    //Vertex order: Lower left, lower right, top right, top left
    //A side: 0, 2, 3, 1
    glTexCoord2f(tx_0,ty_0); glVertex3i( dXC+vX[0]+AC, vY[0]+dY, dZC+vZ[0]);
    glTexCoord2f(tx_1,ty_0); glVertex3i( dXC+vX[2]+AC, vY[2]+dY, dZC+vZ[2]);
    glTexCoord2f(tx_1,ty_1); glVertex3i( dXD+vX[3]+AD, vY[3]+dY, dZD+vZ[3]);
    glTexCoord2f(tx_0,ty_1); glVertex3i( dXD+vX[1]+AD, vY[1]+dY, dZD+vZ[1]);
    //B side: 6, 4, 5, 7
    glTexCoord2f(tx_0,ty_0); glVertex3i( dXC+vX[6]+BC, vY[6]+dY, dZC+vZ[6]);
    glTexCoord2f(tx_1,ty_0); glVertex3i( dXC+vX[4]+BC, vY[4]+dY, dZC+vZ[4]);
    glTexCoord2f(tx_1,ty_1); glVertex3i( dXD+vX[5]+BD, vY[5]+dY, dZD+vZ[5]);
    glTexCoord2f(tx_0,ty_1); glVertex3i( dXD+vX[7]+BD, vY[7]+dY, dZD+vZ[7]);
    //E side: 4, 0, 1, 5
    glTexCoord2f(tx_0,ty_0); glVertex3i( dXC+vX[4], vY[4]+dY, dZC+vZ[4]+EC);
    glTexCoord2f(tx_1,ty_0); glVertex3i( dXC+vX[0], vY[0]+dY, dZC+vZ[0]+EC);
    glTexCoord2f(tx_1,ty_1); glVertex3i( dXD+vX[1], vY[1]+dY, dZD+vZ[1]+ED);
    glTexCoord2f(tx_0,ty_1); glVertex3i( dXD+vX[5], vY[5]+dY, dZD+vZ[5]+ED);
    //F side: 2, 6, 7, 3
    glTexCoord2f(tx_0,ty_0); glVertex3i( dXC+vX[2], vY[2]+dY, dZC+vZ[2]+FC);
    glTexCoord2f(tx_1,ty_0); glVertex3i( dXC+vX[6], vY[6]+dY, dZC+vZ[6]+FC);
    glTexCoord2f(tx_1,ty_1); glVertex3i( dXD+vX[7], vY[7]+dY, dZD+vZ[7]+FD);
    glTexCoord2f(tx_0,ty_1); glVertex3i( dXD+vX[3], vY[3]+dY, dZD+vZ[3]+FD);
    
    //Bottom side (C): 0, 4, 6, 2
    glTexCoord2f(tx_m1,ty_b1); glVertex3i( dXC+vX[0]+AC, C+dY, dZC+vZ[0]+EC);
    glTexCoord2f(tx_m2,ty_b1); glVertex3i( dXC+vX[4]+BC, C+dY, dZC+vZ[4]+EC);
    glTexCoord2f(tx_m2,ty_b2); glVertex3i( dXC+vX[6]+BC, C+dY, dZC+vZ[6]+FC);
    glTexCoord2f(tx_m1,ty_b2); glVertex3i( dXC+vX[2]+AC, C+dY, dZC+vZ[2]+FC);
    
    //Calculate offsets of torch top (linear interpolation of XC<->XD, ZC<->ZD)
    GLfloat dX = (10*dXD + 6*dXC)/16.0;
    GLfloat dZ = (10*dZD + 6*dZC)/16.0;
    //Top side (D): 3, 7, 5, 1
    glTexCoord2f(tx_m1,ty_m1); glVertex3f( dX+vX[3]+AD, H+dY, dZ+vZ[3]+FD);
    glTexCoord2f(tx_m2,ty_m1); glVertex3f( dX+vX[7]+BD, H+dY, dZ+vZ[7]+FD);
    glTexCoord2f(tx_m2,ty_m2); glVertex3f( dX+vX[5]+BD, H+dY, dZ+vZ[5]+ED);
    glTexCoord2f(tx_m1,ty_m2); glVertex3f( dX+vX[1]+AD, H+dY, dZ+vZ[1]+ED);

}


//Draw diode block with torches (meta affects configuration)
void BlockDrawer::drawDiode( uint8_t blockID, uint8_t meta,
    GLint x, GLint y, GLint z, uint8_t vflags) const
{
    //Redstone torches depending on blockID
    uint8_t torchID = Blk::RedTorch;
    if (blockID == Blk::DiodeOn) {
        torchID = Blk::RedTorchOn;
    }
    
    //Determine delay (affects torch position)
    GLint off_x_1=0, off_z_1=0, off_x_2=0, off_z_2=0, delay_offset=1;
    delay_offset = 1 - ((meta & 0xC) >> 1);

    //Determine direction.  Needed for top texture
    face_ID facing;
    switch (meta & 0x03) {
        default:
        case 0: facing = FRONT; off_z_1=-5; off_z_2=-delay_offset;
            break;
        case 1: facing = LEFT;  off_x_1=5; off_x_2=delay_offset;
            break;
        case 2: facing = BACK;  off_z_1=5; off_z_2=delay_offset;
            break;
        case 3: facing = RIGHT; off_x_1=-5; off_x_2=-delay_offset;
            break;
    }

    //Draw the redstone torches    
    drawTorchOffset( torchID, x, y, z, off_x_1, 0, off_z_1);
    drawTorchOffset( torchID, x, y, z, off_x_2, 0, off_z_2);
    
    //Create vertices for diode base
    GLint vX[8], vY[8], vZ[8];
    makeCuboidVertex(x, y, z, 16, 2, 16, vX, vY, vZ, facing);
    
    //Draw diode base
    drawVertexBlock( vX, vY, vZ, blockInfo[blockID].tx,
        blockInfo[blockID].tx_1, blockInfo[blockID].ty,
        blockInfo[blockID].ty_1, vflags&0x20, facing);

}


//Draw signpost (meta affects angle).  Don't use blockID, it has another tex.
void BlockDrawer::drawSignpost( uint8_t /*blockID*/, uint8_t meta,
    GLint x, GLint y, GLint z, uint8_t vflags) const
{

    //Use sign.png
    bindTexture(TEX_SIGN);
    
    //Look up texture coordinates for signboard
    GLfloat tx0[6], tx1[6], ty0[6], ty1[6];
    uint16_t index;
    for (index = 0; index < 6; index++) {
        
        //Get texture coordinates from TextureInfo
        TextureInfo *pti = texInfo[texture_INDEX[TEX_SIGN] + index];
        if (pti != NULL) { pti->getCoords(
            tx0[index], tx1[index], ty0[index], ty1[index]);
        }
    }

    //Cube boundaries
    GLint A = (x << 4) + 0;
    GLint B = (x << 4) + texmap_TILE_LENGTH;
    GLint C = (y << 4) + 0;
    GLint D = (y << 4) + texmap_TILE_LENGTH;
    GLint E = (z << 4) + 0;
    GLint F = (z << 4) + texmap_TILE_LENGTH;
    
    //signboard bottom
    GLint G = C + 9;
    
    //signboard top (D + 2)
    GLint H = D + 2;
    
    //Vertices of signboard
    GLint vX[8], vY[8], vZ[8];
    
    //Y values of vertices always the same
    vY[0] = G  ; vY[2] = G  ; vY[3] = H  ; vY[1] = H  ;
    vY[6] = G  ; vY[4] = G  ; vY[5] = H  ; vY[7] = H  ;

    //Sign orientation depends on metadata
    switch(meta) {
        case 0: //West
            //facing sign, left side: 0, 2, 3, 1
            vX[0] = B  ; vX[2] = B  ; vX[3] = B  ; vX[1] = B;
            vZ[0] = E+9; vZ[2] = E+7; vZ[3] = E+7; vZ[1] = E+9;
            
            //facing sign, right side: 6, 4, 5, 7
            vX[6] = A  ; vX[4] = A  ; vX[5] = A  ; vX[7] = A  ; 
            vZ[6] = E+7; vZ[4] = E+9; vZ[5] = E+9; vZ[7] = E+7;
            break;
        case 1: //West-Northwest
            //facing sign, left side: 0, 2, 3, 1
            vX[0] = B-1; vX[2] = B  ; vX[3] = B  ; vX[1] = B-1;
            vZ[0] = F-4; vZ[2] = F-5; vZ[3] = F-5; vZ[1] = F-4;
            
            //facing sign, right side: 6, 4, 5, 7
            vX[6] = A+1; vX[4] = A  ; vX[5] = A  ; vX[7] = A+1; 
            vZ[6] = E+4; vZ[4] = E+5; vZ[5] = E+5; vZ[7] = E+4;
            break;
        case 2: //Northwest
            //facing sign, left side: 0, 2, 3, 1
            vX[0] = B-2; vX[2] = B-1; vX[3] = B-1; vX[1] = B-2;
            vZ[0] = F-1; vZ[2] = F-2; vZ[3] = F-2; vZ[1] = F-1;

            //facing sign, right side: 6, 4, 5, 7
            vX[6] = A+2; vX[4] = A+1; vX[5] = A+1; vX[7] = A+2;
            vZ[6] = E+1; vZ[4] = E+2; vZ[5] = E+2; vZ[7] = E+1;
            break;
        case 3: //North-Northwest
            //facing sign, left side: 0, 2, 3, 1
            vX[0] = B-5; vX[2] = B-4; vX[3] = B-4; vX[1] = B-5;
            vZ[0] = F  ; vZ[2] = F-1; vZ[3] = F-1; vZ[1] = F  ;
            
            //facing sign, right side: 6, 4, 5, 7
            vX[6] = A+5; vX[4] = A+4; vX[5] = A+4; vX[7] = A+5;
            vZ[6] = E  ; vZ[4] = E+1; vZ[5] = E+1; vZ[7] = E  ;
            break;
        default:
        case 4: //North
            //facing sign, left side: 0, 2, 3, 1
            vX[0] = A+7; vX[2] = A+9; vX[3] = A+9; vX[1] = A+7;
            vZ[0] = F  ; vZ[2] = F  ; vZ[3] = F  ; vZ[1] = F  ;

            //facing sign, right side: 6, 4, 5, 7
            vX[6] = A+9; vX[4] = A+7; vX[5] = A+7; vX[7] = A+9;
            vZ[6] = E  ; vZ[4] = E  ; vZ[5] = E  ; vZ[7] = E  ;
            break;
        case 5: //North-Northeast
            //facing sign, left side: 0, 2, 3, 1
            vX[0] = A+4; vX[2] = A+5; vX[3] = A+5; vX[1] = A+4;
            vZ[0] = F-1; vZ[2] = F  ; vZ[3] = F  ; vZ[1] = F-1;
            
            //facing sign, right side: 6, 4, 5, 7
            vX[6] = B-4; vX[4] = B-5; vX[5] = B-5; vX[7] = B-4;
            vZ[6] = E+1; vZ[4] = E  ; vZ[5] = E  ; vZ[7] = E+1;
            break;
        case 6: //Northeast
            //facing sign, left side: 0, 2, 3, 1
            vX[0] = A+1; vX[2] = A+2; vX[3] = A+2; vX[1] = A+1; 
            vZ[0] = F-2; vZ[2] = F-1; vZ[3] = F-1; vZ[1] = F-2;
        
            //facing sign, right side: 6, 4, 5, 7
            vX[6] = B-1; vX[4] = B-2; vX[5] = B-2; vX[7] = B-1;
            vZ[6] = E+2; vZ[4] = E+1; vZ[5] = E+1; vZ[7] = E+2;
            break;
        case 7: //East-Northeast
            //facing sign, left side: 0, 2, 3, 1
            vX[0] = A  ; vX[2] = A+1; vX[3] = A+1; vX[1] = A  ; 
            vZ[0] = F-5; vZ[2] = F-4; vZ[3] = F-4; vZ[1] = F-5;
            
            //facing sign, right side: 6, 4, 5, 7
            vX[6] = B  ; vX[4] = B-1; vX[5] = B-1; vX[7] = B  ;
            vZ[6] = E+5; vZ[4] = E+4; vZ[5] = E+4; vZ[7] = E+5;
            break;
        case 8: //East
            //facing sign, left side: 0, 2, 3, 1
            vX[0] = A; vX[2] = A; vX[3] = A; vX[1] = A; 
            vZ[0] = E+7; vZ[2] = E+9; vZ[3] = E+9; vZ[1] = E+7;
        
            //facing sign, right side: 6, 4, 5, 7
            vX[6] = B  ; vX[4] = B  ; vX[5] = B  ; vX[7] = B  ;
            vZ[6] = F-7; vZ[4] = F-9; vZ[5] = F-9; vZ[7] = F-7;
            break;
        case 9: //East-Southeast
            //facing sign, left side: 0, 2, 3, 1
            vX[0] = A+1; vX[2] = A  ; vX[3] = A  ; vX[1] = A+1; 
            vZ[0] = E+4; vZ[2] = E+5; vZ[3] = E+5; vZ[1] = E+4;
            
            //facing sign, right side: 6, 4, 5, 7
            vX[6] = B-1; vX[4] = B  ; vX[5] = B  ; vX[7] = B-1;
            vZ[6] = F-4; vZ[4] = F-5; vZ[5] = F-5; vZ[7] = F-4;
            break;
        case 10: //Southeast
            //facing sign, left side: 0, 2, 3, 1
            vX[0] = A+2; vX[2] = A+1; vX[3] = A+1; vX[1] = A+2;
            vZ[0] = E+1; vZ[2] = E+2; vZ[3] = E+2; vZ[1] = E+1;

            //facing sign, right side: 6, 4, 5, 7
            vX[6] = B-2; vX[4] = B-1; vX[5] = B-1; vX[7] = B-2;
            vZ[6] = F-1; vZ[4] = F-2; vZ[5] = F-2; vZ[7] = F-1;
            break;
        case 11: //South-Southeast
            //facing sign, left side: 0, 2, 3, 1
            vX[0] = A+5; vX[2] = A+4; vX[3] = A+4; vX[1] = A+5;
            vZ[0] = E  ; vZ[2] = E+1; vZ[3] = E+1; vZ[1] = E  ;
            
            //facing sign, right side: 6, 4, 5, 7
            vX[6] = B-5; vX[4] = B-4; vX[5] = B-4; vX[7] = B-5;
            vZ[6] = F  ; vZ[4] = F-1; vZ[5] = F-1; vZ[7] = F  ;
            break;
        case 12: //South
            //facing sign, left side: 0, 2, 3, 1
            vX[0] = A+9; vX[2] = A+7; vX[3] = A+7; vX[1] = A+9;
            vZ[0] = E  ; vZ[2] = E  ; vZ[3] = E  ; vZ[1] = E  ;

            //facing sign, right side: 6, 4, 5, 7
            vX[6] = A+7; vX[4] = A+9; vX[5] = A+9; vX[7] = A+7;
            vZ[6] = F  ; vZ[4] = F  ; vZ[5] = F  ; vZ[7] = F  ;
            break;
        case 13: //South-Southwest
            //facing sign, left side: 0, 2, 3, 1
            vX[0] = B-4; vX[2] = B-5; vX[3] = B-5; vX[1] = B-4;
            vZ[0] = E+1; vZ[2] = E  ; vZ[3] = E  ; vZ[1] = E+1;
            
            //facing sign, right side: 6, 4, 5, 7
            vX[6] = A+4; vX[4] = A+5; vX[5] = A+5; vX[7] = A+4; 
            vZ[6] = F-1; vZ[4] = F  ; vZ[5] = F  ; vZ[7] = F-1;
            break;
        case 14: //Southwest
            //facing sign, left side: 0, 2, 3, 1
            vX[0] = B-1; vX[2] = B-2; vX[3] = B-2; vX[1] = B-1;
            vZ[0] = E+2; vZ[2] = E+1; vZ[3] = E+1; vZ[1] = E+2;
        
            //facing sign, right side: 6, 4, 5, 7
            vX[6] = A+1; vX[4] = A+2; vX[5] = A+2; vX[7] = A+1; 
            vZ[6] = F-2; vZ[4] = F-1; vZ[5] = F-1; vZ[7] = F-2;
            break;
        case 15: //West-Southwest
            //facing sign, left side: 0, 2, 3, 1
            vX[0] = B  ; vX[2] = B-1; vX[3] = B-1; vX[1] = B  ;
            vZ[0] = E+5; vZ[2] = E+4; vZ[3] = E+4; vZ[1] = E+5;
            
            //facing sign, right side: 6, 4, 5, 7
            vX[6] = A  ; vX[4] = A+1; vX[5] = A+1; vX[7] = A  ; 
            vZ[6] = F-5; vZ[4] = F-4; vZ[5] = F-4; vZ[7] = F-5;
            break;
    }

    //I want 6 faces, pronto!
    drawVertexBlock(vX, vY, vZ, tx0, tx1, ty0, ty1, vflags&0x10);
    
    //
    //Draw the signpost
    //
    
    //Set signpost texture boundaries
    for (index = 0; index < 6; index++) {
        
        //Get texture coordinates from TextureInfo
        TextureInfo *pti = texInfo[texture_INDEX[TEX_SIGN] + 6 + index];
        if (pti != NULL) { pti->getCoords(
            tx0[index], tx1[index], ty0[index], ty1[index]);
        }
    }
    
    //Y values of vertices always the same
    vY[0] = C  ; vY[2] = C  ; vY[3] = G  ; vY[1] = G  ;
    vY[6] = C  ; vY[4] = C  ; vY[5] = G  ; vY[7] = G  ;

    //left side: 0, 2, 3, 1
    vX[0] = A+7; vX[2] = A+8; vX[3] = A+8; vX[1] = A+7;
    vZ[0] = F-8; vZ[2] = F-7; vZ[3] = F-7; vZ[1] = F-8;
    
    //right side: 6, 4, 5, 7
    vX[6] = B-7; vX[4] = B-8; vX[5] = B-8; vX[7] = B-7; 
    vZ[6] = E+8; vZ[4] = E+7; vZ[5] = E+7; vZ[7] = E+8;

    //Signpost from vertices, don't draw bottom side
    drawVertexBlock(vX, vY, vZ, tx0, tx1, ty0, ty1, 0x20 );

    //Switch back to terrain texture
    bindTexture(TEX_TERRAIN);

}

//Draw a 6-sided volume with specified vertices and tex coords
void BlockDrawer::drawVertexBlock( GLint vX[8], GLint vY[8], GLint vZ[8],
    const GLfloat tx_0[6], const GLfloat tx_1[6],
    const GLfloat ty_0[6], const GLfloat ty_1[6],
    uint8_t vflags, face_ID fid) const
{
    //Adjust vflags to consider facing
    switch (fid) {
        case LEFT: vflags = (vflags&0x80)>>5|(vflags&0x40)>>3|(vflags&0x0C)<<4|
            (vflags&0x33); break;
        case RIGHT:vflags = (vflags&0xC0)>>4|(vflags&0x08)<<3|(vflags&0x04)<<5|
            (vflags&0x33); break;
        case BACK:vflags = (vflags&0x88)>>1 | (vflags&0x44)<<1 | (vflags&0x33);
                break;
        case FRONT:
        default:
            break;  //No change
    }
  
    //Vertex order: Lower left, lower right, top right, top left
    //A side: 0, 2, 3, 1
    if (! (vflags&0x80)) {
        glTexCoord2f(tx_0[0],ty_1[0]); glVertex3i( vX[0], vY[0], vZ[0]);
        glTexCoord2f(tx_1[0],ty_1[0]); glVertex3i( vX[2], vY[2], vZ[2]);
        glTexCoord2f(tx_1[0],ty_0[0]); glVertex3i( vX[3], vY[3], vZ[3]);
        glTexCoord2f(tx_0[0],ty_0[0]); glVertex3i( vX[1], vY[1], vZ[1]);
    }
    
    //B side: 6, 4, 5, 7
    if (! (vflags&0x40)) {
        glTexCoord2f(tx_0[1],ty_1[1]); glVertex3i( vX[6], vY[6], vZ[6]);
        glTexCoord2f(tx_1[1],ty_1[1]); glVertex3i( vX[4], vY[4], vZ[4]);
        glTexCoord2f(tx_1[1],ty_0[1]); glVertex3i( vX[5], vY[5], vZ[5]);
        glTexCoord2f(tx_0[1],ty_0[1]); glVertex3i( vX[7], vY[7], vZ[7]);
    }

    //Bottom side (C): 0, 4, 6, 2
    if (! (vflags&0x20)) {
        glTexCoord2f(tx_0[2],ty_1[2]); glVertex3i( vX[0], vY[0], vZ[0]);
        glTexCoord2f(tx_1[2],ty_1[2]); glVertex3i( vX[4], vY[4], vZ[4]);
        glTexCoord2f(tx_1[2],ty_0[2]); glVertex3i( vX[6], vY[6], vZ[6]);
        glTexCoord2f(tx_0[2],ty_0[2]); glVertex3i( vX[2], vY[2], vZ[2]);
    }
    
    //Top side (D): 3, 7, 5, 1
    if (! (vflags&0x10)) {
        glTexCoord2f(tx_0[3],ty_1[3]); glVertex3f( vX[3], vY[3], vZ[3]);
        glTexCoord2f(tx_1[3],ty_1[3]); glVertex3f( vX[7], vY[7], vZ[7]);
        glTexCoord2f(tx_1[3],ty_0[3]); glVertex3f( vX[5], vY[5], vZ[5]);
        glTexCoord2f(tx_0[3],ty_0[3]); glVertex3f( vX[1], vY[1], vZ[1]);
    }
    
    //E side: 4, 0, 1, 5
    if (! (vflags&0x08)) {
        glTexCoord2f(tx_0[4],ty_1[4]); glVertex3i( vX[4], vY[4], vZ[4]);
        glTexCoord2f(tx_1[4],ty_1[4]); glVertex3i( vX[0], vY[0], vZ[0]);
        glTexCoord2f(tx_1[4],ty_0[4]); glVertex3i( vX[1], vY[1], vZ[1]);
        glTexCoord2f(tx_0[4],ty_0[4]); glVertex3i( vX[5], vY[5], vZ[5]);
    }
    
    //F side: 2, 6, 7, 3
    if (! (vflags&0x04)) {
        glTexCoord2f(tx_0[5],ty_1[5]); glVertex3i( vX[2], vY[2], vZ[2]);
        glTexCoord2f(tx_1[5],ty_1[5]); glVertex3i( vX[6], vY[6], vZ[6]);
        glTexCoord2f(tx_1[5],ty_0[5]); glVertex3i( vX[7], vY[7], vZ[7]);
        glTexCoord2f(tx_0[5],ty_0[5]); glVertex3i( vX[3], vY[3], vZ[3]);

/*
    //Debug coordinates
    cout << "Front tex coords: (" << tx_0[FRONT]*16 << "," << ty_0[FRONT]*16
         << ") (" << tx_1[FRONT]*16 << "," << ty_1[FRONT]*16 << ")" << endl;
*/
    }
    
}


//Draw nether portal (use adjacent obsidian/portals for angle)
void BlockDrawer::drawPortal( uint8_t blockID, uint8_t /*meta*/,
    GLint x, GLint y, GLint z, uint8_t vflags) const
{
    //TODO: Use an animated portal texture (generated, not from texture file)
    drawScaledBlock( blockID, 0, x, y, z, vflags,
        1.0, 1.0, 0.25, true, 0, 0, 8);

}

//Draw fluid, such as water and lava
void BlockDrawer::drawFluid( uint8_t blockID, uint8_t meta,
    GLint x, GLint y, GLint z, uint8_t vflags) const
{
    //TODO: Use metadata and adjacent blocks for flow angles
    drawCubeMeta( blockID, meta, x, y, z, vflags);

}


//Draw fence.  It changes depending on adjacent neighbors
void BlockDrawer::drawFence( uint8_t blockID, uint8_t meta,
    GLint x, GLint y, GLint z, uint8_t vflags) const
{

    //Center post
    drawScaledBlock( blockID, meta, x, y, z, (vflags&0x30),
        0.25, 1.0, 0.25, true, 6, 0, 6);


    //Quit if unable to check neighbors
    if (world == NULL) {
        return;
    }

    //Check adjacent blocks for connected fences (or cubes)
    //A neighbor
    Block block = world->getBlock(x - 1, y, z);
    if (block.blockID == blockID || Blk::isCube[block.blockID]) {
        //Top connecting boards
        drawScaledBlock( blockID, meta, x, y, z, (vflags&0x40)|0x80,
            6/16.0, 3/16.0, 2/16.0, true, 0, 12, 7);        

        //Bottom connecting boards
        drawScaledBlock( blockID, meta, x, y, z, (vflags&0x40)|0x80,
            6/16.0, 3/16.0, 2/16.0, true, 0, 6, 7);        
    }

    //B neighbor
    block = world->getBlock(x + 1, y, z);
    if (block.blockID == blockID || Blk::isCube[block.blockID]) {
        //Top connecting boards
        drawScaledBlock( blockID, meta, x, y, z, (vflags&0x80)|0x40,
            6/16.0, 3/16.0, 2/16.0, true, 10, 12, 7);

        //Bottom connecting boards
        drawScaledBlock( blockID, meta, x, y, z, (vflags&0x80)|0x40,
            6/16.0, 3/16.0, 2/16.0, true, 10, 6, 7);
    }

    //E neighbor
    block = world->getBlock(x, y, z - 1);
    if (block.blockID == blockID || Blk::isCube[block.blockID]) {
        //Top connecting boards
        drawScaledBlock( blockID, meta, x, y, z, (vflags&0x04)|0x08,
            2/16.0, 3/16.0, 6/16.0, true, 7, 12, 0);

        //Bottom connecting boards
        drawScaledBlock( blockID, meta, x, y, z, (vflags&0x04)|0x08,
            2/16.0, 3/16.0, 6/16.0, true, 7, 6, 0);
    }

    //F neighbor
    block = world->getBlock(x, y, z + 1);
    if (block.blockID == blockID || Blk::isCube[block.blockID]) {
        //Top connecting boards
        drawScaledBlock( blockID, meta, x, y, z, (vflags&0x08)|0x04,
            2/16.0, 3/16.0, 6/16.0, true, 7, 12, 10);

        //Bottom connecting boards
        drawScaledBlock( blockID, meta, x, y, z, (vflags&0x08)|0x04,
            2/16.0, 3/16.0, 6/16.0, true, 7, 6, 10);
    }


}


//Draw pane (fence or glass).  It changes depending on adjacent neighbors
void BlockDrawer::drawPane( uint8_t blockID, uint8_t meta,
    GLint x, GLint y, GLint z, uint8_t vflags) const
{

    uint8_t neighbors=0, nID=0;
    
    //Draw a single post if unable to read world
    if (world == NULL) {
        drawScaledBlock( blockID, meta, x, y, z, (vflags&0x30),
            0.25, 1.0, 0.25, false, 0, 0, 0);
        return;
    }

    //A neighbor
    nID = world->getBlock(x - 1, y, z).blockID;
    if ( nID == blockID || Blk::isCube[nID]) {
        neighbors |= 0x80;
    }

    //B neighbor
    nID = world->getBlock(x + 1, y, z).blockID;
    if (nID == blockID || Blk::isCube[nID]) {
        neighbors |= 0x40;
    }

    //E neighbor
    nID = world->getBlock(x, y, z - 1).blockID;
    if ( nID == blockID || Blk::isCube[nID]) {
        neighbors |= 0x08;
    }

    //F neighbor
    nID = world->getBlock(x, y, z + 1).blockID;
    if ( nID == blockID || Blk::isCube[nID]) {
        neighbors |= 0x04;
    }

    //draw pane based on X-axis neighbors
    if ( HAS_FLAGS(neighbors,0xC0) ) {
        //Thin pane on X-axis
        drawScaledBlock( blockID, meta, x, y, z, 0xC0|(vflags&0xF3),
            1.0, 1.0, 2/16.0, true, 0, 0, 7);
    } else if ( HAS_FLAGS(neighbors,0x80) ) {
        //Half pane to left.  Force vflag 'A', ignore vflags 'E','F'
        drawScaledBlock( blockID, meta, x, y, z, 0x80|(vflags&0xF3),
            0.5, 1.0, 2/16.0, true, 0, 0, 7);
    } else if ( HAS_FLAGS(neighbors,0x40) ) {
        //Half pane to right.  Force vflag 'B', ignore vflags 'E','F'
        drawScaledBlock( blockID, meta, x, y, z, 0x40|(vflags&0xF3),
            0.5, 1.0, 2/16.0, true, 8, 0, 7);
    }
    
    //draw pane based on Z-axis neighbors
    if ( HAS_FLAGS(neighbors,0x0C) ) {
        //Thin pane on Z-axis
        drawScaledBlock( blockID, meta, x, y, z, 0x0C|(vflags&0x3F),
            2/16.0, 1.0, 1.0, true, 7, 0, 0);
    } else if ( HAS_FLAGS(neighbors,0x08) ) {
        //Half pane to back.  Force vflag 'E', ignore vflags 'A','B'
        drawScaledBlock( blockID, meta, x, y, z, 0x08|(vflags&0x3F),
            2/16.0, 1.0, 0.5, true, 7, 0, 0);
    } else if ( HAS_FLAGS(neighbors,0x04) ) {
        //Half pane to front.  Force vflag 'F', ignore vflags 'A','B'
        drawScaledBlock( blockID, meta, x, y, z, 0x04|(vflags&0x3F),
            2/16.0, 1.0, 0.5, true, 7, 0, 8);
    } else if ( !(neighbors & 0xCC) ) {
        //No neighbors, draw panes on X and Z
        drawScaledBlock( blockID, meta, x, y, z, (vflags&0xF3),
            1.0, 16.0/16.0, 2/16.0, true, 0, 0, 7);
        drawScaledBlock( blockID, meta, x, y, z, (vflags&0x3F),
            2/16.0, 16.0/16.0, 1.0, true, 7, 0, 0);
    }
    
}


//Draw floor plate (meta affects pressed, depressed)
void BlockDrawer::drawFloorplate( uint8_t blockID, uint8_t meta,
    GLint x, GLint y, GLint z, uint8_t vflags) const
{
    GLint off_Y = 0;
    if (meta & 1) {
        off_Y = -1;
    }

    //Metadata affects z offset (pressed or not)
    drawScaledBlock( blockID, meta, x, y, z, (vflags&0x2),
        0.75, 0.125, 0.75, true, 2, off_Y, 2);

}

//Draw quarter block (meta affects height?)
void BlockDrawer::draw4thBlock( uint8_t blockID, uint8_t meta,
    GLint x, GLint y, GLint z, uint8_t vflags) const
{
    drawScaledBlock( blockID, meta, x, y, z, vflags,
        1, 0.25, 1);
}

//Draw tree log or leaf (meta affects texture): //TODO: more log types
void BlockDrawer::drawTree( uint8_t blockID, uint8_t meta,
    GLint x, GLint y, GLint z, uint8_t vflags) const
{
    //Use meta to change texture of cube
    uint16_t ID = blockID;
    switch( meta ) {
        case 1:
            ID = blockID + 256;
            break;
        case 2:
            ID = blockID + 512;
            break;
        default:
            ID = blockID;
            break;
    }
    drawCubeMeta(ID, meta, x, y, z, vflags);
}

//Draw piece of giant mushroom (meta affects texture)
void BlockDrawer::drawShroom( uint8_t blockID, uint8_t meta,
    GLint x, GLint y, GLint z, uint8_t vflags) const
{
    //Use meta to change texture of cube
    uint16_t ID = blockID;
    switch( blockID ) {
        case Blk::HugeShroomBrown:
            ID = Blk::HugeShroomBrown + 256 + meta;
            break;
        case Blk::HugeShroomRed:
            ID = Blk::HugeShroomBrown + 256 + 16 + meta;
            break;
        default:
            ID = blockID;
            break;
    }
    drawCubeMeta(ID, meta, x, y, z, vflags);
}

//Draw sign on a wall.  Don't use block ID, it has another texture
void BlockDrawer::drawWallSign( uint8_t /*blockID*/, uint8_t meta,
    GLint x, GLint y, GLint z, uint8_t vflags) const
{

    //Use sign.png
    bindTexture(TEX_SIGN);
    
    //Look up texture coordinates for signboard
    GLfloat tx0[6], tx1[6], ty0[6], ty1[6];

    uint16_t index;
    for (index = 0; index < 6; index++) {
        
        //Get texture coordinates from TextureInfo
        TextureInfo *pti = texInfo[texture_INDEX[TEX_SIGN] + index];
        if (pti != NULL) { pti->getCoords(
            tx0[index], tx1[index], ty0[index], ty1[index]);
        }
    }

    //Cube boundaries
    GLint A = (x << 4) + 0;
    GLint B = (x << 4) + texmap_TILE_LENGTH;
    GLint C = (y << 4) + 0;
    GLint D = (y << 4) + texmap_TILE_LENGTH;
    GLint E = (z << 4) + 0;
    GLint F = (z << 4) + texmap_TILE_LENGTH;
    
    //signboard bottom
    GLint G = C + 7;
    
    //signboard top (D + 2)
    GLint H = D - 1;
    
    //Vertices of signboard
    GLint vX[8], vY[8], vZ[8];
    
    //Y values of vertices always the same
    vY[0] = G  ; vY[2] = G  ; vY[3] = H  ; vY[1] = H  ;
    vY[6] = G  ; vY[4] = G  ; vY[5] = H  ; vY[7] = H  ;

    //TODO: Write message on sign texture
    
    uint8_t vmask=0xFF;
    switch (meta & 0x7) {
        case 2: //West side (face east)
            //facing sign, left side: 0, 2, 3, 1
            vX[0] = B  ; vX[2] = B  ; vX[3] = B  ; vX[1] = B  ;
            vZ[0] = F  ; vZ[2] = F-2; vZ[3] = F-2; vZ[1] = F  ;
        
            //facing sign, right side: 6, 4, 5, 7
            vX[6] = A  ; vX[4] = A  ; vX[5] = A  ; vX[7] = A  ; 
            vZ[6] = F-2; vZ[4] = F  ; vZ[5] = F  ; vZ[7] = F-2;
            
            vmask = 0x04;
            break;
        case 3: //East side (face west)
        default:
            //facing sign, left side: 0, 2, 3, 1
            vX[0] = A  ; vX[2] = A  ; vX[3] = A  ; vX[1] = A  ; 
            vZ[0] = E  ; vZ[2] = E+2; vZ[3] = E+2; vZ[1] = E  ;
        
            //facing sign, right side: 6, 4, 5, 7
            vX[6] = B  ; vX[4] = B  ; vX[5] = B  ; vX[7] = B  ;
            vZ[6] = E+2; vZ[4] = E  ; vZ[5] = E  ; vZ[7] = E+2;
            
            vmask = 0x08;
            break;
        case 4: //South side (face north)
            //facing sign, left side: 0, 2, 3, 1
            vX[0] = B  ; vX[2] = B-2; vX[3] = B-2; vX[1] = B  ;
            vZ[0] = E  ; vZ[2] = E  ; vZ[3] = E  ; vZ[1] = E  ;
        
            //facing sign, right side: 6, 4, 5, 7
            vX[6] = B-2; vX[4] = B  ; vX[5] = B  ; vX[7] = B-2; 
            vZ[6] = F  ; vZ[4] = F  ; vZ[5] = F  ; vZ[7] = F  ;
            vmask = 0x40;
            break;
        case 5: //North side (face south)
            //facing sign, left side: 0, 2, 3, 1
            vX[0] = A  ; vX[2] = A+2; vX[3] = A+2; vX[1] = A  ;
            vZ[0] = F  ; vZ[2] = F  ; vZ[3] = F  ; vZ[1] = F  ;
        
            //facing sign, right side: 6, 4, 5, 7
            vX[6] = A+2; vX[4] = A  ; vX[5] = A  ; vX[7] = A+2; 
            vZ[6] = E  ; vZ[4] = E  ; vZ[5] = E  ; vZ[7] = E  ;
            vmask = 0x80;
            break;
    }

    //I want 6 faces, pronto!
    drawVertexBlock(vX, vY, vZ, tx0, tx1, ty0, ty1, vflags&vmask);

    //Change back to terrain
    bindTexture(TEX_TERRAIN);

}

//Draw wall button
void BlockDrawer::drawButton( uint8_t blockID, uint8_t meta,
    GLint x, GLint y, GLint z, uint8_t /*vflags*/) const
{

    //Button is offset 6 right, 5 up, and 0 off wall.  Size is 6x5x2
    GLint dX=6, dY=5, dZ=0, lX=6, lY=5, lZ=2;
    face_ID facing;
    uint16_t blockOffset = blockID;
    
    //Determine facing and wall offset from meta.
    //   Offset is assigned here to avoid rotation calculation.  I know, right!
    switch (meta & 0xF) {
        //Not pressed, with direction
        case 0x1: facing = RIGHT; dX=0;  lX=2; dZ=5; lZ=6; break;
        case 0x2: facing = LEFT;  dX=14; lX=2; dZ=5; lZ=6; break;
        
        case 0x3: facing = FRONT; dX=5; lX=6; dZ=0;  lZ=2; break;
        case 0x4: facing = BACK;  dX=5; lX=6; dZ=14; lZ=2; break;
        
        //Button pressed, with direction
        case 0x9: facing = RIGHT; dX=-1; lX=2; dZ=5; lZ=6; break;
        case 0xA: facing = LEFT;  dX=15; lX=2; dZ=5; lZ=6; break;
        
        case 0xB: facing = FRONT; dX=5; lX=6;  dZ=1; lZ=2; break;
        case 0xC: facing = BACK;  dX=5; lX=6; dZ=13; lZ=2; break;
        default:  facing = FRONT; dX=5; lX=6; dZ=0; lZ=2; 
    }

    //Create vertices for button
    GLint vX[8], vY[8], vZ[8];
    makeCuboidVertex(x, y, z, lX, lY, lZ, vX, vY, vZ, facing);

    //Add location offset to button
    addVertexOffset( vX, vY, vZ, dX, dY, dZ);
    
    //Get block info for texture info
    const BlockInfo& binfo = blockInfo[blockOffset];

    //Draw the block using calculated vertices
    drawVertexBlock( vX, vY, vZ, binfo.tx, binfo.tx_1, binfo.ty, binfo.ty_1,
        0, facing);
}



//Copy block info to struct
void BlockDrawer::setBlockInfo( uint16_t index,
    uint16_t A, uint16_t B,
    uint16_t C, uint16_t D,
    uint16_t E, uint16_t F,
    drawBlock_f drawFunc)
{
    //A - F contain texture ID of the corresponding face (textureID = 0 - 255)

    //For each face,  precompute OpenGL texture offsets in texture map
    // (Remember that texture map was loaded upside down!)
    blockInfo[index].textureID[LEFT] = A;
    blockInfo[index].tx[0] = float(A & (texmap_TILES-1))/((float)texmap_TILES);
    blockInfo[index].ty[0] = float(A/texmap_TILES)/((float)texmap_TILES);
    blockInfo[index].tx_1[0] = blockInfo[index].tx[0] + tmr;
    blockInfo[index].ty_1[0] = blockInfo[index].ty[0] + tmr;

    blockInfo[index].textureID[RIGHT] = B;
    blockInfo[index].tx[1] = float(B & (texmap_TILES-1))/((float)texmap_TILES);
    blockInfo[index].ty[1] = float(B/texmap_TILES)/((float)texmap_TILES);
    blockInfo[index].tx_1[1] = blockInfo[index].tx[1] + tmr;
    blockInfo[index].ty_1[1] = blockInfo[index].ty[1] + tmr;

    blockInfo[index].textureID[BOTTOM] = C;
    blockInfo[index].tx[2] = float(C & (texmap_TILES-1))/((float)texmap_TILES);
    blockInfo[index].ty[2] = float(C/texmap_TILES)/((float)texmap_TILES);
    blockInfo[index].tx_1[2] = blockInfo[index].tx[2] + tmr;
    blockInfo[index].ty_1[2] = blockInfo[index].ty[2] + tmr;

    blockInfo[index].textureID[TOP] = D;
    blockInfo[index].tx[3] = float(D & (texmap_TILES-1))/((float)texmap_TILES);
    blockInfo[index].ty[3] = float(D/texmap_TILES)/((float)texmap_TILES);
    blockInfo[index].tx_1[3] = blockInfo[index].tx[3] + tmr;
    blockInfo[index].ty_1[3] = blockInfo[index].ty[3] + tmr;

    blockInfo[index].textureID[BACK] = E;
    blockInfo[index].tx[4] = float(E & (texmap_TILES-1))/((float)texmap_TILES);
    blockInfo[index].ty[4] = float(E/texmap_TILES)/((float)texmap_TILES);
    blockInfo[index].tx_1[4] = blockInfo[index].tx[4] + tmr;
    blockInfo[index].ty_1[4] = blockInfo[index].ty[4] + tmr;

    blockInfo[index].textureID[FRONT] = F;
    blockInfo[index].tx[5] = float(F & (texmap_TILES-1))/((float)texmap_TILES);
    blockInfo[index].ty[5] = float(F/texmap_TILES)/((float)texmap_TILES);
    blockInfo[index].tx_1[5] = blockInfo[index].tx[5] + tmr;
    blockInfo[index].ty_1[5] = blockInfo[index].ty[5] + tmr;

    drawFunction[index] = drawFunc;

}

//Fill texture ID -> textureInfo map with default values
bool BlockDrawer::loadTexInfo()
{
    uint16_t ID;

    //Assign terrain textures 0-255
    for (ID = texture_INDEX[TEX_TERRAIN];
         ID < texture_INDEX[TEX_TERRAIN] + 256;
         ID++) {
        //Calculate texture map coordinates
        GLfloat tx0 = GLfloat(ID & (texmap_TILES-1))/((GLfloat)texmap_TILES);
        GLfloat ty0 = GLfloat(ID/texmap_TILES)/((GLfloat)texmap_TILES);
        
        //Map ID to texture information
        texInfo[ID] = new TextureInfo(TEX_TERRAIN, tx0, tx0+tmr, ty0, ty0+tmr);
    }

    //Assign item textures 256-511
    for (ID = texture_INDEX[TEX_ITEM];
         ID < texture_INDEX[TEX_ITEM] + 256;
         ID++) {
        //Calculate texture map coordinates
        GLfloat tx0 = GLfloat(ID & (texmap_TILES-1))/((GLfloat)texmap_TILES);
        GLfloat ty0 = GLfloat((ID&0xFF)/texmap_TILES)/((GLfloat)texmap_TILES);
        
        //Map ID to texture information
        texInfo[ID] = new TextureInfo(TEX_ITEM, tx0, tx0+tmr, ty0, ty0+tmr);
    }

    //
    //Assign sign textures 512-523
    //
    
    //Sign board: left, right, bottom, top, back, front
    ID=texture_INDEX[TEX_SIGN];
    texInfo[ID++] = new TextureInfo(TEX_SIGN, 128, 64,  0,  4,  4, 24);
    texInfo[ID++] = new TextureInfo(TEX_SIGN, 128, 64, 52,  4,  4, 24);
    texInfo[ID++] = new TextureInfo(TEX_SIGN, 128, 64, 52,  0, 48,  4);
    texInfo[ID++] = new TextureInfo(TEX_SIGN, 128, 64,  4,  0, 48,  4);
    texInfo[ID++] = new TextureInfo(TEX_SIGN, 128, 64,  4,  4, 48, 24);
    texInfo[ID++] = new TextureInfo(TEX_SIGN, 128, 64, 56,  4, 48, 24);
        
    //Sign post: left, right, bottom, top, back, front
    ID=texture_INDEX[TEX_SIGN]+6;
    texInfo[ID++] = new TextureInfo(TEX_SIGN, 128, 64,  0, 32,  4, 28);
    texInfo[ID++] = new TextureInfo(TEX_SIGN, 128, 64,  8, 32,  4, 28);
    texInfo[ID++] = new TextureInfo(TEX_SIGN, 128, 64,  8, 28,  4,  4);
    texInfo[ID++] = new TextureInfo(TEX_SIGN, 128, 64,  4, 28,  4,  4);
    texInfo[ID++] = new TextureInfo(TEX_SIGN, 128, 64, 12, 32,  4, 28);
    texInfo[ID++] = new TextureInfo(TEX_SIGN, 128, 64,  4, 32,  4, 28);

    //Fill the rest of IDs with sponge texture
    for (; ID < texture_id_MAX; ID++) {
      
        //Associate unknown ID to the "sponge" texture information
        texInfo[ID++] = new TextureInfo(TEX_TERRAIN, 16, 16, 0, 3, 1, 1);
    }
    

    return true;
}

//Associate the block ID to block type information
bool BlockDrawer::loadBlockInfo()
{
    uint8_t ID;

    //Set default block information to sponge!
    for (ID = 0; ID != 0xFF; ID++) {
        setBlockInfo( ID, 48, 48, 48, 48, 48, 48);
    }
    setBlockInfo( 0xFF, 48, 48, 48, 48, 48, 48);

/*  Block geometry

   ADE ---- BDE
   /.       /|
  / .      / |
ADF ---- BDF |
 | ACE . .| BCE
 | .      | /
 |.       |/
ACF ---- BCF

//0xF0: Shape : 0=cube, 1=stairs, 2=lever, 3=halfblock,
//              4=signpost, 5=ladder, 6=track, 7=fire
//              8=portal, 9=fence, A=door, B=floorplate
//              C=short, D=wallsign, E=button, F=planted
//0x08: Bright: 0=dark, 1=lightsource
//0x04: Vision: 0=opqaue, 1=see-through
//0x03: State : 0=solid, 1=loose, 2=liquid, 3=gas

Planted item = 0xF7: planted, dark, see-through, move-through
Normal block = 0x00: cube, dark, opaque, solid
*/

    //Air
    setBlockInfo( Blk::Air, Tex::Web, Tex::Web, Tex::Web,
        Tex::Web, Tex::Web, Tex::Web);
    //Stone
    setBlockInfo( Blk::Stone, Tex::Stone, Tex::Stone, Tex::Stone, Tex::Stone,
        Tex::Stone, Tex::Stone);
    //Grass
    setBlockInfo( Blk::Grass,  Tex::Grass_Side,  Tex::Grass_Side, Tex::Dirt,
        Tex::Grass, Tex::Grass_Side, Tex::Grass_Side);
    //Dirt
    setBlockInfo( Blk::Dirt, Tex::Dirt, Tex::Dirt, Tex::Dirt, Tex::Dirt,
        Tex::Dirt, Tex::Dirt);
    //Cobble
    setBlockInfo( Blk::Cobble, Tex::Cobble, Tex::Cobble, Tex::Cobble,
        Tex::Cobble, Tex::Cobble, Tex::Cobble);
    //Wood //TODO: metadata
    setBlockInfo( Blk::Wood, Tex::Wood, Tex::Wood, Tex::Wood, Tex::Wood,
        Tex::Wood, Tex::Wood);
    //Saplings  //TODO: Different sapling types from metadata
    setBlockInfo( Blk::Sapling, Tex::Sapling, Tex::Sapling_Pine, Tex::Sapling,
        Tex::Sapling, Tex::Sapling_Birch, Tex::Sapling_Jungle,
        &BlockDrawer::drawItem);
    //Bedrock
    setBlockInfo( Blk::Bedrock, Tex::Admin, Tex::Admin, Tex::Admin, Tex::Admin,
        Tex::Admin, Tex::Admin);
    //Water (Flowing)   //TODO: drawFlow
    setBlockInfo( Blk::WaterFlow, Tex::Water_2, Tex::Water_4, Tex::Water,
        Tex::Water, Tex::Water_5, Tex::Water_3);
    //Water (Stationary)
    setBlockInfo( Blk::Water, Tex::Water_2, Tex::Water_4, Tex::Water,
        Tex::Water, Tex::Water_5, Tex::Water_3);
    //Lava (Flowing)   //TODO: drawFlow
    setBlockInfo( Blk::LavaFlow, Tex::Lava_2, Tex::Lava_4, Tex::Lava,
        Tex::Lava, Tex::Lava_5, Tex::Lava_3);
    //Lava (Stationary)
    setBlockInfo( Blk::Lava, Tex::Lava_2, Tex::Lava_4, Tex::Lava,
        Tex::Lava, Tex::Lava_5, Tex::Lava_3);
    //Sand
    setBlockInfo( Blk::Sand, Tex::Sand, Tex::Sand, Tex::Sand, Tex::Sand,
        Tex::Sand, Tex::Sand);
    //Gravel
    setBlockInfo( Blk::Gravel, Tex::Gravel, Tex::Gravel, Tex::Gravel,
        Tex::Gravel, Tex::Gravel, Tex::Gravel);
    //GoldOre
    setBlockInfo( Blk::GoldOre, Tex::GoldOre, Tex::GoldOre, Tex::GoldOre,
        Tex::GoldOre, Tex::GoldOre, Tex::GoldOre);
    //IronOre
    setBlockInfo( Blk::IronOre, Tex::IronOre, Tex::IronOre, Tex::IronOre,
        Tex::IronOre, Tex::IronOre, Tex::IronOre);
    //CoalOre
    setBlockInfo( Blk::CoalOre, Tex::Coal, Tex::Coal, Tex::Coal, Tex::Coal,
        Tex::Coal, Tex::Coal);
    //Log:
    setBlockInfo( Blk::Log, Tex::Log_Side, Tex::Log_Side, Tex::Log_Top,
        Tex::Log_Top, Tex::Log_Side, Tex::Log_Side,&BlockDrawer::drawTree);
    //Leaves
    setBlockInfo( Blk::Leaves, Tex::Leaf, Tex::Leaf, Tex::Leaf, Tex::Leaf1,
        Tex::Leaf, Tex::Leaf,&BlockDrawer::drawTree);
    //Sponge
    setBlockInfo( Blk::Sponge, Tex::Sponge, Tex::Sponge, Tex::Sponge,
        Tex::Sponge, Tex::Sponge, Tex::Sponge);
    //Glass
    setBlockInfo( Blk::Glass, Tex::Glass, Tex::Glass, Tex::Glass, Tex::Glass,
        Tex::Glass, Tex::Glass);
    //Lapis Ore
    setBlockInfo( Blk::LapisOre,160,160,160,160,160,160);
    //Lapis Block
    setBlockInfo( Blk::LapisBlock,144,144,144,144,144,144);
    //Dispenser (*)
    setBlockInfo( Blk::Dispenser, 45, 45, 62, 62, 45, 46,
        &BlockDrawer::drawFaceCube);
    //Sandstone
    setBlockInfo( Blk::Sandstone,192,192,208,176,192,192);
    //Note Block
    setBlockInfo( Blk::NoteBlock, 74, 74, 74, 74, 74, 74);
    
    //Bed (*)
    const GLint bedHeight=9;
    setBlockInfo( Blk::Bed, Tex::BedFoot_Face,Tex::BedHead_Face, Tex::Wood,
        Tex::BedHead_Top, Tex::BedHead_Side,Tex::BedHead_Side,
        &BlockDrawer::drawBed);
    adjustTexture(Blk::Bed, 0, 16-bedHeight,  0, 16, bedHeight, 16);
    //Will have "meta" blocks for bed, for head and foot

    //Powered Rail
    setBlockInfo( Blk::RailPowered, Tex::Track_Off, Tex::Track_On,
        Tex::Track_Off, Tex::Track_On, Tex::Track_Off, Tex::Track_On,
        &BlockDrawer::drawTrack2);
    
    //Detector Rail
    setBlockInfo( Blk::RailDetector, Tex::Track_Sensor, Tex::Track_Sensor,
        Tex::Track_Sensor, Tex::Track_Sensor, Tex::Track_Sensor,
        Tex::Track_Sensor, &BlockDrawer::drawTrack2);

    //Web    
    setBlockInfo( Blk::Web, Tex::Web, Tex::Web, Tex::Web, Tex::Web, Tex::Web,
        Tex::Web, &BlockDrawer::drawItem);

    //Sticky Piston
    setBlockInfo( Blk::StickyPiston, Tex::Piston_Side, Tex::Piston_Side,
        Tex::Piston_Side, Tex::Piston_Side, Tex::Piston_Back,
        Tex::PistonSticky_Front, &BlockDrawer::drawFaceCube);

    //Tall Grass    
    setBlockInfo( Blk::TallGrass, Tex::TallGrass, Tex::TallGrass,
                  Tex::TallGrass, Tex::TallGrass, Tex::TallGrass,
                  Tex::TallGrass, &BlockDrawer::drawBiomeItem);
    //Dead Bush
    setBlockInfo( Blk::DeadBush, Tex::DeadBush, Tex::DeadBush,
                  Tex::DeadBush, Tex::DeadBush, Tex::DeadBush,
                  Tex::DeadBush, &BlockDrawer::drawItem);

    //Piston
    setBlockInfo( Blk::Piston, Tex::Piston_Side, Tex::Piston_Side,
        Tex::Piston_Side, Tex::Piston_Side, Tex::Piston_Back,
        Tex::Piston_Front, &BlockDrawer::drawFaceCube);

    //PistonHead:     //TODO: drawPistonHead
    setBlockInfo( Blk::PistonHead, Tex::Piston_Side, Tex::Piston_Side,
        Tex::Piston_Side, Tex::Piston_Side, Tex::Piston_Back,
        Tex::Piston_Front, &BlockDrawer::drawFaceCube);
    
    //35 = Dyed wool (drawDyed will override metadata)
    setBlockInfo( Blk::Wool, Tex::Wool, Tex::Wool, Tex::Wool, Tex::Wool,
        Tex::Wool, Tex::Wool,&BlockDrawer::drawDyed);
    
    //TODO: draw moving block from entity information
    setBlockInfo( Blk::PistonMoved, Tex::Blue, Tex::Blue, Tex::Blue, Tex::Blue,
        Tex::Blue, Tex::Blue);
    
    //Flower
    setBlockInfo( Blk::Daisy, Tex::Daisy, Tex::Daisy, Tex::Daisy, Tex::Daisy,
        Tex::Daisy, Tex::Daisy,&BlockDrawer::drawItem);
    
    //Rose
    setBlockInfo( Blk::Rose, Tex::Rose, Tex::Rose, Tex::Rose, Tex::Rose,
        Tex::Rose, Tex::Rose,&BlockDrawer::drawItem);
    
    //BrownShroom
    setBlockInfo( Blk::ShroomBrown, Tex::BrownShroom, Tex::BrownShroom,
        Tex::BrownShroom, Tex::BrownShroom, Tex::BrownShroom, Tex::BrownShroom,
        &BlockDrawer::drawItem);
    
    //RedShroom
    setBlockInfo( Blk::ShroomRed, Tex::RedShroom, Tex::RedShroom,
        Tex::RedShroom, Tex::RedShroom, Tex::RedShroom, Tex::RedShroom,
        &BlockDrawer::drawItem);
    //GoldBlock
    setBlockInfo( Blk::GoldBlock, Tex::GoldBlock, Tex::GoldBlock,
        Tex::GoldBlock, Tex::GoldBlock, Tex::GoldBlock, Tex::GoldBlock);
    //IronBlock
    setBlockInfo( Blk::IronBlock, Tex::IronBlock, Tex::IronBlock,
        Tex::IronBlock, Tex::IronBlock, Tex::IronBlock, Tex::IronBlock);
    //DoubleStep
    setBlockInfo( Blk::SlabDouble, Tex::Step_Side,  Tex::Step_Side,
        Tex::Step_Top, Tex::Step_Top, Tex::Step_Side, Tex::Step_Side,
        &BlockDrawer::drawDoubleSlab);
    
    //Step
    setBlockInfo( Blk::Slab, Tex::Step_Side,  Tex::Step_Side,
        Tex::Step_Top, Tex::Step_Top, Tex::Step_Side, Tex::Step_Side,
        &BlockDrawer::drawSlab);
    //Brick
    setBlockInfo( Blk::Bricks, Tex::Brick, Tex::Brick, Tex::Brick, Tex::Brick, 
        Tex::Brick, Tex::Brick );
    //TNT
    setBlockInfo( Blk::TNT, Tex::TNT_Side, Tex::TNT_Side, Tex::TNT_Bottom,
        Tex::TNT_Top, Tex::TNT_Side, Tex::TNT_Side );
    //Bookshelf
    setBlockInfo( Blk::Bookshelf, Tex::Books, Tex::Books, Tex::Wood, Tex::Wood,
        Tex::Books, Tex::Books);
    //Mossy
    setBlockInfo( Blk::MossStone, Tex::CobbleMoss, Tex::CobbleMoss, Tex::Cobble,
        Tex::CobbleMoss, Tex::CobbleMoss, Tex::CobbleMoss);
    //Obsidian
    setBlockInfo( Blk::Obsidian, Tex::Obsidian, Tex::Obsidian, Tex::Obsidian,
        Tex::Obsidian, Tex::Obsidian, Tex::Obsidian);
    
    //Torch
    setBlockInfo( Blk::Torch, Tex::Torch, Tex::Torch, Tex::Torch, Tex::Torch,
        Tex::Torch, Tex::Torch,&BlockDrawer::drawTorch);
    
    //Fire  //TODO: use actual fire texture
    setBlockInfo( Blk::Fire, Tex::FireTex, Tex::FireTex, Tex::FireTex2,
        Tex::FireTex2, Tex::FireTex, Tex::FireTex,&BlockDrawer::drawFire);
        
    //Spawner   //TODO: draw spinning monster from entity ID
    setBlockInfo( Blk::Spawner, Tex::Spawner, Tex::Spawner, Tex::Spawner,
        Tex::Spawner, Tex::Spawner, Tex::Spawner);
    
    //WoodStairs
    setBlockInfo( Blk::StairsWood, Tex::Wood, Tex::Wood, Tex::Wood, Tex::Wood,
        Tex::Wood, Tex::Wood, &BlockDrawer::drawStairs);
    
    //Chest (*) //TODO: use separate chest texture
    setBlockInfo( Blk::Chest, Tex::Chest_Side, Tex::Chest_Side, Tex::Chest_Top,
        Tex::Chest_Top, Tex::Chest_Side, Tex::Chest_Front,
        &BlockDrawer::drawChest);
    
    //Wire (*)
    setBlockInfo( Blk::Wire, Tex::WireX, Tex::Wire, Tex::WireX, Tex::Wire,
        Tex::WireX, Tex::Wire, &BlockDrawer::drawWire);
    //DiamondOre
    setBlockInfo( Blk::DiamondOre, Tex::DiamondOre, Tex::DiamondOre,
        Tex::DiamondOre, Tex::DiamondOre, Tex::DiamondOre, Tex::DiamondOre);
    //DiamondBlock
    setBlockInfo( Blk::DiamondBlock, Tex::DiamondBlock, Tex::DiamondBlock,
        Tex::DiamondBlock, Tex::DiamondBlock, Tex::DiamondBlock,
        Tex::DiamondBlock);
    //Workbench
    setBlockInfo( Blk::Workbench, Tex::Bench_Side, Tex::Bench_Side,
        Tex::Bench_Top, Tex::Bench_Top, Tex::Bench_Front, Tex::Bench_Front);
    
    //Crops (*)
    setBlockInfo( Blk::Crops, Tex::Crops_1, Tex::Crops_2, Tex::Crops_3,
        Tex::Crops_4, Tex::Crops_5, Tex::Crops_7, &BlockDrawer::drawCrops);
    //Soil: //TODO: use metadata for wet/dry
    setBlockInfo( Blk::Soil, Tex::Dirt, Tex::Dirt, Tex::Dirt, Tex::Soil_Wet,
        Tex::Dirt, Tex::Dirt );
    
    //Furnace (+)
    setBlockInfo( Blk::Furnace, Tex::Furnace_Back, Tex::Furnace_Back,
        Tex::Furnace_Top, Tex::Furnace_Top, Tex::Furnace_Back,
        Tex::Furnace_Front, &BlockDrawer::drawFaceCube);
    
    //LitFurnace (+)
    setBlockInfo( Blk::FurnaceOn, Tex::Furnace_Back, Tex::Furnace_Back,
        Tex::Furnace_Top, Tex::Furnace_Top, Tex::Furnace_Back,
        Tex::FurnaceLit_Front,&BlockDrawer::drawFaceCube);
    
    //SignPost (*)
    setBlockInfo( Blk::Signpost, Tex::Wood, Tex::Wood, Tex::Wood, Tex::Wood,
        Tex::Wood, Tex::Wood, &BlockDrawer::drawSignpost);
    
    //WoodDoor (*)
    setBlockInfo( Blk::DoorWood, Tex::Door_Low, Tex::Door_High, Tex::Door_Low,
        Tex::Door_High, Tex::Door_Low, Tex::Door_High, &BlockDrawer::drawDoor);
    
    //Ladder (*)
    setBlockInfo( Blk::Ladder, Tex::Ladder, Tex::Ladder, Tex::Ladder,
        Tex::Ladder, Tex::Ladder, Tex::Ladder, &BlockDrawer::drawWallItem);
    
    //Track (*)
    setBlockInfo( Blk::Track, Tex::Track, Tex::Track_Turn,
       Tex::Track, Tex::Track, Tex::Track, Tex::Track,&BlockDrawer::drawTrack);
    
    //CobbleStairs:  //TODO: metadata for upside down stairs
    setBlockInfo( Blk::StairsCobble, Tex::Cobble, Tex::Cobble, Tex::Cobble,
        Tex::Cobble, Tex::Cobble, Tex::Cobble,&BlockDrawer::drawStairs);
    
    //WallSign (*)
    setBlockInfo( Blk::Wallsign, Tex::Wood, Tex::Wood, Tex::Wood, Tex::Wood,
        Tex::Wood, Tex::Wood, &BlockDrawer::drawWallSign);
    
    //Lever
    setBlockInfo( Blk::Lever, Tex::Handle, Tex::Handle, Tex::Handle,
        Tex::Handle, Tex::Handle, Tex::Cobble, &BlockDrawer::drawLever);
    
    //StonePlate
    setBlockInfo( Blk::PlateStone, Tex::Stone, Tex::Stone, Tex::Stone,
        Tex::Stone, Tex::Stone, Tex::Stone,&BlockDrawer::drawFloorplate);
    
    //IronDoor (*)
    setBlockInfo( Blk::DoorIron, Tex::IronDoor_Low, Tex::IronDoor_High,
        Tex::IronDoor_Low, Tex::IronDoor_High, Tex::IronDoor_Low,
        Tex::IronDoor_High,&BlockDrawer::drawDoor);
    
    //WoodPlate
    setBlockInfo( Blk::PlateWood, Tex::Wood, Tex::Wood, Tex::Wood, Tex::Wood,
        Tex::Wood, Tex::Wood, &BlockDrawer::drawFloorplate);
    //RedstoneOre
    setBlockInfo( Blk::RedstoneOre, Tex::RedOre, Tex::RedOre, Tex::RedOre,
        Tex::RedOre, Tex::RedOre, Tex::RedOre);
    //RedstoneOreLit(*)
    setBlockInfo( Blk::RedstoneOreOn, Tex::RedOre, Tex::RedOre, Tex::RedOre,
        Tex::RedOre, Tex::RedOre, Tex::RedOre);
    
    //RedstoneTorch
    setBlockInfo( Blk::RedTorch, Tex::RedTorch_Off, Tex::RedTorch_Off,
        Tex::RedTorch_Off, Tex::RedTorch_Off, Tex::RedTorch_Off,
        Tex::RedTorch_Off, &BlockDrawer::drawTorch);

    //RedstoneTorchLit
    setBlockInfo( Blk::RedTorchOn, Tex::RedTorch_On, Tex::RedTorch_On,
        Tex::RedTorch_On, Tex::RedTorch_On, Tex::RedTorch_On, Tex::RedTorch_On,
        &BlockDrawer::drawTorch);
    
    //StoneButton
    setBlockInfo( Blk::Button, Tex::Stone, Tex::Stone, Tex::Stone, Tex::Stone,
        Tex::Stone, Tex::Stone,&BlockDrawer::drawButton);
    //Move texture offsets for 6 faces
    adjustTexture(Blk::Button , 0,  5,  0, 6, 4, 2);

    //SnowLayer(*)
    //BlockID 2 (Grass) below a a SnowLayer uses texture 68 on the sides
    setBlockInfo( Blk::Snow, Tex::Snow, Tex::Snow, Tex::Snow, Tex::Snow,
        Tex::Snow, Tex::Snow, &BlockDrawer::draw4thBlock);
    
    //Ice
    setBlockInfo( Blk::Ice, Tex::Ice, Tex::Ice, Tex::Ice, Tex::Ice, Tex::Ice,
        Tex::Ice);
    //SnowBlock
    setBlockInfo( Blk::SnowBlock, Tex::Snow, Tex::Snow, Tex::Snow, Tex::Snow,
        Tex::Snow, Tex::Snow);
    
    //Cactus
    setBlockInfo( Blk::Cactus, Tex::Cactus_Side, Tex::Cactus_Side,
        Tex::Cactus_Bottom, Tex::Cactus_Top, Tex::Cactus_Side,
        Tex::Cactus_Side, &BlockDrawer::drawCactus);
    //Clay
    setBlockInfo( Blk::ClayBlock, Tex::Clay, Tex::Clay, Tex::Clay, Tex::Clay,
        Tex::Clay, Tex::Clay);
    
    //Sugarcane (*)
    setBlockInfo( Blk::SugarCane, Tex::SugarCane, Tex::SugarCane,
        Tex::SugarCane, Tex::SugarCane, Tex::SugarCane, Tex::SugarCane,
        &BlockDrawer::drawItem);
    
    //Jukebox
    setBlockInfo( Blk::Jukebox, Tex::Jukebox_Side, Tex::Jukebox_Side,
        Tex::Bench_Top, Tex::Jukebox_Top, Tex::Jukebox_Side, Tex::Jukebox_Side);
    
    //Fence (*)
    setBlockInfo( Blk::Fence, Tex::Wood, Tex::Wood, Tex::Wood, Tex::Wood,
        Tex::Wood, Tex::Wood, &BlockDrawer::drawFence);
    
    //Pumpkin
    setBlockInfo( Blk::Pumpkin, Tex::Pumpkin_Side, Tex::Pumpkin_Side,
        Tex::Pumpkin_Side, Tex::Pumpkin_Top, Tex::Pumpkin_Side,
        Tex::Pumpkin_Front, &BlockDrawer::drawFaceCube2);
    
    //Netherstone
    setBlockInfo( Blk::Netherrack, Tex::NetherRack, Tex::NetherRack,
        Tex::NetherRack, Tex::NetherRack, Tex::NetherRack, Tex::NetherRack);
    
    //SlowSand
    setBlockInfo( Blk::SoulSand, Tex::SoulSand, Tex::SoulSand, Tex::SoulSand,
        Tex::SoulSand, Tex::SoulSand, Tex::SoulSand);
    
    //Lightstone
    setBlockInfo( Blk::Glowstone, Tex::LightStone, Tex::LightStone,
        Tex::LightStone, Tex::LightStone, Tex::LightStone, Tex::LightStone);
    
    //Portal
    setBlockInfo( Blk::Portal, Tex::Water, Tex::Water_2, Tex::Water_3,
        Tex::Water_4, Tex::Water_5, Tex::Water, &BlockDrawer::drawPortal);
    
    //PumpkinLit
    setBlockInfo( Blk::PumpkinOn, Tex::Pumpkin_Side, Tex::Pumpkin_Side,
        Tex::Pumpkin_Side, Tex::Pumpkin_Top, Tex::Pumpkin_Side,
        Tex::PumpkinLit_Front, &BlockDrawer::drawFaceCube2);
    
    //Cake block (*)
    setBlockInfo( Blk::Cake, Tex::Cake_Cut, Tex::Cake_Side, Tex::Cake_Bottom,
        Tex::Cake_Top, Tex::Cake_Side, Tex::Cake_Side, &BlockDrawer::drawCake);
    
    //Diode off (Repeater)
    setBlockInfo( Blk::Diode, Tex::Diode_Off, Tex::Diode_Off, Tex::Step_Top,
    Tex::Diode_Off, Tex::Diode_Off, Tex::Diode_Off, &BlockDrawer::drawDiode);
    adjustTexture(Blk::Diode  , 0,  0,  0, 16,  2, 16);
    
    //Diode on (Repeater)
    setBlockInfo( Blk::DiodeOn, Tex::Diode_On, Tex::Diode_On, Tex::Step_Top,
    Tex::Diode_On, Tex::Diode_On, Tex::Diode_On, &BlockDrawer::drawDiode);    
    adjustTexture(Blk::DiodeOn, 0,  0,  0, 16,  2, 16);

    //Glow Chest
    setBlockInfo( Blk::ChestGlow, Tex::Chest_Side, Tex::Chest_Side,
        Tex::Chest_Top, Tex::Chest_Top, Tex::Chest_Side, Tex::Chest_Front,
        &BlockDrawer::drawChest);

    //Trap Door (*) TODO: drawTrapDoor
    setBlockInfo( Blk::TrapDoor, Tex::TrapDoor, Tex::TrapDoor, Tex::TrapDoor,
        Tex::TrapDoor, Tex::TrapDoor, Tex::TrapDoor, &BlockDrawer::drawDoor);

    //Monster hiding in block
    setBlockInfo( Blk::Silverfish, Tex::Stone, Tex::Stone, Tex::Cobble,
        Tex::Cobble, Tex::BrickStone, Tex::BrickStone,
        &BlockDrawer::drawEggBlock);

    //Bricks of stone
    setBlockInfo( Blk::StoneBrick, Tex::BrickStone, Tex::BrickStone,
        Tex::BrickStone, Tex::BrickStone, Tex::BrickStone, Tex::BrickStone);

    //Big Mushrooms
    setBlockInfo( Blk::HugeShroomBrown, Tex::Myc_Stem, Tex::Myc_Stem,
        Tex::Myc_Pore, Tex::Cap_Brown, Tex::Myc_Stem, Tex::Myc_Stem,
        &BlockDrawer::drawShroom);
    setBlockInfo( Blk::HugeShroomRed, Tex::Myc_Stem, Tex::Myc_Stem,
        Tex::Myc_Pore, Tex::Cap_Red, Tex::Myc_Stem, Tex::Myc_Stem,
        &BlockDrawer::drawShroom);

    //Iron Bars
    setBlockInfo( Blk::IronBars, Tex::IronBars, Tex::IronBars, Tex::IronBars,
        Tex::IronBars, Tex::IronBars, Tex::IronBars, &BlockDrawer::drawPane);
    
    //Glass Pane
    setBlockInfo( Blk::GlassPane, Tex::Glass, Tex::Glass, Tex::Glass,
        Tex::Glass, Tex::Glass, Tex::Glass, &BlockDrawer::drawPane);

    //Melon, no special drawing required
    setBlockInfo( Blk::Melon, Tex::MelonSide, Tex::MelonSide, Tex::MelonTop,
        Tex::MelonTop, Tex::MelonSide, Tex::MelonSide);

    //Melon stem, height based on metadata
    setBlockInfo( Blk::PumpkinStem, Tex::MelonStem, Tex::MelonStem,
        Tex::MelonStem, Tex::MelonStem, Tex::MelonStem, Tex::MelonStem_Bent,
        &BlockDrawer::drawMelonStem);

    setBlockInfo( Blk::MelonStem, Tex::MelonStem, Tex::MelonStem,
        Tex::MelonStem, Tex::MelonStem, Tex::MelonStem, Tex::MelonStem_Bent,
        &BlockDrawer::drawMelonStem);

    /*      //Blocks left to implement
            Vines, FenceGate, StairsBrick, StairsStone, Mycelium, LilyPad,
            BrickNether, FenceNether, StairsNether, NetherWart, Enchanting,
            Brewing, Cauldron, EndPortal, EndPortalFrame, EndStone, DragonEgg,
            RedLamp, RedLampOn, DoubleSlabWood, SlabWood, CocoaPlant,
            StairsSand=128, EmeraldOre, EndChest, TripwireHook, Tripwire,
            EmeraldBlock, StairsSpruce, StairsBirch, StairsJungle, Command,
            Beacon, CobbleWall, Flowerpot, Carrots, Potatoes, ButtonWood, Head,
    */
    
    //"Metadata" blocks, variants of blocks based on metadata
    //Redwood tree
    setBlockInfo( 256 + 17, 116, 116, 21, 21, 116, 116);
    setBlockInfo( 256 + 18, 132, 132, 132, 132, 132, 132);
    //Birch tree
    setBlockInfo( 512 + 17, 117, 117, 21, 21, 117, 117);
    setBlockInfo( 512 + 18, 133, 133, 133, 133, 133, 133);
    
    //Bed: Foot (256 + Blk::Bed), adjust texture also
    //  Face texture slots adjusted clockwise, due to orientation of top
    setBlockInfo( 282, Tex::BedFoot_Face, Tex::BedFoot_Face,
        Tex::Wood, Tex::BedFoot_Top, Tex::BedFoot_Side, Tex::BedFoot_Side);
    adjustTexture(282, 0, 16-bedHeight, 0, 16, bedHeight, 16);
    
    //Bed: Head ( 256 + Blk::Bed + 1 ), adjust texture also
    //  Face texture slots adjusted clockwise, due to orientation of top
    setBlockInfo( 283, Tex::BedHead_Face, Tex::BedHead_Face,
        Tex::Wood, Tex::BedHead_Top, Tex::BedHead_Side, Tex::BedHead_Side);
    adjustTexture(283, 0, 16-bedHeight, 0, 16, bedHeight, 16);
    
    //Dyed wool (256 + 35 + metadata)
    uint16_t metaID;
    for (metaID = 291; metaID < 307; metaID++) {
        setBlockInfo( metaID, Tex::Wool, Tex::Wool, Tex::Wool, Tex::Wool,
            Tex::Wool, Tex::Wool);    //setBlockColor will assign color here
    }
    
    //Chest (256 + 54=left, 55=right, 56=left X, 57=right X)
    setBlockInfo( 310, 26, 26, 25, 25, 58, 41);
    setBlockInfo( 311, 26, 26, 25, 25, 57, 42);
    setBlockInfo( 312, 58, 41, 25, 25, 26, 26);
    setBlockInfo( 313, 57, 42, 25, 25, 26, 26);

    //Wood door = 256 + Blk::WoodDoor
    setBlockInfo( 320, 97, 97, 97, 97, 97, 97);  //bottom
    setBlockInfo( 321, 81, 81, 81, 81, 81, 81);  //top
    
    //Iron door = 256 + Blk::IronDoor
    setBlockInfo( 327, 98, 98, 98, 98, 98, 98); //bottom
    setBlockInfo( 328, 82, 82, 82, 82, 82, 82); //top

    //Trap door = 256  + Blk::TrapDoor 
    setBlockInfo( 352, Tex::TrapDoor, Tex::TrapDoor,
        Tex::TrapDoor, Tex::TrapDoor, Tex::TrapDoor, Tex::TrapDoor);
    setBlockInfo( 353, Tex::TrapDoor, Tex::TrapDoor,
        Tex::TrapDoor, Tex::TrapDoor, Tex::TrapDoor, Tex::TrapDoor);

    //Glow chest
    setBlockInfo( 256 + Blk::ChestGlow + 0, 26, 26, 25, 25, 58, 41);
    setBlockInfo( 256 + Blk::ChestGlow + 1, 26, 26, 25, 25, 57, 42);
    setBlockInfo( 256 + Blk::ChestGlow + 2, 58, 41, 25, 25, 26, 26);
    setBlockInfo( 256 + Blk::ChestGlow + 3, 57, 42, 25, 25, 26, 26);

    //http://www.minecraftwiki.net/wiki/Data_Values#Huge_brown_and_red_mushroom
    //Huge Brown Shroom: 355 = 256 + Blk::HugeBrownShroom + 0
    setBlockInfo(355, Tex::Myc_Pore, Tex::Myc_Pore,
        Tex::Myc_Pore, Tex::Myc_Pore, Tex::Myc_Pore, Tex::Myc_Pore);
    setBlockInfo(356, Tex::Cap_Brown, Tex::Myc_Pore,
        Tex::Myc_Pore, Tex::Cap_Brown, Tex::Cap_Brown, Tex::Myc_Pore);
    setBlockInfo(357, Tex::Myc_Pore, Tex::Myc_Pore,
        Tex::Myc_Pore, Tex::Cap_Brown, Tex::Cap_Brown, Tex::Myc_Pore);
    setBlockInfo(358, Tex::Myc_Pore, Tex::Cap_Brown,
        Tex::Myc_Pore, Tex::Cap_Brown, Tex::Cap_Brown, Tex::Myc_Pore);
    setBlockInfo(359, Tex::Cap_Brown, Tex::Myc_Pore,
        Tex::Myc_Pore, Tex::Cap_Brown, Tex::Myc_Pore, Tex::Myc_Pore);
    setBlockInfo(360, Tex::Myc_Pore, Tex::Myc_Pore,
        Tex::Myc_Pore, Tex::Cap_Brown, Tex::Myc_Pore, Tex::Myc_Pore);
    setBlockInfo(361, Tex::Myc_Pore, Tex::Cap_Brown,
        Tex::Myc_Pore, Tex::Cap_Brown, Tex::Myc_Pore, Tex::Myc_Pore);
    setBlockInfo(362, Tex::Cap_Brown, Tex::Myc_Pore,
        Tex::Myc_Pore, Tex::Cap_Brown, Tex::Myc_Pore, Tex::Cap_Brown);
    setBlockInfo(363, Tex::Myc_Pore, Tex::Myc_Pore,
        Tex::Myc_Pore, Tex::Cap_Brown, Tex::Myc_Pore, Tex::Cap_Brown);
    setBlockInfo(364, Tex::Myc_Pore, Tex::Cap_Brown,
        Tex::Myc_Pore, Tex::Cap_Brown, Tex::Myc_Pore, Tex::Cap_Brown);
    setBlockInfo(365, Tex::Myc_Stem, Tex::Myc_Stem,
        Tex::Myc_Pore, Tex::Myc_Pore, Tex::Myc_Stem, Tex::Myc_Stem);
    setBlockInfo(369, Tex::Myc_Stem, Tex::Myc_Stem,
        Tex::Myc_Stem, Tex::Myc_Stem, Tex::Myc_Stem, Tex::Myc_Stem);
    setBlockInfo(370, Tex::Cap_Brown, Tex::Cap_Brown,
        Tex::Cap_Brown, Tex::Cap_Brown, Tex::Cap_Brown, Tex::Cap_Brown);

    //Huge Red shroom: 370 = 265 + Blk::HugeBrownShroom + 16
    setBlockInfo(371, Tex::Myc_Pore, Tex::Myc_Pore,
        Tex::Myc_Pore, Tex::Myc_Pore, Tex::Myc_Pore, Tex::Myc_Pore);
    setBlockInfo(372, Tex::Cap_Red, Tex::Myc_Pore,
        Tex::Myc_Pore, Tex::Cap_Red, Tex::Cap_Red, Tex::Myc_Pore);
    setBlockInfo(373, Tex::Myc_Pore, Tex::Myc_Pore,
        Tex::Myc_Pore, Tex::Cap_Red, Tex::Cap_Red, Tex::Myc_Pore);
    setBlockInfo(374, Tex::Myc_Pore, Tex::Cap_Red,
        Tex::Myc_Pore, Tex::Cap_Red, Tex::Cap_Red, Tex::Myc_Pore);
    setBlockInfo(375, Tex::Cap_Red, Tex::Myc_Pore,
        Tex::Myc_Pore, Tex::Cap_Red, Tex::Myc_Pore, Tex::Myc_Pore);
    setBlockInfo(376, Tex::Myc_Pore, Tex::Myc_Pore,
        Tex::Myc_Pore, Tex::Cap_Red, Tex::Myc_Pore, Tex::Myc_Pore);
    setBlockInfo(377, Tex::Myc_Pore, Tex::Cap_Red,
        Tex::Myc_Pore, Tex::Cap_Red, Tex::Myc_Pore, Tex::Myc_Pore);
    setBlockInfo(378, Tex::Cap_Red, Tex::Myc_Pore,
        Tex::Myc_Pore, Tex::Cap_Red, Tex::Myc_Pore, Tex::Cap_Red);
    setBlockInfo(379, Tex::Myc_Pore, Tex::Myc_Pore,
        Tex::Myc_Pore, Tex::Cap_Red, Tex::Myc_Pore, Tex::Cap_Red);
    setBlockInfo(380, Tex::Myc_Pore, Tex::Cap_Red,
        Tex::Myc_Pore, Tex::Cap_Red, Tex::Myc_Pore, Tex::Cap_Red);
    setBlockInfo(381, Tex::Myc_Stem, Tex::Myc_Stem,
        Tex::Myc_Pore, Tex::Myc_Pore, Tex::Myc_Stem, Tex::Myc_Stem);
    setBlockInfo(387, Tex::Myc_Stem, Tex::Myc_Stem,
        Tex::Myc_Stem, Tex::Myc_Stem, Tex::Myc_Stem, Tex::Myc_Stem);
    setBlockInfo(388, Tex::Cap_Red, Tex::Cap_Red,
        Tex::Cap_Red, Tex::Cap_Red, Tex::Cap_Red, Tex::Cap_Red);

    //On the x axis: A = left,    B = right
    //On the y axis: C = down,    D = up
    //On the z axis: E = farther, F = closer

    //Adjust cuboid shapes of blocks.  Needed for blocks that are not cubes.

    return true;
}

//Swap texture cordinates
void BlockDrawer::mirrorCoords( GLfloat& tx_0, GLfloat& tx_1,
                GLfloat& ty_0, GLfloat& ty_1, uint8_t mirror_type) const
{
    GLfloat t_mirror;
    
    if (mirror_type & 2) { t_mirror = tx_0; tx_0 = tx_1; tx_1 = t_mirror; }
    if (mirror_type & 1) { t_mirror = ty_0; ty_0 = ty_1; ty_1 = t_mirror; }
    
}

//Look up texture coordinates for every face of block/other
bool BlockDrawer::getTexInfo(uint16_t blockID, GLfloat tx_0[6], GLfloat tx_1[6],
    GLfloat ty_0[6], GLfloat ty_1[6]) const
{

    uint16_t index;
    for (index = 0; index < 6; index++) {
        
        //Get texture coordinates from TextureInfo
        TextureInfo *pti = texInfo[ blockInfo[blockID].textureID[index] ];
        if (pti == NULL) { return false; }
        pti->getCoords( tx_0[index], tx_1[index], ty_0[index], ty_1[index]);
    }

    return true;
}

//Shortcut for getting one set of texture coordinates from texture ID
bool BlockDrawer::getTexCoords(uint8_t blockID, face_ID faceID,
    GLfloat& tx_0, GLfloat& tx_1, GLfloat& ty_0, GLfloat& ty_1) const
{
    //Return sponge texture if missing texture info
    TextureInfo *tinfo = texInfo[blockInfo[blockID].textureID[faceID]];
    if ( tinfo == NULL) {
        //Return sponge texture coordinates
        uint16_t tex = Tex::Sponge;
        
        //Texture map coordinates in terrain.png (0.0 - 1.0)
        tx_0 = float(tex & (texmap_TILES-1))/((float)texmap_TILES);
        tx_1 = tx_0 + tmr;
        ty_0 = float(tex/texmap_TILES)/((float)texmap_TILES);
        ty_1 = ty_0 + tmr;

        //You may need to flip Y coords ty_0 and ty_1

        return false;
    }
    
    //Texture map coordinates in terrain.png (0.0 - 1.0)
    tx_0 = tinfo->tx_0;
    tx_1 = tinfo->tx_1;
    ty_1 = tinfo->ty_0; //flip Y
    ty_0 = tinfo->ty_1; //flip Y
    
    return true;
}

//Assign vertices for specified cuboid, facing towards direction
void BlockDrawer::makeCuboidVertex(GLint x0, GLint y0, GLint z0,
    GLsizei width, GLsizei height, GLsizei depth,
    GLint vX[8], GLint vY[8], GLint vZ[8],
    face_ID facing
) const
{
    //Cuboid bounds
    GLint A = (x0 << 4);
    GLint B = (x0 << 4) + width;
    GLint C = (y0 << 4);
    GLint D = (y0 << 4) + height;
    GLint E = (z0 << 4);
    GLint F = (z0 << 4) + depth;

    //Y values of vertices for LEFT, RIGHT, BACK, FRONT
    vY[0] = C; vY[2] = C; vY[3] = D; vY[1] = D;
    vY[6] = C; vY[4] = C; vY[5] = D; vY[7] = D;

    switch (facing) {
        case LEFT: //Facing -X (A side)
            //left side: 0, 2, 3, 1
            vX[0] = B; vX[2] = A; vX[3] = A; vX[1] = B;
            vZ[0] = E; vZ[2] = E; vZ[3] = E; vZ[1] = E;
        
            //right side: 6, 4, 5, 7
            vX[6] = A; vX[4] = B; vX[5] = B; vX[7] = A; 
            vZ[6] = F; vZ[4] = F; vZ[5] = F; vZ[7] = F;
            
            break;
        case RIGHT: //Facing +X (B side)
            //left side: 0, 2, 3, 1
            vX[0] = A; vX[2] = B; vX[3] = B; vX[1] = A; 
            vZ[0] = F; vZ[2] = F; vZ[3] = F; vZ[1] = F;
        
            //right side: 6, 4, 5, 7
            vX[6] = B; vX[4] = A; vX[5] = A; vX[7] = B;
            vZ[6] = E; vZ[4] = E; vZ[5] = E; vZ[7] = E;
            
            break;
        case BOTTOM: //Facing -Y (C side)
            //left side: 0, 2, 3, 1
            vX[0] = A; vX[2] = A; vX[3] = A; vX[1] = A; 
            vY[0] = D; vY[2] = C; vY[3] = C; vY[1] = D;
            vZ[0] = E; vZ[2] = E; vZ[3] = F; vZ[1] = F;
        
            //right side: 6, 4, 5, 7
            vX[6] = B; vX[4] = B; vX[5] = B; vX[7] = B;
            vY[6] = C; vY[4] = D; vY[5] = D; vY[7] = C;
            vZ[6] = E; vZ[4] = E; vZ[5] = F; vZ[7] = F;
            
            break;
        case TOP: //Facing +Y (D side)
            //left side: 0, 2, 3, 1
            vX[0] = A; vX[2] = A; vX[3] = A; vX[1] = A;
            vY[0] = C; vY[2] = D; vY[3] = D; vY[1] = C;
            vZ[0] = F; vZ[2] = F; vZ[3] = E; vZ[1] = E;
        
            //right side: 6, 4, 5, 7
            vX[6] = B; vX[4] = B; vX[5] = B; vX[7] = B; 
            vY[6] = D; vY[4] = C; vY[5] = C; vY[7] = D;
            vZ[6] = F; vZ[4] = F; vZ[5] = E; vZ[7] = E;
            
            break;
        case BACK: //Facing +Z (E side)
            //left side: 0, 2, 3, 1
            vX[0] = B; vX[2] = B; vX[3] = B; vX[1] = B;
            vZ[0] = F; vZ[2] = E; vZ[3] = E; vZ[1] = F;
        
            //right side: 6, 4, 5, 7
            vX[6] = A; vX[4] = A; vX[5] = A; vX[7] = A;
            vZ[6] = E; vZ[4] = F; vZ[5] = F; vZ[7] = E;
            break;
        default:
        case FRONT: //Facing -Z
            //left side: 0, 2, 3, 1
            vX[0] = A; vX[2] = A; vX[3] = A; vX[1] = A;
            vZ[0] = E; vZ[2] = F; vZ[3] = F; vZ[1] = E;
        
            //right side: 6, 4, 5, 7
            vX[6] = B; vX[4] = B; vX[5] = B; vX[7] = B; 
            vZ[6] = F; vZ[4] = E; vZ[5] = E; vZ[7] = F;
            break;
    }


}

//Add small offset to the OpenGL vertices.
void BlockDrawer::addVertexOffset( GLint vX[8], GLint vY[8], GLint vZ[8],
        GLint dX, GLint dY, GLint dZ) const
{
    size_t i;
    for (i = 0; i < 8; i++) {
        vX[i] += dX;
        vY[i] += dY;
        vZ[i] += dZ;
    }
}