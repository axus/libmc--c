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
//Create game chunk(s)
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
    if (!world.genTree(11, 64, 5)) {
        cout << "Error: genTree ";
        debugKey(world, 9, 64, 3);
    }
    if (!world.genTree(4, 64, 7, 3, 10, 3)) {
        cout << "Error: genTree ";
        debugKey(world, 3, 64, 6);
    }

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
    
    //Add the single chunk to 0,0 MapChunk
    world.addMapChunk(chunk1);
    delete chunk1;
    
    //Single workbench made from block put in chunk
    mc__::Block workbench1 = {58, 0, 0, 0};//workbench, no metadata
    chunk1 = new mc__::Chunk(0, 0, 0, 8, 64, 8);    //location
    chunk1->block_array[0] = workbench1;
    
    //Add the single chunk to 0,0 MapChunk
    world.addMapChunk(chunk1);
    delete chunk1;
    
    //Move the world in these directions at start
    world.spawn_X = 8;
    world.spawn_Y = 64;
    world.spawn_Z = 16;

    //Mark every chunk as updated
    world.redraw();
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

    //Add some glass cubes for pickup, next to world spawn, 1/8 yaw
    mobiles.addItem( mobiles.newEID(), 20, 8,
        (world.spawn_X - 1) << 5,
        (world.spawn_Y ) << 5,
        (world.spawn_Z - 1) << 5, 0x20);

    //Add some brick cubes for pickup, next to world spawn, 1/6 yaw
    mobiles.addItem( mobiles.newEID(), 45, 8,
        (world.spawn_X - 2) << 5,
        (world.spawn_Y ) << 5,
        (world.spawn_Z - 2) << 5, 0x2B);

    //Choose entity ID for player
    uint32_t playerEID = mobiles.newEID();
    
    //Add a player at the spawn point, and make a reference to it
    Player& player = *(mobiles.addPlayer(playerEID, player_name,
        world.spawn_X << 5, world.spawn_Y << 5, world.spawn_Z << 5,
        128, 0));

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
