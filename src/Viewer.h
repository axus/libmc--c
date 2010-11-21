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

#ifndef MC__VIEWER_H
#define MC__VIEWER_H

//Version 0.10
#define MC__VIEWER_VERSION 0x000A

//DevIL
#include <IL/il.h>
#include <IL/ilu.h>

//SFML
#include <SFML/Graphics.hpp>

//STL
#include <string>

//mc__
#include "World.h"

namespace mc__ {

    enum face_ID { WEST=0, EAST=1, DOWN=2, UP=3, NORTH=4, SOUTH=5, FACE_MAX};
        
    //Physical properties, to associate with blockID (internal to engine)
    typedef struct {
        uint16_t textureID[FACE_MAX];  //texture of faces A, B, C, D, E, F
        GLfloat tx[FACE_MAX];          //X texture coordinate (0.0 - 1.0)
        GLfloat ty[FACE_MAX];          //Y texture coordinate (0.0 - 1.0)
        uint8_t  properties;
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
    
    //Texture map ratio:  tile:texmap length
    const float tmr = 1.0f/((float)texmap_TILES);
            
    class Viewer {
        public:

            Viewer();
            
            //Map block ID to block information
            BlockInfo blockInfo[256];
            
            //Initial camera position
            GLint camera_X;
            GLint camera_Y;
            GLint camera_Z;

            bool init(const std::string &texture_map_file);

            //Load texture map
            ILuint loadImageFile( const std::string &imageFilename);

            //Drawing functions
            void drawCube( uint8_t blockID, GLint x, GLint y, GLint z);
            void drawItem( uint8_t blockID, GLint x, GLint y, GLint z);
            void drawBlock( const mc__::Block& block, GLint x, GLint y, GLint z);
            void drawChunks( const mc__::World& world);
            bool drawWorld(const mc__::World& world);

            //Camera functions
            void lookAt( GLint from_x, GLint from_y, GLint from_z, 
                         GLint at_x, GLint at_y, GLint at_z,
                         GLint up_x=0, GLint up_y=1, GLint up_z=0);
            void move( GLint offset_x, GLint offset_y, GLint offset_z);
            void turn( GLint degrees, GLint axus_x=0, GLint axus_y=1, GLint axus_z=0);
            void viewport( GLint x, GLint y, GLsizei width, GLsizei height);
            void reset();
            
            //RGB settings for leaves, grass :)
            GLubyte leaf_color[4];
            GLubyte grass_color[4];

        protected:
            //Remember texture map filename
            std::string texture_map_file;

            //DevIL textures
            ILuint il_texture_map;
            ILuint ilTextureList[texmap_TILE_MAX];
            
            //openGL image
            GLuint image;
            
            //Init functions
            void startOpenGL();
            void setBlockInfo( uint8_t index, uint8_t A, uint8_t B, uint8_t C,
                uint8_t D, uint8_t E, uint8_t F, uint8_t properties);
            bool loadBlockInfo();
            
            //Change face colors if needed by blockID
            void setBlockColor(uint8_t blockID, face_ID face);

            //Debugging functions
            void outputRGBAData();
    };
}


#endif