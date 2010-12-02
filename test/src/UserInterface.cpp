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
using mc__::Player;
using mc__::UserInterface;

//Constructor, initialize SFML and mc-- objects here
UserInterface::UserInterface(
        const string& name, World& w, Player& p, Events& ev, bool dbg):
    texture_map_filename("terrain.png"),    //block textures
    Settings(32, 8, 0),           //32-bit color, 0 stencil, 0 anti-aliasing
    //800x600, 32-bit color
    App(sf::VideoMode(800, 600, 32), name, sf::Style::Close, Settings),
    world(w), player(p), events(ev), debugging(dbg),
    mouselooking(true), mouse_X(0), mouse_Y(0), center_X(800/2), center_Y(600/2)
{

    //Start with no mouse buttons pressed and click position centered
    mouse_press[sf::Mouse::Left]=false;
    mouse_press_X[sf::Mouse::Left]=0;
    mouse_press_Y[sf::Mouse::Left]=0;
    mouse_press[sf::Mouse::Right]=false;
    mouse_press_X[sf::Mouse::Right]=0;
    mouse_press_Y[sf::Mouse::Right]=0;
    mouse_press[sf::Mouse::Middle]=false;
    mouse_press_X[sf::Mouse::Middle]=0;
    mouse_press_Y[sf::Mouse::Middle]=0;
    mouse_press[sf::Mouse::XButton1]=false;
    mouse_press[sf::Mouse::XButton2]=false;

    //Load terrain.png
    viewer.init(texture_map_filename);

    //Reset camera
    resetCamera();

    //Hide the mouse cursor
    App.ShowMouseCursor(false);
    App.SetCursorPosition( center_X, center_Y);

    //Draw the world once
    App.SetActive();
    viewer.drawWorld(world);
    App.Display();
}

//Close UI
UserInterface::~UserInterface()
{
    App.ShowMouseCursor(true);
}

//Handle game and SFML events, draw game
bool UserInterface::run()
{
    bool Running=true;
    bool something_happened;

    //Handle events set by game
    Running = actions();

    //Set window
    App.SetActive();

    //Check SFML events  
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

//Handle mc__::Events
bool UserInterface::actions()
{
    bool running=true;
    
    bool chunk_received=false;
    
    //Process all events
    Events::Event_t event;
    while ( events.get(event) ) {
        switch( event.type ) {
            case Events::GAME_CHAT_MESSAGE:
                //TODO: draw chat message to screen
                break;
            case Events::GAME_PLAYER_POSLOOK:
                //Update view for current player position/direction
                resetCamera();
                break;
            case Events::GAME_MAPCHUNK:
                chunk_received=true;
                break;
            default:
                break;
        }
    }
    
    //Create some acknowledgement if chunk received?
    if (chunk_received) {
      
      /*
        //Allocate event data, remember to delete it after receiving it
        Events::dataLook *data = new Events::dataLook;
        
        data->yaw = player.yaw;
        data->pitch = player.pitch;
        data->animation = player.animation;

        events.put(Events::ACTION_LOOK, NULL);
      */
    }
  
    return running;
}

//Reset camera to player
void UserInterface::resetCamera()
{
    viewer.reset(16*player.abs_X, 16*player.eyes_Y, 16*player.abs_Z,
        player.yaw, player.pitch);
    
    cout << "Moved camera to player @ " << (int)player.abs_X << ","
        << (int)player.abs_Y << "," << (int)player.abs_Z << "("
        << (int)player.eyes_Y << ")" << endl;

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
                case sf::Key::Space:
                    viewer.move(0, 16, 0);
                    break;
                //Move Down
                case sf::Key::Home:
                case sf::Key::X:
                    viewer.move(0, -16, 0);
                    break;
                //Move left
                case sf::Key::Left:
                case sf::Key::A:
                    viewer.move(-16, 0, 0);
                    break;
                //Move right
                case sf::Key::Right:
                case sf::Key::D:
                    viewer.move(16, 0, 0);
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
                    viewer.turn(-15 );
                    //viewer.cam_yaw -= 15.0f;
                    break;
                //Turn right
                case sf::Key::E:
                case sf::Key::PageDown:
                    viewer.turn(15);
                    //viewer.cam_yaw += 15.0f;
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
                case sf::Key::Back:
                    resetCamera();
                    break;
                //Debugging output
                case sf::Key::Tilde:
                    viewer.saveChunks(world);
                    break;
                case sf::Key::BackSlash:
                    //Print chunk information to stdout
                    viewer.printChunks(world);
                    break;
                default:
                    break;
            }
            break;
            
        //Mousewheel scroll
        case sf::Event::MouseWheelMoved: {
            //Change camera position (in increments of 16).
            viewer.move(0,0,Event.MouseWheel.Delta << 4);   //Zoom forward
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
                    resetCamera();
                    break;
                case sf::Mouse::Right:
                    //Toggle mouselook
                    mouselooking=!mouselooking;
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

            //Update virtual mouse pointer
            mouse_X += (Event.MouseMove.X - center_X);
            mouse_Y += (Event.MouseMove.Y - center_Y);
            int diff_X;
            int diff_Y;

            //Translate camera if moved while holding left button
            if (mouse_press[sf::Mouse::Left]) {

                //Step camera to side for mouse-X motion
                diff_X = mouse_X - mouse_press_X[sf::Mouse::Left];
                
                if (diff_X != 0) {
                    viewer.move(diff_X , 0, 0);
                    
                    //Save new mouse position
                    mouse_press_X[sf::Mouse::Left] = mouse_X;
                }

                //Step camera up for mouse-Y motion
                diff_Y = mouse_press_Y[sf::Mouse::Left] - mouse_Y;
                if (diff_Y != 0) {
                    viewer.move(0, diff_Y ,0);
                    
                    //Save new mouse position
                    mouse_press_Y[sf::Mouse::Left] = mouse_Y;
                }

            }
            else if (mouselooking)
            {    //Don't mouselook if left mouse button is held

                //Use change in X position to rotate about Y-axis
                diff_X = mouse_X - mouse_press_X[sf::Mouse::Right];
                viewer.turn( diff_X/2.0 );

                //Use change in Y position to rotate about side-axis
                diff_Y = mouse_Y - mouse_press_Y[sf::Mouse::Right];
                viewer.tilt( diff_Y/2.0 );
            }
            
            //Remember last mouse position for mouselooking
            mouse_press_X[sf::Mouse::Right] = mouse_X;
            mouse_press_Y[sf::Mouse::Right] = mouse_Y;
            
            //Trap real mouse pointer
            App.SetCursorPosition( center_X, center_Y);

            break;
        //Unhandled events
        default:
            break;
    }
    
    return result;
}
