/*
  mc__::Item
    Represent game Item
  
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

//mc--
#include "Item.hpp"

using mc__::Item;
using mc__::Entity;
using std::string;

//Constructor
Item::Item( uint16_t iid, uint8_t c, uint8_t d, uint32_t eid):
    Entity( eid), itemID(iid), count(c)
{
    hitpoints = d;
}

//Get name for this item's ID
string Item::getName() const
{
    //Check for special cases
    uint16_t ID = itemID;
    switch (itemID) {
        case 351: ID = 1520 + hitpoints; break;
        default: break;
    }
    
    return getString(ID);
}

//(static function) Get string for an item ID
string Item::getString( uint16_t iid, uint8_t offset)
{
    string result;
    
    //String depends on itemID
    switch(iid + offset) {
        case 0: result =  "Air"; break;
        case 1: result =  "Stone"; break;
        case 2: result =  "Grass"; break;
        case 3: result =  "Dirt"; break;
        case 4: result =  "Cobblestone"; break;
        case 5: result =  "Wood"; break;
        case 6: result =  "Sapling"; break;
        case 7: result =  "Bedrock"; break;
        case 8: result =  "Water"; break;
        case 9: result =  "Stationary water"; break;
        case 10: result = "Lava"; break;
        case 11: result = "Stationary lava"; break;
        case 12: result = "Sand"; break;
        case 13: result = "Gravel"; break;
        case 14: result = "Gold ore"; break;
        case 15: result = "Iron ore"; break;
        case 16: result = "Coal ore"; break;
        case 17: result = "Log"; break;
        case 18: result = "Leaves"; break;
        case 19: result = "Sponge"; break;
        case 20: result = "Glass"; break;
        case 21: result = "Lapis ore"; break;
        case 22: result = "Lapis block"; break;
        case 23: result = "Dispenser"; break;
        case 24: result = "Sandstone"; break;
        case 25: result = "Note block"; break;
        case 26: result = "Bed"; break;
        case 27: result = "Unknown 0x27"; break;
        case 28: result = "Unknown 0x28"; break;
        case 29: result = "Unknown 0x29"; break;
        case 30: result = "Unknown 0x30"; break;
        case 31: result = "Unknown 0x31"; break;
        case 32: result = "Unknown 0x32"; break;
        case 33: result = "Unknown 0x33"; break;
        case 34: result = "Unknown 0x34"; break;
        case 35: result = "Wool"; break;
        case 36: result = "Unknown 0x36"; break;
        case 37: result = "Yellow flower"; break;
        case 38: result = "Red rose"; break;
        case 39: result = "Brown mushroom"; break;
        case 40: result = "Red mushroom"; break;
        case 41: result = "Gold block"; break;
        case 42: result = "Iron block"; break;
        case 43: result = "Double stone slab"; break;
        case 44: result = "Stone slab"; break;
        case 45: result = "Brick"; break;
        case 46: result = "TNT"; break;
        case 47: result = "Bookshelf"; break;
        case 48: result = "Moss stone"; break;
        case 49: result = "Obsidian"; break;
        case 50: result = "Torch"; break;
        case 51: result = "Fire"; break;
        case 52: result = "Monster spawn"; break;
        case 53: result = "Wooden stairs"; break;
        case 54: result = "Chest"; break;
        case 55: result = "Redstone wire"; break;
        case 56: result = "Diamond ore"; break;
        case 57: result = "Diamond block"; break;
        case 58: result = "Workbench"; break;
        case 59: result = "Crops"; break;
        case 60: result = "Soil"; break;
        case 61: result = "Furnace"; break;
        case 62: result = "Lit furnace"; break;
        case 63: result = "Sign post"; break;
        case 64: result = "Wooden door"; break;
        case 65: result = "Ladder"; break;
        case 66: result = "Rails"; break;
        case 67: result = "Cobblestone stairs"; break;
        case 68: result = "Wall sign"; break;
        case 69: result = "Lever"; break;
        case 70: result = "Stone floorplate"; break;
        case 71: result = "Iron door"; break;
        case 72: result = "Wooden floorplate"; break;
        case 73: result = "Redstone ore"; break;
        case 74: result = "Glowing redstone"; break;
        case 75: result = "Redstone torch"; break;
        case 76: result = "Lit redstone torch"; break;
        case 77: result = "Stone button"; break;
        case 78: result = "Snow"; break;
        case 79: result = "Ice"; break;
        case 80: result = "Snow block"; break;
        case 81: result = "Cactus"; break;
        case 82: result = "Clay"; break;
        case 83: result = "Reed"; break;
        case 84: result = "Jukebox"; break;
        case 85: result = "Fence"; break;
        case 86: result = "Pumpkin"; break;
        case 87: result = "Netherrack"; break;
        case 88: result = "Soul sand"; break;
        case 89: result = "Glowstone"; break;
        case 90: result = "Portal"; break;
        case 91: result = "Jack-O-Lantern"; break;
        case 92: result = "Cake"; break;
        case 93: result = "Diode"; break;
        case 94: result = "Diode Lit"; break;
        case 256: result = "Iron shovel"; break;
        case 257: result = "Iron pickaxe"; break;
        case 258: result = "Iron axe"; break;
        case 259: result = "Flint and steel"; break;
        case 260: result = "Apple"; break;
        case 261: result = "Bow"; break;
        case 262: result = "Arrow"; break;
        case 263: result = "Coal"; break;
        case 264: result = "Diamond"; break;
        case 265: result = "Iron ingot"; break;
        case 266: result = "Gold ingot"; break;
        case 267: result = "Iron sword"; break;
        case 268: result = "Wooden sword"; break;
        case 269: result = "Wooden shovel"; break;
        case 270: result = "Wooden pickaxe"; break;
        case 271: result = "Wooden axe"; break;
        case 272: result = "Stone sword"; break;
        case 273: result = "Stone shovel"; break;
        case 274: result = "Stone pickaxe"; break;
        case 275: result = "Stone axe"; break;
        case 276: result = "Diamond sword"; break;
        case 277: result = "Diamond shovel"; break;
        case 278: result = "Diamond pickaxe"; break;
        case 279: result = "Diamond axe"; break;
        case 280: result = "Stick"; break;
        case 281: result = "Bowl"; break;
        case 282: result = "Mushroom soup"; break;
        case 283: result = "Gold sword"; break;
        case 284: result = "Gold shovel"; break;
        case 285: result = "Gold pickaxe"; break;
        case 286: result = "Gold axe"; break;
        case 287: result = "String"; break;
        case 288: result = "Feather"; break;
        case 289: result = "Sulphur"; break;
        case 290: result = "Wooden hoe"; break;
        case 291: result = "Stone hoe"; break;
        case 292: result = "Iron hoe"; break;
        case 293: result = "Diamond hoe"; break;
        case 294: result = "Gold hoe"; break;
        case 295: result = "Seeds"; break;
        case 296: result = "Wheat"; break;
        case 297: result = "Bread"; break;
        case 298: result = "Leather helmet"; break;
        case 299: result = "Leather tunic"; break;
        case 300: result = "Leather leggings"; break;
        case 301: result = "Leather boots"; break;
        case 302: result = "Chainmail helmet"; break;
        case 303: result = "Chainmail tunic"; break;
        case 304: result = "Chainmail leggings"; break;
        case 305: result = "Chainmail boots"; break;
        case 306: result = "Iron helmet"; break;
        case 307: result = "Iron mail"; break;
        case 308: result = "Iron leggings"; break;
        case 309: result = "Iron boots"; break;
        case 310: result = "Diamond helmet"; break;
        case 311: result = "Diamond mail"; break;
        case 312: result = "Diamond leggings"; break;
        case 313: result = "Diamond boots"; break;
        case 314: result = "Gold helmet"; break;
        case 315: result = "Gold chestplate"; break;
        case 316: result = "Gold leggings"; break;
        case 317: result = "Gold boots"; break;
        case 318: result = "Flint"; break;
        case 319: result = "Raw porkchop"; break;
        case 320: result = "Cooked porkchop"; break;
        case 321: result = "Paintings"; break;
        case 322: result = "Golden apple"; break;
        case 323: result = "Sign"; break;
        case 324: result = "Wooden door"; break;
        case 325: result = "Bucket"; break;
        case 326: result = "Water bucket"; break;
        case 327: result = "Lava bucket"; break;
        case 328: result = "Mine cart"; break;
        case 329: result = "Saddle"; break;
        case 330: result = "Iron door"; break;
        case 331: result = "Redstone"; break;
        case 332: result = "Snowball"; break;
        case 333: result = "Boat"; break;
        case 334: result = "Leather"; break;
        case 335: result = "Milk"; break;
        case 336: result = "Clay brick"; break;
        case 337: result = "Clay balls"; break;
        case 338: result = "Reed"; break;
        case 339: result = "Paper"; break;
        case 340: result = "Book"; break;
        case 341: result = "Slimeball"; break;
        case 342: result = "Storage cart"; break;
        case 343: result = "Powered cart"; break;
        case 344: result = "Egg"; break;
        case 345: result = "Compass"; break;
        case 346: result = "Fishing rod"; break;
        case 347: result = "Clock"; break;
        case 348: result = "Glowstone dust"; break;
        case 349: result = "Raw fish"; break;
        case 350: result = "Cooked fish"; break;
        case 351: 
            //Offset should have been 1520!
            result = "Dye"; break;
            break;
        case 352: result = "Bone"; break;
        case 353: result = "Sugar"; break;
        case 354: result = "Cake"; break;
        case 355: result = "Bed"; break;
        case 356: result = "Diode"; break;
        case 397: result = "Record"; break; //Not really in the game ;)
        
        //Mad hax time: (("Item ID" - 256) * 16) + damage = fake ID
        case 1520: result = "Ink Sack"; break;
        case 1521: result = "Rose dye"; break;
        case 1522: result = "Cactus dye"; break;
        case 1523: result = "Cocoa dye"; break;
        case 1524: result = "Lapis dye"; break;
        case 1525: result = "Purple dye"; break;
        case 1526: result = "Cyan dye"; break;
        case 1527: result = "Light gray dye"; break;
        case 1528: result = "Gray dye"; break;
        case 1529: result = "Pink dye"; break;
        case 1530: result = "Lime dye"; break;
        case 1531: result = "Yellow dye"; break;
        case 1532: result = "Light Blue dye"; break;
        case 1533: result = "Magenta dye"; break;
        case 1534: result = "Orange dye"; break;
        case 1535: result = "Bone meal"; break;
        
        case 2256: result = "Gold record"; break;
        case 2257: result = "Green record"; break;

        default:
            result="Unknown";
            break;
    }
    
    return result;
}


//Max number of uses for this item
uint8_t Item::maxUses() const
{
    uint8_t result=0;   //Unlimited uses if equal to 0
    
    //String depends on itemID
    switch(itemID) {
        case 0: result =  0; break;
        case 1: result =  0; break;
        case 2: result =  0; break;
        case 3: result =  0; break;
        case 4: result =  0; break;
        case 5: result =  0; break;
        case 6: result =  0; break;
        case 7: result =  0; break;
        case 8: result =  0; break;
        case 9: result =  0; break;
        case 10: result = 0; break;
        case 11: result = 0; break;
        case 12: result = 0; break;
        case 13: result = 0; break;
        case 14: result = 0; break;
        case 15: result = 0; break;
        case 16: result = 0; break;
        case 17: result = 0; break;
        case 18: result = 0; break;
        case 19: result = 0; break;
        case 20: result = 0; break;
        case 21: result = 0; break;
        case 22: result = 0; break;
        case 23: result = 0; break;
        case 24: result = 0; break;
        case 25: result = 0; break;
        case 26: result = 0; break;
        case 27: result = 0; break;
        case 28: result = 0; break;
        case 29: result = 0; break;
        case 30: result = 0; break;
        case 31: result = 0; break;
        case 32: result = 0; break;
        case 33: result = 0; break;
        case 34: result = 0; break;
        case 35: result = 0; break;
        case 36: result = 0; break;
        case 37: result = 0; break;
        case 38: result = 0; break;
        case 39: result = 0; break;
        case 40: result = 0; break;
        case 41: result = 0; break;
        case 42: result = 0; break;
        case 43: result = 0; break;
        case 44: result = 0; break;
        case 45: result = 0; break;
        case 46: result = 0; break;
        case 47: result = 0; break;
        case 48: result = 0; break;
        case 49: result = 0; break;
        case 50: result = 0; break;
        case 51: result = 0; break;
        case 52: result = 0; break;
        case 53: result = 0; break;
        case 54: result = 0; break;
        case 55: result = 0; break;
        case 56: result = 0; break;
        case 57: result = 0; break;
        case 58: result = 0; break;
        case 59: result = 0; break;
        case 60: result = 0; break;
        case 61: result = 0; break;
        case 62: result = 0; break;
        case 63: result = 0; break;
        case 64: result = 0; break;
        case 65: result = 0; break;
        case 66: result = 0; break;
        case 67: result = 0; break;
        case 68: result = 0; break;
        case 69: result = 0; break;
        case 70: result = 0; break;
        case 71: result = 0; break;
        case 72: result = 0; break;
        case 73: result = 0; break;
        case 74: result = 0; break;
        case 75: result = 0; break;
        case 76: result = 0; break;
        case 77: result = 0; break;
        case 78: result = 0; break;
        case 79: result = 0; break;
        case 80: result = 0; break;
        case 81: result = 0; break;
        case 82: result = 0; break;
        case 83: result = 0; break;
        case 84: result = 0; break;
        case 85: result = 0; break;
        case 86: result = 0; break;
        case 87: result = 0; break;
        case 88: result = 0; break;
        case 89: result = 0; break;
        case 90: result = 0; break;
        case 91: result = 0; break;
        case 92: result = 0; break;
        case 93: result = 0; break;
        case 94: result = 0; break;
        case 256: result = 128; break;  //Iron
        case 257: result = 128; break;
        case 258: result = 128; break;
        case 259: result = 128; break;
        case 260: result = 1; break;
        case 261: result = 0; break;
        case 262: result = 0; break;
        case 263: result = 0; break;
        case 264: result = 0; break;
        case 265: result = 0; break;
        case 266: result = 0; break;
        case 267: result = 128; break;  //Iron
        case 268: result = 32; break;
        case 269: result = 32; break;
        case 270: result = 32; break;
        case 271: result = 32; break;
        case 272: result = 64; break;
        case 273: result = 64; break;
        case 274: result = 64; break;
        case 275: result = 64; break;
        case 276: result = 255; break;  //Diamond
        case 277: result = 255; break;
        case 278: result = 255; break;
        case 279: result = 255; break;
        case 280: result = 0; break;
        case 281: result = 0; break;
        case 282: result = 1; break;
        case 283: result = 128; break;
        case 284: result = 128; break;
        case 285: result = 128; break;
        case 286: result = 128; break;
        case 287: result = 0; break;
        case 288: result = 0; break;
        case 289: result = 0; break;
        case 290: result = 32; break;
        case 291: result = 64; break;
        case 292: result = 128; break;
        case 293: result = 255; break;
        case 294: result = 128; break;
        case 295: result = 0; break;
        case 296: result = 0; break;
        case 297: result = 1; break;
        case 298: result = 32; break;
        case 299: result = 32; break;
        case 300: result = 32; break;
        case 301: result = 32; break;
        case 302: result = 64; break;
        case 303: result = 64; break;
        case 304: result = 64; break;
        case 305: result = 64; break;
        case 306: result = 128; break;
        case 307: result = 128; break;
        case 308: result = 128; break;
        case 309: result = 128; break;
        case 310: result = 255; break;  //Diamond
        case 311: result = 255; break;
        case 312: result = 255; break;
        case 313: result = 255; break;
        case 314: result = 128; break;
        case 315: result = 128; break;
        case 316: result = 128; break;
        case 317: result = 128; break;
        case 318: result = 0; break;
        case 319: result = 0; break;
        case 320: result = 1; break;
        case 321: result = 0; break;
        case 322: result = 1; break;
        case 323: result = 0; break;
        case 324: result = 0; break;
        case 325: result = 0; break;
        case 326: result = 1; break;
        case 327: result = 0; break;
        case 328: result = 0; break;
        case 329: result = 0; break;
        case 330: result = 0; break;
        case 331: result = 0; break;
        case 332: result = 0; break;
        case 333: result = 0; break;
        case 334: result = 0; break;
        case 335: result = 1; break;
        case 336: result = 0; break;
        case 337: result = 0; break;
        case 338: result = 0; break;
        case 339: result = 0; break;
        case 340: result = 0; break;
        case 341: result = 0; break;
        case 342: result = 0; break;
        case 343: result = 0; break;
        case 344: result = 0; break;
        case 345: result = 0; break;
        case 346: result = 255; break;
        case 347: result = 0; break;
        case 348: result = 0; break;
        case 349: result = 0; break;
        case 350: result = 1; break;
        case 351: result = 0; break;    //Dye
        case 352: result = 0; break;
        case 353: result = 0; break;
        case 354: result = 6; break;
        case 355: result = 0; break;
        case 356: result = 0; break;
        case 2256: result = 0; break;
        case 2257: result = 0; break;

        default:
            result=0;
            break;
    }
    
    return result;
}



/* Item information from http://www.minecraftwiki.net/wiki/Data_Values
     === === ==========
     Dec Hex Block type
     === === ==========
	 0	 00	Air
	 1	 01	Stone
	 2	 02	Grass
	 3	 03	Dirt
	 4	 04	Cobblestone
	 5	 05	Wood
	 6	 06	Sapling D
	 7	 07	Bedrock
	 8	 08	Water D
	 9	 09	 Stationary water D
	 10	 0A	Lava D
	 11	 0B	 Stationary lava D
	 12	 0C	Sand
	 13	 0D	Gravel
	 14	 0E	Gold ore
	 15	 0F	Iron ore
	 16	 10	Coal ore
	 17	 11	Log
	 18	 12	Leaves
	 19	 13	Sponge
	 20	 14	Glass
	 21	 15	Lapis Ore
	 22	 16	Lapis Block
	 23	 17	Dispenser
	 24	 18	Sandstone
	 25	 19	Note Block
	 26	 1A	Aqua green Cloth
	 27	 1B	Cyan Cloth
	 28	 1C	Blue Cloth
	 29	 1D	Purple Cloth
	 30	 1E	Indigo Cloth
	 31	 1F	Violet Cloth
     === === ==========
     Dec Hex Block type
     === === ==========
	 32	 20	Magenta Cloth
	 33	 21	Pink Cloth
	 34	 22	Black Cloth
	 35	 23	Gray Cloth / Wool
	 36	 24	White Cloth
	 37	 25	 Yellow flower
	 38	 26	 Red rose
	 39	 27	 Brown Mushroom
	 40	 28	 Red Mushroom
	 41	 29	Gold Block
	 42	 2A	Iron Block
	 43	 2B	 Double Stone Slab
	 44	 2C	Stone Slab
	 45	 2D	Brick
	 46	 2E	TNT
	 47	 2F	Bookshelf
	 48	 30	Moss Stone
	 49	 31	Obsidian
	 50	 32	Torch D
	 51	 33	Fire
	 52	 34	Monster Spawner
	 53	 35	Wooden Stairs D
	 54	 36	Chest
	 55	 37	Redstone Wire D I
	 56	 38	Diamond Ore
	 57	 39	Diamond Block
	 58	 3A	Workbench
	 59	 3B	Crops D
	 60	 3C	Soil D
	 61	 3D	Furnace D
	 62	 3E	Burning Furnace D
	 63	 3F	Sign Post D I
     === === ==========
     Dec Hex Block type
     === === ==========
	 64	 40	Wooden Door D I
	 65	 41	Ladder D
	 66	 42	Minecart Rails D
	 67	 43	Cobblestone Stairs D
	 68	 44	Wall Sign D I
	 69	 45	Lever D
	 70	 46	Stone Pressure Plate D
	 71	 47	Iron Door D I
	 72	 48	Wooden Pressure Plate D
	 73	 49	Redstone Ore
	 74	 4A	Glowing Redstone Ore
	 75	 4B	Redstone torch ("off" state) D
	 76	 4C	Redstone torch ("on" state) D
	 77	 4D	Stone Button D
	 78	 4E	Snow
	 79	 4F	Ice
	 80	 50	Snow Block
	 81	 51	Cactus
	 82	 52	Clay
	 83	 53	Reed I
	 84	 54	Jukebox
	 85	 55	Fence
	 86	 56	Pumpkin D
	 87	 57	Netherrack
	 88	 58	Soul Sand
	 89	 59	Glowstone
	 90	 5A	Portal
	 91	 5B	Jack-O-Lantern D
	 92  5C Cake
*/
