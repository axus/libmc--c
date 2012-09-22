/*
  mc__::Blk namespace
  Block physical properties, used for visualization and game logic

  Copyright 2012 axus

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
#include "Block.hpp"

//Static initializer
const bool mc__::Blk::isOpaque[] = {
    false,  // 0 Air
    true,   // 1 Stone
    true,   // 2 Grass
    true,   // 3 Dirt
    true,   // 4 Cobble
    true,   // 5 Wood
    false,  // 6 Sapling
    true,   // 7 Bedrock
    false,  // 8 Water(*)
    false,   // 9 WaterStill
    true,   //10 Lava(*)
    true,   //11 LavaStill
    true,   //12 Sand
    true,   //13 Gravel
    true,   //14 GoldOre
    true,   //15 IronOre
    true,   //16 CoalOre
    true,   //17 Log
    false,  //18 Leaves
    true,   //19 Sponge
    false,  //20 Glass
    true,   //21 Lapis Ore
    true,   //22 Lapis Block
    true,   //23 Dispenser
    true,   //24 Sandstone
    true,   //25 Note Block
    false,  //26 Bed
    false,  //27 Powered Rail
    false,  //28 Detector Rail
    true,   //29 ???
    false,  //30 Web
    false,  //31 Tall Grass
    false,  //32 Dead Bush
    true, true, true, true, //Reserved
    false,  //37 Flower
    false,  //38 Rose
    false,  //39 BrownShroom
    false,  //40 RedShroom
    true,   //41 GoldBlock
    true,   //42 IronBlock
    true,   //43 DoubleStep
    false,  //44 Step
    true,   //45 Brick
    true,   //46 TNT
    true,   //47 Bookshelf
    true,   //48 Mossy
    true,   //49 Obsidian
    false,  //50 Torch
    false,  //51 Fire
    false,  //52 Spawner
    false,  //53 WoodStairs
    false,   //54 Chest (*)
    false,  //55 Wire (*)
    true,   //56 DiamondOre
    true,   //57 DiamondBlock
    true,   //58 Workbench
    false,  //59 Crops (*)
    true,   //60 Soil
    true,   //61 Furnace
    true,   //62 LitFurnace
    false,  //63 SignPost (*)
    false,  //64 WoodDoor (*)
    false,  //65 Ladder (*)
    false,  //66 Track (*)
    false,  //67 CobbleStairs
    false,  //68 WallSign (*)
    false,  //69 Lever
    false,  //70 StonePlate
    false,  //71 IronDoor (*)
    false,  //72 WoodPlate
    true,   //73 RedstoneOre
    true,   //74 RedstoneOreLit(*)
    false,  //75 RedstoneTorch
    false,  //76 RedstoneTorchLit
    false,  //77 StoneButton
    false,  //78 SnowPlate
    true,   //79 Ice
    true,   //80 Snow
    false,  //81 Cactus
    true,   //82 Clay
    false,  //83 Sugarcane (*)
    true,   //84 Jukebox
    false,  //85 Fence (*)
    true,   //86 Pumpkin
    true,   //87 Netherstone
    true,   //88 91 SlowSand
    true,   //89 Lightstone
    false,  //90 Portal (??)
    true,   //91 PumpkinLit
    false,  //92 Cake
    false,  //93 Repeater off
    false,  //94 Repeater on
    true,   //95 Locked chest
    false,  //96 trapdoor
    true,   //97 "Silverfish",
    true,   //98 "StoneBrick,
    true,   //99 "HugeShroomBrown",
    true,   //100 "HugeShroomRed",
    false,  //101 "IronBars",
    false,   //102 "GlassPane", 
    true,   //103 "Melon", 
    false,   //104 "PumpkinStem", 
    false,   //105 "MelonStem",
    false,   //106 "Vines", 
    false,   //107 "FenceGate", 
    false,   //108 "StairsBrick", 
    false,   //109 "StairsStone", 
    true,   //110 "Mycelium Grass", 
    false,   //111 "LilyPad",
    true,   //112  "BrickNether", 
    false,  //113  "FenceNether",
    false,   //114 "StairsNether", 
    false,   //115 "NetherWart", 
    false,   //116 "Enchanting", 
    false,   //117 "Brewing",
    false,   //118 "Cauldron", 
    false,   //119 "EndPortal", 
    false,   //120 "EndPortalFrame", 
    true,    //121 "EndStone", 
    false,   //122 "DragonEgg", 
    true,   //123 "RedLamp",
    true,   //124 "RedLampOn",
    true,   //125 "DoubleSlabWood", 
    false,   //126 "SlabWood", 
    false,   //127 "CocoaPlant", 
    false,   //128 "StairsSand",
    true,   //129 "EmeraldOre", 
    false,   //130 "EndChest", 
    false,   //131 "TripwireHook", 
    false,   //132 "Tripwire", 
    true,   //133 "EmeraldBlock",
    false,   //134 "StairsSpruce", 
    false,   //135 "StairsBirch", 
    false,   //136 "StairsJungle", 
    true,   //137 "Command", 
    false,   //138 "Beacon",
    true,   //139 "CobbleWall", 
    false,   //140 "Flowerpot", 
    false,   //141 "Carrots", 
    false,   //142 "Potatoes", 
    false,   //143 "ButtonWood", 
    false,   //144 "Head", 
    false
};

//Opaque cubes have their faces blocked by non-opaque cubes
//Non-opaque cubes have their faces blocked by any cube
const bool mc__::Blk::isCube[] = {
    false,  // 0 Air
    true,   // 1 Stone
    true,   // 2 Grass
    true,   // 3 Dirt
    true,   // 4 Cobble
    true,   // 5 Wood
    false,  // 6 Sapling
    true,   // 7 Bedrock
    true,   // 8 Water(*)
    true,   // 9 WaterStill
    true,   //10 Lava(*)
    true,   //11 LavaStill
    true,   //12 Sand
    true,   //13 Gravel
    true,   //14 GoldOre
    true,   //15 IronOre
    true,   //16 CoalOre
    true,   //17 Log
    true,   //18 Leaves
    true,   //19 Sponge
    true,   //20 Glass
    true,   //21 Lapis Ore
    true,   //22 Lapis Block
    true,   //23 Dispenser
    true,   //24 Sandstone
    true,   //25 Note Block
    false,  //26 Bed
    false,  //27 Powered Rail
    false,  //28 Detector Rail
    true,   //29 ????
    false,  //30 Web
    false,   //31 Tall Grass
    false,   //32 Dead Bush
    true, true, true, true, //Cloth
    false,  //37 Flower
    false,  //38 Rose
    false,  //39 BrownShroom
    false,  //40 RedShroom
    true,   //41 GoldBlock
    true,   //42 IronBlock
    true,   //43 DoubleStep
    false,  //44 Step       (bottom is blocked)
    true,   //45 Brick
    true,   //46 TNT
    true,   //47 Bookshelf
    true,   //48 Mossy
    true,   //49 Obsidian
    false,  //50 Torch
    false,  //51 Fire
    false,  //52 Spawner
    false,  //53 WoodStairs
    false,   //54 Chest (*)
    false,  //55 Wire (*)
    true,   //56 DiamondOre
    true,   //57 DiamondBlock
    true,   //58 Workbench
    false,  //59 Crops (*)
    true,   //60 Soil
    true,   //61 Furnace
    true,   //62 LitFurnace
    false,  //63 SignPost (*)
    false,  //64 WoodDoor (*)
    false,  //65 Ladder (*)
    false,  //66 Track (*)
    false,  //67 CobbleStairs
    false,  //68 WallSign (*)
    false,  //69 Lever
    false,  //70 StonePlate
    false,  //71 IronDoor (*)
    false,  //72 WoodPlate
    true,   //73 RedstoneOre
    true,   //74 RedstoneOreLit(*)
    false,  //75 RedstoneTorch
    false,  //76 RedstoneTorchLit
    false,  //77 StoneButton
    false,  //78 SnowPlate
    true,   //79 Ice
    true,   //80 Snow
    false,  //81 Cactus (top and bottom are)
    true,   //82 Clay
    false,  //83 Sugarcane (*)
    true,   //84 Jukebox
    false,  //85 Fence (*)
    true,   //86 Pumpkin
    true,   //87 Netherstone
    true,   //88 91 SlowSand
    true,   //89 Lightstone
    false,  //90 Portal (??)
    true,   //91 PumpkinLit
    false,  //92 Cake
    false,  //93 Repeater
    false,  //94 Repeater On
    true,    //95 Locked Chest
    false,   //96 Trap Door
    true,   //97 "Silverfish",
    true,   //98 "StoneBrick,
    true,   //99 "HugeShroomBrown",
    true,   //100 "HugeShroomRed",
    false,  //101 "IronBars",
    false,   //102 "GlassPane", 
    true,   //103 "Melon", 
    false,   //104 "PumpkinStem", 
    false,   //105 "MelonStem",
    false,   //106 "Vines", 
    false,   //107 "FenceGate", 
    false,   //108 "StairsBrick", 
    false,   //109 "StairsStone", 
    true,   //110 "Mycelium Grass", 
    false,   //111 "LilyPad",
    true,   //112  "BrickNether", 
    false,  //113  "FenceNether",
    false,   //114 "StairsNether", 
    false,   //115 "NetherWart", 
    false,   //116 "Enchanting", 
    false,   //117 "Brewing",
    false,   //118 "Cauldron", 
    false,   //119 "EndPortal", 
    false,   //120 "EndPortalFrame", 
    true,    //121 "EndStone", 
    false,   //122 "DragonEgg", 
    true,   //123 "RedLamp",
    true,   //124 "RedLampOn",
    true,   //125 "DoubleSlabWood", 
    false,   //126 "SlabWood", 
    false,   //127 "CocoaPlant", 
    false,   //128 "StairsSand",
    true,   //129 "EmeraldOre", 
    false,   //130 "EndChest", 
    false,   //131 "TripwireHook", 
    false,   //132 "Tripwire", 
    true,   //133 "EmeraldBlock",
    false,   //134 "StairsSpruce", 
    false,   //135 "StairsBirch", 
    false,   //136 "StairsJungle", 
    true,   //137 "Command", 
    true,   //138 "Beacon",
    true,   //139 "CobbleWall", 
    false,   //140 "Flowerpot", 
    false,   //141 "Carrots", 
    false,   //142 "Potatoes", 
    false,   //143 "ButtonWood", 
    false,   //144 "Head", 
    false
};

//Name of placed block, mapped by ID
const char* mc__::Blk::Name[] = {
    "Air",
    "Stone",
    "Grass",
    "Dirt",
    "Cobblestone",
    "Wood",
    "Sapling",
    "Bedrock",
    "Water",
    "Still water",
    "Lava",
    "Still lava",
    "Sand",
    "Gravel",
    "Gold ore",
    "Iron ore",
    "Coal ore",
    "Log",
    "Leaves",
    "Sponge",
    "Glass",
    "Lapis Ore",
    "Lapis Block",
    "Dispesner",
    "Sandstone",
    "Note Block",
    "Bed",
    "Powered Rail",
    "Detector Rail",
    "Item 29",
    "Web",
    "Tall Grass",
    "Dead Bush",
    "Item 33",
    "Item 34",
    "Item 35",
    "Wool",
    "Yellow flower",
    "Red rose",
    "Brown Mushroom",
    "Red Mushroom",
    "Gold Block",
    "Iron Block",
    "Double Stone Slab",
    "Stone Slab",
    "Brick",
    "TNT",
    "Bookshelf",
    "Moss Stone",
    "Obsidian",
    "Torch",
    "Fire",
    "Monster Spawner",
    "Wooden Stairs",
    "Chest",
    "Redstone Wire",
    "Diamond Ore",
    "Diamond Block",
    "Workbench",
    "Crops",
    "Soil",
    "Furnace",
    "Burning Furnace",
    "Sign Post",
    "Wooden Door",
    "Ladder",
    "Minecart Tracks",
    "Cobblestone Stairs",
    "Wall Sign",
    "Lever",
    "Stone Pressure Plate",
    "Iron Door",
    "Wooden Pressure Plate",
    "Redstone Ore",
    "Glowing Redstone Ore",
    "Redstone torch",
    "Redstone torch (on)",
    "Stone Button",
    "Snow",
    "Ice",
    "Snow Block",
    "Cactus",
    "Clay",
    "Sugarcane",
    "Jukebox",
    "Fence",
    "Pumpkin",
    "Netherrack",
    "Soul Sand",
    "Glowstone",
    "Portal",
    "Jack-O-Lantern",
    "Cake",
    "Repeater",
    "Repeater Lit",
    "Mystery Chest",
    "Trap Door",
    "Silverfish",
    "StoneBrick",
    "HugeShroomBrown",
    "HugeShroomRed",
    "IronBars",
    "GlassPane", 
    "Melon", 
    "PumpkinStem", 
    "MelonStem",
    "Vines", 
    "FenceGate", 
    "StairsBrick", 
    "StairsStone", 
    "Mycelium", 
    "LilyPad",
    "BrickNether",
    "FenceNether",
    "StairsNether", 
    "NetherWart", 
    "Enchanting", 
    "Brewing",
    "Cauldron", 
    "EndPortal", 
    "EndPortalFrame", 
    "EndStone", 
    "DragonEgg", 
    "RedLamp",
    "RedLampOn",
    "DoubleSlabWood", 
    "SlabWood", 
    "CocoaPlant", 
    "StairsSand",
    "EmeraldOre", 
    "EndChest", 
    "TripwireHook", 
    "Tripwire", 
    "EmeraldBlock",
    "StairsSpruce", 
    "StairsBirch", 
    "StairsJungle", 
    "Command", 
    "Beacon",
    "CobbleWall", 
    "Flowerpot", 
    "Carrots", 
    "Potatoes", 
    "ButtonWood", 
    "Head", 
    "145"
};

//Check if block is part of redstone circuit
const bool mc__::Blk::isLogic[] = {
    false,  // 0 Air
    false,  // 1 Stone
    false,  // 2 Grass
    false,  // 3 Dirt
    false,  // 4 Cobble
    false,  // 5 Wood
    false,  // 6 Sapling
    false,  // 7 Bedrock
    false,  // 8 Water(*)
    false,  // 9 WaterStill
    false,  //10 Lava(*)
    false,  //11 LavaStill
    false,  //12 Sand
    false,  //13 Gravel
    false,  //14 GoldOre
    false,  //15 IronOre
    false,  //16 CoalOre
    false,  //17 Log
    false,  //18 Leaves
    false,  //19 Sponge
    false,  //20 Glass
    false,  //21 Lapis Ore
    false,  //22 Lapis Block
    true,   //23 Dispenser  (different from vanilla client)
    false,  //24 Sandstone
    true,   //25 Note Block (different from vanilla client)
    false,  //26 Bed
    true,   //27 Powered Rail
    true,   //28 Detector Rail
    false,  //29 ???
    false,  //30 Web
    false,  //31 Tall Grass
    false,  //32 Dead Bush
    false, false, false, false, //Cloth
    false,  //37 Flower
    false,  //38 Rose
    false,  //39 BrownShroom
    false,  //40 RedShroom
    false,  //41 GoldBlock
    false,  //42 IronBlock
    false,  //43 DoubleStep
    false,  //44 Step
    false,  //45 Brick
    false,  //46 TNT
    false,  //47 Bookshelf
    false,  //48 Mossy
    false,  //49 Obsidian
    false,  //50 Torch
    false,  //51 Fire
    false,  //52 Spawner
    false,  //53 WoodStairs
    false,  //54 Chest (*)
    true,   //55 Wire (*)
    false,  //56 DiamondOre
    false,  //57 DiamondBlock
    false,  //58 Workbench
    false,  //59 Crops (*)
    false,  //60 Soil
    false,  //61 Furnace
    false,  //62 LitFurnace
    false,  //63 SignPost (*)
    false,  //64 WoodDoor (*)
    false,  //65 Ladder (*)
    false,  //66 Track (*)
    false,  //67 CobbleStairs
    false,  //68 WallSign (*)
    true,   //69 Lever
    true,   //70 StonePlate
    false,  //71 IronDoor (*)
    true,   //72 WoodPlate
    true,   //73 RedstoneOre
    true,   //74 RedstoneOreLit(*)
    true,   //75 RedstoneTorch
    true,   //76 RedstoneTorchLit
    true,   //77 StoneButton
    false,  //78 SnowCover
    false,  //79 Ice
    false,  //80 Snow
    false,  //81 Cactus
    false,  //82 Clay
    false,  //83 Sugarcane (*)
    false,  //84 Jukebox
    false,  //85 Fence (*)
    false,  //86 Pumpkin
    false,  //87 Netherstone
    false,  //88 91 SlowSand
    false,  //89 Lightstone
    false,  //90 Portal (??)
    false,  //91 PumpkinLit
    false,  //92 Cake
    true,   //93 Repeater
    true,   //94 Repeater Off
    false,   //95 Mystery Chest
    false,   //96 Trap Door (maybe not?)
    false,   //97 "Silverfish",
    false,   //98 "StoneBrick,
    false,   //99 "HugeShroomBrown",
    false,   //100 "HugeShroomRed",
    false,  //101 "IronBars",
    false,   //102 "GlassPane", 
    false,   //103 "Melon", 
    false,   //104 "PumpkinStem", 
    false,   //105 "MelonStem",
    false,   //106 "Vines", 
    false,   //107 "FenceGate", 
    false,   //108 "StairsBrick", 
    false,   //109 "StairsStone", 
    false,   //110 "Mycelium Grass", 
    false,   //111 "LilyPad",
    false,   //112  "BrickNether", 
    false,  //113  "FenceNether",
    false,   //114 "StairsNether", 
    false,   //115 "NetherWart", 
    false,   //116 "Enchanting", 
    false,   //117 "Brewing",
    false,   //118 "Cauldron", 
    false,   //119 "EndPortal", 
    false,   //120 "EndPortalFrame", 
    false,    //121 "EndStone", 
    false,   //122 "DragonEgg", 
    true,   //123 "RedLamp",
    true,   //124 "RedLampOn",
    false,   //125 "DoubleSlabWood", 
    false,   //126 "SlabWood", 
    false,   //127 "CocoaPlant", 
    false,   //128 "StairsSand",
    false,   //129 "EmeraldOre", 
    false,   //130 "EndChest", 
    true,   //131 "TripwireHook", 
    true,   //132 "Tripwire", 
    false,   //133 "EmeraldBlock",
    false,   //134 "StairsSpruce", 
    false,   //135 "StairsBirch", 
    false,   //136 "StairsJungle", 
    true,   //137 "Command", 
    true,   //138 "Beacon",
    false,   //139 "CobbleWall", 
    false,   //140 "Flowerpot", 
    false,   //141 "Carrots", 
    false,   //142 "Potatoes", 
    true,   //143 "ButtonWood", 
    false,   //144 "Head", 
    false
    };

//Check if block is flammable
const bool mc__::Blk::doesBurn[] = {
    false,  // 0 Air
    false,  // 1 Stone
    true,  // 2 Grass
    false,  // 3 Dirt
    false,  // 4 Cobble
    true,  // 5 Wood
    true,  // 6 Sapling
    false,  // 7 Bedrock
    false,  // 8 Water(*)
    false,  // 9 WaterStill
    false,  //10 Lava(*)
    false,  //11 LavaStill
    false,  //12 Sand
    false,  //13 Gravel
    false,  //14 GoldOre
    false,  //15 IronOre
    false,  //16 CoalOre
    true,  //17 Log
    true,  //18 Leaves
    true,  //19 Sponge
    false,  //20 Glass
    false,  //21 Lapis Ore
    false,  //22 Lapis Block
    false,   //23 Dispenser  (different from vanilla client)
    false,  //24 Sandstone
    true,   //25 Note Block (different from vanilla client)
    true,  //26 Bed
    false,   //27 Powered Rail
    false,   //28 Detector Rail
    false,  //29 ???
    true,  //30 Web
    true,  //31 Tall Grass
    true,  //32 Dead Bush
    true, true, true, true, //Cloth
    true,  //37 Flower
    true,  //38 Rose
    false,  //39 BrownShroom
    false,  //40 RedShroom
    false,  //41 GoldBlock
    false,  //42 IronBlock
    false,  //43 DoubleStep
    false,  //44 Step
    false,  //45 Brick
    true,  //46 TNT
    false,  //47 Bookshelf
    false,  //48 Mossy
    false,  //49 Obsidian
    false,  //50 Torch
    false,  //51 Fire
    false,  //52 Spawner
    true,  //53 WoodStairs
    true,  //54 Chest (*)
    false,   //55 Wire (*)
    false,  //56 DiamondOre
    false,  //57 DiamondBlock
    true,  //58 Workbench
    true,  //59 Crops (*)
    false,  //60 Soil
    false,  //61 Furnace
    false,  //62 LitFurnace
    true,  //63 SignPost (*)
    true,  //64 WoodDoor (*)
    true,  //65 Ladder (*)
    false,  //66 Track (*)
    false,  //67 CobbleStairs
    true,  //68 WallSign (*)
    false,   //69 Lever
    false,   //70 StonePlate
    false,  //71 IronDoor (*)
    true,   //72 WoodPlate
    false,   //73 RedstoneOre
    false,   //74 RedstoneOreLit(*)
    false,   //75 RedstoneTorch
    false,   //76 RedstoneTorchLit
    false,   //77 StoneButton
    false,  //78 SnowCover
    false,  //79 Ice
    false,  //80 Snow
    true,  //81 Cactus
    false,  //82 Clay
    true,  //83 Sugarcane (*)
    true,  //84 Jukebox
    false,  //85 Fence (*)
    true,  //86 Pumpkin
    true,  //87 Netherstone
    false,  //88 91 SlowSand
    false,  //89 Lightstone
    false,  //90 Portal (??)
    true,  //91 PumpkinLit
    true,  //92 Cake
    false,   //93 Repeater
    false,   //94 Repeater Off
    true,   //95 Mystery Chest
    true,   //96 Trap Door
    false,   //97 "Silverfish",
    false,   //98 "StoneBrick,
    true,   //99 "HugeShroomBrown",
    true,   //100 "HugeShroomRed",
    false,  //101 "IronBars",
    false,   //102 "GlassPane", 
    true,   //103 "Melon", 
    true,   //104 "PumpkinStem", 
    true,   //105 "MelonStem",
    true,   //106 "Vines", 
    true,   //107 "FenceGate", 
    false,   //108 "StairsBrick", 
    false,   //109 "StairsStone", 
    false,   //110 "Mycelium Grass", 
    false,   //111 "LilyPad",
    false,   //112  "BrickNether", 
    false,  //113  "FenceNether",
    false,   //114 "StairsNether", 
    true,   //115 "NetherWart", 
    false,   //116 "Enchanting", 
    false,   //117 "Brewing",
    false,   //118 "Cauldron", 
    false,   //119 "EndPortal", 
    false,   //120 "EndPortalFrame", 
    false,    //121 "EndStone", 
    false,   //122 "DragonEgg", 
    true,   //123 "RedLamp",
    true,   //124 "RedLampOn",
    true,   //125 "DoubleSlabWood", 
    false,   //126 "SlabWood", 
    true,   //127 "CocoaPlant", 
    false,   //128 "StairsSand",
    false,   //129 "EmeraldOre", 
    false,   //130 "EndChest", 
    true,   //131 "TripwireHook", 
    true,   //132 "Tripwire", 
    false,   //133 "EmeraldBlock",
    false,   //134 "StairsSpruce", 
    false,   //135 "StairsBirch", 
    false,   //136 "StairsJungle", 
    false,   //137 "Command", 
    false,   //138 "Beacon",
    false,   //139 "CobbleWall", 
    false,   //140 "Flowerpot", 
    true,   //141 "Carrots", 
    true,   //142 "Potatoes", 
    true,   //143 "ButtonWood", 
    false,   //144 "Head", 
    false
};
