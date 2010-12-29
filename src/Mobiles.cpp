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

//mc--
#include "Mobiles.hpp"
using mc__::Player;
using mc__::Entity;
using mc__::Item;
using mc__::World;
using mc__::Mobiles;

//STL
#include <iostream>
using std::cerr;
using std::endl;

//Types
//  playerMap_t;
//  itemMap_t;
//  entityMap_t;

Mobiles::Mobiles( mc__::World& w ): world(w), uniqueEID(1)
{
}
            
//Destructor
Mobiles::~Mobiles()
{
    //Free memory for all players, entities, and items
}

//Add named entity (a player) to mobiles in the world
Player* Mobiles::addPlayer(uint32_t eid, const std::string& name,
    int32_t X, int32_t Y, int32_t Z,
    uint8_t yaw, uint8_t pitch)
{
    Player* player = NULL;
    
    //Look up existing player, or create a new one
    playerMap_t::const_iterator iter = playerMap.find( eid );
    if ( iter == playerMap.end()) {

        //Create new player
        player = new Player(eid, name);

        //Keep track of the player by entity ID
        playerMap.insert( playerMap_t::value_type(eid, player));
        
        //Increase EID if needed
        if (uniqueEID < eid) { uniqueEID = eid + 1; }
    } else {
        player = iter->second;
    }
    
    //We don't know the other players height, so set default
    double height = ((double)Y/32.0 + 1.62);
    
    //Set entity properites of the new player
    player->setPosLook( (double)X/32.0, (double)Y/32.0, (double)Z/32.0,
        height, yaw * 360.0f/255.0f, pitch * 360.0f/255.0f);

    return player;
}

//Add item to world
Item* Mobiles::addItem(uint32_t eid, uint16_t itemID, uint8_t count,
    int32_t X, int32_t Y, int32_t Z,
    uint8_t yaw, uint8_t pitch, uint8_t /*roll*/)
{
    Item *item=NULL;
    //Look up existing item, or create a new one
    itemMap_t::const_iterator iter = itemMap.find( eid );
    if ( iter == itemMap.end()) {

        item = new Item(itemID, count, 0, eid);
        
        //Keep track of the item by entity ID
        itemMap.insert( itemMap_t::value_type(eid, item));

        //Increase EID if needed
        if (uniqueEID < eid) { uniqueEID = eid + 1; }
    } else {
        //Already knew about this item
        item = iter->second;
        item->EID = eid;
        item->itemID = itemID;
        item->count = count;
        item->hitpoints = 0;
    }
    //Values not in the constructor
    item->X = X;
    item->Y = Y;
    item->Z = Z;
    item->abs_X = X/32.0;
    item->abs_Y = Y/32.0;
    item->abs_Z = Z/32.0;
    item->yaw = yaw * 360.0f/255.0f;
    item->pitch = pitch * 360.0f/255.0f;
    
    return item;
}

//Add monster/animal/vehicle to world
Entity* Mobiles::addEntity(uint32_t eid, uint8_t type_id,
    int32_t X, int32_t Y, int32_t Z,
    uint8_t yaw, uint8_t pitch)
{
    Entity *entity=NULL;
    //Look up existing item, or create a new one
    entityMap_t::const_iterator iter = entityMap.find( eid );
    if ( iter == entityMap.end()) {

        entity = new Entity(eid, (double)X/32.0, (double)Y/32.0, (double)Z/32.0,
        yaw * 360.0f/255.0f, pitch * 360.0f/255.0f);
        
        //Keep track of the item by entity ID
        entityMap.insert( entityMap_t::value_type(eid, entity));

        //Increase EID if needed
        if (uniqueEID < eid) { uniqueEID = eid + 1; }
    } else {
        //Re-use item ID, update info
        entity = iter->second;
        entity->EID = eid;
        entity->X = X;
        entity->Y = Y;
        entity->Z = Z;
        entity->abs_X = X/32.0;
        entity->abs_Y = Y/32.0;
        entity->abs_Z = Z/32.0;
        entity->yaw = yaw * 360.0f/255.0f;
        entity->pitch = pitch * 360.0f/255.0f;
    }
    
    //Copy other values
    entity->typeID = type_id;
    entity->hitpoints = 0;
    
    return entity;
}


