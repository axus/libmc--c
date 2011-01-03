/*
  libmc--c UserInterface
  User Interface for mc--c library test program

  Copyright 2010 - 2011 axus

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

//UI settings
const unsigned short UI_width=860;      //Pixel width
const unsigned short UI_height=480;     //Pixel height
const unsigned char UI_bpp=32;          //Bits per pixel
const float UI_mouse_sensitivity=2.8;   //Mouselook sensitivity, high is slower

//Constructor
UserInterface::UserInterface(
        const string& name, World& w, Mobiles& m,
            Player& p, Events& ev, bool dbg):

    //initialize objects here
    texture_map_filename("terrain.png"),    //block textures
    item_icon_filename("items.png"),        //item icons
    mouseSensitivity(UI_mouse_sensitivity), //Mouselook sensitivity
    Settings(UI_bpp, 0, 0),                 //32bpp, 0 stencil, 0 anti-aliasing
    App(sf::VideoMode(UI_width, UI_height, UI_bpp), //860x480, 32-bit color
        name, sf::Style::Resize|sf::Style::Close, Settings),    //Resizable
    viewer(UI_width, UI_height),
    world(w), mobiles(m), player(p), events(ev), debugging(dbg),
    mouselooking(false), toggle_mouselook(false), //Start with mouselook off
    center_X(UI_width/2), center_Y(UI_height/2),  //Center in middle of window
    keys_typed(0),                                //Empty keypress buffer
    showStatus(true), frames_elapsed(0), totalFrameTime(0) //FPS
{

    //TODO: Init window settings from configuration file

    //Enable vsync
    //App.UseVerticalSync(true);

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

    //Set initial status message
    status_string.SetText("libmc--c test program");
    status_string.SetSize(20);
    status_string.Move(10.f, 10.f);

    //Load textures   //TODO: configurable
    viewer.init(texture_map_filename, item_icon_filename, true);

    //Reset camera
    resetCamera();

    //Reset cursor to center
    App.SetCursorPosition( center_X, center_Y);
    last_X = center_X;
    last_Y = center_Y;
    toggle_mouselook=true;
    
    //Turn off key repeat
    App.EnableKeyRepeat(false);

    // Do you want to live forever?
    //App.PreserveOpenGLStates(true);

    App.SetActive();

    //Draw the world once
    viewer.clear();
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
    bool Running=true, inputs=false;

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
        inputs=true;
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
    
    //Handle keyboard state (they might be pressed down)
    if (!handleKeys()) { Running = false; }

    //Handle mouse position changes (if there were inputs)
    if (inputs && handleMouse()) {;}

    //Clear the view
    viewer.clear();

    //Redraw the entities, items, etc.
    viewer.drawMobiles(mobiles);
    
    //Redraw the world (terrain)
    viewer.drawWorld(world);
    

    //2D overlay
    //Update status display
    if (showStatus) {
        setDebug();

        //Save attribute bits
        glPushAttrib( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        
        //Save MODEL_VIEW state
        glPushMatrix();
        
        //Save PROJECTION state
        glMatrixMode(GL_PROJECTION); glPushMatrix();
        glDisable(GL_DEPTH_TEST);

        //Draw status text to screen
        App.Draw(status_string);
        
        //Reload PROJECTION, MODEL_VIEW states
        glMatrixMode(GL_PROJECTION); glPopMatrix();
        glMatrixMode(GL_MODELVIEW); glPopMatrix();
        
        //Reload attribute bits
        glPopAttrib( );
        
        //Rebind texture
        viewer.rebindTerrain();
    }

    
    //Update the window
    App.Display();

    return Running;
}

//Draw FPS and other status to screen
void UserInterface::setDebug()
{
    static const float pixratio = texmap_TILE_LENGTH;
    
    //Track number of frames since last update
    frames_elapsed++;
    
    //Track longest frame draw time
    totalFrameTime += App.GetFrameTime();
    
    //Every 100 frames, update the status message
    if (frames_elapsed > 100) {
      
        //Update status string
        char buf[128];
        sprintf(buf, "%3u chunks  Camera @ %3.3f, %3.3f, %3.3f   FPS %3.3f",
            (unsigned int)viewer.glListMap.size(), viewer.cam_X/pixratio,
            viewer.cam_Y/pixratio, viewer.cam_Z/pixratio, 100.f / totalFrameTime);
        status_string.SetText(buf);
        
        //Start over
        frames_elapsed = 0;
        totalFrameTime = 0;
    }
}

//Handle mc__::Events
bool UserInterface::actions()
{
    bool running=true;
    
    bool chunk_received=false;
    
    //Process all events
    Events::Event_t event;
    while ( !events.isEmpty && events.get(event) ) {
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
    
    /*
    //Update values when chunk received?
    if (chunk_received) {
    }
    */
  
    return running;
}

