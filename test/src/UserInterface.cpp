/*
  libmc--c UserInterface
  User Interface for mc--c library test program

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


//Compiler specific options
#ifdef _MSC_VER
    #include "ms_stdint.h"
#else
    #include <stdint.h>
#endif

//Header
#include "UserInterface.hpp"

//Namespace classes
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

using mc__::World;
using mc__::Viewer;
using mc__::UserInterface;

//Constructor, initialize SFML and mc-- objects here
UserInterface::UserInterface(const string& name, mc__::World& w, bool dbg):
    texture_map_filename("terrain.png"),    //block textures
    Settings(32, 0, 0),           //32-bit color, 0 stencil, 0 anti-aliasing
    //800x600, 32-bit color
    App(sf::VideoMode(800, 600, 32), name, sf::Style::Close, Settings),
    world(w), debugging(dbg)
{

    //Start with no mouse buttons pressed
    mouse_press[sf::Mouse::Left]=false;
    mouse_press[sf::Mouse::Right]=false;
    mouse_press[sf::Mouse::Middle]=false;
    mouse_press[sf::Mouse::XButton1]=false;
    mouse_press[sf::Mouse::XButton2]=false;

    //Load terrain.png
    viewer.init(texture_map_filename);
    
    //Move viewer camera to starting position
    viewer.move(world.spawn_X, world.spawn_Y, world.spawn_Z);

    //Draw the world once
    App.SetActive();
    viewer.drawWorld(world);
    App.Display();
}

bool UserInterface::run()
{
    bool Running=true;
    bool something_happened;

    //Set window
    App.SetActive();

    //Check events  
    something_happened=false;
    while (App.GetEvent(lastEvent))
    {
        if (!handleSfEvent(lastEvent)) {
            //Stop running when Esc is pressed
            Running = false;
        }
        something_happened=true;
    }

    //Redraw if something happened?
    if (something_happened) {
      
        //Redraw the world       
        viewer.drawWorld(world);
        
        //Update the window
        App.Display();
    }

    return Running;
}

//Process user input
bool UserInterface::handleSfEvent( const sf::Event& Event )
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
                //Quit
                case sf::Key::Escape:
                    result = false;
                    break;
                //Move Up
                case sf::Key::PageUp:
                    viewer.move(0, -16, 0);
                    break;
                //Move Down
                case sf::Key::Home:        
                    viewer.move(0, 16, 0);
                    break;
                //Move left
                case sf::Key::Left:
                case sf::Key::A:
                    viewer.move(16, 0, 0);
                    break;
                //Move right
                case sf::Key::Right:
                case sf::Key::D:
                    viewer.move(-16, 0, 0);
                    break;
                //Zoom in
                case sf::Key::W:
                case sf::Key::Up:
                    viewer.move(0, 0, 16);
                    break;
                //Zoom out
                case sf::Key::Down:
                case sf::Key::S:
                    viewer.move(0, 0, -16);
                    break;                        
                //Turn left
                case sf::Key::Q:
                case sf::Key::End:
                    viewer.turn(-15, 0, 1, 0);
                    break;
                //Turn right
                case sf::Key::E:
                case sf::Key::PageDown:
                    viewer.turn(15, 0, 1, 0);
                    break;
                //Change red color in tree leaves
                case sf::Key::R:
                    viewer.leaf_color[0] += 16;
                    break;
                //Change green color in tree leaves
                case sf::Key::G:
                    viewer.leaf_color[1] += 16;
                    break;
                //Change blue color in tree leaves
                case sf::Key::B:
                    viewer.leaf_color[2] += 16;
                    break;
                //Debugging output
                case sf::Key::Tilde:
                {
                    //Get the tree at 3,0,3
                    mc__::Chunk *chunk = world.getChunk(3,0,3);
                    
                    //Copy binary chunk data to file
                    if (chunk != NULL) {
                        viewer.writeChunkBin( chunk, "chunk.bin");
                    } else {
                        cerr << "Chunk not found @ (3,0,3)" << endl;
                    }
                }
                    break;
                case sf::Key::BackSlash:
                    //Print chunk information to stdout
                    world.printChunks();
                    break;
                default:
                    break;
            }
            break;
            
        //Mousewheel scroll
        case sf::Event::MouseWheelMoved: {
            //Change camera position (in increments of 16).  Down zooms out.
            int move_Z = (Event.MouseWheel.Delta << 4);
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
                    viewer.reset();
                    viewer.move(world.spawn_X, world.spawn_Y, world.spawn_Z);
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
                int diff_X = (mouse_press_X[sf::Mouse::Left] - mouse_X)/16;
                
                if (diff_X != 0) {
                    viewer.move((diff_X << 4), 0, 0);
                    
                    //Save new mouse position
                    mouse_press_X[sf::Mouse::Left] = mouse_X;
                }

                //Move camera every 16 pixels on Y-axis
                int diff_Y = (mouse_Y - mouse_press_Y[sf::Mouse::Left])/16;
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
