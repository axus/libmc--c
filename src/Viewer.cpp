/*
  mc__::Viewer
  Draw mc__::World objects using OpenGL

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
#include "Viewer.hpp"
using mc__::Viewer;
using mc__::face_ID;
using mc__::World;
using mc__::MapChunk;

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

const float Viewer::PI = std::atan(1.0)*4;


//Library version info
unsigned long mc__::getVersion() { return MC__VIEWER_VERSION; }

Viewer::Viewer(unsigned short width, unsigned short height):
    cam_X(0), cam_Y(0), cam_Z(0), drawDistance(4096.f),
    view_width(width), view_height(height),
    aspectRatio((GLfloat)width/height), fieldOfViewY(70),
    cam_yaw(0), cam_pitch(0), cam_vecX(0), cam_vecY(0), cam_vecZ(0),
    item_rotation(0),
    use_mipmaps(true), use_blending(false), debugging(false)
{
  
    //TODO: depends on mapchunk biome setting
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

//Start up OpenGL
bool Viewer::init(const std::string& filename,
    const std::string& item_filename,
    bool mipmaps)
{

    bool result=true;

    //Compare devIL DLL version to header version
    if (ilGetInteger(IL_VERSION_NUM) < IL_VERSION) {
        cerr << "DevIL wrong DLL version" << endl;
        return false;
    }

    //DevIL textures
    ILuint il_texture_map, il_icon_map;

    //Graphics options
    use_mipmaps=mipmaps;

    //Remember filenames
    texture_map_file = filename;
    item_icon_file = item_filename;

    //Load game block information
    loadBlockInfo();

    //Initialize OpenGL
    startOpenGL();

    //Start DevIL
    ilInit();

    //Load item icon map, copy to openGL texture
    il_icon_map = loadImageFile(item_icon_file);
    if (il_icon_map == 0) {
        result=false;
        cerr << "Error loading " << item_icon_file << endl;
    } else {
        //glBind texture before assigning it
        glBindTexture(GL_TEXTURE_2D, item_tex);
        
        //Copy current DevIL image to OpenGL image.
        glTexImage2D(GL_TEXTURE_2D, 0, ilGetInteger(IL_IMAGE_BPP),
            ilGetInteger(IL_IMAGE_WIDTH), ilGetInteger(IL_IMAGE_HEIGHT), 0,
            ilGetInteger(IL_IMAGE_FORMAT), GL_UNSIGNED_BYTE, ilGetData());
    }

    //Load terrain texture map, bind it to current DevIL image
    il_texture_map = loadImageFile(texture_map_file);
    if (il_texture_map == 0) {
        result = false;   //error, exit program
        cerr << "Error loading " << texture_map_file << endl;
    } else {
        //glBind texture before assigning it
        glBindTexture(GL_TEXTURE_2D, terrain_tex);
        
        //Copy current DevIL image to OpenGL image.
        glTexImage2D(GL_TEXTURE_2D, 0, ilGetInteger(IL_IMAGE_BPP),
            ilGetInteger(IL_IMAGE_WIDTH), ilGetInteger(IL_IMAGE_HEIGHT), 0,
            ilGetInteger(IL_IMAGE_FORMAT), GL_UNSIGNED_BYTE, ilGetData());
    
    }

    //Change camera to model view mode
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    //Generate the ITEM, ENTITY, and PLAYER display lists
    loadItemInfo();

    return result;
}

//change back to texture if needed
void Viewer::rebindTerrain()
{
    //glBind texture before assigning it
    glBindTexture(GL_TEXTURE_2D, terrain_tex);
}

//
//Camera functions
//

//Reset camera angle
void Viewer::reset(GLfloat x, GLfloat y, GLfloat z, GLfloat yaw, GLfloat pitch)
{
    //Update camera position
    cam_X = x;
    cam_Y = y;
    cam_Z = z;
    
    //Update camera angles
    cam_yaw = yaw;
    cam_pitch = pitch;

    //Update camera vector
    float radians = cam_yaw*PI/180.0;
    cam_vecX = sin(radians);
    cam_vecZ = cos(radians);

}

//Move camera without rotating
void Viewer::move( GLfloat side, GLfloat up, GLfloat forward)
{
    //Move up or down
    cam_Y += up;
    
    //Movement along line of site, depends on camera vector (yaw)
    cam_X += int(side*cam_vecZ + forward*cam_vecX);
    cam_Z += int(side*cam_vecX - forward*cam_vecZ);
    
}

//Rotate camera degrees/360 about Y-axus
void Viewer::turn( GLfloat degrees)
{
    //Update yaw
    cam_yaw += degrees;
    
    //Update camera vector
    float radians = cam_yaw*PI/180.0;
    cam_vecX = sin(radians);
    cam_vecZ = cos(radians);
}

//Adjust head view up/down
void Viewer::tilt( GLfloat degrees)
{
    cam_pitch += degrees;
}

/*
//Look at a point from another point.  (up_x, up_y, up_z) is a vector point "up"
void Viewer::lookAt( GLint from_x, GLint from_y, GLint from_z, 
             GLint at_x, GLint at_y, GLint at_z,
             GLint up_x, GLint up_y, GLint up_z)
{
    gluLookAt( from_x, from_y, from_z, at_x, at_y, at_z, up_x, up_y, up_z);
}
*/

//Resize GL viewport
void Viewer::viewport( GLint x, GLint y, GLsizei width, GLsizei height)
{
    //Save matrix mode
    glPushAttrib(GL_TRANSFORM_BIT);
  
    aspectRatio = (GLfloat)width / (GLfloat)height;
    glViewport( x, y, width, height);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(fieldOfViewY, aspectRatio, 1.0f, drawDistance);
    
    //Reload matrix mode
    glPopAttrib();
}

//Resize far draw distance
void Viewer::setDrawDistance( GLdouble d)
{
    gluPerspective(fieldOfViewY, aspectRatio, 1.0f, d);
}

//Set glColor if needed by block type and face
void Viewer::setBlockColor(uint8_t blockID, face_ID face)
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

//Use OpenGL to draw a solid cube with appropriate textures for blockID
void Viewer::drawCube( uint8_t blockID,
    GLint x, GLint y, GLint z, uint8_t vflags)
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

//Draw cactus... almost like a cube, but A, B, E, F are inset 1 pixel
void Viewer::drawCactus( uint8_t blockID,
    GLint x, GLint y, GLint z, uint8_t vflags)
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


//Use OpenGL to draw partial solid cube, with offsets, scale (TODO: rotation)
void Viewer::drawScaledBlock( uint8_t blockID,
    GLint x, GLint y, GLint z, uint8_t vflags,
    GLfloat scale_x, GLfloat scale_y, GLfloat scale_z,
    bool scale_texture,
    GLint off_x, GLint off_y, GLint off_z)
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
void Viewer::drawHalfBlock( uint8_t blockID, GLint x, GLint y, GLint z,
    uint8_t visflags)
{
    //TODO: metadata to determine which half!
    drawScaledBlock( blockID, x, y, z, visflags, 1, 0.5, 1);
}

