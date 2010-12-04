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

//Constructor
UserInterface::UserInterface(
        const string& name, World& w, Player& p, Events& ev, bool dbg):

    //initialize objects here
    texture_map_filename("terrain.png"),    //block textures
    mouseSensitivity(2.8),                  //Mouselook factor, higher is slower
    Settings(32, 0, 0),                     //0 stencil, 0 anti-aliasing
    App(sf::VideoMode(800, 600, 32),        //800x600, 32-bit color window
        name, sf::Style::Close, Settings),
    world(w), player(p), events(ev), debugging(dbg),
    mouselooking(false), toggle_mouselook(false), //Start with mouselook off
    center_X(800/2), center_Y(600/2),       //Center in middle of window
    keys_typed(0)                           //Empty keypress buffer
{

    //Enable vsync
    App.UseVerticalSync(true);

    //Start with no mouse buttons pressed and click position centered
    int i;
    for (i = sf::Mouse::Left; i < sf::Mouse::ButtonCount; i++) {
        mouse_press[i]=false;
        mouse_press_X[i]=0;
        mouse_press_Y[i]=0;
    }
    
    //Start with no keys held
    for (i = 0; i < sf::Key::Count; i++) {
        key_held[i]=false;
    }

    //Load terrain.png
    viewer.init(texture_map_filename);

    //Reset camera
    resetCamera();

    //Reset cursor to center
    App.SetCursorPosition( center_X, center_Y);
    last_X = center_X;
    last_Y = center_Y;
    
    //Turn off key repeat
    App.EnableKeyRepeat(false);

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

    //Handle events set by game
    Running = actions();

    //Set window
    App.SetActive();

    //Check SFML events  
    while (App.GetEvent(lastEvent))
    {
        if (!handleSfEvent(lastEvent)) {
            //Stop running when Esc is pressed
            Running = false;
        }
    }
    
    //Handle mouselook toggle
    if (toggle_mouselook) {
        mouselooking=!mouselooking;
        App.ShowMouseCursor(!mouselooking);
        if (mouselooking) {
            App.SetCursorPosition( center_X, center_Y);
            last_X = mouse_X = center_X;
            last_Y = mouse_Y = center_Y;
        }
        toggle_mouselook=false;
    }
    
    //Handle keyboard state
    if (!handleKeys()) { Running = false; }

    //Handle mouse position changes
    handleMouse();

    //Redraw the world       
    viewer.drawWorld(world);
    
    //Update the window
    App.Display();

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
    //Reset viewer to player position
    viewer.reset(16*player.abs_X, 16*player.eyes_Y, 16*player.abs_Z,
        player.yaw, player.pitch);
    
    //Reset mouse position
    last_X = mouse_X = center_X;
    last_Y = mouse_Y = center_Y;

    
    //Status message
    cout << "Moved camera to player @ " << (int)player.abs_X << ","
        << (int)player.abs_Y << "," << (int)player.abs_Z << "("
        << (int)player.eyes_Y << ")" << endl;

}

