/*
  libmc--c main
  Test program for mc--c library

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


//STL
#include <iostream>
using std::cout;
using std::hex;
using std::dec;
using std::endl;
using std::string;

//mc--
#include <mc--/Mobiles.hpp>
#include "UserInterface.hpp"
using mc__::World;
using mc__::Player;
using mc__::Events;
using mc__::Mobiles;

using mc__::SLOT_EQ_MAX;
using mc__::player_inv_slots;

//SFML
#include <SFML/System/Clock.hpp>

//
// Functions
//

void debugKey(const World& world, int32_t X, int8_t Y, int32_t Z)
{
    cout << (int)X << "," << (short)Y << "," << (int)Z << "0x" << hex
    << world.getKey(X, Z) << endl << dec;
}

//
//Create game chunk(s) for test world
//
void genWorld(World& world)
{
    
    //Create a square mile of solid terrain
    int32_t gen_X, gen_Z;
    for (gen_X = -160; gen_X <= 160; gen_X += 16) {
    for (gen_Z = -160; gen_Z <= 160; gen_Z += 16) {
        //Generate 16x16 terrain at X,0,Z, height of 64
        if (!world.genFlatGrass(gen_X, 0, gen_Z, 64)) {
            cout << "Error: genFlatGrass ";
            debugKey(world, 0,0,16);
        }
    }}
    
    //Create empty void
    world.genClosedVoid(64, 64);

    //Draw all block IDs
    if (!world.genChunkTest(0, 64, 0)) {
        cout << "Error: genChunkTest ";
        debugKey(world, 0,64,0);
    }
    if (!world.genChunkTest(0, 65, 31)) {
        cout << "Error: genChunkTest ";
        debugKey(world, 0,65,31);
    }
    
    //Grow some trees
    world.genTree(11, 64, 5);
    world.genTree(4, 64, 7, 3, 10, 3);

    //Other tree types (metadata 1, 2)
    world.genTree(4, 64, 19, 5, 12, 5, 1);
    world.genTree(12, 64, 19, 5, 8, 5, 2);


    //Glass pillar
    if (!world.genWall(24, 64, 2, 2, 17, 2, 20)) {
        cout << "Error: genWall ";
        debugKey(world, 3, 64, 6);
    }
    //Brick pillar next to glass
    if (!world.genWall(26, 64, 2, 1, 17, 2, 45)) {
        cout << "Error: genWall ";
        debugKey(world, 3, 64, 6);
    }
    //Single wallsign embedded in brick wall
    mc__::Block sign1 = {68, 0, 0, 0}; //wallsign, no metadata
    mc__::Chunk *chunk1 = new mc__::Chunk(0, 0, 0, 26, 65, 3);//location
    chunk1->block_array[0] = sign1;
    
    //Add the chunk with the wallsign
    world.addMapChunk(chunk1);
    delete chunk1;

    //Draw dyed wool    
    mc__::Block dyed_wool = {35, 0, 0, 0}; //wool, 0 metadata
    chunk1 = new mc__::Chunk(15, 0, 0, -16, 65, 0);
    for (gen_X = 0; gen_X < 16; gen_X++) {
        chunk1->block_array[gen_X] = dyed_wool;
        dyed_wool.metadata++;
    }
    world.addMapChunk(chunk1);
    delete chunk1;

    
    //Single workbench made from block put in chunk
    mc__::Block workbench1 = {58, 0, 0, 0};//workbench, no metadata
    chunk1 = new mc__::Chunk(0, 0, 0, 7, 64, 13);    //1x1x1, location
    chunk1->block_array[0] = workbench1;
    
    //Add the chunk with the workbench
    world.addMapChunk(chunk1);
    delete chunk1;
    
    //Table with cakes
    mc__::Block cake1 = { 92, 0, 0, 0};
    chunk1 = new mc__::Chunk(10, 1, 2, 17, 64, 17);    //1x1x1, location
    for (gen_X = 0; gen_X <= 10; gen_X++) {
        for (gen_Z = 0; gen_Z <= 2; gen_Z++) {
            //Y=0 is table
            chunk1->block_array[ gen_X*3*2 + gen_Z*2] = workbench1;
            
            //Place cake at intervals on table
            if (gen_Z == 1 && (gen_X & 1) == 0) {
                chunk1->block_array[ gen_X*3*2 + gen_Z*2 + 1] = cake1;
                cake1.metadata += 1;    //Ate a little cake...
            }
        }
    }
    //Add the chunk with cake
    world.addMapChunk(chunk1);
    delete chunk1;
    
    //Add crops with metadata 0-7
    mc__::Block crops = {59, 0, 0, 0};
    chunk1 = new mc__::Chunk(7, 0, 0, 4, 64, 18);   //Row of 8 crops
    for (gen_X = 0; gen_X < 8; gen_X++) {
        //Increase metadata for each crop
        crops.metadata = gen_X;
        chunk1->block_array[ gen_X ] = crops;
    }
    //Add the chunk with crops
    world.addMapChunk(chunk1);
    delete chunk1;
    
    //
    //Facecube test: dispenser, furnace, litfurnace, pumpkin, jack-o-lantern
    //
    chunk1 = new mc__::Chunk(13, 0, 1, 1, 64, 21);   //5 groups of 4 face items

    //Dispenser
    mc__::Block faceCube = {23, 4, 0, 0};
    faceCube.metadata = 4; chunk1->block_array[0] = faceCube;
    faceCube.metadata = 3; chunk1->block_array[1] = faceCube;
    faceCube.metadata = 2; chunk1->block_array[2] = faceCube;
    faceCube.metadata = 5; chunk1->block_array[3] = faceCube;
    
    //Furnace
    faceCube.blockID = 61;
    faceCube.metadata = 4; chunk1->block_array[6] = faceCube;
    faceCube.metadata = 3; chunk1->block_array[7] = faceCube;
    faceCube.metadata = 2; chunk1->block_array[8] = faceCube;
    faceCube.metadata = 5; chunk1->block_array[9] = faceCube;
    
    //Lit furnace
    faceCube.blockID = 62;
    faceCube.metadata = 4; chunk1->block_array[12] = faceCube;
    faceCube.metadata = 3; chunk1->block_array[13] = faceCube;
    faceCube.metadata = 2; chunk1->block_array[14] = faceCube;
    faceCube.metadata = 5; chunk1->block_array[15] = faceCube;
    
    //Pumpkin
    faceCube.blockID = 86;
    faceCube.metadata = 3; chunk1->block_array[18] = faceCube;
    faceCube.metadata = 2; chunk1->block_array[19] = faceCube;
    faceCube.metadata = 0; chunk1->block_array[20] = faceCube;
    faceCube.metadata = 1; chunk1->block_array[21] = faceCube;
    
    //Jack-o-Lantern
    faceCube.blockID = 91;
    faceCube.metadata = 3; chunk1->block_array[24] = faceCube;
    faceCube.metadata = 2; chunk1->block_array[25] = faceCube;
    faceCube.metadata = 0; chunk1->block_array[26] = faceCube;
    faceCube.metadata = 1; chunk1->block_array[27] = faceCube;

    world.addMapChunk(chunk1);    
    delete chunk1;
    
    //
    //  Redstone wire test
    //
    chunk1 = new mc__::Chunk(5, 0, 2, -8, 64, 21);   //3x6 wire chunk

    //Wire
    mc__::Block wireBlock = {55, 0, 0, 0};
    mc__::Block wireTorch = {76, 5, 0, 0};
    //Square of 9 wires
    for (size_t i = 0; i < 9; i++) {
        wireBlock.metadata = i; chunk1->block_array[i] = wireBlock;
    }
    //3 wires around a redstone torch
    wireBlock.metadata = 12; chunk1->block_array[12] = wireBlock;
    wireTorch.metadata = 5;  chunk1->block_array[13] = wireTorch;
    wireBlock.metadata = 14; chunk1->block_array[14] = wireBlock;
    wireBlock.metadata = 15; chunk1->block_array[16] = wireBlock;
    //Add the chunk
    world.addMapChunk(chunk1);    
    delete chunk1;

    //
    //  Raised redstone and floorplate test
    //
    chunk1 = new mc__::Chunk(3, 1, 3, -8, 65, 9);   //4x2x4 chunk
    mc__::Block glassBlock = { 20, 0, 0, 0};
    mc__::Block woodPlate = { 72, 0, 0, 0};
    mc__::Block stonePlate = { 70, 0, 0, 0};
    wireBlock.metadata = 0; 
    //Surround redstone with glass
    chunk1->block_array[2]  = glassBlock;
    chunk1->block_array[4]  = glassBlock;
    chunk1->block_array[8] = glassBlock;
    chunk1->block_array[10] = wireBlock;
    chunk1->block_array[12] = wireBlock;
    chunk1->block_array[14] = glassBlock;
    chunk1->block_array[16] = glassBlock;
    chunk1->block_array[18] = wireBlock;
    chunk1->block_array[20] = wireBlock;
    chunk1->block_array[22] = glassBlock;
    chunk1->block_array[26]  = glassBlock;
    chunk1->block_array[28]  = glassBlock;
    //Connect to plates on row above
    chunk1->block_array[3] = stonePlate; stonePlate.metadata = 1;
    chunk1->block_array[15] = woodPlate; woodPlate.metadata = 1;
    chunk1->block_array[17] = woodPlate;
    chunk1->block_array[29] = stonePlate;
    //Add the chunk
    world.addMapChunk(chunk1);
    delete chunk1;
    
    //
    //Move the world in these directions at start
    //
    world.spawn_X = -12;
    world.spawn_Y = 64;
    world.spawn_Z = 16;

    //Mark every chunk as updated
    world.redraw();
}

//Add some test objects to mobile items/entities
void addItems( mc__::Mobiles& mobiles, const mc__::World& world) {
    //Add some glass cubes for pickup, next to world spawn, 1/8 yaw
    mobiles.addItem( mobiles.newEID(), 20, 8,
        (world.spawn_X - 1) << 5,
        (world.spawn_Y ) << 5,
        (world.spawn_Z - 2) << 5, 0x20);

    //Add some brick cubes for pickup, next to world spawn, 1/6 yaw
    mobiles.addItem( mobiles.newEID(), 45, 8,
        (world.spawn_X - 2) << 5,
        (world.spawn_Y ) << 5,
        (world.spawn_Z - 3) << 5, 0x2B);

    //Add 1 diamond pick 
    mobiles.addItem( mobiles.newEID(), 278, 1,
        (world.spawn_X ) << 5,
        (world.spawn_Y ) << 5,
        (world.spawn_Z - 3) << 5, 0x2B);
    
    //Add ALL the items
    for (uint16_t i = 256; i < 355; i++) {
        mobiles.addItem( mobiles.newEID(), i, 1,
            (world.spawn_X - 256 + i ) << 5,
            (world.spawn_Y ) << 5,
            (world.spawn_Z - 3) << 5, 0x2B);
    }
        
}

//Give some items to player
void genInventory( mc__::Player& player)
{
    uint8_t slot;
    for (slot = SLOT_EQ_MAX; slot < player_inv_slots; slot++)
    {
        //Give a variety of item stacks
        player.setSlotItem(slot, (uint16_t)slot, 64, 0);
    }

}

int main(int argc, char** argv)
{
    uint32_t max_frames=0;
    bool run_limit=false;
  
    //Command line option
    switch (argc) {
        case 2:
            run_limit = true;
            max_frames = (uint32_t)(atoi( argv[1]));
            break;
        default:
            break;
    }
    
    //Default player name
    string player_name("axus");

    //Game world
    World world;

    //Game events
    Events events;

    //Generate a test world
    cout << "Generating test world..." << endl;
    genWorld(world);

    //Track entities with Mobiles object
    Mobiles mobiles(world);

    //Create some test items
    addItems(mobiles, world);
    
    //Choose entity ID for player
    uint32_t playerEID = mobiles.newEID();
    
    //Add a player at the spawn point, and make a reference to it
    Player& player = *(mobiles.addPlayer(playerEID, player_name,
        world.spawn_X << 5, world.spawn_Y << 5, world.spawn_Z << 5,
        -64/*yaw*/, 0/*pitch*/));

    //Give the player some items
    genInventory(player);

    //Create user interface to world
    cout << "Creating user interface..." << endl;
    mc__::UserInterface ui("libmc--c example",
        world, mobiles, player, events);
        
    //Run until user exits
    while (ui.run()) {
        sf::Sleep(0.010);   //100FPS - draw time
    }
    
    //Send disconnect message
    cout << "Exiting..." << endl;
    
    //Sleep 5 seconds if run to max frames
    if (run_limit) {
        sf::Sleep(5);
    }
    
    //Exit with no error code
    return 0;
}
