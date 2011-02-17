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
    face is on an axis, and is "high" or "low" compared to the opposing face.
     
    On the x axis: A = left,    B = right
    On the y axis: C = down,    D = up
    On the z axis: E = farther, F = closer
    
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
    
*/

//libmc--c
#include "BlockDrawer.hpp"
using mc__::BlockDrawer;
using mc__::face_ID;
using mc__::World;

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

BlockDrawer::BlockDrawer(World* w, GLuint t_tex, GLuint i_tex):
    world(w), terrain_tex(t_tex), item_tex(i_tex)
{
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
void BlockDrawer::rebindTerrain()
{
    //glBind texture before assigning it
    glBindTexture(GL_TEXTURE_2D, terrain_tex);
}


//Set glColor if needed by block type and face
void BlockDrawer::setBlockColor(uint8_t blockID, face_ID face) const
{
    //return; Uncomment to disable block coloring
    
    GLubyte red, green, blue;
    switch (blockID) {
        case 0:
            glColor3ub( 255, 255, 255);
            break;
        case 2:     //Grass
            if (face == mc__::UP) {
                red=grass_color[0]; green=grass_color[1]; blue=grass_color[2];
            } else {
                red=255; green=255; blue=255;
            }
            glColor3ub( red, green, blue);
            break;
        case 18:    //Leaves
            red=leaf_color[0]; green=leaf_color[1]; blue=leaf_color[2];
            glColor3ub( red, green, blue);
            break;
        default:
            red=255; green=255; blue=255;
            break;
    }
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
        tx_0 = blockInfo[blockID].tx[WEST];
        tx_1 = tx_0 + tmr;
        ty_1 = blockInfo[blockID].ty[WEST];
        ty_0 = ty_1 + tmr;    //flip y
        setBlockColor(blockID, WEST);  //Set leaf/grass color if needed
        
        glTexCoord2f(tx_0,ty_0); glVertex3i( A, C, E);  //Lower left:  ACE
        glTexCoord2f(tx_1,ty_0); glVertex3i( A, C, F);  //Lower right: ACF
        glTexCoord2f(tx_1,ty_1); glVertex3i( A, D, F);  //Top right:   ADF
        glTexCoord2f(tx_0,ty_1); glVertex3i( A, D, E);  //Top left:    ADE
    }

    //B
    if (!(vflags & 0x40)) {
        tx_0 = blockInfo[blockID].tx[EAST];
        tx_1 = tx_0 + tmr;
        ty_1 = blockInfo[blockID].ty[EAST];
        ty_0 = ty_1 + tmr;    //flip y
        setBlockColor(blockID, EAST);  //Set leaf/grass color if needed
        
        glTexCoord2f(tx_0,ty_0); glVertex3i( B, C, F);  //Lower left:  BCF
        glTexCoord2f(tx_1,ty_0); glVertex3i( B, C, E);  //Lower right: BCE
        glTexCoord2f(tx_1,ty_1); glVertex3i( B, D, E);  //Top right:   BDE
        glTexCoord2f(tx_0,ty_1); glVertex3i( B, D, F);  //Top left:    BDF
    }
    
    //C
    if (!(vflags & 0x20)) {
        tx_0 = blockInfo[blockID].tx[DOWN];
        tx_1 = tx_0 + tmr;
        ty_1 = blockInfo[blockID].ty[DOWN];
        ty_0 = ty_1 + tmr;    //flip y
        setBlockColor(blockID, DOWN);  //Set leaf/grass color if needed
        
        glTexCoord2f(tx_0,ty_0); glVertex3i( A, C, E);  //Lower left:  ACE
        glTexCoord2f(tx_1,ty_0); glVertex3i( B, C, E);  //Lower right: BCE
        glTexCoord2f(tx_1,ty_1); glVertex3i( B, C, F);  //Top right:   BCF
        glTexCoord2f(tx_0,ty_1); glVertex3i( A, C, F);  //Top left:    ACF
    }
    
    //D
    if (!(vflags & 0x10)) {
        tx_0 = blockInfo[blockID].tx[UP];
        tx_1 = tx_0 + tmr;
        ty_1 = blockInfo[blockID].ty[UP];
        ty_0 = ty_1 + tmr;    //flip y
        setBlockColor(blockID, UP);  //Set leaf/grass color if needed
    
        glTexCoord2f(tx_0,ty_0); glVertex3i( A, D, F);  //Lower left:  ADF
        glTexCoord2f(tx_1,ty_0); glVertex3i( B, D, F);  //Lower right: BDF
        glTexCoord2f(tx_1,ty_1); glVertex3i( B, D, E);  //Top right:   BDE
        glTexCoord2f(tx_0,ty_1); glVertex3i( A, D, E);  //Top left:    ADE
    }
    
    //E
    if (!(vflags & 0x08)) {
        tx_0 = blockInfo[blockID].tx[NORTH];
        tx_1 = tx_0 + tmr;
        ty_1 = blockInfo[blockID].ty[NORTH];
        ty_0 = ty_1 + tmr;    //flip y
        setBlockColor(blockID, NORTH);  //Set leaf/grass color if needed
        
        glTexCoord2f(tx_0,ty_0); glVertex3i( B, C, E);  //Lower left:  BCE
        glTexCoord2f(tx_1,ty_0); glVertex3i( A, C, E);  //Lower right: ACE
        glTexCoord2f(tx_1,ty_1); glVertex3i( A, D, E);  //Top right:   ADE
        glTexCoord2f(tx_0,ty_1); glVertex3i( B, D, E);  //Top left:    BDE
    }
    
    //F
    if (!(vflags & 0x04)) {
        tx_0 = blockInfo[blockID].tx[SOUTH];
        tx_1 = tx_0 + tmr;
        ty_1 = blockInfo[blockID].ty[SOUTH];
        ty_0 = ty_1 + tmr;    //flip y
        setBlockColor(blockID, SOUTH);  //Set leaf/grass color if needed
        
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

    //Use "WEST" as "side", "SOUTH" as "face" setBlock
    mc__::face_ID west=WEST, east=WEST, north=WEST, south=WEST;
    
    //Choose face depending on metadata
    switch (meta) {
        case 2:
            east = SOUTH; break;
        case 3:
            west = SOUTH; break;
        case 4:
            north = SOUTH; break;
        case 5:
            south = SOUTH; break;
        default:
            south = SOUTH; break;
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
        tx_0 = blockInfo[blockID].tx[DOWN];
        tx_1 = blockInfo[blockID].tx[DOWN] + tmr;
        ty_0 = blockInfo[blockID].ty[DOWN] + tmr;
        ty_1 = blockInfo[blockID].ty[DOWN];
        
        glTexCoord2f(tx_0,ty_0); glVertex3i( A, C, E);  //Lower left:  ACE
        glTexCoord2f(tx_1,ty_0); glVertex3i( B, C, E);  //Lower right: BCE
        glTexCoord2f(tx_1,ty_1); glVertex3i( B, C, F);  //Top right:   BCF
        glTexCoord2f(tx_0,ty_1); glVertex3i( A, C, F);  //Top left:    ACF
    }
    
    //D
    if (!(vflags & 0x10)) {
        tx_0 = blockInfo[blockID].tx[UP];
        tx_1 = blockInfo[blockID].tx[UP] + tmr;
        ty_0 = blockInfo[blockID].ty[UP] + tmr;
        ty_1 = blockInfo[blockID].ty[UP];
    
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
    
    //Face coordinates (in pixels)
    GLint A = (x << 4) + 0;
    GLint B = (x << 4) + texmap_TILE_LENGTH;
    GLint C = (y << 4) + 0;
    GLint D = (y << 4) + meta + 1/*texmap_TILE_LENGTH*/;
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

    // For cactus, the face coordinates are inset from the face
    GLfloat tmr_y = tmr*(meta + 1.0)/16.0;

    //A always visible
        tx_0 = blockInfo[blockID].tx[WEST];
        tx_1 = blockInfo[blockID].tx[WEST] + tmr;
        ty_0 = blockInfo[blockID].ty[WEST] + tmr_y;    //flip y
        ty_1 = blockInfo[blockID].ty[WEST];
        setBlockColor(blockID, WEST);  //Set leaf/grass color if needed
        
        glTexCoord2f(tx_0,ty_0); glVertex3i( A+1, C, E);
        glTexCoord2f(tx_1,ty_0); glVertex3i( A+1, C, F);
        glTexCoord2f(tx_1,ty_1); glVertex3i( A+1, D, F);
        glTexCoord2f(tx_0,ty_1); glVertex3i( A+1, D, E);

    //B
        tx_0 = blockInfo[blockID].tx[EAST];
        tx_1 = blockInfo[blockID].tx[EAST] + tmr;
        ty_0 = blockInfo[blockID].ty[EAST] + tmr_y;
        ty_1 = blockInfo[blockID].ty[EAST];
        setBlockColor(blockID, EAST);  //Set leaf/grass color if needed
        
        glTexCoord2f(tx_0,ty_0); glVertex3i( B-1, C, F);  //Lower left:  BCF
        glTexCoord2f(tx_1,ty_0); glVertex3i( B-1, C, E);  //Lower right: BCE
        glTexCoord2f(tx_1,ty_1); glVertex3i( B-1, D, E);  //Top right:   BDE
        glTexCoord2f(tx_0,ty_1); glVertex3i( B-1, D, F);  //Top left:    BDF
    
    //C not always visible
    if (!(vflags & 0x20)) {
        tx_0 = blockInfo[blockID].tx[DOWN];
        tx_1 = blockInfo[blockID].tx[DOWN] + tmr;
        ty_0 = blockInfo[blockID].ty[DOWN] + tmr;
        ty_1 = blockInfo[blockID].ty[DOWN];
        setBlockColor(blockID, DOWN);  //Set leaf/grass color if needed
        
        glTexCoord2f(tx_0,ty_0); glVertex3i( A, C, E);  //Lower left:  ACE
        glTexCoord2f(tx_1,ty_0); glVertex3i( B, C, E);  //Lower right: BCE
        glTexCoord2f(tx_1,ty_1); glVertex3i( B, C, F);  //Top right:   BCF
        glTexCoord2f(tx_0,ty_1); glVertex3i( A, C, F);  //Top left:    ACF
    }
    
    //D
    if (!(vflags & 0x10)) {
        tx_0 = blockInfo[blockID].tx[UP];
        tx_1 = blockInfo[blockID].tx[UP] + tmr;
        ty_0 = blockInfo[blockID].ty[UP] + tmr;
        ty_1 = blockInfo[blockID].ty[UP];
        setBlockColor(blockID, UP);  //Set leaf/grass color if needed
    
        glTexCoord2f(tx_0,ty_0); glVertex3i( A, D, F);  //Lower left:  ADF
        glTexCoord2f(tx_1,ty_0); glVertex3i( B, D, F);  //Lower right: BDF
        glTexCoord2f(tx_1,ty_1); glVertex3i( B, D, E);  //Top right:   BDE
        glTexCoord2f(tx_0,ty_1); glVertex3i( A, D, E);  //Top left:    ADE
    }
    
    //E always visible
        tx_0 = blockInfo[blockID].tx[NORTH];
        tx_1 = blockInfo[blockID].tx[NORTH] + tmr;
        ty_0 = blockInfo[blockID].ty[NORTH] + tmr_y;
        ty_1 = blockInfo[blockID].ty[NORTH];
        setBlockColor(blockID, NORTH);  //Set leaf/grass color if needed
        
        glTexCoord2f(tx_0,ty_0); glVertex3i( B, C, E+1);  //Lower left:  BCE
        glTexCoord2f(tx_1,ty_0); glVertex3i( A, C, E+1);  //Lower right: ACE
        glTexCoord2f(tx_1,ty_1); glVertex3i( A, D, E+1);  //Top right:   ADE
        glTexCoord2f(tx_0,ty_1); glVertex3i( B, D, E+1);  //Top left:    BDE
    
    //F
        tx_0 = blockInfo[blockID].tx[SOUTH];
        tx_1 = blockInfo[blockID].tx[SOUTH] + tmr;
        ty_0 = blockInfo[blockID].ty[SOUTH] + tmr_y;
        ty_1 = blockInfo[blockID].ty[SOUTH];
        setBlockColor(blockID, SOUTH);  //Set leaf/grass color if needed
        
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
    mc__::face_ID leftTex = EAST;       //"unsliced" side texture
    GLint A_offset = A + offset;    //Offset by 1 for transparent texture
    if (meta > 0) {
        leftTex = WEST; //Use "sliced" texture on left side, if eaten
        A_offset = A;   //No offset for transparent texture
    } 
    
    //Texture coordinates
    tx_0 = blockInfo[blockID].tx[leftTex];
    tx_1 = blockInfo[blockID].tx[leftTex] + tmr;
    ty_0 = blockInfo[blockID].ty[leftTex] + tmr;    //flip y
    ty_1 = blockInfo[blockID].ty[leftTex];
    
    glTexCoord2f(tx_0,ty_0); glVertex3i( A_offset, C - half, E);
    glTexCoord2f(tx_1,ty_0); glVertex3i( A_offset, C - half, F);
    glTexCoord2f(tx_1,ty_1); glVertex3i( A_offset, D - half, F);
    glTexCoord2f(tx_0,ty_1); glVertex3i( A_offset, D - half, E);

    //B
    tx_0 = blockInfo[blockID].tx[EAST];
    tx_1 = blockInfo[blockID].tx[EAST] + tmr;
    ty_0 = blockInfo[blockID].ty[EAST] + tmr;
    ty_1 = blockInfo[blockID].ty[EAST];
    
    glTexCoord2f(tx_0,ty_0); glVertex3i( B - offset, C - half, F);
    glTexCoord2f(tx_1,ty_0); glVertex3i( B - offset, C - half, E);
    glTexCoord2f(tx_1,ty_1); glVertex3i( B - offset, D - half, E);
    glTexCoord2f(tx_0,ty_1); glVertex3i( B - offset, D - half, F);

    //C (might be blocked from below)
    if (!(vflags & 0x20)) {
        tx_0 = blockInfo[blockID].tx[DOWN] + tmr_eat;
        tx_1 = blockInfo[blockID].tx[DOWN] + tmr;
        ty_0 = blockInfo[blockID].ty[DOWN] + tmr;
        ty_1 = blockInfo[blockID].ty[DOWN];
        
        glTexCoord2f(tx_0,ty_0); glVertex3i( A, C, E);
        glTexCoord2f(tx_1,ty_0); glVertex3i( B, C, E);
        glTexCoord2f(tx_1,ty_1); glVertex3i( B, C, F);
        glTexCoord2f(tx_0,ty_1); glVertex3i( A, C, F);
    }
    
    //D
    tx_0 = blockInfo[blockID].tx[UP] + tmr_eat;
    tx_1 = blockInfo[blockID].tx[UP] + tmr;
    ty_0 = blockInfo[blockID].ty[UP] + tmr;
    ty_1 = blockInfo[blockID].ty[UP];

    glTexCoord2f(tx_0,ty_0); glVertex3i( A, D - half, F);
    glTexCoord2f(tx_1,ty_0); glVertex3i( B, D - half, F);
    glTexCoord2f(tx_1,ty_1); glVertex3i( B, D - half, E);
    glTexCoord2f(tx_0,ty_1); glVertex3i( A, D - half, E);

    //E always visible
    tx_0 = blockInfo[blockID].tx[NORTH];
    tx_1 = blockInfo[blockID].tx[NORTH] + tmr - tmr_eat;
    ty_0 = blockInfo[blockID].ty[NORTH] + tmr;
    ty_1 = blockInfo[blockID].ty[NORTH];
    
    glTexCoord2f(tx_0,ty_0); glVertex3i( B, C - half, E + offset);
    glTexCoord2f(tx_1,ty_0); glVertex3i( A, C - half, E + offset);
    glTexCoord2f(tx_1,ty_1); glVertex3i( A, D - half, E + offset);
    glTexCoord2f(tx_0,ty_1); glVertex3i( B, D - half, E + offset);

    //F
    tx_0 = blockInfo[blockID].tx[SOUTH] + tmr_eat;
    tx_1 = blockInfo[blockID].tx[SOUTH] + tmr;
    ty_0 = blockInfo[blockID].ty[SOUTH] + tmr;
    ty_1 = blockInfo[blockID].ty[SOUTH];
    
    glTexCoord2f(tx_0,ty_0); glVertex3i( A, C - half, F - offset);
    glTexCoord2f(tx_1,ty_0); glVertex3i( B, C - half, F - offset);
    glTexCoord2f(tx_1,ty_1); glVertex3i( B, D - half, F - offset);
    glTexCoord2f(tx_0,ty_1); glVertex3i( A, D - half, F - offset);
    
}

//Use OpenGL to draw partial solid cube, with offsets, scale, mirroring
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
    
    //Face coordinates (in pixels)
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
        tx_0 = blockInfo[blockID].tx[WEST] + tmr_off_z;
        tx_1 = blockInfo[blockID].tx[WEST] + tmr_off_z + tmr_z;
        ty_0 = blockInfo[blockID].ty[WEST] + tmr_off_y + tmr_y;
        ty_1 = blockInfo[blockID].ty[WEST] + tmr_off_y;
        
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
        tx_0 = blockInfo[blockID].tx[EAST] + tmr_off_z + tmr_z;
        tx_1 = blockInfo[blockID].tx[EAST] + tmr_off_z;
        ty_0 = blockInfo[blockID].ty[EAST] + tmr_off_y + tmr_y;
        ty_1 = blockInfo[blockID].ty[EAST] + tmr_off_y;
        
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
        tx_0 = blockInfo[blockID].tx[DOWN] + tmr_off_x;
        tx_1 = blockInfo[blockID].tx[DOWN] + tmr_off_x + tmr_x;
        ty_0 = blockInfo[blockID].ty[DOWN] + tmr_off_z + tmr_z;
        ty_1 = blockInfo[blockID].ty[DOWN] + tmr_off_z;
        
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
        tx_0 = blockInfo[blockID].tx[UP] + tmr_off_x;
        tx_1 = blockInfo[blockID].tx[UP] + tmr_off_x + tmr_x;
        ty_0 = blockInfo[blockID].ty[UP] + tmr_off_z + tmr_z;
        ty_1 = blockInfo[blockID].ty[UP] + tmr_off_z;
        
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
        tx_0 = blockInfo[blockID].tx[NORTH] + tmr_off_x + tmr_x;
        tx_1 = blockInfo[blockID].tx[NORTH] + tmr_off_x;
        ty_0 = blockInfo[blockID].ty[NORTH] + tmr_off_y + tmr_y;
        ty_1 = blockInfo[blockID].ty[NORTH] + tmr_off_y;
        
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
        tx_0 = blockInfo[blockID].tx[SOUTH] + tmr_off_x;
        tx_1 = blockInfo[blockID].tx[SOUTH] + tmr_off_x + tmr_x;
        ty_0 = blockInfo[blockID].ty[SOUTH] + tmr_off_y + tmr_y;
        ty_1 = blockInfo[blockID].ty[SOUTH] + tmr_off_y;
        
        //Swap coordinates if needed
        if (mirror & 0x04) { mirrorCoords(tx_0, tx_1, ty_0, ty_1); }
        
        glTexCoord2f(tx_0,ty_0); glVertex3i( A, C, F);  //Lower left:  ACF
        glTexCoord2f(tx_1,ty_0); glVertex3i( B, C, F);  //Lower right: BCF
        glTexCoord2f(tx_1,ty_1); glVertex3i( B, D, F);  //Top right:   BDF
        glTexCoord2f(tx_0,ty_1); glVertex3i( A, D, F);  //Top left:    ADF
    }
}

