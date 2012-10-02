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

#ifndef MC__BLOCKDRAWER_H
#define MC__BLOCKDRAWER_H

//libmc--c
#include "World.hpp"
#include "TextureInfo.hpp"

//DevIL
#include <IL/il.h>

//STL
#include <string>
//#include <unordered_map>

//OpenGL
#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glext.h>

namespace mc__ {

    //
    // Types for mc__ namespace
    //
    
    //TODO: enums in namespace
    // Index for face of cube or cuboid
    enum face_ID { LEFT=0, RIGHT=1, BOTTOM=2, TOP=3, BACK=4, FRONT=5, FACE_MAX};

    //Physical properties, to associate with blockID (internal to engine)
    typedef struct {
        uint16_t textureID[FACE_MAX];  //texture of faces A, B, C, D, E, F
        GLfloat tx[FACE_MAX];          //X texture coordinate (0.0)
        GLfloat ty[FACE_MAX];          //Y texture coordinate (0.0)
        GLfloat tx_1[FACE_MAX];        //X texture coordinate (1.0)
        GLfloat ty_1[FACE_MAX];        //Y texture coordinate (1.0)
        uint8_t  properties;
        uint16_t dataOffset;            //If != 0, ID = dataOffset + hitpoints

    //Properties (not used yet)        
    //0x08: Bright: 0=dark, 1=lightsource
    //0x04: Vision: 0=opqaue, 1=see-through
    //0x03: State : 0=solid, 1=loose, 2=liquid, 3=gas
    } BlockInfo;

    //Constants
    const size_t texmap_TILE_LENGTH = 16;   //openGL coords per tile
    const float TILE_LENGTH = 16.0;
    const size_t texmap_TILES = 16;         //tiles in map (1D)
    const unsigned short texmap_TILE_MAX = texmap_TILES * texmap_TILES;
    const uint16_t block_id_MAX = 256;
    const uint16_t item_id_MAX = 2304;
    const uint16_t entity_type_MAX = 128;
    const uint16_t texture_id_MAX = 1024;
    
    //Offsets in texInfo array for texture IDs
    const uint16_t texture_INDEX[mc__::TEX_MAX] = { 0, 256, 512, 528, 544, 560, 578, 594, 610};

    //Texture map ratio:  tile:texmap length
    const float tmr = 1.0f/((float)texmap_TILES);

    class BlockDrawer {
        public:

            //
            //  in-class types
            //

            //Class specific function
            typedef void (BlockDrawer::*drawBlock_f)(uint8_t, uint8_t,
                GLint, GLint, GLint, uint8_t) const;

            //typedef std::unordered_map< uint16_t, mc__::TextureInfo > texInfo_t;

            //
            // Data
            //
            
            //World
            mc__::World *world;
            
            //GL IDs for textures loaded by viewer
            GLuint textures[mc__::TEX_MAX];

            //Store information for block ID (> 256 are my own shortcuts)
            BlockInfo blockInfo[768];
            
            //Texture information for texture ID (as found in blockInfo)
            //texInfo_t texInfo;
            mc__::TextureInfo *texInfo[texture_id_MAX];
            
            //Block drawing function for ID (> 256 are my own shortcuts)
            drawBlock_f drawFunction[768];

            //
            // Functions
            //

            //Constructor        
            BlockDrawer( mc__::World* w, GLuint tex_array[mc__::TEX_MAX] );

            //Draw a block ID, choose it's drawing function and change metadata
            void draw( uint8_t blockID, uint8_t meta,
                GLint x, GLint y, GLint z, uint8_t visflags=0) const;

