
/*
  mc__::Block
  Single voxel block.  Has ID, metadata, and lighting
  
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

//Compiler specific options
#ifdef _MSC_VER
    #include "ms_stdint.h"
#else
    #include <stdint.h>
#endif


namespace mc__ {
    //Block in the game world
    typedef struct {
        uint8_t blockID; uint8_t metadata; uint8_t lighting; uint8_t padding;
    } Block;
    
    //Named values for block IDs
    namespace Blk {
        enum Block_t {
            Air=0, Stone, Grass, Dirt, Cobble, Wood, Sapling, Bedrock,
            Water, WaterFlow, Lava, LavaFlow, Sand, Gravel, GoldOre, IronOre,
            CoalOre=16, Log, Leaves, Sponge, Glass, LapisOre, LapisBlock,
            Dispenser, Sandstone, NoteBlock, Bed, RailPowered, RailDetector,
            StickyPiston, Web, TallGrass, DeadBush=32, Piston, PistonHead,
            Wool=35, Daisy, Rose, ShroomBrown, ShroomRed=40, GoldBlock,
            IronBlock, SlabDouble, Slab, Bricks, TNT, Bookshelf, MossStone,
            Obsidian, Torch, Fire, Spawner, StairsWood, Chest, Wire,
            DiamondOre, DiamondBlock, Workbench, Crops, Soil, Furnace,
            FurnaceOn, Signpost, DoorWood=64, Ladder, Track, StairsCobble,
            Wallsign, Lever, PlateStone, DoorIron, PlateWood, RedstoneOre,
            RedstoneOreOn, RedTorch, RedTorchOn, Button, Snow, Ice, SnowBlock,
            Cactus, ClayBlock, SugarCane, Jukebox, Fence, Pumpkin, Netherrack,
            SoulSand, Glowstone, Portal, PumpkinOn, Cake, Diode, DiodeOn,
            ChestGlow, TrapDoor
        };
    }
}

/*

    82          0x52        Clay Block           
    83          0x53        Sugarcane           Growth      0-15
    84          0x54        Jukebox              
    85          0x55        Fence                
    86          0x56        Pumpkin             Orientation 0-3
    87          0x57        Netherrack           
    88          0x58        Soul Sand            
    89          0x59        Glowstone Block     
    90          0x5A        Portal               
    91          0x5B        Jack-O-Lantern      Orientation 0-3
    92          0x5C        Cake                Eaten       0-6
    93          0x5D        Diode (off)         Ori.,Delay  0-15? 
    94          0x5E        Diode (on)          Ori.,Delay  0-15? 
*/

/*
    //Block IDs and associated metadata
    Decimal     Hex         Name                Data        Range
    0           0x00        Air                 
    1           0x01        Stone               
    2           0x02        Grass             
    3           0x03        Dirt                
    4           0x04        Cobble            
    5           0x05        Wood                Type        0-2
    6           0x06        Sapling             Growth      0-15
    7           0x07        Bedrock           
    8           0x08        Water (moving)      Spread      0-7
    9           0x09        Water                     
    10          0x0A        Lava (moving)       Spread      0-3
    11          0x0B        Lava                
    12          0x0C        Sand                 
    13          0x0D        Gravel               
    14          0x0E        Gold Ore             
    15          0x0F        Iron Ore             
    16          0x10        Coal Ore             
    17          0x11        Log                 Tree Type   0-2
    18          0x12        Leaves              Tree Type   0-2
    19          0x13        Sponge               
    20          0x14        Glass                
    21          0x15        Lapis Ore            
    22          0x16        Lapis Block          
    23          0x17        Dispenser           Orientation 2-5
    24          0x18        Sandstone            
    25          0x19        Note Block           
    26          0x1A        Bed                 Piece|Orien 0-3,8-11
    29          0x1D        Sticky Piston       Orien|extended 1-5, +8
    33          0x21        Piston              Orien|extended 1-5, +8
    34          0x22        Piston Head         Orien|extended 1-5, +8
    35          0x23        Wool                Color       0-15
    37          0x25        Daisy                
    38          0x26        Rose                 
    39          0x27        Brown Shroom        
    40          0x28        Red Shroom           
    41          0x29        Gold Block        
    42          0x2A        Iron Block           
    43          0x2B        DoubleSlab          Texture     0-3
    44          0x2C        Slab                Texture     0-3
    45          0x2D        Bricks               
    46          0x2E        TNT                 
    47          0x2F        Bookshelf            
    48          0x30        Moss Stone           
    49          0x31        Obsidian             
    50          0x32        Torch               Orientation 1-5
    51          0x33        Fire                Burning
    52          0x34        Spawner              
    53          0x35        Wood Stairs         Orientation 0-9
    54          0x36        Chest                
    55          0x37        Redstone Wire      
    56          0x38        Diamond Ore          
    57          0x39        Diamond Block       
    58          0x3A        Workbench           
    59          0x3B        Crops               Growth      0-7
    60          0x3C        Dirt/Soil           Irrigation  0-8
    61          0x3D        Furnace             Orientation 2-5
    62          0x3D        Lit Furnace         Orientation 2-5
    63          0x3F        SignPost            Orientation 0-15
    64          0x40        Wooden Door         Open|Hinge  0-7
    65          0x41        Ladder              Orientation 2-5
    66          0x42        Rails               Orientation 0-9
    67          0x43        Rock Stairs         Orientation 0-3
    68          0x44        WallSign            Orientation 2-5
    69          0x45        Lever               Pull|Ori.   0-15
    70          0x46        Stone Plate         Activated   0-1 
    71          0x47        Iron Door           Open|Hinge  0-7
    72          0x48        Wood Plate          Activated   0-1 
    73          0x49        Redstone Ore         
    74          0x4A        Redstone Ore, lit
    75          0x4B        Redstone Torch
    76          0x4C        Redstone Torch, lit
    77          0x4D        Stone Button        Push|Ori.   0-7
    78          0x4E        Snow                
    79          0x4F        Ice                 
    80          0x50        Snow Block  
    81          0x51        Cactus              Growth      0-15
    82          0x52        Clay Block           
    83          0x53        Sugarcane           Growth      0-15
    84          0x54        Jukebox              
    85          0x55        Fence                
    86          0x56        Pumpkin             Orientation 0-3
    87          0x57        Netherrack           
    88          0x58        Soul Sand            
    89          0x59        Glowstone Block     
    90          0x5A        Portal               
    91          0x5B        Jack-O-Lantern      Orientation 0-3
    92          0x5C        Cake                Eaten       0-6
    93          0x5D        Diode (off)         Ori.,Delay  0-15? 
    94          0x5E        Diode (on)          Ori.,Delay  0-15? 
*/