//Process user input
bool UserInterface::handleSfEvent( const sf::Event& Event )
{
    bool result=true;
    
    //Input state variables?
    
    switch( Event.Type) {
      
        //Window resize
        case sf::Event::Resized:
            viewer.viewport(0,0, Event.Size.Width, Event.Size.Height);
            break;
        
        //Key pressed
        case sf::Event::KeyPressed:
        
            //If key was pressed for the first time
            if (key_held[Event.Key.Code] == false) {
                
                //Add to keypress buffer
                key_buffer[keys_typed++] = Event.Key.Code;
            }
            key_held[Event.Key.Code] = true;
            break;
            
        //Key released
        case sf::Event::KeyReleased:
            key_held[Event.Key.Code] = false;
            
            //DEBUG
            if (Event.Key.Code == sf::Key::Quote) {
                App.SetCursorPosition( center_X, center_Y);
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
            
            //Handle button pressed
            switch( Event.MouseButton.Button ) {
                case sf::Mouse::Left:
                    //cerr << "Left-click @ " <<mouse_X<<","<<mouse_Y<< endl;
                    break;
                case sf::Mouse::Middle:
                    resetCamera();
                    break;
                case sf::Mouse::Right:
                    //Toggle mouselook
                    toggle_mouselook=true;
                    //cerr << "Right-click @ " <<mouse_X<<","<<mouse_Y<< endl;
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

            //Update virtual mouse pointer, depending on mouselook
            if (mouselooking) {
                mouse_X += (Event.MouseMove.X - center_X);
                mouse_Y += (Event.MouseMove.Y - center_Y);
                /*
                cerr << "X=" << Event.MouseMove.X << "Y=" << Event.MouseMove.Y
                    << ", mouseX=" << mouse_X << ", mouseY=" << mouse_Y
                    << ", lastX=" << last_X << ", lastY=" << last_Y << endl;
                */
            } else {
                mouse_X = Event.MouseMove.X;
                mouse_Y = Event.MouseMove.Y;
            }
            
            break;
        //Unhandled events
        default:
            break;
    }
    
    return result;
}

//Change viewer based on total mouse movements
bool UserInterface::handleMouse()
{
    //Translate camera if moved while holding left button
    if (mouse_press[sf::Mouse::Left]) {

        //Step camera to side for mouse-X motion
        int diff_X = mouse_X - mouse_press_X[sf::Mouse::Left];
        
        if (diff_X != 0) {
            viewer.move(diff_X , 0, 0);
            
            //Save new mouse position
            mouse_press_X[sf::Mouse::Left] = mouse_X;
        }

        //Step camera up for mouse-Y motion
        int diff_Y = mouse_press_Y[sf::Mouse::Left] - mouse_Y;
        if (diff_Y != 0) {
            viewer.move(0, diff_Y ,0);
            
            //Save new mouse position
            mouse_press_Y[sf::Mouse::Left] = mouse_Y;
        }

    }
    else if (mouselooking)
    {    //Don't mouselook if left mouse button is held

        //Use change in X position to rotate about Y-axis
        int diff_X = mouse_X - last_X;

        //Turn left/right based on mouse sensitivity
        if (diff_X != 0) {
            viewer.turn( (float)diff_X/mouseSensitivity );
        }

        //Tilt up/down based on mouse sensitivity
        int diff_Y = mouse_Y - last_Y;
        if (diff_Y != 0) {
            viewer.tilt( (float)diff_Y/mouseSensitivity );
        }
        
    }
    
    //Trap real mouse pointer in center of window if mouselooking
    if (mouselooking) {
        App.SetCursorPosition( center_X, center_Y);
        
        //Remember last mouse position for mouselooking
        last_X = mouse_X;
        last_Y = mouse_Y;
    }

    return true;    //no way to exit game via mouse ;)
}

//After reading in all events, handle keypresses separately
bool UserInterface::handleKeys()
{
    bool result=true;
    size_t index;
    
    //Init movement
    bool movement[MOVE_COUNT];
    for (index = 0; index < MOVE_COUNT; index++)
    {
        movement[index] = false;
    }
    
    //Handle single keypresses in order
    if (keys_typed >= 1024) { keys_typed = 1024; }
    for( index = 0; index < keys_typed; index++) {
      
        //Game in "PLAYING" state... Do something else for text input
        switch ( key_buffer[index]) {
            //Quit
            case sf::Key::Escape:
                result = false;
                break;
            //Move Up
            case sf::Key::PageUp:
            case sf::Key::Space:
                movement[MOVE_UP] = true;
                break;
            //Move Down
            case sf::Key::Home:
            case sf::Key::X:
                movement[MOVE_DOWN] = true;
                break;
            //Move left
            case sf::Key::Left:
            case sf::Key::A:
                movement[MOVE_LEFT] = true;
                break;
            //Move right
            case sf::Key::Right:
            case sf::Key::D:
                movement[MOVE_RIGHT] = true;
                break;
            //Zoom in
            case sf::Key::W:
            case sf::Key::Up:
                movement[MOVE_FORWARD] = true;
                break;
            //Zoom out
            case sf::Key::Down:
            case sf::Key::S:
                movement[MOVE_BACK] = true;
                break;                        
            //Turn left
            case sf::Key::Q:
            case sf::Key::End:
                movement[TURN_LEFT] = true;
                break;
            //Turn right
            case sf::Key::E:
            case sf::Key::PageDown:
                movement[TURN_RIGHT] = true;
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
    }
    keys_typed=0;   //Reset key buffer
    
    //Game actions in response to held keys
    //Moving up
    if (key_held[sf::Key::PageUp] || key_held[sf::Key::Space]) {
        movement[MOVE_UP] = true;
    }

    //Moving down
    if (key_held[sf::Key::Home] || key_held[sf::Key::X]) {
        movement[MOVE_DOWN] = true;
    }
    
    //Moving left
    if (key_held[sf::Key::Left] || key_held[sf::Key::A]) {
        movement[MOVE_LEFT] = true;
    }

    //Moving right
    if (key_held[sf::Key::Right] || key_held[sf::Key::D]) {
        movement[MOVE_RIGHT] = true;
    }

    //Moving forward
    if (key_held[sf::Key::W] || key_held[sf::Key::Up]) {
        movement[MOVE_FORWARD] = true;
    }
    
    //Moving back
    if (key_held[sf::Key::S] || key_held[sf::Key::Down]) {
        movement[MOVE_BACK] = true;
    }

    //Turning left
    if (key_held[sf::Key::Q] || key_held[sf::Key::End]) {
        movement[TURN_LEFT] = true;
    }
    
    //Turning right
    if (key_held[sf::Key::E] || key_held[sf::Key::PageDown]) {
        movement[TURN_RIGHT] = true;
    }
    
    //Change red color in tree leaves
    if (key_held[sf::Key::R]) {
        viewer.leaf_color[0] += 2;
    }
    //Change green color in tree leaves
    if (key_held[sf::Key::G]) {
        viewer.leaf_color[1] += 2;
    }
    //Change blue color in tree leaves
    if (key_held[sf::Key::B]) {
        viewer.leaf_color[2] += 2;
    }

    //Finally, move the viewer based on movement options
    if (movement[MOVE_BACK]) {
        viewer.move(0, 0, -4);
    }
    if (movement[MOVE_FORWARD]) {
        viewer.move(0, 0, 4);
    }
    if (movement[MOVE_LEFT]) {
        viewer.move(-4, 0, 0);
    }
    if (movement[MOVE_RIGHT]) {
        viewer.move(4, 0, 0);
    }
    if (movement[MOVE_UP]) {
        viewer.move(0, 4, 0);
    }
    if (movement[MOVE_DOWN]) {
        viewer.move(0, -4, 0);
    }
    if (movement[TURN_LEFT]) {
        viewer.turn(-5);
    }
    if (movement[TURN_RIGHT]) {
        viewer.turn(5);
    }
    
    return result;
}