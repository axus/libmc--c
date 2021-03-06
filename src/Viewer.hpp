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

//Version 0.5.0
#define MC__VIEWER_VERSION 0x0500

//mc__
#include "World.hpp"
#include "Mobiles.hpp"
#include "BlockDrawer.hpp"

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

    class Viewer {
        public:

            //relate MapChunk* -> GL List number
            typedef std::unordered_map< mc__::MapChunk*, GLuint>
                mapChunkUintMap_t;

            //Constructor
            Viewer( World* w,
                unsigned short width, unsigned short height);
            
            //Map item ID to item information
            BlockInfo itemInfo[item_id_MAX];

            //World pointer
            mc__::World* world;
            
            //Use this to draw individual blocks
            BlockDrawer *blockDraw;
            
            //Current camera position
            GLfloat cam_X, cam_Y, cam_Z;

            bool init( const std::string filenames[mc__::TEX_MAX],
                bool mipmaps=true);

            //Load texture map
            ILuint loadImageFile( const std::string &imageFilename);
            bool configureTexture(GLuint texture_ID); 
            
            //change back to terrain texture
            void rebindTerrain();   

            //
            // Drawing functions
            //

            //Erase all polygons from openGL
            void clear();

            //Offset model view from current camera position
            void drawFromCamera();

            //Single block drawing functions
            void drawBlock(const mc__::Block& block,
                GLint x, GLint y, GLint z, uint8_t visflags=0);

            //dropped item drawing function (for display lists)
            void drawDroppedItem( uint16_t blockID );

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


            //
            //
            //

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
            bool saveLocalBlocks(const mc__::World& world) const;

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
            //std::string texture_map_file, item_icon_file;
            //std::string texture_files[mc__::TEX_MAX];

            //openGL image (for texture)
            GLuint textures[mc__::TEX_MAX]; //terrain_tex, item_tex;
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
            void setItemInfo( uint16_t index, uint8_t A,
                uint8_t properties, uint16_t offset=0);
            bool loadItemInfo();

            //Create display list for ID after loadItemInfo has been called
            bool createItemModel( uint16_t ID);

            //Create display lists for all IDs
            //bool createItemModels();
            //bool createEntityModels();
            
            //Change face colors if needed by blockID
            void setBlockColor(uint8_t blockID, face_ID face);

            //Debugging functions
            void outputRGBAData();

        public:
            //Yaw offset for rotating items
            float item_rotation;
        
            //Graphics options
            bool use_mipmaps, use_blending;
            
            //Debugging flag
            bool debugging;

            //Float pi
            static const float PI;
    };
}


#endif