            //Specific block drawing functions
            //      (able to use same function pointer for each)
            void drawCube( uint8_t blockID, uint8_t meta,
                GLint x, GLint y, GLint z, uint8_t visflags=0) const;
            void drawDyed( uint8_t blockID, uint8_t meta,
                GLint x, GLint y, GLint z, uint8_t visflags=0) const;
            void drawStairs( uint8_t blockID, uint8_t meta,
                GLint x, GLint y, GLint z, uint8_t visflags=0) const;
            void drawLever( uint8_t blockID, uint8_t meta,
                GLint x, GLint y, GLint z, uint8_t visflags=0) const;
            void drawSlab( uint8_t blockID, uint8_t meta,
                GLint x, GLint y, GLint z, uint8_t visflags=0) const;
            void drawDoubleSlab( uint8_t blockID, uint8_t meta,
                GLint x, GLint y, GLint z, uint8_t visflags=0) const;
            void drawSignpost( uint8_t blockID, uint8_t meta,
                GLint x, GLint y, GLint z, uint8_t visflags=0) const;
            void drawWallItem( uint8_t blockID, uint8_t meta,
                GLint x, GLint y, GLint z, uint8_t visflags=0) const;
            void drawDoor( uint8_t blockID, uint8_t meta,
                GLint x, GLint y, GLint z, uint8_t visflags=0) const;
            void drawTrack( uint8_t blockID, uint8_t meta,
                GLint x, GLint y, GLint z, uint8_t visflags=0) const;
            void drawTrack2( uint8_t blockID, uint8_t meta,
                GLint x, GLint y, GLint z, uint8_t visflags=0) const;
            //Cube with 3 sides same, and one "face"
            void drawFaceCube(uint8_t blockID, uint8_t meta,
                GLint x, GLint y, GLint z, uint8_t visflags=0) const;
            //Variation of "face cube" used for pumpkins
            void drawFaceCube2(uint8_t blockID, uint8_t meta,
                GLint x, GLint y, GLint z, uint8_t visflags=0) const;
            void drawChest(uint8_t blockID, uint8_t meta,
                GLint x, GLint y, GLint z, uint8_t visflags=0) const;
            void drawEggBlock(uint8_t blockID, uint8_t meta,
                GLint x, GLint y, GLint z, uint8_t visflags=0) const;
            
            void drawFluid( uint8_t blockID, uint8_t meta,
                GLint x, GLint y, GLint z, uint8_t visflags=0) const;
            void drawPortal( uint8_t blockID, uint8_t meta,
                GLint x, GLint y, GLint z, uint8_t visflags=0) const;
            void drawFence( uint8_t blockID, uint8_t meta,
                GLint x, GLint y, GLint z, uint8_t visflags=0) const;
            void drawPane( uint8_t blockID, uint8_t meta,
                GLint x, GLint y, GLint z, uint8_t visflags=0) const;
            void drawFloorplate( uint8_t blockID, uint8_t meta,
                GLint x, GLint y, GLint z, uint8_t visflags=0) const;
            void draw4thBlock( uint8_t blockID, uint8_t meta,
                GLint x, GLint y, GLint z, uint8_t visflags=0) const;
            void drawTree( uint8_t blockID, uint8_t meta,
                GLint x, GLint y, GLint z, uint8_t visflags=0) const;
            void drawShroom( uint8_t blockID, uint8_t meta,
                GLint x, GLint y, GLint z, uint8_t visflags=0) const;
            void drawWallSign( uint8_t blockID, uint8_t meta,
                GLint x, GLint y, GLint z, uint8_t visflags=0) const;
            void drawButton( uint8_t blockID, uint8_t meta,
                GLint x, GLint y, GLint z, uint8_t visflags=0) const;
            void drawCactus( uint8_t blockID, uint8_t meta,
                GLint x, GLint y, GLint z, uint8_t visflags=0) const;
            void drawCake( uint8_t blockID, uint8_t meta,
                GLint x, GLint y, GLint z, uint8_t visflags=0) const;
            void drawBed( uint8_t blockID, uint8_t meta,
                GLint x, GLint y, GLint z, uint8_t visflags=0) const;
            void drawItem( uint8_t blockID, uint8_t meta,
                GLint x, GLint y, GLint z, uint8_t visflags=0) const;
            void drawBiomeItem( uint8_t blockID, uint8_t meta,
                GLint x, GLint y, GLint z, uint8_t visflags=0) const;
            void drawSapling( uint8_t blockID, uint8_t meta,
                GLint x, GLint y, GLint z, uint8_t visflags=0) const;
            void drawTorch( uint8_t blockID, uint8_t meta,
                GLint x, GLint y, GLint z, uint8_t visflags=0) const;
            void drawTorchOffset( uint8_t blockID, GLint x, GLint y, GLint z,
                GLint off_x=0, GLint off_y=0, GLint off_z=0) const;
            void drawDiode( uint8_t blockID, uint8_t meta,
                GLint x, GLint y, GLint z, uint8_t visflags=0) const;
            void drawFire( uint8_t blockID, uint8_t meta,
                GLint x, GLint y, GLint z, uint8_t visflags=0) const;
            void drawWire( uint8_t blockID, uint8_t meta,
                GLint x, GLint y, GLint z, uint8_t visflags=0) const;
            void drawCrops( uint8_t blockID, uint8_t meta,
                GLint x, GLint y, GLint z, uint8_t visflags=0) const;
            void drawMelonStem( uint8_t blockID, uint8_t meta,
                GLint x, GLint y, GLint z, uint8_t visflags=0) const;
            