//Draw item blockID which is placed flat on the ground
void Viewer::drawTrack( uint8_t blockID, GLint x, GLint y, GLint z)
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
void Viewer::drawWallItem( uint8_t blockID, GLint x, GLint y, GLint z)
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
    //if (!(vflags & 0x08)) {
        tx_0 = blockInfo[blockID].tx[NORTH];
        tx_1 = blockInfo[blockID].tx[NORTH] + tmr;
        ty_0 = blockInfo[blockID].ty[NORTH] + tmr;
        ty_1 = blockInfo[blockID].ty[NORTH];
        
        glTexCoord2f(tx_0,ty_0); glVertex3i( B, C, E);  //Lower left:  BCE
        glTexCoord2f(tx_1,ty_0); glVertex3i( A, C, E);  //Lower right: ACE
        glTexCoord2f(tx_1,ty_1); glVertex3i( A, D, E);  //Top right:   ADE
        glTexCoord2f(tx_0,ty_1); glVertex3i( B, D, E);  //Top left:    BDE
    //}
    
    //F
    //if (!(vflags & 0x04)) {
        tx_0 = blockInfo[blockID].tx[SOUTH];
        tx_1 = blockInfo[blockID].tx[SOUTH] + tmr;
        ty_0 = blockInfo[blockID].ty[SOUTH] + tmr;
        ty_1 = blockInfo[blockID].ty[SOUTH];
        
        glTexCoord2f(tx_0,ty_0); glVertex3i( A, C, F);  //Lower left:  ACF
        glTexCoord2f(tx_1,ty_0); glVertex3i( B, C, F);  //Lower right: BCF
        glTexCoord2f(tx_1,ty_1); glVertex3i( B, D, F);  //Top right:   BDF
        glTexCoord2f(tx_0,ty_1); glVertex3i( A, D, F);  //Top left:    ADF
    //}
}

//Draw item blockID which is placed as a block
void Viewer::drawItem( uint8_t blockID, GLint x, GLint y, GLint z)
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

//Draw a dropped item that can be picked up (caller must translate to X,Y,Z)
void Viewer::drawDroppedItem( uint16_t itemID )
{

    //To always face player, translate then rotate at camera(?)

    //Scale the size of the item picture, and offset up by 2,2,2
    GLint width, height, depth;
    
    width  = texmap_TILE_LENGTH >> 1;   //half openGL length = 8
    height = texmap_TILE_LENGTH >> 1;
    depth  = texmap_TILE_LENGTH >> 1;
    
    //Face coordinates (in pixels)
    GLint A = -(texmap_TILE_LENGTH >> 2);  //offset 1/4 openGL length
    GLint B = -(texmap_TILE_LENGTH >> 2) + width;
    GLint C = 0;
    GLint D = height;
    GLint G = 0;    //No offset from middle

    //Texture map coordinates (0.0 - 1.0)
    GLfloat tx_0, tx_1, ty_0, ty_1;

    //Look up texture coordinates for the item
    tx_0 = itemInfo[itemID].tx[WEST];
    tx_1 = itemInfo[itemID].tx[WEST] + tmr;
    ty_0 = itemInfo[itemID].ty[WEST] + tmr;    //flip y
    ty_1 = itemInfo[itemID].ty[WEST];
    glTexCoord2f(tx_0,ty_0); glVertex3i( A, C, G);  //Lower left:  ACG
    glTexCoord2f(tx_1,ty_0); glVertex3i( B, C, G);  //Lower right: BCG
    glTexCoord2f(tx_1,ty_1); glVertex3i( B, D, G);  //Top right:   BDG
    glTexCoord2f(tx_0,ty_1); glVertex3i( A, D, G);  //Top left:    ADG

    //Back face
    glTexCoord2f(tx_0,ty_0); glVertex3i( A, C, G);  //Lower left:  ACG
    glTexCoord2f(tx_0,ty_1); glVertex3i( A, D, G);  //Top left:    ADG
    glTexCoord2f(tx_1,ty_1); glVertex3i( B, D, G);  //Top right:   BDG
    glTexCoord2f(tx_1,ty_0); glVertex3i( B, C, G);  //Lower right: BCG

}


//Draw a placed mc__::Block in openGL
void Viewer::drawBlock( const mc__::Block& block,
    GLint x, GLint y, GLint z, uint8_t vflags)
{

    //Don't draw invisible blocks
    if (block.blockID == 0 || (vflags & 0x2)) {
        return;
    }

    //Drawing function depends on shape (as determined from properties)
    //0xF0: Shape : 0=cube, 1=stairs, 2=lever, 3=halfblock,
    //              4=signpost, 5=ladder, 6=track, 7=fire
    //              8=portal, 9=fence, A=door, B=floorplate
    //              C=snow cover, D=wallsign, E=button, F=plant

    //Use different drawing function for different model types
    switch ( (blockInfo[block.blockID].properties & 0xF0)>>4 ) {
        case 0x0:
            //Cube
            drawCube(block.blockID, x, y, z, vflags);
            break;
        case 0x1:
            //Stairs
            drawScaledBlock(block.blockID, x, y, z, vflags,
                1, 0.5, 0.5, true, 0, 8, 0);
            drawHalfBlock(block.blockID, x, y, z, vflags);
            break;
        case 0x2:   //Lever
            drawItem(block.blockID, x, y, z);
            
            //Cobblestone base
            drawScaledBlock(4, x, y, z, 0,
                0.25, 0.25, 0.5, true, 6, 0, 4);
            break;
        case 0x3:   //half-block
            drawHalfBlock(block.blockID, x, y, z, vflags);
            break;
        case 0x4:   //Signpost
            //TODO: drawSignpost();
            
            //Sign
            drawScaledBlock(block.blockID, x, y, z, 0,
                0.75, 0.5, 0.125, true, 2, 7, 8);
                
            //Post
            drawScaledBlock(block.blockID, x, y, z, (vflags&0x30)|0x10,
                0.125, 0.4375, 0.125, true, 7, 0, 8);
            break;
        case 0x5:   //Ladder
        case 0x7:   //fire
        case 0xA:   //Door
            drawWallItem(block.blockID, x, y, z);
            break;
        case 0x6:   //Track
            drawTrack(block.blockID, x, y, z);
            break;
        case 0x8:   //Portal
            drawScaledBlock(block.blockID, x, y, z, 0,
                1.0, 1.0, 0.25, true, 0, 0, 8);
            break;
        case 0x9:   //Fence
            //Top of fence
            drawScaledBlock(block.blockID, x, y, z, vflags&0xC0,
                1, 0.25, 0.25, true, 0, 6, 6);
            //Fence legs
            drawScaledBlock(block.blockID, x, y, z, (vflags&0x30)|0x10,
                0.25, 0.375, 0.25, true, 2, 0, 6);
            drawScaledBlock(block.blockID, x, y, z, (vflags&0x30)|0x10,
                0.25, 0.375, 0.25, true, 10, 0, 6);
            break;
        case 0xB:   //Floorplate
            drawScaledBlock(block.blockID, x, y, z, 0,
                0.75, 0.125, 0.75, true, 2, 0, 2);
            break;
        case 0xC:   //Snow, Cake, Cactus (other special semi-cubes)
            drawScaledBlock(block.blockID, x, y, z, vflags,
                1, 0.25, 1);
            break;
        case 0xD:   //Wallsign
            drawScaledBlock(block.blockID, x, y, z, (vflags&0x08),
                0.75, 0.5, 0.125, true, 2, 7, 0);
            break;
        case 0xE:   //Button
            drawScaledBlock(block.blockID, x, y, z, 0,
                0.25, 0.25, 0.125, true, 8, 8, 0);
            break;
        case 0xF:   //Plant
            if (block.blockID != 0x51) {
                drawItem(block.blockID, x, y, z);
            } else {    //Special case cactus
                drawCactus(block.blockID, x, y, z, vflags);
            }
            break;
        default:    //Draw unknown types as a cube
            drawCube(block.blockID, x, y, z, vflags);
            break;
    }

}

using mc__::chunkSet_t;