//Reset camera to player
void UserInterface::resetCamera()
{
    //Reset viewer to player position (change YAW 180 degrees)
    viewer.reset(16*player.abs_X, 16*player.eyes_Y, 16*player.abs_Z,
        player.yaw + 180, player.pitch);
    
    //Reset mouse position
    last_X = mouse_X = center_X;
    last_Y = mouse_Y = center_Y;

    
    //Status message
     cout << "Moved camera to player @ " << player.abs_X << ", "
         << player.abs_Y << "(" << player.eyes_Y << "), " << player.abs_Z
         << "  Yaw=" << player.yaw << " Pitch=" << player.pitch<< endl;

}

//Process user input
bool UserInterface::handleSfEvent( const sf::Event& Event )
{
    bool result=true;
    
    //Input state variables?
    
    switch( Event.Type) {
      
        //Window resize (DISABLED) or restore
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
                int view_X = (int)viewer.cam_X >> 4;
                int view_Y = (int)viewer.cam_Y >> 4;
                int view_Z = (int)viewer.cam_Z >> 4;
                cout << "Cam @ " << view_X << ", " << view_Y
                    << ", " << view_Z << " Player @ " << player.abs_X
                    << ", " << player.abs_Y << ", " << player.abs_Z << endl;
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
                case sf::Mouse::Right:
                    //Toggle mouselook
                    toggle_mouselook=true;
                    break;
                case sf::Mouse::XButton1:
                case sf::Mouse::XButton2:
                    //Move camera back to player
                    resetCamera();
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
    static int diff_X, diff_Y, diff_Z;
    bool moved=false, inhibit_mouselook=false;
    
    diff_X=0; diff_Y=0; diff_Z=0;
  
    //Translate camera in X or Y if moved while holding left button
    if (mouse_press[sf::Mouse::Left]) {

        //Step camera to side for mouse-X motion
        diff_X = mouse_X - mouse_press_X[sf::Mouse::Left];
        
        if (diff_X != 0) {
            //viewer.move(diff_X , 0, 0);
            moved=true;
            
            //Save new mouse position
            mouse_press_X[sf::Mouse::Left] = mouse_X;
        }

        //Step camera up for mouse-Y motion
        diff_Y = mouse_press_Y[sf::Mouse::Left] - mouse_Y;
        if (diff_Y != 0) {
            //viewer.move(0, diff_Y ,0);
            moved=true;
            
            //Save new mouse position
            mouse_press_Y[sf::Mouse::Left] = mouse_Y;
        }
        inhibit_mouselook=true;
    }
    //Translate camera in Z if moved while holding middle button
    if (mouse_press[sf::Mouse::Middle]) {

        //Step camera to side for mouse-Y motion
        diff_Z = mouse_press_Y[sf::Mouse::Middle] - mouse_Y;
        
        if (diff_Z != 0) {
            //viewer.move(0 , 0, diff_Z);
            moved=true;

            //Save new mouse position
            mouse_press_Y[sf::Mouse::Middle] = mouse_Y;
        }

        //Step camera to side for mouse-X motion
        diff_X = mouse_X - mouse_press_X[sf::Mouse::Middle];
        
        if (diff_X != 0) {
            //viewer.move(diff_X , 0, 0);
            moved=true;
            
            //Save new mouse position
            mouse_press_X[sf::Mouse::Middle] = mouse_X;
        }
        inhibit_mouselook=true;
    }
    if (mouselooking && !inhibit_mouselook)
    {    //Don't mouselook if left mouse button is held

        //Use change in X position to rotate about Y-axis
        int look_X = mouse_X - last_X;

        //Turn left/right based on mouse sensitivity
        if (look_X != 0) {
            viewer.turn( (float)look_X/mouseSensitivity );
        }

        //Tilt up/down based on mouse sensitivity
        int look_Y = mouse_Y - last_Y;
        if (look_Y != 0) {
            viewer.tilt( (float)look_Y/mouseSensitivity );
        }
        
    }
    
    //Move viewer if needed
    if (moved) {
        viewer.move(diff_X, diff_Y, diff_Z);
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
    bool movement[MOVE_COUNT] = {false, false, false, false, false, false, false, false};
    
    //Handle single keypresses in order
    if (keys_typed >= 1024) { keys_typed = 1024; }
    for( index = 0; index < keys_typed; index++) {
      
        //Game in "PLAYING" state... Do something else for text input
        //  if (inputState == PLAYING)
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
            //Return camera to player
            case sf::Key::Back:
                resetCamera();
                break;
            //Toggle status display
            case sf::Key::F3:
                showStatus = !showStatus;
                break;
            //Redraw everything
            case sf::Key::F5:
                cout << "Recalculating visibility of all chunks" << endl;
                world.redraw();
                break;
            default:
                customHandleKey(key_buffer[index]);
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

//Responses to keys read from key buffer during PLAYING input
void UserInterface::customHandleKey(sf::Key::Code keycode)
{
    switch (keycode) {
        case sf::Key::Tilde:
            //Write chunk data to files
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
