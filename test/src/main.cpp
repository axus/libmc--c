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
#include "UserInterface.hpp"
using mc__::World;
using mc__::Player;
using mc__::Events;

//
// Functions
//

void debugKey(const World& world, int32_t X, int8_t Y, int32_t Z)
{
    cout << (int)X << "," << (short)Y << "," << (int)Z << "0x" << hex
    << world.getKey(X, Z) << endl << dec;
}

void genWorld(World& world)
{
    //
    //Create game chunk(s)
    //
    
     //512 block floor at (-8,-2,-8)
    if (!world.genFlatGrass(-8, 0, -8)) {
        cout << "Error: genFlatGrass ";
        debugKey(world, -8,0,-8);
    }

    //208 block wall at (-8, 0, -8)
    if (!world.genChunkTest(-8,2, -8)) {
        cout << "Error: genChunkTest ";
        debugKey(world, -8,2,-8);
    }
    
    //Grow some trees
    if (!world.genTree(5, 2, 5)) {
        cout << "Error: genTree ";
        debugKey(world, 3, 2, 3);
    }
    if (!world.genTree(-2, 2, 7, 3, 10, 3)) {
        cout << "Error: genTree ";
        debugKey(world, -3, 2, 6);
    }
    
    //Create chunks behind the spawn
    if (!world.genFlatGrass(-8, 0, 9)) {
        cout << "Error: genFlatGrass ";
        debugKey(world, -8,0,9);
    }
    if (!world.genChunkTest(-8, 3, 24)) {
        cout << "Error: genChunkTest ";
        debugKey(world, -8,3,24);
    }

    //Move the world in these directions at start
    world.spawn_X = 0;
    world.spawn_Y = 3;
    world.spawn_Z = 16;

}


int main(int argc, char** argv)
{
    uint32_t frames=0, max_frames=0;
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

    //Game player
    Player player( player_name);
    
    //Game events
    Events events;

    //Generate a test world
    genWorld(world);

    //Set player initial position and viewpoint
    player.setPosLook( world.spawn_X, world.spawn_Y, world.spawn_Z,
        world.spawn_Y + 1, 0, 0);
/*
    player.abs_X = world.spawn_X;
    player.abs_Y = world.spawn_Y;
    player.abs_Z = world.spawn_Z;
    
    //Set player viewpoint
    player.eyes_Y = player.abs_Y + 1;
*/

    //Create user interface to world
    mc__::UserInterface ui("libmc--c example", world, player, events);
        
    //Run until user exits
    while (ui.run() && (!run_limit || (frames < max_frames) )) {
        //Sleep some to decrease CPU usage in between UI updates
        sf::Sleep(0.01f);
        frames++;
    }
    
    //Sleep 5 seconds if run to max frames
    if (run_limit) {
        sf::Sleep(5);
    }
    
    //Exit with no error code
    return 0;
}