//Draw half a block
void BlockDrawer::drawHalfBlock( uint8_t blockID, uint8_t meta,
    GLint x, GLint y, GLint z, uint8_t vflags) const
{
    //Use metadata to determine which half!
    if (meta) {
        //Draw top half
        drawScaledBlock( blockID, meta, x, y, z, vflags, 1, 0.5, 1,
            true, 0, 8, 0);
    } else {
        //Draw bottom half
        drawScaledBlock( blockID, meta, x, y, z, vflags, 1, 0.5, 1);
    }
}

//Draw minecart track (meta affects angle, direction, intersection)
void BlockDrawer::drawTrack( uint8_t blockID, uint8_t meta,
    GLint x, GLint y, GLint z, uint8_t /*vflags*/) const
{
    //TODO: metadata to determine track type and orientation

    //Texture map coordinates (0.0 - 1.0)
    GLfloat tx_0, tx_1, ty_0, ty_1;

    //Object boundaries... flat square 1 pixel off the ground
    GLint A = (x << 4) + 0;
    GLint B = (x << 4) + texmap_TILE_LENGTH;
    GLint C = (y << 4) + 0;
    GLint D = (y << 4) + 1;
    GLint E = (z << 4) + 0;
    GLint F = (z << 4) + texmap_TILE_LENGTH;

    //C
    tx_0 = blockInfo[blockID].tx[DOWN];
    tx_1 = blockInfo[blockID].tx[DOWN] + tmr;
    ty_0 = blockInfo[blockID].ty[DOWN] + tmr;
    ty_1 = blockInfo[blockID].ty[DOWN];
    
    glTexCoord2f(tx_0,ty_0); glVertex3i( A, C, E);  //Lower left:  ACE
    glTexCoord2f(tx_1,ty_0); glVertex3i( B, C, E);  //Lower right: BCE
    glTexCoord2f(tx_1,ty_1); glVertex3i( B, C, F);  //Top right:   BCF
    glTexCoord2f(tx_0,ty_1); glVertex3i( A, C, F);  //Top left:    ACF

    //D
    tx_0 = blockInfo[blockID].tx[UP];
    tx_1 = blockInfo[blockID].tx[UP] + tmr;
    ty_0 = blockInfo[blockID].ty[UP] + tmr;
    ty_1 = blockInfo[blockID].ty[UP];

    glTexCoord2f(tx_0,ty_0); glVertex3i( A, D, F);  //Lower left:  ADF
    glTexCoord2f(tx_1,ty_0); glVertex3i( B, D, F);  //Lower right: BDF
    glTexCoord2f(tx_1,ty_1); glVertex3i( B, D, E);  //Top right:   BDE
    glTexCoord2f(tx_0,ty_1); glVertex3i( A, D, E);  //Top left:    ADE

}