            //Draw using IDs higher than 256
            void drawCubeMeta( uint16_t blockID, uint8_t meta,
                GLint x, GLint y, GLint z, uint8_t visflags=0) const;                

            //Draw a cube with dimensions scaled and location offset
            //  scale factor is multiplier, use 0 - 1
            void drawScaledBlock( uint16_t blockID, uint8_t meta,
                GLint x, GLint y, GLint z, uint8_t visflags=0,
                GLfloat scale_x=1, GLfloat scale_y=1, GLfloat scale_z=1,
                bool scale_textures=true,
                GLint off_x=0, GLint off_y=0, GLint off_z=0,
                uint8_t mirror_mask=0) const;
            
            //Draw a 6-sided volume with specified vertices and tex coords
            void drawVertexBlock( GLint vX[8], GLint vY[8], GLint vZ[8],
                const GLfloat tx_0[6], const GLfloat tx_1[6],
                const GLfloat ty_0[6], const GLfloat ty_1[6],
                uint8_t visflags=0, face_ID fid=FRONT) const;


            //Use terrain texture again
            void bindTexture(tex_t bindme) const;
            
            //Use biome color on block face
            void setBlockColor(uint16_t blockID, face_ID face) const;
            
            //Initialization functions
            bool loadBlockInfo();
            void setBlockInfo( uint16_t index, uint16_t A, uint16_t B, uint16_t C,
                uint16_t D, uint16_t E, uint16_t F,
                drawBlock_f drawFunc = &mc__::BlockDrawer::drawCube);
                //12 men died to bring me knowledge of class function pointers
                
            bool loadTexInfo( );    //Fill texture ID -> textureInfo map
                
        protected:
            //RGB settings for leaves, grass. TODO: use biome flag from MapChunk
            GLubyte leaf_color[4];
            GLubyte grass_color[4];
            
            //Texture mirror. mirror_type mask: 1=vertical, 2=horizontal
            void mirrorCoords( GLfloat& tx_0, GLfloat& tx_1,
                GLfloat& ty_0, GLfloat& ty_1, uint8_t mirror_type=2) const;
            
            //Look up texture coordinates for block/other
            bool getTexInfo(uint16_t texID, GLfloat tx_0[6], GLfloat tx_1[6],
                GLfloat ty_0[6], GLfloat ty_1[6]) const;
            //Translate texture ID to texture coordinates
            bool getTexCoords(uint8_t blockID, face_ID faceID, GLfloat& tx_0, GLfloat& tx_1,
                GLfloat& ty_0, GLfloat& ty_1) const;
            
            //Adjust cuboid shape of block
            void adjustTexture(uint16_t blockID,
                GLint off_x, GLint off_y, GLint off_z,
                GLsizei width, GLsizei height, GLsizei depth);
            
            //Assign vertices for cuboid
            void makeCuboidVertex(GLint x0, GLint y0, GLint z0,
                GLsizei width, GLsizei height, GLsizei depth,
                GLint vX[8], GLint vY[8], GLint vZ[8],
                face_ID facing=FRONT
            ) const;
            
            //Add offset to cuboid vertices
            void addVertexOffset(  GLint vX[8], GLint vY[8], GLint vZ[8],
                GLint dX, GLint dY, GLint dZ) const;

            
            //TODO: GLfloat version
    };
}

#endif