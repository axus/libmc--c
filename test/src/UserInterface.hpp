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

#ifndef MC__USERINTERFACE_HPP
#define MC__USERINTERFACE_HPP

//libmc--c classes
#include <mc--/World.hpp>
#include <mc--/Mobiles.hpp>
#include <mc--/Viewer.hpp>
#include <mc--/Player.hpp>
#include <mc--/Events.hpp>

//SFML
#include <SFML/Graphics.hpp>
#include <SFML/System.hpp>

//STL
#include <iostream>
#include <string>
#include <iomanip>
#include <fstream>
#include <vector>


namespace mc__ {

    class UserInterface {
        public:
        
            //Types of movement
            enum movetype { MOVE_FORWARD, MOVE_BACK, MOVE_LEFT, MOVE_RIGHT,
                MOVE_UP, MOVE_DOWN, TURN_LEFT, TURN_RIGHT, MOVE_COUNT };
            
            //Texture file
            const std::string texture_map_filename;
            const std::string item_icon_filename;
            
            //Constructor, initialize SFML and mc-- objects here
            UserInterface(const std::string& title, mc__::World& w,
                mc__::Mobiles& m, mc__::Player& player,
                mc__::Events& ev, bool dbg=false);
            ~UserInterface();
            
            bool init();
            
            //Get input events and draw the game to App window
            bool run();
            
            //Handle events from game
            bool actions();
            
            //Move camera to player position
            void resetCamera();
            
            //Mouse sensitivity
            float mouseSensitivity;

        protected:
        
            //Process user input
            bool handleSfEvent( const sf::Event& Event );
            
            //Process keys typed
            bool handleKeys();
            virtual void customHandleKey(sf::Key::Code);
            
            //Process mouse movement
            bool handleMouse();
            
            //Draw debugging info to screen
            void setDebug();

            //SFML
            sf::WindowSettings Settings;
            sf::RenderWindow App;

            //mc--
            mc__::Viewer viewer;
            mc__::World& world;
            mc__::Mobiles& mobiles;
            mc__::Player& player;
            mc__::Events& events;
            bool debugging;
            
            //Event loop    
            sf::Event lastEvent;
            
            //Mouse-looking
            bool mouselooking, toggle_mouselook;
            
            //Current mouse pointer position (virtual)
            int mouse_X, mouse_Y;
            
            //Previous mouse position
            int last_X, last_Y;
            
            //Center coordinates in window
            int center_X, center_Y;
        
            //Current mouse button states
            bool mouse_press[sf::Mouse::ButtonCount];

            //Remember previous mouse pointer locations
            int mouse_press_X[sf::Mouse::ButtonCount];
            int mouse_press_Y[sf::Mouse::ButtonCount];
            
            //Remember keyboard key state
            bool key_held[sf::Key::Count];
            
            //Remember order keys were typed in
            sf::Key::Code key_buffer[1024];
            size_t keys_typed;
            
            //Frame counter and status message
            bool showStatus;
            size_t frames_elapsed;
            float totalFrameTime;
            sf::String status_string;

    };
}

#endif
