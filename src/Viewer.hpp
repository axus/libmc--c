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

#ifndef MC__VIEWER_H
#define MC__VIEWER_H

//Version 0.1.2
#define MC__VIEWER_VERSION 0x0102

//mc__
#include "World.hpp"
#include "Mobiles.hpp"

//DevIL
#include <IL/il.h>

//STL
#include <string>
#include <map>

//OpenGL
//#include <GL/glew.h>
#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glext.h>

namespace mc__ {

    //Library version checker in mc__ namespace
    unsigned long getVersion();
    
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

    //Re-use BlockInfo type for iteminfo.  Use these values for properties:
    // 0x00 = terrain cube
    // 0x01 = terrain item
    // 0x02 = non-placeable item (items.png)
    // 0x04 = stackable
    // 0x10 = chest
    // 0x20 = lever
    // 0x30 = double-chest
    // 0x40 = sign
    // 0x50 = furnace
    // 0x60 = workbench

    //Constants
    const size_t texmap_TILE_LENGTH = 16;   //openGL coords per tile
    const size_t texmap_TILES = 16;         //tiles in map (1D)
    const unsigned short texmap_TILE_MAX = texmap_TILES * texmap_TILES;
    const uint16_t block_id_MAX = 256;
    const uint16_t item_id_MAX = 2304;
    const uint16_t entity_type_MAX = 128;

    //Texture map ratio:  tile:texmap length
    const float tmr = 1.0f/((float)texmap_TILES);

    class Viewer {
        public:

            //relate MapChunk* -> GL List number
            typedef std::unordered_map< mc__::MapChunk*, GLuint>
                mapChunkUintMap_t;

            //Constructor
            Viewer(unsigned short width, unsigned short height);
            
            //Map block ID to block information
            BlockInfo blockInfo[block_id_MAX];
            
            //Map item ID to item information
            BlockInfo itemInfo[item_id_MAX];
            
            //Current camera position
            GLfloat cam_X, cam_Y, cam_Z;

            bool init(const std::string &texture_map_file,
                const std::string &item_icon_file,
                bool mipmaps=true);

            //Load texture map
            ILuint loadImageFile( const std::string &imageFilename);
            void rebindTerrain();   //change back to texture if needed

            //Single block drawing functions
            void drawBlock(const mc__::Block& block,
                GLint x, GLint y, GLint z, uint8_t visflags=0);

            //Specific block drawing functions
            void drawCube( uint8_t blockID, GLint x, GLint y, GLint z,
                uint8_t visflags=0);
            void drawHalfBlock( uint8_t blockID, GLint x, GLint y, GLint z,
                uint8_t visflags=0);
            void drawItem( uint8_t blockID, GLint x, GLint y, GLint z);
            void drawTrack( uint8_t blockID, GLint x, GLint y, GLint z);
            void drawWallItem( uint8_t blockID, GLint x, GLint y, GLint z);
            void drawCactus( uint8_t blockID, GLint x, GLint y, GLint z,
                uint8_t visflags=0);

            //Draw a cube with dimensions scaled and location offset
            //  scale factor is multiplier, use 0 - 1
            void drawScaledBlock( uint8_t blockID,
                GLint x, GLint y, GLint z, uint8_t visflags=0,
                GLfloat scale_x=1, GLfloat scale_y=1, GLfloat scale_z=1,
                bool scale_textures=true,
                GLint off_x=0, GLint off_y=0, GLint off_z=0);

            //dropped item drawing function (for display lists)
            void drawDroppedItem( uint16_t blockID);

            //Draw minichunks only
            void drawChunks( const mc__::World& world);
            
            //Draw a 16x128x16 chunk, unmark "UPDATED" flag
            void drawMapChunk(mc__::MapChunk* mc);
            
            //Draw all the mapchunks
            void drawMapChunks( const mc__::World& world);
            
            //Draw all terrain and placed blocks
            bool drawWorld(const mc__::World& world);
            
            //Draw all moving objects (entities)
            bool drawMobiles(const mc__::Mobiles& mobiles);
            
            //Camera functions
            void move( GLfloat side, GLfloat up, GLfloat forward);
            void turn( GLfloat degrees);  //Change current yaw by "degrees"
            void tilt( GLfloat degrees);  //Change current pitch by "degrees"
            void reset(GLfloat x, GLfloat y, GLfloat z,
                GLfloat yaw, GLfloat pitch);
                
            //Camera perspective functions
            void viewport( GLint x, GLint y, GLsizei width, GLsizei height);
            void setDrawDistance( GLdouble d);
            
            //Export functions
            bool writeChunkBin(mc__::Chunk *chunk,
                const std::string& filename) const;
            void printChunks(const mc__::World& world) const;
            bool saveChunks(const mc__::World& world) const;
            
            //RGB settings for leaves, grass. TODO: use biome flag from MapChunk
            GLubyte leaf_color[4];
            GLubyte grass_color[4];

            //Relate world mapchunks to GL lists
            mapChunkUintMap_t glListMap;
            mapChunkUintMap_t glListMapOccluded;
            
        protected:
            
            //View settings: draw distance, width x height, aspect ratio
            GLdouble drawDistance;
            unsigned short view_width, view_height;
            GLfloat aspectRatio, fieldOfViewY;
            
            //Current camera angle
            GLfloat cam_yaw, cam_pitch, cam_vecX, cam_vecY, cam_vecZ;
            
            //Remember texture map filename
            std::string texture_map_file, item_icon_file;

            //openGL image (for texture)
            GLuint terrain_tex, item_tex;
            GLuint entity_tex[entity_type_MAX];

            //GL display list of terrain display lists that player can see
            GLuint glListPlayer;

            //GL display list of terrain display lists that player cannot see
            GLuint glListCamera;

            //Map ID to GL display list
            GLuint itemModels[item_id_MAX];
            GLuint entityModels[entity_type_MAX];

            //Init functions
            void startOpenGL();
            void setBlockInfo( uint8_t index, uint8_t A, uint8_t B, uint8_t C,
                uint8_t D, uint8_t E, uint8_t F, uint8_t properties);
            void setItemInfo( uint16_t index, uint8_t A, uint8_t properties);
            bool loadBlockInfo();
            bool loadItemInfo();

            //Create display list for ID after loadItemInfo has been called
            bool createItemModel( uint16_t ID);

            //Create display lists for all IDs
            bool createItemModels();
            bool createEntityModels();
            
            //Change face colors if needed by blockID
            void setBlockColor(uint8_t blockID, face_ID face);

            //Debugging functions
            void outputRGBAData();

        public:
            //Graphics options
            bool use_mipmaps, use_blending;
            
            //Debugging flag
            bool debugging;

            //Float pi
            static const float PI;
    };
}


#endif
