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

//libmc--
#include "Game.hpp"

using mc__::Events;
using mc__::Player;
using mc__::World;
using mc__::Game;

using std::string;
using std::unordered_map;

 //Construct with references to world, player, and event list
Game::Game( World& w, Player& p, Events& ev):
    gameState(GAME_MENU), gameTime(0), gameMode(0), mapDimension(0),
    difficulty(0), maxPlayers(1), world(w), player(p), events(ev)
{
}
   
//Set/get "global" values
//Map name to value
void Game::set( const string& name, const string& value)
{
    optionMap[ name ] = value;
}

//Get value mapped by name
string Game::get( const string& name ) const
{
    string result;
    
    unordered_map< string, string >::const_iterator iter = optionMap.find( name );
    if (iter != optionMap.end()) {
        result = iter->second;
    }
    
    return result;
}
