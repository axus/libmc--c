/*
  mc__::Player
    Represent 3D game player (or named entity) and their inventory
  
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

//mc--
#include "Player.hpp"
using mc__::Player;
using mc__::Entity;
using mc__::InvItem;

//STL
using std::string;

//Constructor
Player::Player(uint32_t eid, const std::string& entity_name):
    Entity(eid), name(entity_name)
{
}

bool Player::setPosition( double x, double y, double z, double h)
{
    //Absolute position (double)
    abs_X = x;
    abs_Y = y;
    abs_Z = z;
    
    //Player eye location
    eyes_Y = h;
    
    //Absolute position (integer)
    X = (int32_t)(x * 32);
    Y = (int32_t)(y * 32);
    Z = (int32_t)(z * 32);

    return true;
}

bool Player::setLook( float yaw_, float pitch_)
{
    //Looking in this direction
    yaw = yaw_;
    pitch = pitch_;
    
    return true;
}

//Set position and look direction
bool Player::setPosLook( double x, double y, double z,
    double h, float yaw_, float pitch_)
{
    //At this location (eyes at h)
    setPosition(x, y, z, h);

    //Looking in this direction
    setLook(yaw_, pitch_);
    
    return true;
}

//Add item to inventory (put in best available slot)
bool Player::addItem( const InvItem& item)
{
    bool added=false;
    const uint8_t player_quickslot=player_inv_slots - 9;

    //Start with left quickslot of backpack
    uint8_t slot=player_quickslot;
    while (slot < player_backpack_slots && !added) {
        //Check for empty slot
        InvItem& nextItem = inventory[slot];
        if ( nextItem.itemID == emptyID) {
            nextItem = item;
            added=true;
        }
    }
    
    //Now, check slots in backpack that are not quickslots
    slot=SLOT_EQ_MAX;
    while (slot < player_quickslot && !added) {
        //Check for empty slot
        InvItem& nextItem = inventory[slot];
        if ( nextItem.itemID == emptyID) {
            nextItem = item;
            added=true;
        }
    }
    
    return added;
}

//Set item in a slot
bool Player::setSlot( uint8_t to_slot, uint16_t itemID,
    uint8_t count, uint8_t used)
{
    //Check if slot is invalid number
    if ( to_slot > player_inv_slots) {
        return false;
    }
    //Check if slot is illegal for item ID
    if ( to_slot == 0) {
        //TODO: ONLY EQUIPMENT IN SLOTS SLOT_EQ_HEAD to SLOT_EQ_FEET
        return false;
    }
    
    //Reference inventory slot (old item is overwritten)
    InvItem& item = inventory[to_slot];

    //Set item in slot    
    item.itemID = itemID;
    item.count = count;
    item.hitpoints = used;
    
    return true;
}


//Swap items inside equipment and inventory
bool Player::moveItem( uint8_t from_slot, uint8_t to_slot)
{
    //Check if slot is invalid number
    if (from_slot > player_inv_slots || to_slot > player_inv_slots) {
        return false;
    }
    //Check if slot is illegal for item ID
    if (from_slot == 0 || to_slot == 0) {
        //TODO: ONLY EQUIPMENT IN SLOTS SLOT_EQ_HEAD to SLOT_EQ_FEET
        return false;
    }
    
    //Swap items
    InvItem& from_item = inventory[from_slot];
    InvItem& to_item = inventory[to_slot];
    InvItem copy_item = to_item;
    to_item = from_item;
    from_item = copy_item;

    return true;
}

//Remove item from inventory slot
bool Player::removeItem( uint8_t from_slot)
{
    //Static empty item
    static InvItem empty_item = { emptyID, 0, 0};

    //Set inventory slot to empty
    inventory[from_slot] = empty_item;

    return true;
}
