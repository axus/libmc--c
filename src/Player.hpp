/*
  mc__::Player
    Represent minecraft player (or named entity) and their inventory
  
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

#ifndef MC__PLAYER
#define MC__PLAYER

//mc-- classes
#include "Entity.hpp"
#include "Chunk.hpp"

using mc__::Entity;
using mc__::Item;

//STL
#include <string>

//Definition
namespace mc__ {
  
    //Types
    enum inv_type_t { INVTYPE_MAIN=0, INVTYPE_EQUIPPED=1, INVTYPE_CRAFTING=2,
        INVTYPE_MAX };
        
    //Constants
    const uint8_t invSlotsMax = 36;
    const uint8_t invSlots[INVTYPE_MAX]={ invSlotsMax, 4, 4 };
    
    class Player : public Entity {
        public:
            //Constructor
            Player( const std::string& entity_name);
            
            //Movement and looking
            bool setPosLook( double x, double y, double z, double h,
                float yaw_, float pitch_);
            
            //Inventory functions
            bool addItem( const Item& item);
            bool moveItem( inv_type_t from_type, uint8_t from_slot,
                inv_type_t to_type, uint8_t to_slot);
            bool removeItem( inv_type_t from_type, uint8_t from_slot);
            
            //Player inventory... access this directly to read/write inventory
            Item inventory[INVTYPE_MAX][ invSlotsMax ];
            
            //Held item
            Item held_item;
            
            //Entity ID used as vehicle
            uint32_t VID;
            
            //Entity name
            std::string name;            
    };
}

#endif