//Draw item blockID which is placed flat on the wall
void BlockDrawer::drawWallItem( uint8_t blockID, uint8_t meta,
    GLint x, GLint y, GLint z, uint8_t /*vflags*/) const
{
    //Texture coords
    GLfloat tx_0 = blockInfo[blockID].tx[WEST];
    GLfloat tx_1 = blockInfo[blockID].tx[WEST] + tmr;
    GLfloat ty_0 = blockInfo[blockID].ty[WEST] + tmr;
    GLfloat ty_1 = blockInfo[blockID].ty[WEST];

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
    tx_0 = blockInfo[blockID].tx[WEST];
    tx_1 = blockInfo[blockID].tx[WEST] + tmr;
    ty_0 = blockInfo[blockID].ty[WEST] + tmr;    //flip y
    ty_1 = blockInfo[blockID].ty[WEST];
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
    tx_0 = blockInfo[blockID].tx[EAST];
    tx_1 = blockInfo[blockID].tx[EAST] + tmr;
    ty_0 = blockInfo[blockID].ty[EAST] + tmr;    //flip y
    ty_1 = blockInfo[blockID].ty[EAST];
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
    GLfloat tx_0 = blockInfo[blockID].tx[WEST];
    GLfloat tx_1 = blockInfo[blockID].tx[WEST] + tmr;
    GLfloat ty_0 = blockInfo[blockID].ty[WEST] + tmr;    //flip y
    GLfloat ty_1 = blockInfo[blockID].ty[WEST];
    
    GLfloat tx_m1 = blockInfo[blockID].tx[WEST] + tmr*7.0/16.0;
    GLfloat tx_m2 = blockInfo[blockID].tx[WEST] + tmr*9.0/16.0;
    GLfloat ty_m1 = blockInfo[blockID].ty[WEST] + tmr*8.0/16.0;
    GLfloat ty_m2 = blockInfo[blockID].ty[WEST] + tmr*6.0/16.0;

    //Torch location depends on metadata
    switch(meta) {
        //TODO: metadata for walls
        case 1:
        case 2:
        case 3:
        case 4:
        case 5:     //Torch on ground, center of block
        default:
        
            //Cube boundaries
            GLint A = (x << 4) + 0;
            GLint B = (x << 4) + texmap_TILE_LENGTH;
            GLint C = (y << 4) + 0;
            GLint D = (y << 4) + texmap_TILE_LENGTH;
            GLint E = (z << 4) + 0;
            GLint F = (z << 4) + texmap_TILE_LENGTH;
            
            //middle torch boundaries
            GLint G = A + 7;
            GLint H = A + 9;
            GLint I = E + 7;
            GLint J = E + 9;
            GLint K = C + 10;
            
            //A side
            glTexCoord2f(tx_0,ty_0); glVertex3i( G, C, E);  //Lower left:  ACE
            glTexCoord2f(tx_1,ty_0); glVertex3i( G, C, F);  //Lower right: ACF
            glTexCoord2f(tx_1,ty_1); glVertex3i( G, D, F);  //Top right:   ADF
            glTexCoord2f(tx_0,ty_1); glVertex3i( G, D, E);  //Top left:    ADE
            //B side
            glTexCoord2f(tx_0,ty_0); glVertex3i( H, C, F);  //Lower left
            glTexCoord2f(tx_1,ty_0); glVertex3i( H, C, E);  //Lower right
            glTexCoord2f(tx_1,ty_1); glVertex3i( H, D, E);  //Top right
            glTexCoord2f(tx_0,ty_1); glVertex3i( H, D, F);  //Top left
            //E side
            glTexCoord2f(tx_0,ty_0); glVertex3i( B, C, I);  //Lower left:  BCE
            glTexCoord2f(tx_1,ty_0); glVertex3i( A, C, I);  //Lower right: ACE
            glTexCoord2f(tx_1,ty_1); glVertex3i( A, D, I);  //Top right:   ADE
            glTexCoord2f(tx_0,ty_1); glVertex3i( B, D, I);  //Top left:    BDE
            //F side
            glTexCoord2f(tx_0,ty_0); glVertex3i( A, C, J);  //Lower left:  ACF
            glTexCoord2f(tx_1,ty_0); glVertex3i( B, C, J);  //Lower right: BCF
            glTexCoord2f(tx_1,ty_1); glVertex3i( B, D, J);  //Top right:   BDF
            glTexCoord2f(tx_0,ty_1); glVertex3i( A, D, J);  //Top left:    ADF
            
            //Top side (D)
            glTexCoord2f(tx_m1,ty_m1); glVertex3i( G, K, J);  //Lower left:  ADF
            glTexCoord2f(tx_m2,ty_m1); glVertex3i( H, K, J);  //Lower right: BDF
            glTexCoord2f(tx_m2,ty_m2); glVertex3i( H, K, I);  //Top right:   BDE
            glTexCoord2f(tx_m1,ty_m2); glVertex3i( G, K, I);  //Top left:    ADE
    }
}

