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

#include "Player.hpp"

using mc__::Player;

using std::string;

//Constructor
Player::Player(const std::string& entity_name): Entity(), name(entity_name)
{
}

//Add item to inventory (return false if full)
bool Player::addItem( const Item& item)
{
    size_t slot=0;
    bool added=false;
    
    while (slot < invSlots[INVTYPE_MAIN] && !added) {
        //Check for empty slot
        Item& nextItem = inventory[INVTYPE_MAIN][slot];
        if ( nextItem.itemID == 0) {
            nextItem = item;
            added=true;
        }
    }
    
    return added;
}

//Swap items inside inventory
bool Player::moveItem( inv_type_t from_type, uint8_t from_slot,
                       inv_type_t to_type,   uint8_t to_slot)
{
    //TODO: check if item is moving to invalid slot
    
    //Swap items
    Item& from_item = inventory[from_type][from_slot];
    Item& to_item = inventory[to_type][to_slot];
    Item copy_item = to_item;
    to_item = from_item;
    from_item = copy_item;

    return true;
}

//Remove item from inventory slot
bool Player::removeItem( inv_type_t from_type, uint8_t from_slot)
{
    //Static empty item
    static Item empty_item = { 0, 0, 0};

    //Set inventory slot to empty
    inventory[from_type][from_slot] = empty_item;

    return true;
}
