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
    enum slot_equipment_t { SLOT_EQ_CREATED=0,
        SLOT_EQ_CRAFT1=1, SLOT_EQ_CRAFT2=2, SLOT_EQ_CRAFT3=3, SLOT_EQ_CRAFT4=4,
        SLOT_EQ_HEAD=5, SLOT_EQ_TORSO=6, SLOT_EQ_LEGS=7, SLOT_EQ_FEET=8,
        SLOT_EQ_MAX };
    
    typedef struct {
        uint16_t itemID;
        uint8_t count;
        uint8_t hitpoints;
    } InvItem;
    
    //Player inventory max slot
    const uint8_t player_backpack_slots=36;
    const uint8_t player_inv_slots=SLOT_EQ_MAX + player_backpack_slots;
        
    //Number of window specific slots
    const uint8_t invWindowSlots[INV_WINDOW_MAX]= {
        SLOT_EQ_MAX, 10, 3, 27, 54};
        
    //Constant unsigned short value for empty slot
    const uint16_t emptyID=0xFFFF;
    
    //Player class definition
    class Player : public mc__::Entity {
        public:
            //Constructor
            Player( uint32_t eid, const std::string& entity_name);
            
            //Movement and looking
            bool setPosition( double x, double y, double z, double h);
            bool setLook( float yaw_, float pitch_);
            bool setPosLook( double x, double y, double z, double h,
                float yaw_, float pitch_);
            
            //Inventory functions
            bool addItem( const InvItem& item);
            bool setSlot( uint8_t to_slot, uint16_t itemID,
                uint8_t count, uint8_t used);
            bool moveItem( uint8_t from_slot, uint8_t to_slot);
            bool removeItem( uint8_t from_slot);
            
            //Player inventory: equipment and backpack
            mc__::InvItem inventory[ SLOT_EQ_MAX + player_backpack_slots];
            
            //Entity ID used as vehicle
            uint32_t VID;
            
            //Entity name
            std::string name;
    };
}

#endif