//Draw fire burning (use item texture)
void BlockDrawer::drawFire( uint8_t blockID, uint8_t meta,
    GLint x, GLint y, GLint z, uint8_t vflags) const
{
    drawCube(blockID, meta, x, y, z, vflags);
    //TODO: draw on wall(s) depending on meta
}

//Draw dyed block (use item texture, determined by meta)
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
        if (Chunk::isLogic[block.blockID]) {
            mask |= 1;
        }
        //B neighbor
        block = world->getBlock(x + 1, y, z);
        if (Chunk::isLogic[block.blockID]) {
            mask |= 2;
        }
        //E neighbor
        block = world->getBlock(x, y, z - 1);
        if (Chunk::isLogic[block.blockID]) {
            mask |= 4;
        }
        //F neighbor
        block = world->getBlock(x, y, z + 1);
        if (Chunk::isLogic[block.blockID]) {
            mask |= 8;
        }
    }
        
    //Mask for adjacent "y+1" blocks
    uint8_t up_mask=0;
    if (world != NULL) {
        //A neighbor
        Block block = world->getBlock(x - 1, y + 1, z);
        if (Chunk::isLogic[block.blockID]) {
            up_mask |= 1;
            mask |= 1;
        }
        //B neighbor
        block = world->getBlock(x + 1, y + 1, z);
        if (Chunk::isLogic[block.blockID]) {
            up_mask |= 2;
            mask |= 2;
        }
        //E neighbor
        block = world->getBlock(x, y + 1, z - 1);
        if (Chunk::isLogic[block.blockID]) {
            up_mask |= 4;
            mask |= 4;
        }
        //F neighbor
        block = world->getBlock(x, y + 1, z + 1);
        if (Chunk::isLogic[block.blockID]) {
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

//Draw part of door (meta affects top/bottom, side of block)
void BlockDrawer::drawDoor( uint8_t blockID, uint8_t meta,
    GLint x, GLint y, GLint z, uint8_t vflags) const
{
    //Change texture and wall depending on meta
    uint16_t ID = 256 + blockID;
    
    //Check top bit
    if (meta & 0x8) {
        ID++;
    }
    
    //Door position and side depends on metadata
    switch (meta & 0x7) {
        case 0:     //West side, closed
            drawScaledBlock( ID, meta, x, y, z, vflags,
                3.0/16.0, 1.0, 1.0, true, 0, 0, 0, 0xC0);   //mirror A+B
            break;
        case 1:     //North side, closed
            drawScaledBlock( ID, meta, x, y, z, vflags,
                1.0, 1.0, 3.0/16.0, true, 0, 0, 0, 0x00);
            break;
        case 2:     //East side, closed
            drawScaledBlock( ID, meta, x, y, z, vflags,
                3.0/16.0, 1.0, 1.0, true, 13, 0, 0, 0x00);
            break;
        case 3:     //South side, closed
            drawScaledBlock( ID, meta, x, y, z, vflags,
                1.0, 1.0, 3.0/16.0, true, 0, 0, 13, 0x0C);   //mirror E+F
            break;
        case 4:     //West side, open
            drawScaledBlock( ID, meta, x, y, z, vflags,
                1.0, 1.0, 3.0/16.0, true, 0, 0, 0, 0x0C);   //mirror E+F
            break;
        case 5:     //North side, open
            drawScaledBlock( ID, meta, x, y, z, vflags,
                3.0/16.0, 1.0, 1.0, true, 13, 0, 0, 0xC0);   //mirror A+B
            break;
        case 6:     //East side, open
            drawScaledBlock( ID, meta, x, y, z, vflags,
                1.0, 1.0, 3.0/16.0, true, 0, 0, 13, 0x00);
            break;
        case 7:     //South side, open
            drawScaledBlock( ID, meta, x, y, z, vflags,
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
    drawHalfBlock( blockID, 0, x, y, z, vflags);

}

//Draw floor or wall lever (meta affects position)
void BlockDrawer::drawLever( uint8_t blockID, uint8_t meta,
    GLint x, GLint y, GLint z, uint8_t /*vflags*/) const
{
    //TODO: handle angle and base location depend on meta
    
    //Handle
    drawItem( blockID, 0, x, y, z);
    
    //Cobblestone base
    drawScaledBlock(4, 0, x, y, z, 0,
        0.25, 0.25, 0.5, true, 6, 0, 4);

}

//Draw signpost (meta affects angle)
void BlockDrawer::drawSignpost( uint8_t blockID, uint8_t meta,
    GLint x, GLint y, GLint z, uint8_t vflags) const
{
    //TODO: use words at location for texture
    
    //Sign
    drawScaledBlock( blockID, 0, x, y, z, 0,
        3.0/4.0, 1.0/2.0, 1.0/8.0, true, 2, 7, 8);
        
    //Post
    drawScaledBlock( blockID, 0, x, y, z, (vflags&0x30)|0x10,
        1.0/8.0, 7.0/16.0, 1.0/8.0, true, 7, 0, 8);

}

//Draw nether portal (use adjacent obsidian/portals for angle)
void BlockDrawer::drawPortal( uint8_t blockID, uint8_t /*meta*/,
    GLint x, GLint y, GLint z, uint8_t vflags) const
{
    //TODO: Use an animated portal texture (generated, not from texture file)
    drawScaledBlock( blockID, 0, x, y, z, vflags,
        1.0, 1.0, 0.25, true, 0, 0, 8);

}

//Draw fence (meta affects angle)
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

    //Check adjacent blocks for connected fences
    //A neighbor
    Block block = world->getBlock(x - 1, y, z);
    if (block.blockID == blockID) {
        //Top connecting boards
        drawScaledBlock( blockID, meta, x, y, z, (vflags&0x40)|0x80,
            6/16.0, 3/16.0, 2/16.0, true, 0, 12, 7);        

        //Bottom connecting boards
        drawScaledBlock( blockID, meta, x, y, z, (vflags&0x40)|0x80,
            6/16.0, 3/16.0, 2/16.0, true, 0, 6, 7);        
    }

    //B neighbor
    block = world->getBlock(x + 1, y, z);
    if (block.blockID == blockID) {
        //Top connecting boards
        drawScaledBlock( blockID, meta, x, y, z, (vflags&0x80)|0x40,
            6/16.0, 3/16.0, 2/16.0, true, 10, 12, 7);

        //Bottom connecting boards
        drawScaledBlock( blockID, meta, x, y, z, (vflags&0x80)|0x40,
            6/16.0, 3/16.0, 2/16.0, true, 10, 6, 7);
    }

    //E neighbor
    block = world->getBlock(x, y, z - 1);
    if (block.blockID == blockID) {
        //Top connecting boards
        drawScaledBlock( blockID, meta, x, y, z, (vflags&0x04)|0x08,
            2/16.0, 3/16.0, 6/16.0, true, 7, 12, 0);

        //Bottom connecting boards
        drawScaledBlock( blockID, meta, x, y, z, (vflags&0x04)|0x08,
            2/16.0, 3/16.0, 6/16.0, true, 7, 6, 0);
    }

    //F neighbor
    block = world->getBlock(x, y, z + 1);
    if (block.blockID == blockID) {
        //Top connecting boards
        drawScaledBlock( blockID, meta, x, y, z, (vflags&0x08)|0x04,
            2/16.0, 3/16.0, 6/16.0, true, 7, 12, 10);

        //Bottom connecting boards
        drawScaledBlock( blockID, meta, x, y, z, (vflags&0x08)|0x04,
            2/16.0, 3/16.0, 6/16.0, true, 7, 6, 10);
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

//Draw tree log or leaf (meta affects texture)
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

//Draw sign on a wall
void BlockDrawer::drawWallSign( uint8_t blockID, uint8_t meta,
    GLint x, GLint y, GLint z, uint8_t vflags) const
{
    
    //TODO: Write message on sign texture
    switch (meta & 0x7) {
        case 2: //South side (face north)
            drawScaledBlock( blockID, meta, x, y, z, (vflags&0x40),
                1.0/16.0, 0.5, 12.0/16.0, true, 15, 7, 2);
            break;
        case 3: //North side (face south)
            drawScaledBlock( blockID, meta, x, y, z, (vflags&0x80),
                1.0/16.0, 0.5, 12.0/16.0, true, 0, 7, 2);
            break;
        case 4: //West side (face east)
            drawScaledBlock( blockID, meta, x, y, z, (vflags&0x04),
                12.0/16.0, 0.5, 1.0/16.0, true, 2, 7, 15);
            break;
        case 5: //East side (face west)
        default:
            drawScaledBlock( blockID, meta, x, y, z, (vflags&0x08),
                12.0/16.0, 0.5, 1.0/16.0, true, 2, 7, 0);
            break;
    }
}

//Draw wall button
void BlockDrawer::drawButton( uint8_t blockID, uint8_t meta,
    GLint x, GLint y, GLint z, uint8_t /*vflags*/) const
{
    //TODO: meta affects which wall the button is on, and pressed/not pressed
    drawScaledBlock( blockID, meta, x, y, z, 0,
        0.5, 0.25, 0.125, true, 4, 8, 0);

}



//Copy block info to struct
void BlockDrawer::setBlockInfo( uint16_t index,
    uint8_t A, uint8_t B,
    uint8_t C, uint8_t D,
    uint8_t E, uint8_t F,
    drawBlock_f drawFunc)
{
    //A - F contain texture ID of the corresponding face (textureID = 0 - 255)

    //For each face,  precompute OpenGL texture offsets in texture map
    // (Remember that texture map was loaded upside down!)
    blockInfo[index].textureID[0] = A;
    blockInfo[index].tx[0] = float(A & (texmap_TILES-1))/((float)texmap_TILES);
    blockInfo[index].ty[0] = float(A/texmap_TILES)/((float)texmap_TILES);

    blockInfo[index].textureID[1] = B;
    blockInfo[index].tx[1] = float(B & (texmap_TILES-1))/((float)texmap_TILES);
    blockInfo[index].ty[1] = float(B/texmap_TILES)/((float)texmap_TILES);

    blockInfo[index].textureID[2] = C;
    blockInfo[index].tx[2] = float(C & (texmap_TILES-1))/((float)texmap_TILES);
    blockInfo[index].ty[2] = float(C/texmap_TILES)/((float)texmap_TILES);

    blockInfo[index].textureID[3] = D;
    blockInfo[index].tx[3] = float(D & (texmap_TILES-1))/((float)texmap_TILES);
    blockInfo[index].ty[3] = float(D/texmap_TILES)/((float)texmap_TILES);

    blockInfo[index].textureID[4] = E;
    blockInfo[index].tx[4] = float(E & (texmap_TILES-1))/((float)texmap_TILES);
    blockInfo[index].ty[4] = float(E/texmap_TILES)/((float)texmap_TILES);
    
    blockInfo[index].textureID[5] = F;
    blockInfo[index].tx[5] = float(F & (texmap_TILES-1))/((float)texmap_TILES);
    blockInfo[index].ty[5] = float(F/texmap_TILES)/((float)texmap_TILES);
    
    drawFunction[index] = drawFunc;

}

//Map block ID to block type information
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

// 11 is the transparent texture

    //Set specific blocks
    setBlockInfo( 0, 11, 11, 11, 11, 11, 11);     //Air
    setBlockInfo( 1, 1, 1, 1, 1, 1, 1      );     //Stone
    setBlockInfo( 2, 3, 3, 2, 0, 3, 3      );     //Grass
    setBlockInfo( 3, 2, 2, 2, 2, 2, 2      );     //Dirt
    setBlockInfo( 4, 16, 16, 16, 16, 16, 16);     //Cobble
    setBlockInfo( 5, 4, 4, 4, 4, 4, 4      );     //Wood
         //Sapling
    setBlockInfo( 6, 15, 15, 15, 15, 15, 15, &BlockDrawer::drawItem);
    setBlockInfo( 7, 17, 17, 17, 17, 17, 17);     //Bedrock
    setBlockInfo( 8, 0xCE, 0xDE, 0xCD, 0xCD, 0xDF, 0xCF);     //Water(*)
    setBlockInfo( 9, 0xCD, 0xCD, 0xCD, 0xCD, 0xCD, 0xCD);     //WaterStill
    setBlockInfo( 10, 0xEE, 0xFE, 0xED, 0xED, 0xFF, 0xEF);    //Lava(*)
    setBlockInfo( 11, 0xED, 0xED, 0xED, 0xED, 0xED, 0xED);    //LavaStill
    setBlockInfo( 12, 18, 18, 18, 18, 18, 18);    //Sand
    setBlockInfo( 13, 19, 19, 19, 19, 19, 19);    //Gravel
    setBlockInfo( 14, 32, 32, 32, 32, 32, 32);    //GoldOre
    setBlockInfo( 15, 33, 33, 33, 33, 33, 33);    //IronOre
    setBlockInfo( 16, 34, 34, 34, 34, 34, 34);    //CoalOre
        //Log
    setBlockInfo( 17, 20, 20, 21, 21, 20, 20,&BlockDrawer::drawTree);
        //Leaves
    setBlockInfo( 18, 52, 52, 52, 53, 52, 52,&BlockDrawer::drawTree);
    setBlockInfo( 19, 48, 48, 48, 48, 48, 48);    //Sponge
    setBlockInfo( 20, 49, 49, 49, 49, 49, 49);    //Glass
    setBlockInfo( 21,160,160,160,160,160,160);    //Lapis Ore
    setBlockInfo( 22,144,144,144,144,144,144);    //Lapis Block
        //Dispenser (*)
    setBlockInfo( 23, 45, 45, 62, 62, 45, 46,&BlockDrawer::drawFaceCube);
    setBlockInfo( 24,192,192,208,176,192,192);    //Sandstone
    setBlockInfo( 25, 74, 74, 74, 74, 74, 74);    //Note Block

    //26 - 36 = Dyed wool (drawDyed will override metadata)
    setBlockInfo( 26, 64, 64, 64, 64, 64, 64);
    setBlockInfo( 27, 64, 64, 64, 64, 64, 64);
    setBlockInfo( 28, 64, 64, 64, 64, 64, 64);
    setBlockInfo( 29, 64, 64, 64, 64, 64, 64);
    setBlockInfo( 30, 64, 64, 64, 64, 64, 64);
    setBlockInfo( 31, 64, 64, 64, 64, 64, 64);
    setBlockInfo( 32, 64, 64, 64, 64, 64, 64);
    setBlockInfo( 33, 64, 64, 64, 64, 64, 64);
    setBlockInfo( 34, 64, 64, 64, 64, 64, 64);
    setBlockInfo( 35, 64, 64, 64, 64, 64, 64,&BlockDrawer::drawDyed);
    setBlockInfo( 36, 64, 64, 64, 64, 64, 64);
    
        //Flower
    setBlockInfo( 37, 13, 13, 13, 13, 13, 13,&BlockDrawer::drawItem);
        //Rose
    setBlockInfo( 38, 12, 12, 12, 12, 12, 12,&BlockDrawer::drawItem);
        //BrownShroom
    setBlockInfo( 39, 29, 29, 29, 29, 29, 29,&BlockDrawer::drawItem);
        //RedShroom
    setBlockInfo( 40, 28, 28, 28, 28, 28, 28,&BlockDrawer::drawItem);
    setBlockInfo( 41, 23, 23, 23, 23, 23, 23);    //GoldBlock
    setBlockInfo( 42, 22, 22, 22, 22, 22, 22);    //IronBlock
    setBlockInfo( 43, 5,  5,  6,  6,  5,  5 );    //DoubleStep
        //Step
    setBlockInfo( 44, 5,  5,  6,  6,  5,  5,&BlockDrawer::drawHalfBlock);
    setBlockInfo( 45, 7,  7,  7,  7,  7,  7 );    //Brick
    setBlockInfo( 46, 8,  8, 10,  9,  8,  8 );    //TNT
    setBlockInfo( 47, 35, 35, 4,  4,  35, 35);    //Bookshelf
    setBlockInfo( 48, 36, 36, 16, 36, 36, 36);    //Mossy
    setBlockInfo( 49, 37, 37, 37, 37, 37, 37);    //Obsidian
        //Torch
    setBlockInfo( 50, 80, 80, 80, 80, 80, 80,&BlockDrawer::drawTorch);
        //Fire
    setBlockInfo( 51, 31, 31, 47, 47, 31, 31,&BlockDrawer::drawFire);
    setBlockInfo( 52, 65, 65, 65, 65, 65, 65);    //Spawner
        //WoodStairs
    setBlockInfo( 53, 4,  4,  4,  4,  4,  4, &BlockDrawer::drawStairs);
        //Chest (*)
    setBlockInfo( 54, 26, 26, 25, 25, 26, 27,&BlockDrawer::drawChest);
        //Wire (*)
    setBlockInfo( 55, 84, 85, 100,101,84,100,&BlockDrawer::drawWire);
    setBlockInfo( 56, 50, 50, 50, 50, 50, 50);    //DiamondOre
    setBlockInfo( 57, 24, 24, 24, 24, 24, 24);    //DiamondBlock
    setBlockInfo( 58, 60, 60, 43, 43, 59, 59);    //Workbench
        //Crops (*)
    setBlockInfo( 59, 88, 89, 90, 91, 93, 95, &BlockDrawer::drawCrops);
    setBlockInfo( 60, 2,  2,  2,  86, 2,  2 );    //Soil
        //Furnace (+)
    setBlockInfo( 61, 45, 45, 62, 62, 45, 44,&BlockDrawer::drawFaceCube);
        //LitFurnace (+)
    setBlockInfo( 62, 45, 45, 62, 62, 45, 61,&BlockDrawer::drawFaceCube);
        //SignPost (*)
    setBlockInfo( 63, 4,  4,  4,  4,  4,  4,&BlockDrawer::drawSignpost);
        //WoodDoor (*)
    setBlockInfo( 64, 97, 81, 97, 81, 97, 81,&BlockDrawer::drawDoor);
        //Ladder (*)
    setBlockInfo( 65, 83, 83, 83, 83, 83, 83, &BlockDrawer::drawWallItem);
        //Track (*)
    setBlockInfo( 66, 112,112,128,128,128,128,&BlockDrawer::drawTrack);
        //CobbleStairs
    setBlockInfo( 67, 16, 16, 16, 16, 16, 16,&BlockDrawer::drawStairs);
        //WallSign (*)
    setBlockInfo( 68, 4,  4,  4,  4,  4,  4, &BlockDrawer::drawWallSign);
        //Lever
    setBlockInfo( 69, 96, 96, 96, 96, 96, 16,&BlockDrawer::drawLever);
        //StonePlate
    setBlockInfo( 70, 1,  1,  1,  1,  1,  1,&BlockDrawer::drawFloorplate);
        //IronDoor (*)
    setBlockInfo( 71, 98, 82, 98, 82, 98, 82,&BlockDrawer::drawDoor);
        //WoodPlate
    setBlockInfo( 72, 4,  4,  4,  4,  4,  4,&BlockDrawer::drawFloorplate);
    setBlockInfo( 73, 51, 51, 51, 51, 51, 51);    //RedstoneOre
    setBlockInfo( 74, 51, 51, 51, 51, 51, 51);    //RedstoneOreLit(*)
        //RedstoneTorch
    setBlockInfo( 75, 115,115,115,115,115,115,&BlockDrawer::drawTorch);
        //RedstoneTorchLit
    setBlockInfo( 76, 99, 99, 99, 99, 99, 99,&BlockDrawer::drawTorch);
        //StoneButton
    setBlockInfo( 77, 1,  1,  1,  1,  1,  1,&BlockDrawer::drawButton);
        //SnowLayer(*)
    setBlockInfo( 78, 66, 66, 66, 66, 66, 66,&BlockDrawer::draw4thBlock);
        //BlockID 2 (Grass) below a a SnowLayer uses texture 68 on the sides
    setBlockInfo( 79, 67, 67, 67, 67, 67, 67);    //Ice
    setBlockInfo( 80, 66, 66, 66, 66, 66, 66);    //SnowBlock
        //Cactus
    setBlockInfo( 81, 70, 70, 71, 69, 70, 70,&BlockDrawer::drawCactus);
    setBlockInfo( 82, 72, 72, 72, 72, 72, 72);    //Clay
        //Sugarcane (*)
    setBlockInfo( 83, 73, 73, 73, 73, 73, 73,&BlockDrawer::drawItem);
    setBlockInfo( 84, 74, 74, 43, 75, 74, 74);    //Jukebox
        //Fence (*)
    setBlockInfo( 85, 4,  4,  4,  4,  4,  4,&BlockDrawer::drawFence);
        //Pumpkin
    setBlockInfo( 86, 118,118,118,102,118,119,&BlockDrawer::drawFaceCube2);
    setBlockInfo( 87, 103,103,103,103,103,103);    //Netherstone
    setBlockInfo( 88, 104,104,104,104,104,104);    //SlowSand
    setBlockInfo( 89, 105,105,105,105,105,105);    //Lightstone
        //Portal
    setBlockInfo( 90, 205,206,207,222,223,205,&BlockDrawer::drawPortal);
        //PumpkinLit
    setBlockInfo( 91, 118,118,118,102,118,120,&BlockDrawer::drawFaceCube2);
        //Cake block (*)
    setBlockInfo( 92, 123,122,124,121,122,122,&BlockDrawer::drawCake);
    
    
    //extra info for metadata blocks
    
    //Redwood tree
    setBlockInfo( 256 + 17, 116, 116, 21, 21, 116, 116);
    setBlockInfo( 256 + 18, 132, 132, 132, 132, 132, 132);
    //Birch tree
    setBlockInfo( 512 + 17, 117, 117, 21, 21, 117, 117);
    setBlockInfo( 512 + 18, 133, 133, 133, 133, 133, 133);
    
    //Dyed wool (256 + 35 + metadata)
    setBlockInfo( 256 + 35, 64, 64, 64, 64, 64, 64);
    uint8_t dyed_id;
    for (ID = 1; ID < 8; ID++) {
        dyed_id = 226 - (ID<<4);
        setBlockInfo( 256 + 35 + ID, dyed_id, dyed_id, dyed_id,
            dyed_id, dyed_id, dyed_id);    //draw cube
    }
    for (ID = 8; ID < 16; ID++) {
        dyed_id = 225 - ((ID - 8)<<4);
        setBlockInfo( 256 + 35 + ID, dyed_id, dyed_id, dyed_id,
            dyed_id, dyed_id, dyed_id);    //draw cube
    }
    
    //Chest (256 + 54=left, 55=right, 56=left X, 57=right X)
    setBlockInfo( 256 + 54 + 0, 26, 26, 25, 25, 58, 41);
    setBlockInfo( 256 + 54 + 1, 26, 26, 25, 25, 57, 42);
    setBlockInfo( 256 + 54 + 2, 58, 41, 25, 25, 26, 26);
    setBlockInfo( 256 + 54 + 3, 57, 42, 25, 25, 26, 26);

    //Wood door
    setBlockInfo( 256 + 64, 97, 97, 97, 97, 97, 97);  //bottom
    setBlockInfo( 256 + 65, 81, 81, 81, 81, 81, 81);  //top
    
    //Iron door
    setBlockInfo( 256 + 71, 98, 98, 98, 98, 98, 98); //bottom
    setBlockInfo( 256 + 72, 82, 82, 82, 82, 82, 82); //top

//0xF0: Shape : 0=cube, 1=stairs, 2=toggle, 3=halfblock,
//              4=signpost, 5=ladder, 6=track, 7=fire
//              8=portal, 9=fence, A=door, B=floorplate
//              C=snow?, D=wallsign, E=button, F=planted
//0x08: Bright: 0=dark, 1=lightsource
//0x04: Vision: 0=opqaue, 1=see-through
//0x03: State : 0=solid, 1=loose, 2=liquid, 3=gas

    return true;
}

void BlockDrawer::mirrorCoords( GLfloat& tx_0, GLfloat& tx_1,
                GLfloat& ty_0, GLfloat& ty_1, uint8_t mirror_type) const
{
    GLfloat t_mirror;
    
    if (mirror_type & 2) { t_mirror = tx_0; tx_0 = tx_1; tx_1 = t_mirror; }
    if (mirror_type & 1) { t_mirror = ty_0; ty_0 = ty_1; ty_1 = t_mirror; }
    
}