//Create GL display list for mapchunk, if needed
void Viewer::drawMapChunk(MapChunk* mapchunk)
{
    MapChunk& myChunk = *mapchunk;

    //Don't draw invisible or unloaded mapchunks
    if ( (myChunk.flags & MapChunk::DRAWABLE) != MapChunk::DRAWABLE ) {
        return;
    }

    //If the chunk has not uncompressed it's data, do so now
    if (! myChunk.isUnzipped) {        
        //Debug output
        if (debugging) {
            cout << "Unzipping chunk @ " << (int)myChunk.X << ","
                << (int)myChunk.Y << "," << (int)myChunk.Z << endl;
        }
        //Unzip the chunk
        if (!myChunk.unzip(true)) {
            cerr << "ERROR UNZIPPING" << endl;
            return;
        }
        
        //Requires new glList
        myChunk.flags |= MapChunk::UPDATED;
    }

    //Get gl_list associated with map chunk
    GLuint gl_list=0;
    mapChunkUintMap_t::const_iterator iter = glListMap.find(mapchunk);
    if (iter != glListMap.end()) {
        gl_list = iter->second;

        //Draw the precompiled list (might be recalculated after)
        glCallList(gl_list);

    } else {
        //Create new list to be calculated
        gl_list = glGenLists(1);
        glListMap[mapchunk] = gl_list;
        myChunk.flags |= MapChunk::UPDATED;
    }

    //Compile GL list if needed (drawing to screen happens elsewhere)
    if (myChunk.flags & MapChunk::UPDATED) {

        //DEBUG updates
        //cout << "MapChunk UPDATED flag: " << (int)myChunk.X << ","
        //        << (int)myChunk.Y << "," << (int)myChunk.Z << endl;

        //chunk vars
        GLint X=myChunk.X>>4;   //chunk coord for far block face culling
        GLint Y=64;
        GLint Z=myChunk.Z>>4;   //chunk coord for far block face culling
        indexList_t& visibleIndices = myChunk.visibleIndices;
        indexList_t::const_iterator iter;

        //Chunk coordinates to compare... remove GL coordinate and last 4 bits
        GLint view_X = ((int)cam_X >> 8);
        GLint view_Z = ((int)cam_Z >> 8);

        //Calculate facemask to apply to visflags, based on chunk X/Z
        //Mark faces player cannot see from their MapChunk as invisible
        uint8_t pvflags = 0;
        pvflags |= (X < view_X ? 0x80 : (X > view_X ? 0x40 : 0x00));
        pvflags |= (Z < view_Z ? 0x08 : (Z > view_Z ? 0x04 : 0x00));
                
        //Start the GL_COMPILING! Don't execute.
        glNewList(gl_list, GL_COMPILE);

        //Rebind terrain png
        glBindTexture( GL_TEXTURE_2D, terrain_tex);  

        glBegin(GL_QUADS);
        
        //Draw the visible blocks
        for (iter = visibleIndices.begin(); iter != visibleIndices.end(); iter++)
        {
            //When indexing block in chunk array,
            //index = y + (z << 7) + (x << 11)
            uint16_t index = *iter;
            X = myChunk.X + (index >> 11);
            Y = myChunk.Y + (index & 0x7F);
            Z = myChunk.Z + ((index >> 7) & 0xF);
            
            //Base visibility flags
            uint8_t vflags = myChunk.visflags[index];
            
            //Draw the block (based on block type)
            drawBlock(myChunk.block_array[index], X, Y, Z, vflags/*pvflags|*/);
            //Uncomment pvflags to hide faces player can't see
        }
        //End the list
        glEnd();
        glEndList();

        //Finished drawing chunk, no longer "updated"
        myChunk.flags &= ~(MapChunk::UPDATED);
    }
}

//Draw all moving objects (entities)
bool Viewer::drawMobiles(const mc__::Mobiles& mobiles)
{
    itemMap_t::const_iterator item_iter;
    
    const itemMap_t& itemMap = mobiles.itemMap;
    GLuint displayList;
    
    //Go through all items in the world that we know about
    //  TODO: items in visible range
    for (item_iter = itemMap.begin(); item_iter != itemMap.end(); item_iter++) {
        //NO ERROR CHECKING HERE!  FOR GREAT JUSTICE
        
        
        //Get display list for item
        const Item *item = item_iter->second;
        
        //item ID might depend on "damage" field.
        uint16_t itemID = item->itemID;
        if (itemInfo[item->itemID].dataOffset != 0) {
            itemID = itemInfo[item->itemID].dataOffset + item->hitpoints;
        }
        
        //Draw the 
        displayList = itemModels[ itemID ];

        //Translate world to item coordinates (offset from camera)
        drawFromCamera();
        glTranslatef( (item->X >> 1) + texmap_TILE_LENGTH/2,
                      (item->Y >> 1) + 2,
                      (item->Z >> 1) + texmap_TILE_LENGTH/2);
        glRotatef( item->yaw + item_rotation, 0.0f, 1.0f, 0.0f);

        //Draw the precompiled list
        glCallList(displayList);

    }

    
    return true;
}

/*
bool Viewer::createEntityModels()
{
    //typeID    Description
    //------    -----------
    //0         Player
                
    //1         Boat
    //10        MineCart
    //11        ChestCart
    //12        FurnaceCart
    
    //50        Creeper
    //51        Skeleton
    //52        Spider
    //53        Giant Zombie
    //54        Zombie
    //55        Slime
    //56        Ghast
    //57        Zombie Pigman
    
    //90        Pig
    //91        Sheep
    //92        Cow
    //93        Chicken

    return true;
}
*/

//Draw the megachunks in mc__::World
void Viewer::drawMapChunks( const World& world)
{
    //Use the mapChunkList of all map chunks to draw them
    const mapChunkList_t& mapChunks = world.mapChunks;
    mapChunkList_t::const_iterator iter;
    for (iter = mapChunks.begin(); iter != mapChunks.end(); iter++)
    {
        drawMapChunk(*iter);
    }
    
}

//OpenGL Set up buffer, perspective, blah blah blah
void Viewer::startOpenGL() {
    //Set color and depth clear value
    glClearDepth(1.0f);
    glClearColor(0.f, 0.f, 0.f, 1.f);

    // Enable Z-buffer read and write
    glEnable(GL_DEPTH_TEST);
    glDepthMask(GL_TRUE);

    //Set viewport and perspective
    viewport(0, 0, view_width, view_height);

    //Enable texture mapping
    glEnable(GL_TEXTURE_2D);
    glEnable(GL_CULL_FACE); glCullFace(GL_BACK);
    
    //Transparency in textures
    glEnable(GL_ALPHA_TEST);
    glAlphaFunc(GL_GREATER, 0.5f);
    
    //Clear screen and set location to origin in modelview mode
    glMatrixMode(GL_MODELVIEW);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
    glLoadIdentity();

    //Save the original viewpoint
    glPushMatrix(); 

    //Create memory, set options for openGL terrain texture
    glGenTextures(1, &terrain_tex);
    configureTexture(terrain_tex);
    
    //...for item texture
    glGenTextures(1, &item_tex);
    configureTexture(item_tex);
    
    //...for entity textures
    glGenTextures(entity_type_MAX, entity_tex);
    //TODO: configureTexture for each one
    
    //Blending setting
    if (!use_blending) {
        glDisable(GL_BLEND);
    } else {
        glEnable(GL_BLEND);
    }

}

