/*
  mc__::Game
    Handle game logic: create actions and respond to events
  
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

//libmc--
#include <mc--/World.hpp>
#include <mc--/Player.hpp>
#include <mc--/Events.hpp>

//STL
#include <string>

namespace mc__ {
    class Game {
        public:

        //Keep track of what phase of the game we're at
        enum gameStates { GAME_MENU, GAME_LOADING, PLAYING, PLAYER_DEAD };
        loginStates gameState;

        //Create with references to world, player, and event list
        Game(mc__::World& w, mc__::Player& p, mc__::Events& ev);

        //Set/get "global" values
        void set( const std::string& name, const std::string& value);
        std::string get( const std::string& name ) const;

        //Update the game world, add events, etc
        bool run();
        
        //Generate a new world, move player to spawn point
        bool genWorld();

    };
}
