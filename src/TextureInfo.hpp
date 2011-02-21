/*
  mc__::TextureInfo
  Store texture ID and offset information for one face

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

#ifndef MC__TEXTUREINFO_H
#define MC__TEXTUREINFO_H

//OpenGL
#include <GL/gl.h>


namespace mc__ {

    enum tex_t { TEX_TERRAIN, TEX_ITEM, TEX_SIGN, TEX_MAX};
  
    class TextureInfo {
      public:
      
        //Constructors
        TextureInfo( tex_t tt, GLfloat tx0, GLfloat tx1,
            GLfloat ty0, GLfloat ty1);
        TextureInfo( tex_t tt, GLsizei max_width, GLsizei max_height,
            GLsizei x0, GLsizei y0, GLsizei tex_width, GLsizei tex_height);
        
        //Get values
        void getCoords( GLfloat& tx0, GLfloat& tx1,
            GLfloat& ty0, GLfloat& ty1) const;
        
        //Calculate texture coordinates (utility function)
        static void setTexCoords(GLsizei max_width, GLsizei max_height,
            GLsizei x0, GLsizei y0, GLsizei tex_width, GLsizei tex_height,
            GLfloat& tx0, GLfloat& tx1,
            GLfloat& ty0, GLfloat& ty1);

        //Variables
        tex_t texType;
        GLfloat tx_0, tx_1, ty_0, ty_1;

    };
    
    //Texture enumerator
    namespace Tex {
        //Enumerator for terrain textures
        enum Terrain {
            Grass=0, Stone, Dirt, Grass_Side, Wood, Step_Side, Step_Top, Brick,
            TNT_Side, TNT_Top, TNT_Bottom, Web, Rose, Daisy, Sapling=15,
            Cobble=16, Admin, Sand, Gravel, Log_Side, Log_Top, IronBlock,
            GoldBlock, DiamondBlock=24, Chest_Top, Chest_Back, Chest_Front,
            RedShroom, BrownShroom, FireText=31, GoldOre=32, IronOre, Coal,
            Books, CobbleMoss, Obsidian, Chest_LFront=40, Chest_RFront,
            Bench_Top, Furnace_Front, Furnace_Back, Dispenser,
            Sponge=48, Glass, DiamondOre, RedOre, Leaf, Leaf1,
            Chest_RBack, Chest_LBack, Bench_Front, Bench_Side, FurnaceLit_Front,
            Furnace_Top, Wool=64, Spawner, Snow, Ice, Snowy_Side, Cactus_Top,
            Cactus_Side, Cactus_Bottom, Clay, SugarCane, Jukebox_Side,
            Jukebox_Top, Torch=80, Door_High, IronDoor_High, Ladder,
            WireX_Off, Wire_Off, Soil_Wet, Soil_Dry, Crops_1, Crops_2, Crops_3,
            Crops_4, Crops_5, Crops_6, Crops_7, Crops_8, Handle=96, Door_Low,
            IronDoor_Low, RedTorch_On, WireX_On, Wire_On, Pumpkin_Top,
            NetherRack, SoulSand, LightStone, Track_Turn=112, Wool_Black,
            Wool_DarkGray, RedTorch_Off, Log2_Side, Log3_Side, Pumpkin_Side,
            Pumpkin_Front, PumpkinLit_Front, Cake_Top, Cake_Side, Cake_Cut,
            Cake_Bottom, Track=128, Wool_Red, Wool_Pink, Leaf2=132, Leaf3,
            Cake=140, LapisBlock=144, Wool_DarkGreen, Wool_Green, LapisOre=160,
            Wool_Brown, Wool_Yellow, SandStone_Top=176, Wool_Blue,
            Wool_LightBlue, Sandstone_Side=192, Wool_Purple, Wool_Magenta,
            Water=205, Water_2, Water_3, Sandstone_Bottom=208, Wool_Aqua,
            Wool_Orange, Water_4=222, Water_5, Wool_Gray=225, Lava=237, Lava_2,
            Lava_3, Crack_1, Crack_2, Crack_3, Crack_4, Crack_5, Crack_6,
            Crack_7, Crack_8, Lava_4=254, Lava_5
        };
    }
}


#endif