//Load image file using DevIL
ILuint Viewer::loadImageFile( const string &imageFilename) {
  
    ILuint il_texture;
  
    ilGenImages(1, &il_texture);
    ilBindImage(il_texture);

    if (ilLoad(IL_PNG, imageFilename.c_str())) {
      
        //Convert image to RGBA
        if (!ilConvertImage(IL_RGBA, IL_UNSIGNED_BYTE)) {
            cerr << "Error converting " << imageFilename << endl;
            ILenum ilErrorCode = 0;
            while ( (ilErrorCode = ilGetError()) != IL_NO_ERROR) {
                cerr << "Error code " << ilErrorCode << endl;
            }
            return 0;
        }
    
    } else {
        cerr << "Error loading " << imageFilename << endl;
        ILenum ilErrorCode = 0;
        while ( (ilErrorCode = ilGetError()) != IL_NO_ERROR) {
            cerr << "Error code " << ilErrorCode << endl;
        }
        return 0;
    }

    return il_texture;
}

//Set texture parameters
bool Viewer::configureTexture(GLuint texture_ID)
{
    glBindTexture(GL_TEXTURE_2D, texture_ID);
    
    //Set out-of-range texture coordinates
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);

    //Make textures "blocky" when up close
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    
    //MipMap setting for far away textures
    if (use_mipmaps) {
        //Create OpenGL texture
        glHint(GL_GENERATE_MIPMAP_HINT_SGIS, GL_NICEST);//Nice mipmaps
        
        //Use texture mipmaps when far away
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
            GL_NEAREST_MIPMAP_NEAREST);
    
        //Generate mipmaps
        glTexParameteri(GL_TEXTURE_2D,GL_GENERATE_MIPMAP_SGIS,GL_TRUE);
    } else {
        //Use nearest texture when far away
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    }
    
    //TODO: Check OpenGL error
    return true;
}


//Clear old polygons and colors
void Viewer::clear() 
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
}

//Offset model view from current camera position
void Viewer::drawFromCamera()
{
    //Reset camera
    glLoadIdentity();
    glRotatef( cam_yaw, 0.0f, 1.0f, 0.0f);
    
    //Change roll/pitch to look up/down
    glRotatef( cam_pitch, cam_vecZ, 0.0f, cam_vecX);
    
    //Bring the world to the camera, not the camera to the world
    glTranslatef( -cam_X, -cam_Y, -cam_Z );
}


//OpenGL rendering of cubes.  No update to camera.
bool Viewer::drawWorld(const World& world)
{
    //Reset camera
    drawFromCamera();
    
    //Draw the map-chunks
    drawMapChunks(world);
    
    return true;
}


//Copy block info to struct
void Viewer::setBlockInfo( uint8_t index,
    uint8_t A, uint8_t B,
    uint8_t C, uint8_t D,
    uint8_t E, uint8_t F,
    uint8_t properties)
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
    
    //Assign properties
    //0xF0: Shape : 0=cube, 1=stairs, 2=lever, 3=halfblock,
    //              4=signpost, 5=ladder, 6=track, 7=fire?,
    //              8=portal, 9=fence, A=door, B=floorplate
    //              C=1/3 block, D=wallsign, E=button, F=plant
    //0x08: Bright: 0=dark, 1=lightsource
    //0x04: Vision: 0=opqaue, 1=see-through
    //0x03: State : 0=solid, 1=loose, 2=liquid, 3=gas
    blockInfo[index].properties = properties;
}

//Map item ID to texture coordinates and appropriate texture map (properties)
void Viewer::setItemInfo( uint16_t index, uint8_t A, uint8_t properties, uint16_t offset)
{
    //A - F contain texture ID of the corresponding face (textureID = 0 - 255)
    BlockInfo& iteminf = itemInfo[index];
    
    //For each face,  precompute OpenGL texture offsets in texture map
    // (Remember that texture map was loaded upside down!)
    iteminf.textureID[0] = A;
    iteminf.tx[0] = float(A & (texmap_TILES-1))/((float)texmap_TILES);
    iteminf.ty[0] = float(A/texmap_TILES)/((float)texmap_TILES);

    //Copy properties
    iteminf.properties = properties;
    iteminf.dataOffset = offset;

    //draw to openGL model
    createItemModel(index);
    
    //createItemTexture(index);
}

//Create texture and model for item
bool Viewer::createItemModel( uint16_t index)
{
    BlockInfo& iteminf = itemInfo[index];

    //Create display list for item model
    itemModels[index] = glGenLists(1);
    glNewList(itemModels[index], GL_COMPILE);

    //Draw model to display list
    switch (iteminf.properties & 0x07) {
        case 0:
            //Terrain cube (as item 75% size)
            glBindTexture( GL_TEXTURE_2D, terrain_tex);
            glBegin(GL_QUADS);
            //Centered at current position
            drawScaledBlock( index&0xFF, 0, 0, 0, 0, 0.25, 0.25, 0.25,
                false, -2, 0, -2);
            glEnd();
            break;
        case 1:
            //Terrain item (as item 75% size)
            glBindTexture( GL_TEXTURE_2D, terrain_tex);
            glBegin(GL_QUADS);
            //Centered at current position
            drawScaledBlock( index&0xFF, 0, 0, 0, 0, 0.25, 0.25, 0.25,
                false, -2, 0, -2);
            glEnd();
            break;
        case 2:     //Regular item icon :)
        case 6:     //Item icon depends on damage field, change ID at run time
            //Inventory item
            glBindTexture( GL_TEXTURE_2D, item_tex);
            glBegin(GL_QUADS);
            drawDroppedItem( index);
            glEnd();
            break;
        case 3:
            //Special inventory item
            glBindTexture( GL_TEXTURE_2D, item_tex);
            glBegin(GL_QUADS);
            drawDroppedItem( index);
            glEnd();
            break;
            
        default:
            break;
    }
    
    glEndList();
    
    return true;
}

/*
//Create the display lists that will be used for drawing mobiles
bool Viewer::createItemModels()
{
    uint16_t ID;
    
    //Block item models
    for (ID=1; ID < Chunk::maxBlockID; ID++) {
        createItemModel(ID);
    }
    
    return true;
}
*/

//Map block ID to block type information
bool Viewer::loadBlockInfo()
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
//              4=signpost, 5=ladder, 6=track, 7=1/4 block
//              8=portal, 9=fence, A=door, B=floorplate
//              C=snow?, D=wallsign, E=button, F=planted
//0x08: Bright: 0=dark, 1=lightsource
//0x04: Vision: 0=opqaue, 1=see-through
//0x03: State : 0=solid, 1=loose, 2=liquid, 3=gas

Planted item = 0xF7: planted, dark, see-through, move-through
Normal block = 0x00: cube, dark, opaque, solid
*/

// 11 is the transparent texture

    //Set specific blocks
    setBlockInfo( 0, 11, 11, 11, 11, 11, 11, 0x07);     //Air   (should not be drawn!)
    setBlockInfo( 1, 1, 1, 1, 1, 1, 1,       0x00);     //Stone
    setBlockInfo( 2, 3, 3, 2, 0, 3, 3,       0x00);     //Grass
    setBlockInfo( 3, 2, 2, 2, 2, 2, 2,       0x00);     //Dirt
    setBlockInfo( 4, 16, 16, 16, 16, 16, 16, 0x00);     //Cobble
    setBlockInfo( 5, 4, 4, 4, 4, 4, 4,       0x00);     //Wood
    setBlockInfo( 6, 15, 15, 15, 15, 15, 15, 0xF7);     //Sapling
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
    setBlockInfo( 17, 20, 20, 21, 21, 20, 20, 0x00);    //Log
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
    setBlockInfo( 37, 13, 13, 13, 13, 13, 13, 0xF7);    //Flower
    setBlockInfo( 38, 12, 12, 12, 12, 12, 12, 0xF7);    //Rose
    setBlockInfo( 39, 29, 29, 29, 29, 29, 29, 0xF7);    //BrownShroom
    setBlockInfo( 40, 28, 28, 28, 28, 28, 28, 0xF7);    //RedShroom
