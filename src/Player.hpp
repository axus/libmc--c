/*
  mc__::Player
    Represent 3D game player (or named entity) and their inventory
  
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

//STL
#include <string>


//Definition
namespace mc__ {
  
    //Types
    enum inv_type_t { INVTYPE_MAIN=0, INVTYPE_EQUIPPED=1, INVTYPE_CRAFTING=2,
        INVTYPE_MAX };

    typedef struct {
        uint16_t itemID;
        uint8_t count;
        uint8_t hitpoints;
    } InvItem;
    
    //Constants
    const uint8_t invSlotsMax = 36;
    const uint8_t invSlots[INVTYPE_MAX]={ invSlotsMax, 4, 4 };
    
    class Player : public mc__::Entity {
        public:
            //Constructor
            Player( const std::string& entity_name);
            
            //Movement and looking
            bool setPosition( double x, double y, double z, double h);
            bool setLook( float yaw_, float pitch_);
            bool setPosLook( double x, double y, double z, double h,
                float yaw_, float pitch_);
            
            //Inventory functions
            bool addItem( const InvItem& item);
            bool moveItem( inv_type_t from_type, uint8_t from_slot,
                inv_type_t to_type, uint8_t to_slot);
            bool removeItem( inv_type_t from_type, uint8_t from_slot);
            
            //Player inventory... access this directly to read/write inventory
            mc__::InvItem inventory[INVTYPE_MAX][ invSlotsMax ];
            
            //Entity ID used as vehicle
            uint32_t VID;
            
            //Entity name
            std::string name;
    };
}

#endif
