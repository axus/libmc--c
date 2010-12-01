/*
  mc__::Viewer
  Draw mc__::World objects using OpenGL

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


/*
    OpenGL has specific meaning for x, y, and z axes.
    
    z-axis is distance from the initial viewpoint.  As Z increases, the
    location gets closer to the initial viewpoint. (negative is farther)
    
    y-axis is "up" relative to the initial viewpoint.  As Y increases,
    the location moves further "up".
    
    x-axis is "right" relative to the intial viewpoint.  As X increases,
    the location moves further "right".

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

//C
#include <cmath>    //fmod

//More STL
#include <iostream>
#include <iomanip>
#include <fstream>
#include <sstream>

using std::cout;
using std::cerr;
using std::endl;
using std::string;
using std::hex;
using std::dec;
using std::setw;
using std::setfill;
using std::ofstream;
using std::ios;
using std::flush;
using std::stringstream;

//libmc--c
#include "Viewer.hpp"
using mc__::Viewer;
using mc__::face_ID;
using mc__::World;

const float Viewer::PI = std::atan(1.0)*4;

Viewer::Viewer():
    cam_X(0), cam_Y(0), cam_Z(0), cam_yaw(0), cam_pitch(0), debugging(false)
{
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
bool Viewer::init(const std::string &filename)
{

    //Compare devIL DLL version to header version
    if (ilGetInteger(IL_VERSION_NUM) < IL_VERSION) {
        cerr << "DevIL wrong DLL version" << endl;
        return 1;
    }

    texture_map_file = filename;

    //Load game block information
    loadBlockInfo();

    //Initialize OpenGL
    startOpenGL();

    //Start DevIL
    ilInit();

    //Load terrain texture map, bind it to current DevIL image
    il_texture_map = loadImageFile(texture_map_file);
    if (il_texture_map == 0) {
        return 0;   //error, exit program
    }

    //glBind texture before assigning it
    glBindTexture(GL_TEXTURE_2D, image);
    
    //Copy current DevIL image to OpenGL image.  
    glTexImage2D(GL_TEXTURE_2D, 0, ilGetInteger(IL_IMAGE_BPP),
        ilGetInteger(IL_IMAGE_WIDTH), ilGetInteger(IL_IMAGE_HEIGHT), 0,
        ilGetInteger(IL_IMAGE_FORMAT), GL_UNSIGNED_BYTE, ilGetData());

    //Change camera to model view mode
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    return true;
}

//
//Camera functions
//

//Look at a point from another point.  (up_x, up_y, up_z) is a vector point "up"
void Viewer::lookAt( GLint from_x, GLint from_y, GLint from_z, 
             GLint at_x, GLint at_y, GLint at_z,
             GLint up_x, GLint up_y, GLint up_z)
{
    gluLookAt( from_x, from_y, from_z, at_x, at_y, at_z, up_x, up_y, up_z);
}

void Viewer::reset()
{
    glLoadIdentity();
}

//Move camera without rotating
void Viewer::move( GLfloat side, GLfloat up, GLfloat forward)
{
    //Move up or down
    cam_Y += up;
    
    //Movement along line of site, depends on yaw
    //GLfloat yaw_mod = fmod( cam_yaw, 360.0f);
    
    float radians = cam_yaw*PI/180.0;
    cam_X += int(side*cos( radians ) + forward*sin(radians));
    cam_Z += int(side*sin( radians ) - forward*cos(radians));
    
}

//Rotate camera degrees/360 about vector (axis_x, axis_y, axis_z)
void Viewer::turn( GLint degrees, GLint axus_x, GLint axus_y, GLint axus_z)
{
    glRotatef( degrees, axus_x, axus_y, axus_z);
}

//Resize viewport
void Viewer::viewport( GLint x, GLint y, GLsizei width, GLsizei height)
{
    //Save matrix mode
    glPushAttrib(GL_TRANSFORM_BIT);
  
    GLfloat aspectRatio = (GLfloat)width / (GLfloat)height;
    glViewport( x, y, width, height);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(60, aspectRatio, 1.f, 1024.f);
    
    //Reload matrix mode
    glPushAttrib(GL_TRANSFORM_BIT);
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
        case 2:
            if (face == mc__::UP) {
                red=grass_color[0]; green=grass_color[1]; blue=grass_color[2];
            } else {
                red=255; green=255; blue=255;
            }
            glColor3ub( red, green, blue);
            break;
        case 18:
            red=leaf_color[0]; green=leaf_color[1]; blue=leaf_color[2];
            glColor3ub( red, green, blue);
            break;
        default:
            red=255; green=255; blue=255;
            break;
    }
}

//Use OpenGL to draw a solid cube with appropriate textures for blockID
void Viewer::drawCube( uint8_t blockID, GLint x, GLint y, GLint z)
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

    //For each face, load the appropriate texture for the block ID, and draw square
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

//TODO: compile lists for each blockID!

    //A
    tx_0 = blockInfo[blockID].tx[WEST];
    tx_1 = blockInfo[blockID].tx[WEST] + tmr;
    ty_0 = blockInfo[blockID].ty[WEST] + tmr;    //flip y
    ty_1 = blockInfo[blockID].ty[WEST];
    setBlockColor(blockID, WEST);  //Set leaf/grass color if needed
    
    glTexCoord2f(tx_0,ty_0); glVertex3i( A, C, E);  //Lower left:  ACE
    glTexCoord2f(tx_1,ty_0); glVertex3i( A, C, F);  //Lower right: ACF
    glTexCoord2f(tx_1,ty_1); glVertex3i( A, D, F);  //Top right:   ADF
    glTexCoord2f(tx_0,ty_1); glVertex3i( A, D, E);  //Top left:    ADE

    //B
    tx_0 = blockInfo[blockID].tx[EAST];
    tx_1 = blockInfo[blockID].tx[EAST] + tmr;
    ty_0 = blockInfo[blockID].ty[EAST] + tmr;
    ty_1 = blockInfo[blockID].ty[EAST];
    setBlockColor(blockID, EAST);  //Set leaf/grass color if needed
    
    glTexCoord2f(tx_0,ty_0); glVertex3i( B, C, F);  //Lower left:  BCF
    glTexCoord2f(tx_1,ty_0); glVertex3i( B, C, E);  //Lower right: BCE
    glTexCoord2f(tx_1,ty_1); glVertex3i( B, D, E);  //Top right:   BDE
    glTexCoord2f(tx_0,ty_1); glVertex3i( B, D, F);  //Top left:    BDF
    
    //C
    tx_0 = blockInfo[blockID].tx[DOWN];
    tx_1 = blockInfo[blockID].tx[DOWN] + tmr;
    ty_0 = blockInfo[blockID].ty[DOWN] + tmr;
    ty_1 = blockInfo[blockID].ty[DOWN];
    setBlockColor(blockID, DOWN);  //Set leaf/grass color if needed
    
    glTexCoord2f(tx_0,ty_0); glVertex3i( A, C, E);  //Lower left:  ACE
    glTexCoord2f(tx_1,ty_0); glVertex3i( B, C, E);  //Lower right: BCE
    glTexCoord2f(tx_1,ty_1); glVertex3i( B, C, F);  //Top right:   BCF
    glTexCoord2f(tx_0,ty_1); glVertex3i( A, C, F);  //Top left:    ACF
    
    //D
    tx_0 = blockInfo[blockID].tx[UP];
    tx_1 = blockInfo[blockID].tx[UP] + tmr;
    ty_0 = blockInfo[blockID].ty[UP] + tmr;
    ty_1 = blockInfo[blockID].ty[UP];
    setBlockColor(blockID, UP);  //Set leaf/grass color if needed

    glTexCoord2f(tx_0,ty_0); glVertex3i( A, D, F);  //Lower left:  ADF
    glTexCoord2f(tx_1,ty_0); glVertex3i( B, D, F);  //Lower right: BDF
    glTexCoord2f(tx_1,ty_1); glVertex3i( B, D, E);  //Top right:   BDE
    glTexCoord2f(tx_0,ty_1); glVertex3i( A, D, E);  //Top left:    ADE
    
    //E
    tx_0 = blockInfo[blockID].tx[NORTH];
    tx_1 = blockInfo[blockID].tx[NORTH] + tmr;
    ty_0 = blockInfo[blockID].ty[NORTH] + tmr;
    ty_1 = blockInfo[blockID].ty[NORTH];
    setBlockColor(blockID, NORTH);  //Set leaf/grass color if needed
    
    glTexCoord2f(tx_0,ty_0); glVertex3i( B, C, E);  //Lower left:  BCE
    glTexCoord2f(tx_1,ty_0); glVertex3i( A, C, E);  //Lower right: ACE
    glTexCoord2f(tx_1,ty_1); glVertex3i( A, D, E);  //Top right:   ADE
    glTexCoord2f(tx_0,ty_1); glVertex3i( B, D, E);  //Top left:    BDE
    
    //F
    tx_0 = blockInfo[blockID].tx[SOUTH];
    tx_1 = blockInfo[blockID].tx[SOUTH] + tmr;
    ty_0 = blockInfo[blockID].ty[SOUTH] + tmr;
    ty_1 = blockInfo[blockID].ty[SOUTH];
    setBlockColor(blockID, SOUTH);  //Set leaf/grass color if needed
    
    glTexCoord2f(tx_0,ty_0); glVertex3i( A, C, F);  //Lower left:  ACF
    glTexCoord2f(tx_1,ty_0); glVertex3i( B, C, F);  //Lower right: BCF
    glTexCoord2f(tx_1,ty_1); glVertex3i( B, D, F);  //Top right:   BDF
    glTexCoord2f(tx_0,ty_1); glVertex3i( A, D, F);  //Top left:    ADF
    
    //Return color to normal
    setBlockColor( 0, (face_ID)0);
}

//Draw item blockID which is placed as a block
void Viewer::drawItem( uint8_t blockID, GLint x, GLint y, GLint z)
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

    tx_0 = blockInfo[blockID].tx[EAST];
    tx_1 = blockInfo[blockID].tx[EAST] + tmr;
    ty_0 = blockInfo[blockID].ty[EAST] + tmr;    //flip y
    ty_1 = blockInfo[blockID].ty[EAST];
    glTexCoord2f(tx_0,ty_0); glVertex3i( H, C, F);  //Lower left:  HCF
    glTexCoord2f(tx_1,ty_0); glVertex3i( H, C, E);  //Lower right: HCE
    glTexCoord2f(tx_1,ty_1); glVertex3i( H, D, E);  //Top right:   HDE
    glTexCoord2f(tx_0,ty_1); glVertex3i( H, D, F);  //Top left:    HDF
}

//Draw a placed mc__::Block in openGL
void Viewer::drawBlock( const mc__::Block& block, GLint x, GLint y, GLint z)
{

    //Don't draw air blocks
    if (block.blockID == 0) {
        return;
    }

    //Drawing function depends on shape (as determined from properties)
    //0xF0: Shape : 0=cube, 1=stairs, 2=lever, 3=halfblock,
    //              4=wallsign, 5=ladder, 6=track, 7=1/4 block
    //              8=portal, 9=fence, A=door, B=floorplate
    //              C=1/3 block, D=wallsign, E=button, F=plant

    switch ( (blockInfo[block.blockID].properties & 0xF0)>>4 ) {
        case 0x0:
            drawCube(block.blockID, x, y, z);
            break;
        case 0xF:
            drawItem(block.blockID, x, y, z);
            break;
        //TODO: test for other shapes
        default:    //Draw unknown types as a item
            drawItem(block.blockID, x, y, z);
            break;
    }

}

using mc__::XZChunksMap_t;
using mc__::YChunkMap_t;

//Draw the chunks in mc__::World
void Viewer::drawChunks( const World& world)
{
    //Reference to world chunks data structure
    const XZChunksMap_t& coordChunksMap = world.coordChunksMap;
    
    //Variables to iterate through list of chunks
    XZChunksMap_t::const_iterator iter_xz;
    YChunkMap_t::const_iterator iter_y;
    uint64_t key;
    GLint off_x, off_y, off_z;
    GLint X, Y, Z;
    
    //For all chunk stacks (X,Z)
    for (iter_xz = coordChunksMap.begin();
        iter_xz != coordChunksMap.end(); iter_xz++)
    {
        key = iter_xz->first;
        YChunkMap_t *chunks=iter_xz->second;
        
        //For all chunks in stack (Y)
        for (iter_y = chunks->begin(); iter_y != chunks->end(); iter_y++)
        {
            Chunk *chunk = iter_y->second;
            mc__::Chunk& myChunk = *chunk;

            //If the chunk has not uncompressed it's data, do so now
            if (! myChunk.isUnzipped) {
                
                //Debug output
                if (debugging) {
                    cout << "Unzipping chunk @ " << (int)myChunk.X << ","
                        << (int)myChunk.Y << "," << (int)myChunk.Z << endl;
                }
                
                //Unzip the chunk
                if (!myChunk.unzip()) {
                    cerr << "ERROR UNZIPPING" << endl;
                    continue;
                }
            }
            
            //When indexing block in chunk array,
            //index = y + (z * (Size_Y+1)) + (x * (Size_Y+1) * (Size_Z+1))
    
            //Draw every block in chunk.  x,y,z determined by position in array.
            size_t index=0;
            for (off_x=0, X=myChunk.X; off_x <= myChunk.size_X; off_x++, X++) {
            for (off_z=0, Z=myChunk.Z; off_z <= myChunk.size_Z; off_z++, Z++) {
            for (off_y=0, Y=myChunk.Y; off_y <= myChunk.size_Y; off_y++, Y++) {
                drawBlock( myChunk.block_array[index], X, Y, Z);
                index++;
            }}}

        }
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
    viewport(0, 0, 800, 600);

    //Enable texture mapping
    glEnable(GL_TEXTURE_2D);
    
    //Transparency in textures
    glEnable(GL_ALPHA_TEST);
    glAlphaFunc(GL_GREATER, 0.5f);
    
    //Clear screen and set location to origin in modelview mode
    glMatrixMode(GL_MODELVIEW);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
    glLoadIdentity();

    //Save the original viewpoint
    glPushMatrix(); 

    //Create OpenGL texture
    glGenTextures(1, &image);
    glBindTexture(GL_TEXTURE_2D, image);    //bind empty texture
    
    //Set texture zoom filters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);  //GL_LINEAR?
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    
    //Set out-of-range texture coordinates
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
    
    //No blending
    glDisable(GL_BLEND);

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


//OpenGL rendering of cubes.  No update to camera.
bool Viewer::drawWorld(const World& world)
{
    //Erase openGL world
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

    //Reset camera
    glLoadIdentity();
    glRotatef( cam_yaw, 0.0f, 1.0f, 0.0f);
    
    //Change roll/pitch to look up/down
    //glRotatef( cam_pitch, 1.0f, 0.0f, 0.0f);
    //glRotatef( cam_roll, 0.0f, 0.0f, 1.0f);
    
    //Bring the world to the camera, not the camera to the world
    glTranslatef( -cam_X, -cam_Y, -cam_Z );

    //Start putting quads in memory
    glBegin(GL_QUADS);

    //Draw the loaded chunks
    drawChunks(world);
    
    mc__::Block block1 = {58, 0, 0};   //Workbench
    drawBlock( block1, 0, 0, 2);

    //Finish putting quads in memory, and draw
    glEnd();
    
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
    //              4=wallsign, 5=ladder, 6=track, 7=1/4 block
    //              8=portal, 9=fence, A=door, B=floorplate
    //              C=1/3 block, D=wallsign, E=button, F=plant
    //0x08: Bright: 0=dark, 1=lightsource
    //0x04: Vision: 0=opqaue, 1=see-through
    //0x03: State : 0=solid, 1=loose, 2=liquid, 3=gas
    blockInfo[index].properties = properties;
}

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
//              4=wallsign, 5=ladder, 6=track, 7=1/4 block
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
    
    for (ID = 21; ID < 37; ID++) {
        setBlockInfo( ID, 64, 64, 64, 64, 64, 64, 0x00);   //Cloth
    }
    setBlockInfo( 37, 13, 13, 13, 13, 13, 13, 0xF7);    //Flower
    setBlockInfo( 38, 12, 12, 12, 12, 12, 12, 0xF7);    //Rose
    setBlockInfo( 39, 29, 29, 29, 29, 29, 29, 0xF7);    //BrownShroom
    setBlockInfo( 40, 28, 28, 28, 28, 28, 28, 0xF7);    //RedShroom
    setBlockInfo( 41, 39, 39, 55, 23, 39, 39, 0x00);    //GoldBlock
    setBlockInfo( 42, 38, 38, 54, 22, 38, 38, 0x00);    //IronBlock
    setBlockInfo( 43, 5,  5,  6,  6,  5,  5,  0x00);    //DoubleStep
    setBlockInfo( 44, 5,  5,  6,  6,  5,  5,  0x20);    //Step
    setBlockInfo( 45, 7,  7,  7,  7,  7,  7,  0x00);    //Brick
    setBlockInfo( 46, 8,  8, 10,  9,  8,  8,  0x00);    //TNT
    setBlockInfo( 47, 35, 35, 4,  4,  35, 35, 0x00);    //Bookshelf
    setBlockInfo( 48, 36, 36, 16, 36, 36, 36, 0x00);    //Mossy
    setBlockInfo( 49, 37, 37, 37, 37, 37, 37, 0x00);    //Obsidian
    setBlockInfo( 50, 80, 80, 80, 80, 80, 80, 0xFF);    //Torch
    setBlockInfo( 51, 30, 31, 47, 47, 30, 31, 0x7F);    //Fire
    setBlockInfo( 52, 65, 65, 65, 65, 65, 65, 0x04);    //Spawner
    setBlockInfo( 53, 4,  4,  4,  4,  4,  4,  0x10);    //WoodStairs
    setBlockInfo( 54, 26, 26, 25, 25, 26, 27, 0x00);    //Chest (*)
    setBlockInfo( 55, 84, 100,85, 101,84, 100,0x2F);    //Wire (*)
    setBlockInfo( 56, 50, 50, 50, 50, 50, 50, 0x00);    //DiamondOre
    setBlockInfo( 57, 40, 40, 56, 24, 40, 40, 0x00);    //DiamondBlock
    setBlockInfo( 58, 60, 60, 43, 43, 59, 59, 0x00);    //Workbench
    setBlockInfo( 59, 90, 91, 92, 93, 94, 95, 0xF7);    //Crops (*)
    setBlockInfo( 60, 2,  2,  2,  86, 2,  2,  0x00);    //Soil
    setBlockInfo( 61, 45, 45, 45, 45, 45, 44, 0x00);    //Furnace
    setBlockInfo( 62, 45, 45, 45, 45, 45, 61, 0x08);    //LitFurnace
    setBlockInfo( 63, 4,  4,  4,  4,  4,  4,  0x47);    //SignPost (*)
    setBlockInfo( 64, 97, 81, 97, 81, 97, 81, 0xA7);    //WoodDoor (*)
    setBlockInfo( 65, 83, 83, 83, 83, 83, 83, 0x57);    //Ladder (*)
    setBlockInfo( 66, 128,112,128,112,112,112,0x57);    //Track (*)
    setBlockInfo( 67, 16, 16, 16, 16, 16, 16, 0x10);    //CobbleStairs
    setBlockInfo( 68, 4,  4,  4,  4,  4,  4,  0xD7);    //WallSign (*)
    setBlockInfo( 69, 96, 16, 96, 16, 96, 16, 0x27);    //Lever
    setBlockInfo( 70, 1,  1,  1,  1,  1,  1,  0xB7);    //StonePlate
    setBlockInfo( 71, 98, 82, 98, 82, 98, 82, 0xA7);    //IronDoor (*)
    setBlockInfo( 72, 4,  4,  4,  4,  4,  4,  0xB7);    //WoodPlate
    setBlockInfo( 73, 51, 51, 51, 51, 51, 51, 0x00);    //RedstoneOre
    setBlockInfo( 74, 51, 51, 51, 51, 51, 51, 0x08);    //RedstoneOreLit(*)
    setBlockInfo( 75, 115,115,115,115,115,115,0xF7);    //RedstoneTorch
    setBlockInfo( 76, 99, 99, 99, 99, 99, 99, 0xFF);    //RedstoneTorchLit
    setBlockInfo( 77, 1,  1,  1,  1,  1,  1,  0xE7);    //StoneButton
    setBlockInfo( 78, 66, 68, 66, 68, 66, 68, 0xC3);    //SnowPlate
    setBlockInfo( 79, 67, 67, 67, 67, 67, 67, 0x04);    //Ice
    setBlockInfo( 80, 66, 66, 66, 66, 66, 66, 0x00);    //Snow
    setBlockInfo( 81, 70, 70, 71, 69, 70, 70, 0x00);    //Cactus
    setBlockInfo( 82, 72, 72, 72, 72, 72, 72, 0x00);    //Clay
    setBlockInfo( 83, 73, 73, 73, 73, 73, 73, 0xF7);    //Reed (*)
    setBlockInfo( 84, 74, 74, 43, 75, 74, 74, 0x00);    //Jukebox
    setBlockInfo( 85, 4,  4,  4,  4,  4,  4,  0x94);    //Fence (*)
    setBlockInfo( 86, 118,118,118,102,118,119,0x00);    //Pumpkin
    setBlockInfo( 87, 103,103,103,103,103,103,0x00);    //Netherstone
    setBlockInfo( 88, 104,104,104,104,104,104,0x01);    //SlowSand
    setBlockInfo( 89, 105,105,105,105,105,105,0x08);    //Lightstone
    setBlockInfo( 90, 49, 49, 49, 49, 49, 49, 0x8F);    //Portal (??)
    setBlockInfo( 91, 118,118,118,102,118,120,0x08);    //PumpkinLit

//0xF0: Shape : 0=cube, 1=stairs, 2=toggle, 3=halfblock,
//              4=signpost, 5=ladder, 6=track, 7=fire
//              8=portal, 9=fence, A=door, B=floorplate
//              C=snow?, D=wallsign, E=button, F=planted
//0x08: Bright: 0=dark, 1=lightsource
//0x04: Vision: 0=opqaue, 1=see-through
//0x03: State : 0=solid, 1=loose, 2=liquid, 3=gas

    return true;
}

//Write binary data of uncompressed chunk
bool Viewer::writeChunkBin( mc__::Chunk *chunk, const string& filename) const
{
    //Validate pointer
    if (chunk == NULL) { return false; }
    
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

//Write binary data of all chunks to files
bool Viewer::saveChunks(const mc__::World& world) const
{
    XZChunksMap_t::const_iterator iter_xz;
    YChunkMap_t::const_iterator iter_y;
    
    uint64_t key;
    int32_t X, Z;
    int8_t Y;
    stringstream filename;
    
    //Reference world chunk data structure
    const XZChunksMap_t& coordChunksMap = world.coordChunksMap;
    
    //For all chunk stacks (X,Z)
    for (iter_xz = coordChunksMap.begin();
        iter_xz != coordChunksMap.end(); iter_xz++)
    {
        key = iter_xz->first;
        X = (key >> 32);
        Z = (key & 0xFFFFFFFF);
        YChunkMap_t *chunks=iter_xz->second;

        //For all chunks in stack (Y)
        for (iter_y = chunks->begin(); iter_y != chunks->end(); iter_y++)
        {
            Y = iter_y->first;
            Chunk *chunk = iter_y->second;
            
            //Filename from X,Y,Z
            filename.str("");
            filename << "chunk_"
                << (int)X << "_" << (int)Y << "_" << (int)Z << ".bin";
            
            //Copy binary chunk data to file
            if (chunk != NULL) {
                //Pack and unpack...
                chunk->packBlocks();
                chunk->unpackBlocks();
                
                writeChunkBin( chunk, filename.str());
            } else {
                cerr << "Chunk not found @ "
                    << (int)X << "," << (int)Y << "," << (int)Z << endl;

            }
        }
    }
    
    //TODO: return false if unable to write chunks
    return true;
}

//List all the chunks to stdout
void Viewer::printChunks(const mc__::World& world) const
{
    XZChunksMap_t::const_iterator iter_xz;
    YChunkMap_t::const_iterator iter_y;
    
    uint64_t key;
    int32_t X, Z;
    int8_t Y;
    
    //Reference world chunk data structure
    const XZChunksMap_t& coordChunksMap = world.coordChunksMap;
    
    //For all chunk stacks (X,Z)
    for (iter_xz = coordChunksMap.begin();
        iter_xz != coordChunksMap.end(); iter_xz++)
    {
        key = iter_xz->first;
        X = (key >> 32);
        Z = (key & 0xFFFFFFFF);
        YChunkMap_t *chunks=iter_xz->second;
        
        //For all chunks in stack (Y)
        for (iter_y = chunks->begin(); iter_y != chunks->end(); iter_y++)
        {
            Y = iter_y->first;
            Chunk *chunk = iter_y->second;
            cout << "Chunk @ "
                << (int)X << "," << (int)Y << "," << (int)Z
                << " [" << chunk->X << "," << (int)chunk->Y << "," << chunk->Z
                << "] " << (int)chunk->size_X << "," << (int)chunk->size_Y
                << "," << (int)chunk->size_Z << endl;
            cout << "\t" << chunk->zipped_length << " bytes zipped";
            if (chunk->isUnzipped) {
                cout << ", " << chunk->array_length << " blocks, "
                << chunk->byte_length << " bytes unzipped";
            }
            cout << "." << endl;
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
//Blit from texture map to current OpenGL texture
ILuint Viewer::blitTexture( ILuint texmap, ILuint SrcX, ILuint SrcY,
        ILuint Width, ILuint Height) {
    //Take out a specific block from the texture grid (Blit)
    ILuint il_blocktex;
    ilGenImages(1, &il_blocktex);
    ilBindImage(il_blocktex);
    ilTexImage( Width, Height, 1, 4, IL_RGBA, IL_UNSIGNED_BYTE, NULL);

    //Blit the rectangle from texmap to il_blocktex
    ilBlit( texmap, 0, 0, 0, SrcX, SrcY, 0, Width, Height, 1);

    //Use single OpenGL "image"
    glBindTexture(GL_TEXTURE_2D, image);

    //Copy current DevIL image to OpenGL image
    glTexImage2D(GL_TEXTURE_2D, 0, ilGetInteger(IL_IMAGE_BPP),
        ilGetInteger(IL_IMAGE_WIDTH), ilGetInteger(IL_IMAGE_HEIGHT), 0,
        ilGetInteger(IL_IMAGE_FORMAT), GL_UNSIGNED_BYTE, ilGetData());

    //free memory used by DevIL
    ilDeleteImages(1, &il_blocktex);  
    return il_blocktex;
}
*/