//    setBlockInfo( 41, 39, 39, 55, 23, 39, 39, 0x00);    //GoldBlock
//    setBlockInfo( 42, 38, 38, 54, 22, 38, 38, 0x00);    //IronBlock
    setBlockInfo( 41, 23, 23, 23, 23, 23, 23, 0x00);    //GoldBlock
    setBlockInfo( 42, 22, 22, 22, 22, 22, 22, 0x00);    //IronBlock
    setBlockInfo( 43, 5,  5,  6,  6,  5,  5,  0x00);    //DoubleStep
    setBlockInfo( 44, 5,  5,  6,  6,  5,  5,  0x30);    //Step
    setBlockInfo( 45, 7,  7,  7,  7,  7,  7,  0x00);    //Brick
    setBlockInfo( 46, 8,  8, 10,  9,  8,  8,  0x00);    //TNT
    setBlockInfo( 47, 35, 35, 4,  4,  35, 35, 0x00);    //Bookshelf
    setBlockInfo( 48, 36, 36, 16, 36, 36, 36, 0x00);    //Mossy
    setBlockInfo( 49, 37, 37, 37, 37, 37, 37, 0x00);    //Obsidian
    setBlockInfo( 50, 80, 80, 80, 80, 80, 80, 0xFF);    //Torch
    setBlockInfo( 51, 31, 31, 47, 47, 31, 31, 0x7F);    //Fire
    setBlockInfo( 52, 65, 65, 65, 65, 65, 65, 0x04);    //Spawner
    setBlockInfo( 53, 4,  4,  4,  4,  4,  4,  0x10);    //WoodStairs
    setBlockInfo( 54, 26, 26, 25, 25, 26, 27, 0x00);    //Chest (*)
    setBlockInfo( 55, 84, 85, 84, 100,100,101,0x6F);    //Wire (*)
    setBlockInfo( 56, 50, 50, 50, 50, 50, 50, 0x00);    //DiamondOre
//    setBlockInfo( 57, 40, 40, 56, 24, 40, 40, 0x00);    //DiamondBlock
    setBlockInfo( 57, 24, 24, 24, 24, 24, 24, 0x00);    //DiamondBlock
    setBlockInfo( 58, 60, 60, 43, 43, 59, 59, 0x00);    //Workbench
    setBlockInfo( 59, 90, 91, 92, 93, 94, 95, 0xF7);    //Crops (*)
    setBlockInfo( 60, 2,  2,  2,  86, 2,  2,  0x00);    //Soil
    setBlockInfo( 61, 45, 45, 62, 62, 45, 44, 0x00);    //Furnace (+)
    setBlockInfo( 62, 45, 45, 62, 62, 45, 61, 0x08);    //LitFurnace (+)
    setBlockInfo( 63, 4,  4,  4,  4,  4,  4,  0x47);    //SignPost (*)
    setBlockInfo( 64, 97, 81, 97, 81, 97, 81, 0xA7);    //WoodDoor (*)
    setBlockInfo( 65, 83, 83, 83, 83, 83, 83, 0x57);    //Ladder (*)
    setBlockInfo( 66, 112,112,128,128,128,128,0x67);    //Track (*)
    setBlockInfo( 67, 16, 16, 16, 16, 16, 16, 0x10);    //CobbleStairs
    setBlockInfo( 68, 4,  4,  4,  4,  4,  4,  0xD7);    //WallSign (*)
    setBlockInfo( 69, 96, 96, 96, 96, 96, 16, 0x27);    //Lever
    setBlockInfo( 70, 1,  1,  1,  1,  1,  1,  0xB7);    //StonePlate
    setBlockInfo( 71, 98, 82, 98, 82, 98, 82, 0xA7);    //IronDoor (*)
    setBlockInfo( 72, 4,  4,  4,  4,  4,  4,  0xB7);    //WoodPlate
    setBlockInfo( 73, 51, 51, 51, 51, 51, 51, 0x00);    //RedstoneOre
    setBlockInfo( 74, 51, 51, 51, 51, 51, 51, 0x08);    //RedstoneOreLit(*)
    setBlockInfo( 75, 115,115,115,115,115,115,0xF7);    //RedstoneTorch
    setBlockInfo( 76, 99, 99, 99, 99, 99, 99, 0xFF);    //RedstoneTorchLit
    setBlockInfo( 77, 1,  1,  1,  1,  1,  1,  0xE7);    //StoneButton
    setBlockInfo( 78, 66, 66, 66, 66, 66, 66, 0xC0);    //SnowLayer(*)
        //BlockID 2 (Grass) below a a SnowLayer uses texture 68 on the sides
    setBlockInfo( 79, 67, 67, 67, 67, 67, 67, 0x04);    //Ice
    setBlockInfo( 80, 66, 66, 66, 66, 66, 66, 0x00);    //SnowBlock
    setBlockInfo( 81, 70, 70, 71, 69, 70, 70, 0xF0);    //Cactus
    setBlockInfo( 82, 72, 72, 72, 72, 72, 72, 0x00);    //Clay
    setBlockInfo( 83, 73, 73, 73, 73, 73, 73, 0xF7);    //Sugarcane (*)
    setBlockInfo( 84, 74, 74, 43, 75, 74, 74, 0x00);    //Jukebox
    setBlockInfo( 85, 4,  4,  4,  4,  4,  4,  0x94);    //Fence (*)
    setBlockInfo( 86, 118,118,118,102,118,119,0x00);    //Pumpkin
    setBlockInfo( 87, 103,103,103,103,103,103,0x00);    //Netherstone
    setBlockInfo( 88, 104,104,104,104,104,104,0x01);    //SlowSand
    setBlockInfo( 89, 105,105,105,105,105,105,0x08);    //Lightstone
    setBlockInfo( 90, 49, 49, 49, 49, 49, 49, 0x8F);    //Portal (??)
    setBlockInfo( 91, 118,118,118,102,118,120,0x08);    //PumpkinLit
    setBlockInfo( 92, 122,122,124,121,123,123,0xF0);    //Cake (*)
    

//0xF0: Shape : 0=cube, 1=stairs, 2=toggle, 3=halfblock,
//              4=signpost, 5=ladder, 6=track, 7=fire
//              8=portal, 9=fence, A=door, B=floorplate
//              C=snow?, D=wallsign, E=button, F=planted
//0x08: Bright: 0=dark, 1=lightsource
//0x04: Vision: 0=opqaue, 1=see-through
//0x03: State : 0=solid, 1=loose, 2=liquid, 3=gas

    return true;
}

