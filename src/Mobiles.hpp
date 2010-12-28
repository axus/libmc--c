/*
  mc__::Mobiles
    Store entities visible in the game Mobiles
  
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

#ifndef MC__MOBILES_H
#define MC__MOBILES_H

//mc--
#include "Player.hpp"
#include "Item.hpp"
#include "Entity.hpp"

#include "World.hpp"

//STL
#include <unordered_map>      //map / unordered_map / hash_map
#include <vector>

//Define class inside mc__ namespace
namespace mc__ {
    
    //Map Entity ID to players
    typedef std::unordered_map< uint32_t, mc__::Player* > playerMap_t;
    //Map Entity ID to items
    typedef std::unordered_map< uint32_t, mc__::Item* > itemMap_t;
    //Map Entity ID to monsters
    typedef std::unordered_map< uint32_t, mc__::Entity* > entityMap_t;

    //Mobiles class
    class Mobiles {
        
        public:
            //Constructor
            Mobiles( mc__::World& w );
            
            //Destructor
            ~Mobiles();
            
            //Add named entity (a player) to world
            mc__::Player* addPlayer(uint32_t eid, const std::string& name,
                uint32_t X, uint32_t Y, uint32_t Z,
                uint8_t yaw=0, uint8_t pitch=0);
            //Set a players visible equipment
            void setPlayerEquip(uint32_t eid, uint16_t slot, uint16_t item);

            //Add item to world
            mc__::Item* addItem(uint32_t eid, uint16_t item, uint8_t count,
                uint32_t X, uint32_t Y, uint32_t Z,
                uint8_t yaw=0, uint8_t pitch=0, uint8_t roll=0);

            //Add monster/animal/vehicle to world
            mc__::Entity* addEntity(uint32_t eid, uint8_t type_id,
                uint32_t X, uint32_t Y, uint32_t Z,
                uint8_t yaw=0, uint8_t pitch=0 );
            
            //Entity changed position
            void movePlayer( uint32_t eid, uint8_t dX, uint8_t dY, uint8_t dZ);
            void moveEntity( uint32_t eid, uint8_t dX, uint8_t dY, uint8_t dZ);
            
            //Entity changed direction
            void lookPlayer( uint32_t eid, float yaw, float pitch);
            void lookEntity( uint32_t eid, float yaw, float pitch);

            //Get entity pointer for EID (null if not found)
            mc__::Entity* getEntity(uint32_t eid);
                        
            //Access this to see chunks in the Mobiles
            playerMap_t playerMap;    //EID -> Player*
            itemMap_t itemMap;        //EID -> Item*
            entityMap_t entityMap;    //EID -> Entity*
            
        protected:
            mc__::World& world; //World info

    };

}

#endif