/*
//choose OpenGL drawing texture from image list
ILuint Viewer::chooseTexture( ILuint* ilImages, size_t index) {
  
    //ilBind image from list
    ILuint ilImage = ilImages[index];
    ilBindImage( ilImage );
    
    //glBind texture
    glBindTexture(GL_TEXTURE_2D, image);
    glTexImage2D(GL_TEXTURE_2D, 0, ilGetInteger(IL_IMAGE_BPP),
        ilGetInteger(IL_IMAGE_WIDTH), ilGetInteger(IL_IMAGE_HEIGHT), 0,
        ilGetInteger(IL_IMAGE_FORMAT), GL_UNSIGNED_BYTE, ilGetData());
        
    return ilImage;
}
*//*
//Chop texture map into ilImages array (don't bind anything to GL)
bool Viewer::splitTextureMap( ILuint texmap, size_t tiles_x, size_t tiles_y,
    ILuint* ilImages)
{
    ILuint SrcX, SrcY, Width, Height;
    Width = texmap_TILE_LENGTH;
    Height = texmap_TILE_LENGTH;
    
    //User should have allocated ilImages array
    if (ilImages == NULL) {
        return false;
    }
    
    //Create blank images in the array
    ilGenImages(tiles_x * tiles_y, ilImages);
    
    //Copy alpha channel when blitting, don't blend it.
    ilDisable(IL_BLIT_BLEND);

    //For each tile...
    size_t x, y, index;
    for (y = 0, index=0; y < tiles_y; y++) {
        for (x = 0; x < tiles_x; x++, index++) {
            //Bind the image
            ilBindImage(ilImages[index]);
            ilTexImage( Width, Height, 1, 4, IL_RGBA, IL_UNSIGNED_BYTE, NULL);

            //Blit the rectangle from texmap to il_blocktex
            SrcX = x * Width; SrcY = y * Height;
            ilBlit( texmap, 0, 0, 0, SrcX, SrcY, 0, Width, Height, 1);
        }
    }
    
    return true;
}
*/