//Map item ID to item information
bool Viewer::loadItemInfo()
{
    uint16_t ID;

    //  properties:
    // 0x00 = terrain cube (icon is single terrain cube)
    // 0x01 = terrain item (icon is single terrain texture)
    // 0x02 = item (icon is single tile from items.png)
    // 0x03 = special icon
    // 0x04 = item depends on damage data (0x06 for dyes)
    // 0x08 = biome colored
    // 0x10 = chest
    // 0x20 = lever
    // 0x40 = sign
    // 0x50 = furnace
    // 0x60 = workbench
    // 0xFF = air

// 11 is the transparent texture

    //Set "cube" blocks: ID, texture, properties
    setItemInfo( 0, 11, 0xFF);     //Air   (should not be drawn!)
    setItemInfo( 1, 1,  0x00);     //Stone
    setItemInfo( 2, 3,  0x08);     //Grass
    setItemInfo( 3, 2,  0x00);     //Dirt
    setItemInfo( 4, 16, 0x00);     //Cobble
    setItemInfo( 5, 4,  0x00);     //Wood
    setItemInfo( 6, 15, 0x01);     //Sapling
    setItemInfo( 7, 17, 0x00);     //Bedrock
    setItemInfo( 8, 0xCE, 0x00);     //Water(*)
    setItemInfo( 9, 0xCD, 0x00);     //WaterStill
    setItemInfo( 10, 0xEE,0x00);    //Lava(*)
    setItemInfo( 11, 0xED,0x00);    //LavaStill
    setItemInfo( 12, 18, 0x00);    //Sand
    setItemInfo( 13, 19, 0x00);    //Gravel
    setItemInfo( 14, 32, 0x00);    //GoldOre
    setItemInfo( 15, 33, 0x00);    //IronOre
    setItemInfo( 16, 34, 0x00);    //CoalOre
    setItemInfo( 17, 20, 0x00);    //Log
    setItemInfo( 18, 52, 0x08);    //Leaves
    setItemInfo( 19, 48, 0x00);    //Sponge
    setItemInfo( 20, 49, 0x00);    //Glass

    setItemInfo( 21, 140, 0x00);    //Lapis Ore
    setItemInfo( 22, 128, 0x00);    //Lapis Block
    setItemInfo( 23, 46, 0x00);    //Dispenser
    setItemInfo( 24, 156, 0x00);    //Sandstone
    setItemInfo( 25, 90, 0x00);    //Note Block
    
    for (ID = 26; ID < 37; ID++) {
        setItemInfo( ID, 64, 0x00);   //Cloth
        //TODO: unsure which item IDs are in the game for cloth
    }
    setItemInfo( 37, 13, 0x01);    //Flower
    setItemInfo( 38, 12, 0x01);    //Rose
    setItemInfo( 39, 29, 0x01);    //BrownShroom
    setItemInfo( 40, 28, 0x01);    //RedShroom
    setItemInfo( 41, 39, 0x00);    //GoldBlock
    setItemInfo( 42, 38, 0x00);    //IronBlock
    setItemInfo( 43, 5,  0x00);    //DoubleStep
    setItemInfo( 44, 5,  0x00);    //Step
    setItemInfo( 45, 7,  0x00);    //Brick
    setItemInfo( 46, 8,  0x00);    //TNT
    setItemInfo( 47, 35, 0x00);    //Bookshelf
    setItemInfo( 48, 36, 0x00);    //Mossy
    setItemInfo( 49, 37, 0x00);    //Obsidian
    setItemInfo( 50, 80, 0x01);    //Torch
    setItemInfo( 51, 30, 0x03);    //Fire
    setItemInfo( 52, 65, 0x00);    //Spawner
    setItemInfo( 53, 4,  0x00);    //WoodStairs
    setItemInfo( 54, 26, 0x10);    //Chest
    setItemInfo( 55, 84, 0x01);    //Wire
    setItemInfo( 56, 50, 0x00);    //DiamondOre
    setItemInfo( 57, 40, 0x00);    //DiamondBlock
    setItemInfo( 58, 60, 0x60);    //Workbench
    setItemInfo( 59, 90, 0x01);    //Crops
    setItemInfo( 60, 2,  0x00);    //Soil
    setItemInfo( 61, 44, 0x50);    //Furnace
    setItemInfo( 62, 61, 0x53);    //LitFurnace
    setItemInfo( 63, 42, 0x03);    //SignPost
    setItemInfo( 64, 43, 0x02);    //WoodDoor
    setItemInfo( 65, 83, 0x01);    //Ladder
    setItemInfo( 66, 112,0x01);    //Track
    setItemInfo( 67, 16, 0x00);    //CobbleStairs
    setItemInfo( 68, 42, 0x02);    //WallSign
    setItemInfo( 69, 96, 0x00);    //Lever
    setItemInfo( 70, 1,  0x00);    //StonePlate
    setItemInfo( 71, 44, 0x02);    //IronDoor
    setItemInfo( 72, 4,  0x00);    //WoodPlate
    setItemInfo( 73, 51, 0x00);    //RedstoneOre
    setItemInfo( 74, 51, 0x00);    //RedstoneOreLit
    setItemInfo( 75, 115,0x01);    //RedstoneTorch
    setItemInfo( 76, 99, 0x01);    //RedstoneTorchLit
    setItemInfo( 77, 1,  0x00);    //StoneButton
    setItemInfo( 78, 66, 0x00);    //SnowLayer
    setItemInfo( 79, 67, 0x00);    //Ice
    setItemInfo( 80, 66, 0x00);    //SnowBlock
    setItemInfo( 81, 70, 0x00);    //Cactus
    setItemInfo( 82, 72, 0x00);    //Clay Block
    setItemInfo( 83, 73, 0x01);    //Sugarcane
    setItemInfo( 84, 74, 0x00);    //Jukebox
    setItemInfo( 85, 4,  0x00);    //Fence
    setItemInfo( 86, 118,0x00);    //Pumpkin
    setItemInfo( 87, 103,0x00);    //Netherstone
    setItemInfo( 88, 104,0x00);    //SlowSand
    setItemInfo( 89, 105,0x00);    //Lightstone
    setItemInfo( 90, 49, 0x03);    //Portal
    setItemInfo( 91, 118,0x00);    //PumpkinLit
    setItemInfo( 92, 119,0x00);    //Cake block

    //Set default item information to sponge!
    for (ID = 93; ID < 256; ID++) {
        setItemInfo( ID, 48, 0x00);
    }

    //Set inventory items info: ID, texture, properties
    setItemInfo(256, 82,0x02);//    Iron Shovel
    setItemInfo(257, 98,0x02);//    Iron Pick
    setItemInfo(258,114,0x02);//    Iron Axe
    setItemInfo(259,  5,0x02);//    Flint n Steel
    setItemInfo(260, 10,0x02);//    Apple
    setItemInfo(261, 21,0x02);//    Bow
    setItemInfo(262, 37,0x02);//    Arrow
    setItemInfo(263,  7,0x02);//    Coal
    setItemInfo(264, 55,0x02);//    Diamond
    setItemInfo(265, 23,0x02);//    Iron Ingot
    setItemInfo(266, 39,0x02);//    Gold Ingot
    setItemInfo(267, 66,0x02);//    Iron Sword
    setItemInfo(268, 64,0x02);//    Wooden Sword
    setItemInfo(269, 80,0x02);//    Wooden Shovel
    setItemInfo(270, 96,0x02);//    Wooden Pickaxe
    setItemInfo(271,112,0x02);//    Wooden Axe
    setItemInfo(272, 65,0x02);//    Stone Sword
    setItemInfo(273, 81,0x02);//    Stone Shovel
    setItemInfo(274, 97,0x02);//    Stone Pickaxe
    setItemInfo(275,113,0x02);//    Stone Axe
    setItemInfo(276, 67,0x02);//    Diamond Sword
    setItemInfo(277, 83,0x02);//    Diamond Shovel
    setItemInfo(278, 99,0x02);//    Diamond Pickaxe
    setItemInfo(279,115,0x02);//    Diamond Axe
    setItemInfo(280, 53,0x02);//    Stick
    setItemInfo(281, 71,0x02);//    Bowl
    setItemInfo(282, 72,0x02);//    Mushroom Soup
    setItemInfo(283, 68,0x02);//    Gold Sword
    setItemInfo(284, 84,0x02);//    Gold Shovel
    setItemInfo(285,100,0x02);//    Gold Pickaxe
    setItemInfo(286,116,0x02);//    Gold Axe
    setItemInfo(287,  8,0x02);//    String
    setItemInfo(288, 24,0x02);//    Feather
    setItemInfo(289, 40,0x02);//    Sulphur
    setItemInfo(290,128,0x02);//    Wooden Hoe
    setItemInfo(291,129,0x02);//    Stone Hoe
    setItemInfo(292,130,0x02);//    Iron Hoe
    setItemInfo(293,131,0x02);//    Diamond Hoe
    setItemInfo(294,132,0x02);//    Gold Hoe
    setItemInfo(295,  9,0x02);//    Seeds
    setItemInfo(296, 25,0x02);//    Wheat
    setItemInfo(297, 41,0x02);//    Bread
    setItemInfo(298,  0,0x02);//    Leather Helmet
    setItemInfo(299, 16,0x02);//    Leather Chestplate
    setItemInfo(300, 32,0x02);//    Leather Leggings
    setItemInfo(301, 48,0x02);//    Leather Boots
    setItemInfo(302,  1,0x02);//    Chainmail Helmet
    setItemInfo(303, 17,0x02);//    Chainmail Chestplate
    setItemInfo(304, 33,0x02);//    Chainmail Leggings
    setItemInfo(305, 49,0x02);//    Chainmail Boots
    setItemInfo(306,  2,0x02);//    Iron Helmet
    setItemInfo(307, 18,0x02);//    Iron Chestplate
    setItemInfo(308, 34,0x02);//    Iron Leggings
    setItemInfo(309, 50,0x02);//    Iron Boots
    setItemInfo(310,  3,0x02);//    Diamond Helmet
    setItemInfo(311, 19,0x02);//    Diamond Chestplate
    setItemInfo(312, 35,0x02);//    Diamond Leggings
    setItemInfo(313, 51,0x02);//    Diamond Boots
    setItemInfo(314,  4,0x02);//    Gold Helmet
    setItemInfo(315, 20,0x02);//    Gold Chestplate
    setItemInfo(316, 36,0x02);//    Gold Leggings
    setItemInfo(317, 52,0x02);//    Gold Boots
    setItemInfo(318,  6,0x02);//    Flint
    setItemInfo(319, 87,0x02);//    Raw Porkchop
    setItemInfo(320, 88,0x02);//    Cooked Porkchop
    setItemInfo(321, 26,0x02);//    Paintings
    setItemInfo(322, 11,0x02);//    Golden apple
    setItemInfo(323, 42,0x02);//    Sign
    setItemInfo(324, 43,0x02);//    Wooden door
    setItemInfo(325, 74,0x02);//    Bucket
    setItemInfo(326, 75,0x02);//    Water bucket
    setItemInfo(327, 76,0x02);//    Lava bucket
    setItemInfo(328,135,0x02);//    Mine cart
    setItemInfo(329,104,0x02);//    Saddle
    setItemInfo(330, 44,0x02);//    Iron door
    setItemInfo(331, 56,0x02);//    Redstone
    setItemInfo(332, 14,0x02);//    Snowball
    setItemInfo(333,136,0x02);//    Boat
    setItemInfo(334,103,0x02);//    Leather
    setItemInfo(335, 77,0x02);//    Milk
    setItemInfo(336, 22,0x02);//    Clay Brick
    setItemInfo(337, 57,0x02);//    Clay Balls
    setItemInfo(338, 27,0x02);//    Sugarcane
    setItemInfo(339, 58,0x02);//    Paper
    setItemInfo(340, 59,0x02);//    Book
    setItemInfo(341, 30,0x02);//    Slimeball
    setItemInfo(342,141,0x02);//    Storage Minecart
    setItemInfo(343,157,0x02);//    Powered Minecart
    setItemInfo(344, 12,0x02);//    Egg
    setItemInfo(345, 54,0x03);//    Compass
    setItemInfo(346, 69,0x02);//    Fishing Rod
    setItemInfo(347, 70,0x03);//    Clock
    setItemInfo(348, 73,0x02);//    Glowstone Dust
    setItemInfo(349, 89,0x02);//    Raw Fish
    setItemInfo(350, 90,0x02);//    Cooked Fish
    setItemInfo(351, 78,0x06, 1520);//Dyes -> Refer to 1520 - 1535
    setItemInfo(352, 28,0x02);//    Bone
    setItemInfo(353, 13,0x02);//    Sugar
    setItemInfo(354, 29,0x02);//    Cake item

    //Set unimplemented item information to saddle
    for (ID = 355; ID < 1520; ID++) {
        setItemInfo( ID,104, 0x02);    //[Unimplemented item]
    }

    //Set phony dye item info
    setItemInfo(1520, 78,0x02);//    "Ink Sack"
    setItemInfo(1521, 94,0x02);//    "Rose Dye"
    setItemInfo(1522,110,0x02);//    "Cactus Dye"
    setItemInfo(1523,126,0x02);//    "Cocoa Dye"
    setItemInfo(1524,142,0x02);//    "Lapis Dye"
    setItemInfo(1525,158,0x02);//    "Purple Dye"
    setItemInfo(1526,174,0x02);//    "Cyan Dye"
    setItemInfo(1527,190,0x02);//    "Light Gray Dye"
    setItemInfo(1528, 79,0x02);//    "Gray Dye"
    setItemInfo(1529, 95,0x02);//    "Pink Dye"
    setItemInfo(1530,111,0x02);//    "Lime Dye"
    setItemInfo(1531,127,0x02);//    "Yellow Dye"
    setItemInfo(1352,143,0x02);//    "Light Blue Dye"
    setItemInfo(1353,159,0x02);//    "Magenta Dye"
    setItemInfo(1354,175,0x02);//    "Orange Dye"
    setItemInfo(1355,191,0x02);//    "Bone Meal"

    //Set unimplemented item information to saddle
    for (ID = 1536; ID < 2256; ID++) {
        setItemInfo( ID,104, 0x02);    //[Unimplemented item]
    }

    setItemInfo(2256,240,0x02);//   Gold Record
    setItemInfo(2257,241,0x02);//   Green Record

    //  properties:
    // 0x00 = terrain cube (icon is single terrain cube)
    // 0x01 = terrain item (icon is single terrain texture)
    // 0x02 = item (icon is single tile from items.png)
    // 0x03 = special icon (not from just terrain.png/items.png)
    // 0x06 = item (icon from items.png depends on damage data)

    return true;
}


