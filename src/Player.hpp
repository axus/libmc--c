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
    
    //Window types
    enum inv_window_t { INV_PLAYER=0, INV_WORKBENCH=1, INV_FURNACE=2,
        INV_CHEST=3, INV_CHEST_BIG=4, INV_WINDOW_MAX };
        
    //Window specific slot enums
    enum inv_slot_player_t { SLOT_PL_CREATED=0,
        SLOT_PL_CRAFT1=1, SLOT_PL_CRAFT2=2, SLOT_PL_CRAFT3=3, SLOT_PL_CRAFT4=4,
        SLOT_PL_HEAD=5, SLOT_PL_TORSO=6, SLOT_PL_LEGS=7, SLOT_PL_FEET=8,
        SLOT_PL_MAX };
    
    typedef struct {
        uint16_t itemID;
        uint8_t count;
        uint8_t hitpoints;
    } InvItem;
    
    //Number of player backpack slots
    const uint8_t player_backpack_slots=36;
    
    //Number of window specific slots
    const uint8_t invWindowSlots[INV_WINDOW_MAX]= {
        SLOT_PL_MAX, 10, 3, 27, 54};
    
    //Player class definition
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
            bool moveItem( uint8_t from_slot, uint8_t to_slot);
            bool removeItem( uint8_t from_slot);
            
            //Player inventory... access this directly to read/write inventory
            mc__::InvItem inventory[INVTYPE_MAX][ invSlotsMax ];
            
            //Entity ID used as vehicle
            uint32_t VID;
            
            //Entity name
            std::string name;
    };
}

#endif
