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

//STL
#include <iostream>
#include <string>
#include <iomanip>
#include <fstream>
#include <vector>

//SFML
#include <SFML/Graphics.hpp>

//libmc--c classes
#include <mc--/World.hpp>
#include <mc--/Viewer.hpp>


namespace mc__ {

    class UserInterface {
        public:
        
            //Texture file
            const std::string texture_map_filename;

            //Constructor, initialize SFML and mc-- objects here
            UserInterface(const std::string& title, mc__::World&);
            
            bool init();
            
            //Get input events and draw the game to App window
            bool run();
            
        protected:
        
            //Process user input
            bool handleSfEvent( const sf::Event& Event );

            //SFML
            //sf::Clock& clock;
            sf::WindowSettings Settings;
            sf::RenderWindow App;

            //mc--
            mc__::Viewer viewer;
            mc__::World& world;
            
            //Event loop    
            sf::Event lastEvent;
            
            //Current mouse pointer position
            int mouse_X, mouse_Y;
        
            //Current mouse button states
            bool mouse_press[sf::Mouse::ButtonCount];

            //Remember locations buttons were clicked at
            int mouse_press_X[sf::Mouse::ButtonCount];
            int mouse_press_Y[sf::Mouse::ButtonCount];
    };
}