//Write binary data of uncompressed chunk
bool Viewer::writeChunkBin( mc__::Chunk *chunk, const string& filename) const
{
    //Validate pointer
    if (chunk == NULL) { return false; }
    
    //Zip if needed
    if (chunk->zipped == NULL && chunk->block_array != NULL) {
        cerr << "Zipped chunk before writing" << endl;
        chunk->packBlocks();
        chunk->zip();
    }
    
    //Unzip if needed
    if (chunk->byte_array == NULL && chunk->zipped != NULL) {
        cerr << "Unzipped chunk before writing" << endl;
        chunk->unzip(false);
        chunk->unpackBlocks(false);
    }
    
    //Open binary file for output
    ofstream binFile(filename.c_str(), ios::out | ios::binary);
    uint8_t *bytes = chunk->byte_array;
    binFile.write( (char*)bytes, chunk->byte_length);
    binFile.close();
    cerr << "Wrote " << chunk->byte_length << " bytes to " << filename << endl;

    //Zipped file... without that pesky zip header :)
    string filename_zip( filename + "_zip");
    ofstream zipFile( filename_zip.c_str(), ios::out | ios::binary);
    zipFile.write( (char*)chunk->zipped, chunk->zipped_length);
    zipFile.close();
    cerr << "Wrote " << chunk->zipped_length
        << " bytes to " << filename_zip << endl;
    
    return true;
}

