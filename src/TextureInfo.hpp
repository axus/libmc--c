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
        enum Terrain_t {
            Grass=0, Stone, Dirt, Grass_Side, Wood, Step_Side, Step_Top, Brick,
            TNT_Side, TNT_Top, TNT_Bottom, Web, Rose, Daisy, Sapling=15,
            Cobble=16, Admin, Sand, Gravel, Log_Side, Log_Top, IronBlock,
            GoldBlock, DiamondBlock=24, Chest_Top, Chest_Back, Chest_Front,
            RedShroom, BrownShroom, FireTex=31, GoldOre=32, IronOre, Coal,
            Books, CobbleMoss, Obsidian, Chest_LFront=40, Chest_RFront,
            Bench_Top, Furnace_Front, Furnace_Back, Dispenser,FireTex2,
            Sponge=48, Glass, DiamondOre, RedOre, Leaf, Leaf1,
            Chest_RBack, Chest_LBack, Bench_Front, Bench_Side, FurnaceLit_Front,
            Furnace_Top, Sapling_Pine, Wool=64, Spawner, Snow, Ice, Snowy_Side,
            Cactus_Top, Cactus_Side, Cactus_Bottom, Clay, SugarCane,
            Jukebox_Side, Jukebox_Top, Sapling_Birch=79, Torch, Door_High,
            IronDoor_High, Ladder, Soil_Wet=86, Soil_Dry, Crops_1, Crops_2,
            Crops_3, Crops_4, Crops_5, Crops_6, Crops_7, Crops_8, Handle=96,
            Door_Low, IronDoor_Low, RedTorch_On, Pumpkin_Top=102,
            NetherRack, SoulSand, LightStone, Track_Turn=112, Wool_Black,
            Wool_DarkGray, RedTorch_Off, Log2_Side, Log3_Side, Pumpkin_Side,
            Pumpkin_Front, PumpkinLit_Front, Cake_Top, Cake_Side, Cake_Cut,
            Cake_Bottom, Track=128, Wool_Red, Wool_Pink, Diode_Off,
            Leaf2=132, Leaf3, BedFoot_Top, BedHead_Top, 
            Cake=140, LapisBlock=144, Wool_DarkGreen, Wool_Green, Diode_On,
            BedFoot_Side=149, BedFoot_Face, BedHead_Face, BedHead_Side,
            LapisOre=160, Wool_Brown, Wool_Yellow, Rail_Off, WireX, Wire,
            SandStone_Top=176, Wool_Blue, Wool_LightBlue, Rail_On, None1,None2,
            Sandstone_Side=192, Wool_Purple, Wool_Magenta, Rail_Sensor,
            Water=205, Water_2, Water_3, Sandstone_Bottom=208, Wool_Aqua,
            Wool_Orange, Water_4=222, Water_5, Wool_Gray=225, Lava=237, Lava_2,
            Lava_3, Crack_1, Crack_2, Crack_3, Crack_4, Crack_5, Crack_6,
            Crack_7, Crack_8, Lava_4=254, Lava_5
        };
        
        enum Item_t {
            I_LeatherHelm=0, I_ChainHelm, I_IronHelm, I_DiamondHelm, I_GoldHelm,
            I_FlintSteel,I_Flint,I_Coal,I_Thread, I_Seeds, I_Apple, I_GoldApple,
            I_Egg, I_Sugar, I_Snowball, I_AirHelm, I_LeatherChest, I_ChainChest,
            I_IronChest, I_DiamondChest, I_GoldChest, I_Bow, I_Brick,
            I_IronIngot, I_Feather, I_Wheat, I_Painting, I_SugarCane, I_Bone,
            I_Cake, I_SlimeBall, I_AirChest, I_LeatherLegs, I_ChainLegs,
            I_IronLegs, I_DiamondLegs, I_GoldLegs, I_Arrow, I_Quiver,
            I_GoldIngot, I_Sulphur, I_Bread, I_Sign, I_Door, I_IronDoor,
            I_LeatherBoots=48, I_ChainBoots, I_IronBoots, I_DiamondBoots,
            I_GoldBoots, I_Stick, I_Compass, I_Diamond, I_Clay, I_RedDust,
            I_Paper, I_Book, I_WoodSword=64, I_StoneSword, I_IronSword,
            I_DiamondSword, I_GoldSword, I_FishingRod, I_Watch, I_Bowl,
            I_BowlSoup, I_LightDust, I_EmptyBucket, I_WaterBucket, I_LavaBucket,
            I_MilkBucket, I_InkSack, I_DarkGrayDye, I_WoodShovel, I_StoneShovel,
            I_IronShovel, I_DiamondShovel, I_GoldShovel, I_Pork=87,
            I_PorkCooked, I_Fish, I_FishCooked, I_Cookie=92, I_RoseDye=94,
            I_PinkDye, I_WoodPick, I_StonePick, I_IronPick, I_DiamondPick,
            I_GoldPick, I_Leather=103, I_Saddle, I_CactusDye=110, I_LimeDye,
            I_WoodAxe, I_StoneAxe, I_IronAxe, I_DiamondAxe, I_GoldAxe,
            I_CocoaDye=126, I_YellowDye, I_WoodHoe, I_StoneHoe, I_IronHoe,
            I_DiamondHoe, I_GoldHoe, I_Minecart=135, I_Boat, I_LapisDye=142,
            I_LightBlueDye, I_ChestCart=151, I_PurpleDye=158, I_MagentaDye,
            I_PowerCart=167, I_CyanDye=174, I_OrangeDye, I_LightGrayDye=190,
            I_BoneMeal=191, I_GoldRecord=240, I_GreenRecord
        };



    }
}


#endif