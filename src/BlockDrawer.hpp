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

//DevIL
#include <IL/il.h>

//STL
#include <string>
#include <map>

//OpenGL
#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glext.h>

namespace mc__ {

    //
    // Types for mc__ namespace
    //
    enum face_ID { WEST=0, EAST=1, DOWN=2, UP=3, NORTH=4, SOUTH=5, FACE_MAX};

    //Physical properties, to associate with blockID (internal to engine)
    typedef struct {
        uint16_t textureID[FACE_MAX];  //texture of faces A, B, C, D, E, F
        GLfloat tx[FACE_MAX];          //X texture coordinate (0.0 - 1.0)
        GLfloat ty[FACE_MAX];          //Y texture coordinate (0.0 - 1.0)
        uint8_t  properties;
        uint16_t dataOffset;            //If != 0, ID = dataOffset + hitpoints
        
    //0xF0: Shape : 0=cube, 1=stairs, 2=lever, 3=halfblock,
    //              4=wallsign, 5=ladder, 6=track, 7=fire
    //              8=portal, 9=fence, A=door, B=floorplate
    //              C=snow, D=wallsign, E=button, F=plant
    //0x08: Bright: 0=dark, 1=lightsource
    //0x04: Vision: 0=opqaue, 1=see-through
    //0x03: State : 0=solid, 1=loose, 2=liquid, 3=gas
    } BlockInfo;

    //Constants
    const size_t texmap_TILE_LENGTH = 16;   //openGL coords per tile
    const size_t texmap_TILES = 16;         //tiles in map (1D)
    const unsigned short texmap_TILE_MAX = texmap_TILES * texmap_TILES;
    const uint16_t block_id_MAX = 256;
    const uint16_t item_id_MAX = 2304;
    const uint16_t entity_type_MAX = 128;

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

            //
            // Data
            //
            
            //World
            mc__::World *world;
            
            //GL IDs for textures loaded by viewer
            GLuint terrain_tex, item_tex;

            //Texture information for ID (> 256 are my own shortcuts)
            BlockInfo blockInfo[768];
            
            //Block drawing function for ID (> 256 are my own shortcuts)
            drawBlock_f drawFunction[768];

            //
            // Functions
            //

            //Constructor        
            BlockDrawer( mc__::World* w, GLuint t_tex, GLuint i_tex );

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
            void drawHalfBlock( uint8_t blockID, uint8_t meta,
                GLint x, GLint y, GLint z, uint8_t visflags=0) const;
            void drawSignpost( uint8_t blockID, uint8_t meta,
                GLint x, GLint y, GLint z, uint8_t visflags=0) const;
            void drawWallItem( uint8_t blockID, uint8_t meta,
                GLint x, GLint y, GLint z, uint8_t visflags=0) const;
            void drawDoor( uint8_t blockID, uint8_t meta,
                GLint x, GLint y, GLint z, uint8_t visflags=0) const;
            void drawTrack( uint8_t blockID, uint8_t meta,
                GLint x, GLint y, GLint z, uint8_t visflags=0) const;
            //Cube with 3 sides same, and one "face"
            void drawFaceCube(uint8_t blockID, uint8_t meta,
                GLint x, GLint y, GLint z, uint8_t visflags=0) const;
            //Variation of "face cube" used for pumpkins
            void drawFaceCube2(uint8_t blockID, uint8_t meta,
                GLint x, GLint y, GLint z, uint8_t visflags=0) const;
            void drawChest(uint8_t blockID, uint8_t meta,
                GLint x, GLint y, GLint z, uint8_t visflags=0) const;
            
            void drawPortal( uint8_t blockID, uint8_t meta,
                GLint x, GLint y, GLint z, uint8_t visflags=0) const;
            void drawFence( uint8_t blockID, uint8_t meta,
                GLint x, GLint y, GLint z, uint8_t visflags=0) const;
            void drawFloorplate( uint8_t blockID, uint8_t meta,
                GLint x, GLint y, GLint z, uint8_t visflags=0) const;
            void draw4thBlock( uint8_t blockID, uint8_t meta,
                GLint x, GLint y, GLint z, uint8_t visflags=0) const;
            void drawTree( uint8_t blockID, uint8_t meta,
                GLint x, GLint y, GLint z, uint8_t visflags=0) const;
            void drawWallSign( uint8_t blockID, uint8_t meta,
                GLint x, GLint y, GLint z, uint8_t visflags=0) const;
            void drawButton( uint8_t blockID, uint8_t meta,
                GLint x, GLint y, GLint z, uint8_t visflags=0) const;
            void drawCactus( uint8_t blockID, uint8_t meta,
                GLint x, GLint y, GLint z, uint8_t visflags=0) const;
            void drawCake( uint8_t blockID, uint8_t meta,
                GLint x, GLint y, GLint z, uint8_t visflags=0) const;
            void drawItem( uint8_t blockID, uint8_t meta,
                GLint x, GLint y, GLint z, uint8_t visflags=0) const;
            void drawTorch( uint8_t blockID, uint8_t meta,
                GLint x, GLint y, GLint z, uint8_t visflags=0) const;
            void drawFire( uint8_t blockID, uint8_t meta,
                GLint x, GLint y, GLint z, uint8_t visflags=0) const;
            void drawWire( uint8_t blockID, uint8_t meta,
                GLint x, GLint y, GLint z, uint8_t visflags=0) const;
            void drawCrops( uint8_t blockID, uint8_t meta,
                GLint x, GLint y, GLint z, uint8_t visflags=0) const;

            //Draw using IDs higher than 256
            void drawCubeMeta( uint16_t blockID, uint8_t meta,
                GLint x, GLint y, GLint z, uint8_t visflags=0) const;                

            //Draw a cube with dimensions scaled and location offset
            //  scale factor is multiplier, use 0 - 1
            void drawScaledBlock( uint8_t blockID, uint8_t meta,
                GLint x, GLint y, GLint z, uint8_t visflags=0,
                GLfloat scale_x=1, GLfloat scale_y=1, GLfloat scale_z=1,
                bool scale_textures=true,
                GLint off_x=0, GLint off_y=0, GLint off_z=0) const;

            //Use terrain texture again
            void rebindTerrain();
            
            //Use biome color on block face
            void setBlockColor(uint8_t blockID, face_ID face) const;
            
            //Initialization functions
            bool loadBlockInfo();
            void setBlockInfo( uint16_t index, uint8_t A, uint8_t B, uint8_t C,
                uint8_t D, uint8_t E, uint8_t F, uint8_t properties,
                drawBlock_f drawFunc = &mc__::BlockDrawer::drawCube);
                //12 men died to bring me knowledge of class function pointers
                
        protected:
            //RGB settings for leaves, grass. TODO: use biome flag from MapChunk
            GLubyte leaf_color[4];
            GLubyte grass_color[4];

    };
}

#endif