//Find player pointer, insert UNKNOWN if not found
Player* Mobiles::findPlayer(uint32_t eid)
{
    Player *player=NULL;
    //Look up existing player, or create a new one
    playerMap_t::const_iterator iter = playerMap.find( eid );
    if ( iter == playerMap.end()) {
        player = new Player(eid, "UNKNOWN");
        //Keep track of the player by entity ID
        playerMap.insert( playerMap_t::value_type(eid, player));
        cerr << "ERROR! Unknown player entity ID " << eid << endl;
    } else {
        player = iter->second;
    }
    return player;
}

//Find item pointer, insert default one if not found
Item* Mobiles::findItem(uint32_t eid)
{
    Item *item=NULL;
    //Look up existing player, or create a new one
    itemMap_t::const_iterator iter = itemMap.find( eid );
    if ( iter == itemMap.end()) {
        item = new Item(eid);
        //Keep track of the player by entity ID
        itemMap.insert( itemMap_t::value_type(eid, item));
        cerr << "ERROR! Unknown item entity ID " << eid << endl;
    } else {
        item = iter->second;
    }
    
    return item;
}


//Find entity pointer, insert default one if not found
Entity* Mobiles::findEntity(uint32_t eid)
{
    Entity *entity=NULL;
    //Look up existing player, or create a new one
    entityMap_t::const_iterator iter = entityMap.find( eid );
    if ( iter == entityMap.end()) {
        entity = new Entity(eid);
        //Keep track of the player by entity ID
        entityMap.insert( entityMap_t::value_type(eid, entity));
        cerr << "ERROR! Unknown entity ID " << eid << endl;
    } else {
        entity = iter->second;
    }
    
    return entity;
}


//Set a players visible equipment
void Mobiles::setPlayerEquip(uint32_t eid, uint16_t slot, uint16_t item)
{
    //Lookup player pointer
    Player* player = findPlayer(eid);
    
    //Set equipment item (1 item, item health unknown)
    player->setSlot( (uint8_t)slot, item, 1, 0);
}

//Entity changed position
void Mobiles::movePlayer( uint32_t eid, int8_t dX, int8_t dY, int8_t dZ)
{
    //Lookup player pointer
    Player* player = findPlayer(eid);
        
    //Set entity properites of the new player
    player->move( dX, dY, dZ);
    
}

//Entity position updated
void Mobiles::moveEntity( uint32_t eid, int8_t dX, int8_t dY, int8_t dZ)
{
    //Lookup player pointer
    Entity* entity = findEntity(eid);
        
    //Set entity properites of the new player
    entity->move( dX, dY, dZ);
}

//Player changed direction
void Mobiles::turnPlayer( uint32_t eid,  int8_t dYaw, int8_t dPitch)
{
    //Lookup player pointer
    Player* player = findPlayer(eid);
        
    //Update player direction
    player->look( dYaw, dPitch);
}

//Update entity direction
void Mobiles::turnEntity( uint32_t eid,  int8_t dYaw, int8_t dPitch)
{
    //Lookup entity pointer
    Entity* entity = findEntity(eid);
        
    //Update player direction
    entity->look( dYaw, dPitch);
}

//Get entity pointer for EID (null if not found)
mc__::Entity* Mobiles::getEntity(uint32_t eid)
{
    Entity* entity = NULL;
    
    //Look for animal/monster, return if found
    entityMap_t::const_iterator ent_iter = entityMap.find( eid );
    if ( ent_iter != entityMap.end()) {
        entity = ent_iter->second;
        return entity;
    }
    
    //Look for player, return if found
    playerMap_t::const_iterator pl_iter = playerMap.find( eid );
    if ( pl_iter != playerMap.end()) {
        entity = pl_iter->second;
        return entity;
    }
    
    //Look for item, return if found
    itemMap_t::const_iterator item_iter = itemMap.find( eid );
    if ( item_iter != itemMap.end()) {
        entity = item_iter->second;
        return entity;
    }
    
    //NOT FOUND
    return NULL;
}
