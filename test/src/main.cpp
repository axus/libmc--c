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

#include "UserInterface.hpp"

//
// Functions
//

int main()
{   
    //Game world
    mc__::World world;

    //Create game chunk(s)
    world.genFlatGrass(-8, -2, -8); //512 block floor at (-8,-2,-8)
    world.genChunkTest(-8, 0, -8);  //208 block wall at (-8, 0, -8)
    
    //Grow some trees
    world.genTree(5, 0, 5);    //Normal tree
    world.genTree(-2, 0, 7, 3, 10, 3); //skinny tall tree
    
    //Create chunks behind the spawn
    world.genFlatGrass(-8, -2, 9);
    world.genChunkTest(-8, 1, 24);

    //Move the world in these directions at start
    world.spawn_X = 0;
    world.spawn_Y = -16;
    world.spawn_Z = -320;

    //Create user interface to world
    mc__::UserInterface ui("libmc--c example", world);
    
    //Run until user exits
    while (ui.run()) {
        //Sleep some to decrease CPU usage in between UI updates
        sf::Sleep(0.01f);
    }
    
    //Exit with no error code
    return 0;
}