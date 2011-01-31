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

BlockDrawer::BlockDrawer(GLuint t_tex, GLuint i_tex):
    terrain_tex(t_tex), item_tex(i_tex)
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
    drawBlock_f f = drawFunction[blockID];
    
    //Metadata hackery to conform special cases
    
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
        tx_1 = blockInfo[blockID].tx[WEST] + tmr;
        ty_0 = blockInfo[blockID].ty[WEST] + tmr;    //flip y
        ty_1 = blockInfo[blockID].ty[WEST];
        setBlockColor(blockID, WEST);  //Set leaf/grass color if needed
        
        glTexCoord2f(tx_0,ty_0); glVertex3i( A, C, E);  //Lower left:  ACE
        glTexCoord2f(tx_1,ty_0); glVertex3i( A, C, F);  //Lower right: ACF
        glTexCoord2f(tx_1,ty_1); glVertex3i( A, D, F);  //Top right:   ADF
        glTexCoord2f(tx_0,ty_1); glVertex3i( A, D, E);  //Top left:    ADE
    }

    //B
    if (!(vflags & 0x40)) {
        tx_0 = blockInfo[blockID].tx[EAST];
        tx_1 = blockInfo[blockID].tx[EAST] + tmr;
        ty_0 = blockInfo[blockID].ty[EAST] + tmr;
        ty_1 = blockInfo[blockID].ty[EAST];
        setBlockColor(blockID, EAST);  //Set leaf/grass color if needed
        
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
    
    //E
    if (!(vflags & 0x08)) {
        tx_0 = blockInfo[blockID].tx[NORTH];
        tx_1 = blockInfo[blockID].tx[NORTH] + tmr;
        ty_0 = blockInfo[blockID].ty[NORTH] + tmr;
        ty_1 = blockInfo[blockID].ty[NORTH];
        setBlockColor(blockID, NORTH);  //Set leaf/grass color if needed
        
        glTexCoord2f(tx_0,ty_0); glVertex3i( B, C, E);  //Lower left:  BCE
        glTexCoord2f(tx_1,ty_0); glVertex3i( A, C, E);  //Lower right: ACE
        glTexCoord2f(tx_1,ty_1); glVertex3i( A, D, E);  //Top right:   ADE
        glTexCoord2f(tx_0,ty_1); glVertex3i( B, D, E);  //Top left:    BDE
    }
    
    //F
    if (!(vflags & 0x04)) {
        tx_0 = blockInfo[blockID].tx[SOUTH];
        tx_1 = blockInfo[blockID].tx[SOUTH] + tmr;
        ty_0 = blockInfo[blockID].ty[SOUTH] + tmr;
        ty_1 = blockInfo[blockID].ty[SOUTH];
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

//Draw "treasure" chest (meta affects direction, large chest)
void BlockDrawer::drawChest( uint8_t blockID, uint8_t meta,
    GLint x, GLint y, GLint z, uint8_t vflags) const
{
    drawFaceCube(blockID, meta, x, y, z, vflags);
    //TODO: textures change depending on meta
}

//Draw cactus... almost like a cube, but A, B, E, F are inset 1 pixel
void BlockDrawer::drawCactus( uint8_t blockID, uint8_t meta,
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

    // For cactus, the face coordinates are inset from the face

    //A always visible
        tx_0 = blockInfo[blockID].tx[WEST];
        tx_1 = blockInfo[blockID].tx[WEST] + tmr;
        ty_0 = blockInfo[blockID].ty[WEST] + tmr;    //flip y
        ty_1 = blockInfo[blockID].ty[WEST];
        setBlockColor(blockID, WEST);  //Set leaf/grass color if needed
        
        glTexCoord2f(tx_0,ty_0); glVertex3i( A+1, C, E);
        glTexCoord2f(tx_1,ty_0); glVertex3i( A+1, C, F);
        glTexCoord2f(tx_1,ty_1); glVertex3i( A+1, D, F);
        glTexCoord2f(tx_0,ty_1); glVertex3i( A+1, D, E);

    //B
        tx_0 = blockInfo[blockID].tx[EAST];
        tx_1 = blockInfo[blockID].tx[EAST] + tmr;
        ty_0 = blockInfo[blockID].ty[EAST] + tmr;
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
        ty_0 = blockInfo[blockID].ty[NORTH] + tmr;
        ty_1 = blockInfo[blockID].ty[NORTH];
        setBlockColor(blockID, NORTH);  //Set leaf/grass color if needed
        
        glTexCoord2f(tx_0,ty_0); glVertex3i( B, C, E+1);  //Lower left:  BCE
        glTexCoord2f(tx_1,ty_0); glVertex3i( A, C, E+1);  //Lower right: ACE
        glTexCoord2f(tx_1,ty_1); glVertex3i( A, D, E+1);  //Top right:   ADE
        glTexCoord2f(tx_0,ty_1); glVertex3i( B, D, E+1);  //Top left:    BDE
    
    //F
        tx_0 = blockInfo[blockID].tx[SOUTH];
        tx_1 = blockInfo[blockID].tx[SOUTH] + tmr;
        ty_0 = blockInfo[blockID].ty[SOUTH] + tmr;
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

    //C always visible
    tx_0 = blockInfo[blockID].tx[DOWN] + tmr_eat;
    tx_1 = blockInfo[blockID].tx[DOWN] + tmr;
    ty_0 = blockInfo[blockID].ty[DOWN] + tmr;
    ty_1 = blockInfo[blockID].ty[DOWN];
    
    glTexCoord2f(tx_0,ty_0); glVertex3i( A, C, E);
    glTexCoord2f(tx_1,ty_0); glVertex3i( B, C, E);
    glTexCoord2f(tx_1,ty_1); glVertex3i( B, C, F);
    glTexCoord2f(tx_0,ty_1); glVertex3i( A, C, F);

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

//Use OpenGL to draw partial solid cube, with offsets, scale (TODO: rotation)
void BlockDrawer::drawScaledBlock( uint8_t blockID, uint8_t meta,
    GLint x, GLint y, GLint z, uint8_t vflags,
    GLfloat scale_x, GLfloat scale_y, GLfloat scale_z,
    bool scale_texture,
    GLint off_x, GLint off_y, GLint off_z) const
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

    //Texture map coordinates (0.0 - 1.0)
    GLfloat tx_0, tx_1, ty_0, ty_1;
    
    //Scaled texture map ratios (1/block length)
    GLfloat tmr_x, tmr_y, tmr_z, tmr_off_x, tmr_off_y, tmr_off_z;
    if (scale_texture) {
        tmr_x  = scale_x * tmr;
        tmr_y = scale_y * tmr;
        tmr_z  = scale_z * tmr;
        tmr_off_x = tmr*off_x/texmap_TILE_LENGTH;
        tmr_off_y = tmr*off_y/texmap_TILE_LENGTH;
        tmr_off_z = tmr*off_z/texmap_TILE_LENGTH;
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
        
        glTexCoord2f(tx_0,ty_0); glVertex3i( A, C, E);  //Lower left:  ACE
        glTexCoord2f(tx_1,ty_0); glVertex3i( A, C, F);  //Lower right: ACF
        glTexCoord2f(tx_1,ty_1); glVertex3i( A, D, F);  //Top right:   ADF
        glTexCoord2f(tx_0,ty_1); glVertex3i( A, D, E);  //Top left:    ADE
    }

    //B
    if (!(vflags & 0x40) && (scale_y != 0.0 && scale_z != 0.0) ) {
        //Use offsets and scaling for texture coordinates
        tx_0 = blockInfo[blockID].tx[EAST] + tmr_off_z;
        tx_1 = blockInfo[blockID].tx[EAST] + tmr_off_z + tmr_z;
        ty_0 = blockInfo[blockID].ty[EAST] + tmr_off_y + tmr_y;
        ty_1 = blockInfo[blockID].ty[EAST] + tmr_off_y;
        
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
    
        glTexCoord2f(tx_0,ty_0); glVertex3i( A, D, F);  //Lower left:  ADF
        glTexCoord2f(tx_1,ty_0); glVertex3i( B, D, F);  //Lower right: BDF
        glTexCoord2f(tx_1,ty_1); glVertex3i( B, D, E);  //Top right:   BDE
        glTexCoord2f(tx_0,ty_1); glVertex3i( A, D, E);  //Top left:    ADE
    }
    
    //E
    if (!(vflags & 0x08) && (scale_x != 0.0 && scale_y != 0.0) ) {
        //Use offsets and scaling for texture coordinates
        tx_0 = blockInfo[blockID].tx[NORTH] + tmr_off_x;
        tx_1 = blockInfo[blockID].tx[NORTH] + tmr_off_x + tmr_x;
        ty_0 = blockInfo[blockID].ty[NORTH] + tmr_off_y + tmr_y;
        ty_1 = blockInfo[blockID].ty[NORTH] + tmr_off_y;
        
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
    //TODO: metadata to determine which half!
    drawScaledBlock( blockID, meta, x, y, z, vflags, 1, 0.5, 1);
}

//Draw minecart track (meta affects angle, direction, intersection)
void BlockDrawer::drawTrack( uint8_t blockID, uint8_t meta,
    GLint x, GLint y, GLint z, uint8_t vflags) const
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
    GLint x, GLint y, GLint z, uint8_t vflags) const
{
    //TODO: metadata

    //Texture map coordinates (0.0 - 1.0)
    GLfloat tx_0, tx_1, ty_0, ty_1;

    //Object boundaries... flat square 1 pixel off the wall
    GLint A = (x << 4) + 0;
    GLint B = (x << 4) + texmap_TILE_LENGTH;
    GLint C = (y << 4) + 0;
    GLint D = (y << 4) + texmap_TILE_LENGTH;
    GLint E = (z << 4) + 0;
    GLint F = (z << 4) + 1;

    //E
    tx_0 = blockInfo[blockID].tx[NORTH];
    tx_1 = blockInfo[blockID].tx[NORTH] + tmr;
    ty_0 = blockInfo[blockID].ty[NORTH] + tmr;
    ty_1 = blockInfo[blockID].ty[NORTH];
    
    glTexCoord2f(tx_0,ty_0); glVertex3i( B, C, E);  //Lower left:  BCE
    glTexCoord2f(tx_1,ty_0); glVertex3i( A, C, E);  //Lower right: ACE
    glTexCoord2f(tx_1,ty_1); glVertex3i( A, D, E);  //Top right:   ADE
    glTexCoord2f(tx_0,ty_1); glVertex3i( B, D, E);  //Top left:    BDE
    
    //F
    tx_0 = blockInfo[blockID].tx[SOUTH];
    tx_1 = blockInfo[blockID].tx[SOUTH] + tmr;
    ty_0 = blockInfo[blockID].ty[SOUTH] + tmr;
    ty_1 = blockInfo[blockID].ty[SOUTH];
    
    glTexCoord2f(tx_0,ty_0); glVertex3i( A, C, F);  //Lower left:  ACF
    glTexCoord2f(tx_1,ty_0); glVertex3i( B, C, F);  //Lower right: BCF
    glTexCoord2f(tx_1,ty_1); glVertex3i( B, D, F);  //Top right:   BDF
    glTexCoord2f(tx_0,ty_1); glVertex3i( A, D, F);  //Top left:    ADF
}

//Draw item blockID which is placed as a block
void BlockDrawer::drawItem( uint8_t blockID, uint8_t meta,
    GLint x, GLint y, GLint z, uint8_t vflags) const
{

    //TODO: quad always faces player somehow

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
    GLint x, GLint y, GLint z, uint8_t vflags) const
{
    drawItem(blockID, meta, x, y, z, vflags);
    //TODO: draw on wall at and angle depending on meta
}

//Draw fire burning (use item texture)
void BlockDrawer::drawFire( uint8_t blockID, uint8_t meta,
    GLint x, GLint y, GLint z, uint8_t vflags) const
{
    drawCube(blockID, meta, x, y, z, vflags);
    //TODO: draw on wall(s) depending on meta
}

//Draw redstone wire (active or inactive
void BlockDrawer::drawWire( uint8_t blockID, uint8_t meta,
    GLint x, GLint y, GLint z, uint8_t vflags) const
{
    //TODO: change texture and rotation depending on meta
    drawFloorplate(blockID, meta, x, y, z, vflags);
}

//Draw planted crops (meta affects texture)
void BlockDrawer::drawCrops( uint8_t blockID, uint8_t meta,
    GLint x, GLint y, GLint z, uint8_t vflags) const
{
    //TODO: change texture depending on meta
    drawItem(blockID, meta, x, y, z, vflags);
}

//Draw part of door (meta affects top/bottom, side of block)
void BlockDrawer::drawDoor( uint8_t blockID, uint8_t meta,
    GLint x, GLint y, GLint z, uint8_t vflags) const
{
    //TODO: change texture and wall depending on meta
    drawWallItem(blockID, meta, x, y, z, vflags);
}


//Draw stairs (meta affects orientation)
void BlockDrawer::drawStairs( uint8_t blockID, uint8_t meta,
    GLint x, GLint y, GLint z, uint8_t vflags) const
{
    //Top step: TODO: dimensions depend on meta
    drawScaledBlock(blockID, meta, x, y, z, vflags,
        1, 0.5, 0.5, true, 0, 8, 0);
        
    //Bottom step
    drawHalfBlock( blockID, meta, x, y, z, vflags);

}

//Draw floor or wall lever (meta affects position)
void BlockDrawer::drawLever( uint8_t blockID, uint8_t meta,
    GLint x, GLint y, GLint z, uint8_t vflags) const
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
    //Sign
    drawScaledBlock( blockID, 0, x, y, z, 0,
        3.0/4.0, 1.0/2.0, 1.0/8.0, true, 2, 7, 8);
        
    //Post
    drawScaledBlock( blockID, 0, x, y, z, (vflags&0x30)|0x10,
        1.0/8.0, 7.0/16.0, 1.0/8.0, true, 7, 0, 8);

}

//Draw nether portal (no meta)
void BlockDrawer::drawPortal( uint8_t blockID, uint8_t meta,
    GLint x, GLint y, GLint z, uint8_t vflags) const
{
    //TODO: use meta to change proportions
  
    //TODO: use portal texture from item_tex
  
    drawScaledBlock( blockID, meta, x, y, z, 0,
        1.0, 1.0, 0.25, true, 0, 0, 8);

}

//Draw fence (meta affects angle)
void BlockDrawer::drawFence( uint8_t blockID, uint8_t meta,
    GLint x, GLint y, GLint z, uint8_t vflags) const
{
    //TODO: meta determines if this is a single fence, or has neighbors
    
    //Top of fence
    drawScaledBlock( blockID, meta, x, y, z, vflags&0xC0,
        1, 0.25, 0.25, true, 0, 6, 6);
    //Fence legs
    drawScaledBlock( blockID, meta, x, y, z, (vflags&0x30)|0x10,
        0.25, 0.375, 0.25, true, 2, 0, 6);
    drawScaledBlock( blockID, meta, x, y, z, (vflags&0x30)|0x10,
        0.25, 0.375, 0.25, true, 10, 0, 6);

}

//Draw floor plate (meta affects pressed, depressed)
void BlockDrawer::drawFloorplate( uint8_t blockID, uint8_t meta,
    GLint x, GLint y, GLint z, uint8_t vflags) const
{
    drawScaledBlock( blockID, meta, x, y, z, 0,
        0.75, 0.125, 0.75, true, 2, 0, 2);

}

//Draw quarter block (meta affects height?)
void BlockDrawer::draw4thBlock( uint8_t blockID, uint8_t meta,
    GLint x, GLint y, GLint z, uint8_t vflags) const
{
    drawScaledBlock( blockID, meta, x, y, z, vflags,
        1, 0.25, 1);
}

//Draw tree log (meta affects texture)
void BlockDrawer::drawLog( uint8_t blockID, uint8_t meta,
    GLint x, GLint y, GLint z, uint8_t vflags) const
{
    //Use meta to change texture of cube
    drawCube(blockID, meta, x, y, z, vflags);
}

//Draw sign on a wall
void BlockDrawer::drawWallSign( uint8_t blockID, uint8_t meta,
    GLint x, GLint y, GLint z, uint8_t vflags) const
{
    //TODO: meta affects which wall the sign is on
    drawScaledBlock( blockID, meta, x, y, z, (vflags&0x08),
        0.75, 0.5, 0.125, true, 2, 7, 0);
}

//Draw wall button
void BlockDrawer::drawButton( uint8_t blockID, uint8_t meta,
    GLint x, GLint y, GLint z, uint8_t vflags) const
{
    //TODO: meta affects which wall the button is on, and pressed/not pressed
    drawScaledBlock( blockID, meta, x, y, z, 0,
        0.25, 0.25, 0.125, true, 8, 8, 0);

}



//Copy block info to struct
void BlockDrawer::setBlockInfo( uint8_t index,
    uint8_t A, uint8_t B,
    uint8_t C, uint8_t D,
    uint8_t E, uint8_t F,
    uint8_t properties,
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
        setBlockInfo( ID, 48, 48, 48, 48, 48, 48, 0);
    }
    setBlockInfo( 0xFF, 48, 48, 48, 48, 48, 48, 0);

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
    setBlockInfo( 0, 11, 11, 11, 11, 11, 11, 0x07);     //Air
    setBlockInfo( 1, 1, 1, 1, 1, 1, 1,       0x00);     //Stone
    setBlockInfo( 2, 3, 3, 2, 0, 3, 3,       0x00);     //Grass
    setBlockInfo( 3, 2, 2, 2, 2, 2, 2,       0x00);     //Dirt
    setBlockInfo( 4, 16, 16, 16, 16, 16, 16, 0x00);     //Cobble
    setBlockInfo( 5, 4, 4, 4, 4, 4, 4,       0x00);     //Wood
         //Sapling
    setBlockInfo( 6, 15, 15, 15, 15, 15, 15, 0xF7, &BlockDrawer::drawItem);
    setBlockInfo( 7, 17, 17, 17, 17, 17, 17, 0x00);     //Bedrock
    setBlockInfo( 8, 0xCE, 0xDE, 0xCD, 0xCD, 0xDF, 0xCF, 0x06);     //Water(*)
    setBlockInfo( 9, 0xCD, 0xCD, 0xCD, 0xCD, 0xCD, 0xCD, 0x06);     //WaterStill
    setBlockInfo( 10, 0xEE, 0xFE, 0xED, 0xED, 0xFF, 0xEF, 0x02);    //Lava(*)
    setBlockInfo( 11, 0xED, 0xED, 0xED, 0xED, 0xED, 0xED, 0x02);    //LavaStill
    setBlockInfo( 12, 18, 18, 18, 18, 18, 18, 0x01);    //Sand
    setBlockInfo( 13, 19, 19, 19, 19, 19, 19, 0x01);    //Gravel
    setBlockInfo( 14, 32, 32, 32, 32, 32, 32, 0x00);    //GoldOre
    setBlockInfo( 15, 33, 33, 33, 33, 33, 33, 0x00);    //IronOre
    setBlockInfo( 16, 34, 34, 34, 34, 34, 34, 0x00);    //CoalOre
    
        //Log
    setBlockInfo( 17, 20, 20, 21, 21, 20, 20, 0x00,&BlockDrawer::drawLog);
    setBlockInfo( 18, 52, 52, 52, 53, 52, 52, 0x04);    //Leaves
    setBlockInfo( 19, 48, 48, 48, 48, 48, 48, 0x00);    //Sponge
    setBlockInfo( 20, 49, 49, 49, 49, 49, 49, 0x04);    //Glass
    setBlockInfo( 21,160,160,160,160,160,160, 0x00);    //Lapis Ore
    setBlockInfo( 22,144,144,144,144,144,144, 0x00);    //Lapis Block
    setBlockInfo( 23, 45, 45, 62, 62, 45, 46, 0x00);    //Dispenser (*)
    setBlockInfo( 24,192,192,208,176,192,192, 0x00);    //Sandstone
    setBlockInfo( 25, 74, 74, 74, 74, 74, 74, 0x00);    //Note Block
    
    for (ID = 26; ID < 37; ID++) {
        setBlockInfo( ID, 64, 64, 64, 64, 64, 64, 0x00);   //Cloth (only 1 used)
    }
        //Flower
    setBlockInfo( 37, 13, 13, 13, 13, 13, 13, 0xF7,&BlockDrawer::drawItem);
        //Rose
    setBlockInfo( 38, 12, 12, 12, 12, 12, 12, 0xF7,&BlockDrawer::drawItem);
        //BrownShroom
    setBlockInfo( 39, 29, 29, 29, 29, 29, 29, 0xF7,&BlockDrawer::drawItem);
        //RedShroom
    setBlockInfo( 40, 28, 28, 28, 28, 28, 28, 0xF7,&BlockDrawer::drawItem);
    setBlockInfo( 41, 23, 23, 23, 23, 23, 23, 0x00);    //GoldBlock
    setBlockInfo( 42, 22, 22, 22, 22, 22, 22, 0x00);    //IronBlock
    setBlockInfo( 43, 5,  5,  6,  6,  5,  5,  0x00);    //DoubleStep
        //Step
    setBlockInfo( 44, 5,  5,  6,  6,  5,  5, 0x30,&BlockDrawer::drawHalfBlock);
    setBlockInfo( 45, 7,  7,  7,  7,  7,  7,  0x00);    //Brick
    setBlockInfo( 46, 8,  8, 10,  9,  8,  8,  0x00);    //TNT
    setBlockInfo( 47, 35, 35, 4,  4,  35, 35, 0x00);    //Bookshelf
    setBlockInfo( 48, 36, 36, 16, 36, 36, 36, 0x00);    //Mossy
    setBlockInfo( 49, 37, 37, 37, 37, 37, 37, 0x00);    //Obsidian
        //Torch
    setBlockInfo( 50, 80, 80, 80, 80, 80, 80, 0xFF,&BlockDrawer::drawTorch);
        //Fire
    setBlockInfo( 51, 31, 31, 47, 47, 31, 31, 0x7F,&BlockDrawer::drawFire);
    setBlockInfo( 52, 65, 65, 65, 65, 65, 65, 0x04);    //Spawner
        //WoodStairs
    setBlockInfo( 53, 4,  4,  4,  4,  4,  4,  0x10,&BlockDrawer::drawStairs);
        //Chest (*)
    setBlockInfo( 54, 26, 26, 25, 25, 26, 27, 0x00,&BlockDrawer::drawChest);
        //Wire (*)
    setBlockInfo( 55, 84, 85, 84, 100,100,101,0x6F,&BlockDrawer::drawWire);
    setBlockInfo( 56, 50, 50, 50, 50, 50, 50, 0x00);    //DiamondOre
    setBlockInfo( 57, 24, 24, 24, 24, 24, 24, 0x00);    //DiamondBlock
    setBlockInfo( 58, 60, 60, 43, 43, 59, 59, 0x00);    //Workbench
        //Crops (*)
    setBlockInfo( 59, 90, 91, 92, 93, 94, 95, 0xF7,&BlockDrawer::drawCrops);
    setBlockInfo( 60, 2,  2,  2,  86, 2,  2,  0x00);    //Soil
        //Furnace (+)
    setBlockInfo( 61, 45, 45, 62, 62, 45, 44, 0x00,&BlockDrawer::drawFaceCube);
        //LitFurnace (+)
    setBlockInfo( 62, 45, 45, 62, 62, 45, 61, 0x08,&BlockDrawer::drawFaceCube);
        //SignPost (*)
    setBlockInfo( 63, 4,  4,  4,  4,  4,  4,  0x47,&BlockDrawer::drawSignpost);
        //WoodDoor (*)
    setBlockInfo( 64, 97, 81, 97, 81, 97, 81, 0xA7,&BlockDrawer::drawDoor);
        //Ladder (*)
    setBlockInfo( 65, 83, 83, 83, 83, 83, 83, 0x57,&BlockDrawer::drawWallItem);
        //Track (*)
    setBlockInfo( 66, 112,112,128,128,128,128,0x67,&BlockDrawer::drawTrack);
        //CobbleStairs
    setBlockInfo( 67, 16, 16, 16, 16, 16, 16, 0x10,&BlockDrawer::drawStairs);
        //WallSign (*)
    setBlockInfo( 68, 4,  4,  4,  4,  4,  4,  0xD7,&BlockDrawer::drawWallSign);
        //Lever
    setBlockInfo( 69, 96, 96, 96, 96, 96, 16, 0x27,&BlockDrawer::drawLever);
        //StonePlate
    setBlockInfo( 70, 1,  1,  1,  1,  1,  1, 0xB7,&BlockDrawer::drawFloorplate);
        //IronDoor (*)
    setBlockInfo( 71, 98, 82, 98, 82, 98, 82, 0xA7,&BlockDrawer::drawDoor);
        //WoodPlate
    setBlockInfo( 72, 4,  4,  4,  4,  4,  4, 0xB7,&BlockDrawer::drawFloorplate);
    setBlockInfo( 73, 51, 51, 51, 51, 51, 51, 0x00);    //RedstoneOre
    setBlockInfo( 74, 51, 51, 51, 51, 51, 51, 0x08);    //RedstoneOreLit(*)
        //RedstoneTorch
    setBlockInfo( 75, 115,115,115,115,115,115,0xF7,&BlockDrawer::drawTorch);
        //RedstoneTorchLit
    setBlockInfo( 76, 99, 99, 99, 99, 99, 99, 0xFF,&BlockDrawer::drawTorch);
        //StoneButton
    setBlockInfo( 77, 1,  1,  1,  1,  1,  1,  0xE7,&BlockDrawer::drawButton);
        //SnowLayer(*)
    setBlockInfo( 78, 66, 66, 66, 66, 66, 66, 0xC0,&BlockDrawer::draw4thBlock);
        //BlockID 2 (Grass) below a a SnowLayer uses texture 68 on the sides
    setBlockInfo( 79, 67, 67, 67, 67, 67, 67, 0x04);    //Ice
    setBlockInfo( 80, 66, 66, 66, 66, 66, 66, 0x00);    //SnowBlock
        //Cactus
    setBlockInfo( 81, 70, 70, 71, 69, 70, 70, 0xF0,&BlockDrawer::drawCactus);
    setBlockInfo( 82, 72, 72, 72, 72, 72, 72, 0x00);    //Clay
        //Sugarcane (*)
    setBlockInfo( 83, 73, 73, 73, 73, 73, 73, 0xF7,&BlockDrawer::drawItem);
    setBlockInfo( 84, 74, 74, 43, 75, 74, 74, 0x00);    //Jukebox
        //Fence (*)
    setBlockInfo( 85, 4,  4,  4,  4,  4,  4,  0x94,&BlockDrawer::drawFence);
        //Pumpkin
    setBlockInfo( 86, 118,118,118,102,118,119,0x00,&BlockDrawer::drawFaceCube);
    setBlockInfo( 87, 103,103,103,103,103,103,0x00);    //Netherstone
    setBlockInfo( 88, 104,104,104,104,104,104,0x01);    //SlowSand
    setBlockInfo( 89, 105,105,105,105,105,105,0x08);    //Lightstone
        //Portal
    setBlockInfo( 90, 49, 49, 49, 49, 49, 49, 0x8F,&BlockDrawer::drawPortal);
        //PumpkinLit
    setBlockInfo( 91, 118,118,118,102,118,120,0x08,&BlockDrawer::drawFaceCube);
        //Cake block (*)
    setBlockInfo( 92, 123,122,124,121,122,122,0xC4,&BlockDrawer::drawCake);
    
//0xF0: Shape : 0=cube, 1=stairs, 2=toggle, 3=halfblock,
//              4=signpost, 5=ladder, 6=track, 7=fire
//              8=portal, 9=fence, A=door, B=floorplate
//              C=snow?, D=wallsign, E=button, F=planted
//0x08: Bright: 0=dark, 1=lightsource
//0x04: Vision: 0=opqaue, 1=see-through
//0x03: State : 0=solid, 1=loose, 2=liquid, 3=gas

    return true;
}
