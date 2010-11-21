
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

//Compiler specific options
#ifdef _MSC_VER
    #include "ms_stdint.h"
#else
    #include <stdint.h>
#endif

//libmc--c classes
#include "World.h"
#include "Viewer.h"

//
// Global variables
//

const string texture_map_filename("terrain.png");

//Input state variables
bool mouse_press[sf::Mouse::ButtonCount];
int mouse_press_X[sf::Mouse::ButtonCount];
int mouse_press_Y[sf::Mouse::ButtonCount];
int mouse_X, mouse_Y;

mc__::World world;
mc__::Viewer viewer;

//
// Functions
//

//Process user input
bool handleSfEvent( const sf::Event& Event )
{
    bool result=true;
    
    switch( Event.Type) {
      
        //Window resize
        case sf::Event::Resized:
            viewer.viewport(0,0, Event.Size.Width, Event.Size.Height);
            break;
        
        //Key pressed
        case sf::Event::KeyPressed:
            switch ( Event.Key.Code) {
                case sf::Key::Escape:       //Quit
                    result = false;
                    break;
                case sf::Key::Up:           //Move Up
                    viewer.move(0, -16, 0);
                    break;
                case sf::Key::Down:         //Move Down
                    viewer.move(0, 16, 0);
                    break;
                case sf::Key::Left:         //Move left
                    viewer.move(16, 0, 0);
                    break;
                case sf::Key::Right:        //Move right
                    viewer.move(-16, 0, 0);
                    break;
                default:
                    break;
            }
            break;
            
        //Mousewheel scroll
        case sf::Event::MouseWheelMoved: {
            //Change camera position (in increments of 16).  Down zooms out.
            GLint move_Z = (Event.MouseWheel.Delta << 4);
            viewer.move(0, 0, move_Z);
            break;
        }
        
        //Mouse clicks
        case sf::Event::MouseButtonPressed:
        
            //Remember where this mouse button was pressed
            mouse_press[Event.MouseButton.Button] = true;
            mouse_press_X[Event.MouseButton.Button] = mouse_X;
            mouse_press_Y[Event.MouseButton.Button] = mouse_Y;
            
            
            //Reset camera if middle mouse button pressed
            switch( Event.MouseButton.Button ) {
                case sf::Mouse::Middle:
                    glLoadIdentity();
                    glTranslatef( viewer.camera_X, viewer.camera_Y, viewer.camera_Z);
                    break;
                default:
                    break;
            }
            break;
            
        //Mouse un-clicked
        case sf::Event::MouseButtonReleased:
        
            //Forget that this mouse button was pressed
            mouse_press[Event.MouseButton.Button] = false;
            break;
        
        //Mouse move
        case sf::Event::MouseMoved:

            //Get latest position from event
            mouse_X = Event.MouseMove.X;
            mouse_Y = Event.MouseMove.Y;

            //Translate camera if moved while holding left button
            if (mouse_press[sf::Mouse::Left]) {

                //Move camera every 16 pixels on X-axis
                GLint diff_X = (mouse_press_X[sf::Mouse::Left] - mouse_X)/16;
                
                if (diff_X != 0) {
                    viewer.move((diff_X << 4), 0, 0);
                    
                    //Save new mouse position
                    mouse_press_X[sf::Mouse::Left] = mouse_X;
                }

                //Move camera every 16 pixels on Y-axis
                GLint diff_Y = (mouse_Y - mouse_press_Y[sf::Mouse::Left])/16;
                if (diff_Y != 0) {
                    viewer.move(0, diff_Y << 4,0);
                    
                    //Save new mouse position
                    mouse_press_Y[sf::Mouse::Left] = mouse_Y;
                }
                
                /*//Move camera 1 pixel at a time on X-axis
                viewer.move(mouse_press_X[sf::Mouse::Left] - mouse_X, 0, 0);
                mouse_press_X[sf::Mouse::Left] = mouse_X;
                
                //Move camera one pixel at a time on Y-axis
                viewer.move(0, mouse_Y - mouse_press_Y[sf::Mouse::Left], 0);
                mouse_press_Y[sf::Mouse::Left] = mouse_Y;
                */
            }

            //Rotate camera if mouse moved while holding right button
            if (mouse_press[sf::Mouse::Right]) {
              
                //Use change in X position to rotate about Y-axis
                viewer.turn(mouse_press_X[sf::Mouse::Right] - mouse_X, 0, 1, 0);

                //Don't rotate about X-axis
                //viewer.turn(mouse_Y - mouse_press_Y[sf::Mouse::Right], 1, 0, 0);
                
                //Update mouse press position for right mouse button
                mouse_press_X[sf::Mouse::Right] = mouse_X;
                mouse_press_Y[sf::Mouse::Right] = mouse_Y;
            }
            break;
        //Unhandled events
        default:
            break;
    }
    
    return result;
}

int main()
{   
    //Load game chunk(s)
    world.genFlatGrass(-8, -2, -8); //512 blocks at (-8,-2,-8)
    world.genChunkTest(-8, 0, -8);  //208 blocks at (-8, 0, -8)
    
    //Grow some trees
    world.genTree(5, 0, 5);
    world.genTree(-2, 0, 7, 1); //skinny tall tree
    
    //Load chunks behind the camera
    world.genFlatGrass(-8, -2, 9);
    world.genChunkTest(-8, 1, 24);
    
    //SFML variables
    sf::Clock Clock;
    sf::WindowSettings Settings;
    Settings.DepthBits = 24;
    Settings.StencilBits = 0;
    Settings.AntialiasingLevel = 0;

    //Create the program window, with settings
    sf::RenderWindow App(sf::VideoMode(800, 600, 32),
        "SFML OpenGL", sf::Style::Close, Settings);

    //Initialize the viewer
    viewer.init(texture_map_filename);

    //Move the camera
    viewer.move(0, -16, -320);

    //Initial drawing
    App.SetActive();
    viewer.drawWorld(world);
    App.Display();
    sf::Sleep(0.01f);

    //Event loop    
    sf::Event Event;
    bool Running = true;
    bool something_happened;
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
    return 0;
}