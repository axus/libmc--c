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

//mc--
#include "UserInterface.hpp"
using mc__::World;

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
    if (!world.genFlatGrass(-8, -2, -8)) {
        cout << "Error: genFlatGrass ";
        debugKey(world, -8,-2,-8);
    }

    //208 block wall at (-8, 0, -8)
    if (!world.genChunkTest(-8, 0, -8)) {
        cout << "Error: genChunkTest ";
        debugKey(world, -8,0,-8);
    }
    
    //Grow some trees
    if (!world.genTree(5, 0, 5)) {
        cout << "Error: genTree ";
        debugKey(world, 3, 0, 3);
    }
    if (!world.genTree(-2, 0, 7, 3, 10, 3)) {
        cout << "Error: genTree ";
        debugKey(world, -3, 0, 6);
    }
    
    //Create chunks behind the spawn
    if (!world.genFlatGrass(-8, -2, 9)) {
        cout << "Error: genFlatGrass ";
        debugKey(world, -8,-2,9);
    }
    if (!world.genChunkTest(-8, 1, 24)) {
        cout << "Error: genChunkTest ";
        debugKey(world, -8,1,24);
    }

    //Move the world in these directions at start
    world.spawn_X = 0;
    world.spawn_Y = -16;
    world.spawn_Z = -320;

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

    //Game world
    World world;
    genWorld(world);

    //Create user interface to world
    mc__::UserInterface ui("libmc--c example", world);
    
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