//Write binary data of map chunks to files
bool Viewer::saveChunks(const mc__::World& world) const
{
    int32_t X, Z;
    int8_t Y;
    stringstream filename;
    
    //Reference map chunks data structure
    const XZMapChunk_t& coordMapChunks = world.coordMapChunks;
    XZMapChunk_t::const_iterator iter_xz;
    
    //For all megachunks (X,Z)
    for (iter_xz = coordMapChunks.begin();
        iter_xz != coordMapChunks.end(); iter_xz++)
    {
        Chunk *chunk = iter_xz->second;
        X = chunk->X;
        Y = chunk->Y;
        Z = chunk->Z;
        
        //Filename from X,Y,Z
        filename.str("");
        filename << "chunk_"
            << (int)X << "_" << (int)Y << "_" << (int)Z << ".bin";
        
        //Copy binary chunk data to file
        if (chunk != NULL) {
            //Pack blocks to byte_array
            chunk->packBlocks();
            writeChunkBin( chunk, filename.str());
        } else {
            cerr << "Chunk not found @ "
                << (int)X << "," << (int)Y << "," << (int)Z << endl;
        }
    }

    //TODO: return false if unable to write chunks
    return true;
}

//List all the Map Chunks to stdout
void Viewer::printChunks(const mc__::World& world) const
{
    
    //Reference world chunk data structure
    const XZMapChunk_t& coordMapChunks = world.coordMapChunks;
    XZMapChunk_t::const_iterator iter_xz;
    
    //For all map chunks (X,Z)
    for (iter_xz = coordMapChunks.begin();
        iter_xz != coordMapChunks.end(); iter_xz++)
    {
        uint64_t key = iter_xz->first;
        MapChunk *mc = iter_xz->second;
        if (mc == NULL) {
            cerr << "Print NULL mapchunk" << endl;
            continue;
        }
        long X = (key >> 32);
        short Y = 0;
        long Z = (key & 0xFFFFFFFF);
        cout << "MapChunk @ "
            << X << "," << Y << "," << Z
            << " [" << (long)mc->X << "," << (long)mc->Y << "," << (long)mc->Z
            << "] " << (long)mc->size_X << "," << (short)mc->size_Y
            << "," << (long)mc->size_Z << endl;
        cout << "\t" << mc->zipped_length << " bytes zipped";
        if (mc->isUnzipped) {
            cout << ", " << mc->array_length << " blocks, "
            << mc->byte_length << " bytes unzipped";
        }
    }
    
    //For all mini-chunks
    const chunkSet_t& chunkUpdates = world.chunkUpdates;
    chunkSet_t::const_iterator iter_chunk;
    
    for (iter_chunk =  chunkUpdates.begin();
         iter_chunk != chunkUpdates.end(); iter_chunk++)
    {
        Chunk *chunk = *iter_chunk;
        //Crash here if they screwed up :)
        mc__::Chunk& myChunk = *chunk;

        cout << "Chunk @ " << (long)myChunk.X << "," << (long)myChunk.Y << ","
             << (long)myChunk.Z << "size=" << (long)myChunk.size_X << ","
             << (short)myChunk.size_Y << "," << (long)myChunk.size_Z << endl;
        cout << "\t" << myChunk.zipped_length << " bytes zipped";
        if (myChunk.isUnzipped) {
            cout << ", " << myChunk.array_length << " blocks, "
            << myChunk.byte_length << " bytes unzipped";
        }
    }

}


//Dump DevIL texture info to "texture.log"
void Viewer::outputRGBAData() {
    
    //Return if wrong format
    if (ilGetInteger(IL_IMAGE_FORMAT) != IL_RGBA) {
        cerr << "Not RGBA" << endl;
        return;
    }
    
    //Get image information
    size_t width = ilGetInteger(IL_IMAGE_WIDTH);
    size_t height = ilGetInteger(IL_IMAGE_HEIGHT);
    size_t bpp = ilGetInteger(IL_IMAGE_BPP);
    size_t bytes_used = width*height*bpp;
    ILubyte *data = ilGetData();

    //Open output file (append)
    ofstream outfile;
    outfile.open("texture.log", ios::app);
    
    //Texture information
    outfile << "Size=" << bytes_used << " BPP=" << bpp << " Width="
        << width << " Height=" << height << endl;
    
    //Repeat for each byte
    size_t bytenum, x, y, index;
    index = 0;    
    for (y = 0; y < height; y++) {
        outfile << setw(4) << setfill(' ') << y << ": ";
        for (x = 0; x < width; x++) {
            for (bytenum = 0; bytenum < bpp; bytenum++, index++) {
                outfile << hex << setw(2) << setfill('0') << (short)data[index];
            }
            outfile << " ";
        }
        outfile << endl << dec;
    }
    
    //Close file
    outfile.close();
}

/*
//choose OpenGL drawing texture from image list
bool Viewer::chooseTexture( size_t index) {
  
    //glBind texture
    glBindTexture(GL_TEXTURE_2D, image_array[index]);
    
    //TODO: check for GL error
    return true;
}

//Chop texture map into ilImages array (don't bind anything to GL)
bool Viewer::splitTextureMap( ILuint texmap, ILuint tiles_x, ILuint tiles_y)
{
    ILuint SrcX, SrcY, Width, Height, bytePerPixel, ilFormat;
    
    //Get information from texture map
    ilBindImage(texmap);
    Width = ilGetInteger(IL_IMAGE_WIDTH)/tiles_x;
    Height = ilGetInteger(IL_IMAGE_HEIGHT)/tiles_y;
    bytePerPixel = ilGetInteger(IL_IMAGE_BPP);
    ilFormat = ilGetInteger(IL_IMAGE_FORMAT);
    
    cout << "Split Texture: " << Width << ", " << Height << ", "
        << bytePerPixel << " 0x" << hex << ilFormat << endl << dec;
    
    //Copy alpha channel when blitting, don't blend it.
    ilDisable(IL_BLIT_BLEND);

    //For each tile...
    size_t x=0, y=0;
    ILuint index=0;
    for (y = 0; y < tiles_y; y++) {
        for (x = 0; x < tiles_x; x++) {

            //Blit the rectangle from texture map ID to texture array ID
            SrcX = x * Width;
            SrcY = y * Height;

            //Take out a specific block from the texture grid (Blit)
            ILuint il_blocktex;
            ilGenImages(1, &il_blocktex);
            ilBindImage(il_blocktex);
            ilTexImage( Width, Height, 1, bytePerPixel,
                ilFormat, IL_UNSIGNED_BYTE, NULL);

            ilBlit( texmap, 0, 0, 0, SrcX, SrcY, 0, Width, Height, 1);

            //Use single OpenGL "image"
            glBindTexture(GL_TEXTURE_2D, image_array[index]);
            
            //Copy current DevIL image to OpenGL image
            glTexImage2D(GL_TEXTURE_2D, 0, ilGetInteger(IL_IMAGE_BPP),
                ilGetInteger(IL_IMAGE_WIDTH), ilGetInteger(IL_IMAGE_HEIGHT), 0,
                ilGetInteger(IL_IMAGE_FORMAT), GL_UNSIGNED_BYTE, ilGetData());

            //Set out-of-range texture coordinates
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        
            //Make textures "blocky" when up close
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
            
            //Use texture mipmaps when far away
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
                GL_NEAREST_MIPMAP_NEAREST);
        
            //Generate mipmaps when calling glTexImage2D
            glTexParameteri(GL_TEXTURE_2D,GL_GENERATE_MIPMAP_SGIS,GL_TRUE);

            //free memory used by DevIL for texture
            ilDeleteImages(1, &il_blocktex);
    
            index++;
        }
    }
    
    return true;
}
*/
