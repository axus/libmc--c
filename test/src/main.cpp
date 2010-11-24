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

/*
//SFML
#include <SFML/Graphics.hpp>

//STL
#include <iostream>
#include <string>
#include <iomanip>
#include <fstream>
#include <vector>

using std::cout;
using std::cerr;
using std::endl;
using std::string;
using std::hex;
using std::dec;
using std::setw;
using std::setfill;
using std::ofstream;
using std::ios;
using std::vector;
using std::map;
*/

//
// Global variables
//

/*
//Input state variables
bool mouse_press[sf::Mouse::ButtonCount];
int mouse_press_X[sf::Mouse::ButtonCount];
int mouse_press_Y[sf::Mouse::ButtonCount];
int mouse_X, mouse_Y;
*/

//Game world and viewer
mc__::World world;

/*
mc__::Viewer viewer;
*/

//
// Functions
//


int main()
{   
    //Load game chunk(s)
    world.genFlatGrass(-8, -2, -8); //512 blocks at (-8,-2,-8)
    world.genChunkTest(-8, 0, -8);  //208 blocks at (-8, 0, -8)
    
    //Grow some trees
    world.genTree(5, 0, 5);    //Normal tree
    world.genTree(-2, 0, 7, 3, 10, 3); //skinny tall tree
    
    //Load chunks behind the camera
    world.genFlatGrass(-8, -2, 9);
    world.genChunkTest(-8, 1, 24);

    //Move the world in these directions at start
    world.spawn_X = 0;
    world.spawn_Y = -16;
    world.spawn_Z = -320;

    //SFML variables
    //sf::Clock Clock;

//    sf::WindowSettings Settings;
/*
    Settings.DepthBits = 24;
    Settings.StencilBits = 0;
    Settings.AntialiasingLevel = 0;

    //Create the program window, with settings
    sf::RenderWindow App(sf::VideoMode(800, 600, 32),
        "SFML OpenGL", sf::Style::Close, Settings);

    //Initialize the viewer
    viewer.init(texture_map_filename);

    //Move the camera to start
    viewer.move(start_X, start_Y, start_Z);

    //Initial drawing
    App.SetActive();
    viewer.drawWorld(world);
    App.Display();
    sf::Sleep(0.01f);

    //Event loop    
    sf::Event Event;
    bool Running = true;
    bool something_happened;
*/

    //Create user interface to world
    mc__::UserInterface ui("libmc--c example", world);
    
    //Run until user exits
    while (ui.run()) {
        //Sleep some to decrease CPU usage
        sf::Sleep(0.01f);
    }
/*
    while (Running && App.IsOpened()) {
      
        //Set window
        App.SetActive();

        //Check events  
        something_happened=false;
        while (App.GetEvent(Event))
        {
            if (!handleSfEvent(Event)) {
                //Stop running when Esc is pressed
                Running = false;
            }
            something_happened=true;
        }
        
        //Redraw the world       
        viewer.drawWorld(world);
        
        //Update the window
        App.Display();

        //Sleep some to decrease CPU usage
        sf::Sleep(0.01f);
    }
*/
    return 0;
}