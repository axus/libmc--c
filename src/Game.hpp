/*
  mc__::Game
    Handle game logic: create actions and respond to events
  
  Copyright 2012 axus

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

#ifndef MC__GAME_HPP
#define MC__GAME_HPP

//libmc--
#include <mc--/World.hpp>
#include <mc--/Player.hpp>
#include <mc--/Events.hpp>

//STL
#include <string>
#include <unordered_map>

namespace mc__ {
    class Game {
      public:

        //Keep track of what phase of the game we're at
        enum gameStates { GAME_MENU, GAME_LOADING, PLAYING, PLAYER_DEAD };

        gameStates gameState;      //State of client (see gameStates)
        int64_t gameTime;           //Current 64-bit time
        uint8_t gameMode;           //0: survival, 1:creative, 2:adventure, &0x8:hardcore
        uint8_t mapDimension;     //0:overworld, 1:end, 0xFF:hell
        uint8_t difficulty;         //0: peaceful, 1:easy, 2:normal, 3:hard
        uint8_t maxPlayers;         //Player limit on this server

        //Map option name to option value (strings)
        std::unordered_map< std::string, std::string> optionMap;
        
        //Construct with references to world, player, and event list
        Game(mc__::World& w, mc__::Player& p, mc__::Events& ev);

        //Set/get "global" values
        void set( const std::string& name, const std::string& value);
        std::string get( const std::string& name ) const;

      protected:
        //References to game objects
        World& world;
        Player& player;
        Events& events;
    };
}

#endif