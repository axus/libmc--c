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

//Types
//  playerMap_t;
//  itemMap_t;
//  entityMap_t;

Mobiles::Mobiles( mc__::World& w ): world(w)
{
}
            
//Destructor
Mobiles::~Mobiles()
{
    //Free memory for all players, entities, and items
}

//Add named entity (a player) to mobiles in the world
Player* Mobiles::addPlayer(uint32_t eid, const std::string& name,
    uint32_t X, uint32_t Y, uint32_t Z,
    uint8_t yaw, uint8_t pitch)
{
    Player* player = NULL;
    
    //Look up existing player, or create a new one
    playerMap::const_iterator iter = playerMap.find( eid );
    if ( iter == playerMap.end()) {
        player = new Player(name);
        //Keep track of the player by entity ID
        playerMap.insert( eid, player);
    } else {
        player = iter->second;
    }
    
    //Set entity properites of the new player
    player->setPosLook( (double)X/32.0, double)Y/32.0, double)Z/32.0,
        yaw * 360.0/255.0, pitch * 360.0/255.0);

    return player;
}

//Set a players visible equipment
void Mobiles::setPlayerEquip(uint32_t eid, uint16_t slot, uint16_t item)
{
}

//Add item to world
Item* addItem(uint32_t eid, uint16_t item, uint8_t count,
    uint32_t X, uint32_t Y, uint32_t Z,
    uint8_t yaw=0, uint8_t pitch=0, uint8_t roll=0)
{
}

//Add monster/animal/vehicle to world
Entity* Mobiles::addEntity(uint32_t eid, uint8_t type_id,
    uint32_t X, uint32_t Y, uint32_t Z,
    uint8_t yaw=0, uint8_t pitch=0 )
{
}

//Entity changed position
void Mobiles::movePlayer( uint32_t eid, uint8_t dX, uint8_t dY, uint8_t dZ)
{
    Player* player = NULL;
    
    //Look up existing player, or create a new one
    playerMap::const_iterator iter = playerMap.find( eid );
    if ( iter == playerMap.end()) {
        player = new Player("UNKNOWN");
        //Keep track of the player by entity ID
        playerMap.insert( eid, player);
        cerr << "ERROR! Unknown player entity ID " << eid << endl;
    } else {
        player = iter->second;
    }
    
    //Set entity properites of the new player
    player->setPosition( (double)X/32.0, double)Y/32.0, double)Z/32.0);
    
}

//Entity position updated
void Mobiles::moveEntity( uint32_t eid, uint8_t dX, uint8_t dY, uint8_t dZ)
{
    Entity* entity = NULL;
    
    //Look up existing player, or create a new one
    entityMap::const_iterator iter = entityMap.find( eid );
    if ( iter == entityMap.end()) {
        entity = new Entity("UNKNOWN");
        //Keep track of the player by entity ID
        entityMap.insert( eid, entity);
        cerr << "ERROR! Unknown entity ID " << eid << endl;
    } else {
        entity = iter->second;
    }
    
    //Set entity properites of the new player
    entity->setPosition( (double)X/32.0, double)Y/32.0, double)Z/32.0);
    
}

//Player changed direction
void Mobiles::lookPlayer( uint32_t eid, float yaw, float pitch)
{
    Player* player = NULL;
    
    //Look up existing player, or create a new one
    playerMap::const_iterator iter = playerMap.find( eid );
    if ( iter == playerMap.end()) {
        player = new Player(name);
        //Keep track of the player by entity ID
        playerMap.insert( eid, player);
        cerr << "ERROR! Unknown player entity ID " << eid << endl;
    } else {
        player = iter->second;
    }
    
    //Set player idrection
    player->setLook( yaw * 360.0/255.0, pitch * 360.0/255.0);

}

//Update entity direction
void Mobiles::lookEntity( uint32_t eid, float yaw, float pitch)
{
    Entity* entity = NULL;
    
    //Look up existing player, or create a new one
    entityMap::const_iterator iter = entityMap.find( eid );
    if ( iter == entityMap.end()) {
        entity = new Entity(name);
        //Keep track of the player by entity ID
        entityMap.insert( eid, entity);
        cerr << "ERROR! Unknown entity ID " << eid << endl;
    } else {
        entity = iter->second;
    }
    
    //Set player idrection
    entity->setLook( yaw * 360.0/255.0, pitch * 360.0/255.0);
}

//Get entity pointer for EID (null if not found)
mc__::Entity* Mobiles::getEntity(uint32_t eid)
{
    Entity* entity = NULL;
    
    //Look for animal/monster, return if found
    entityMap::const_iterator ent_iter = entityMap.find( eid );
    if ( ent_iter != entityMap.end()) {
        entity = ent_iter->second;
        return entity;
    }
    
    //Look for player, return if found
    playerMap::const_iterator pl_iter = playerMap.find( eid );
    if ( pl_iter != playerMap.end()) {
        entity = pl_iter->second;
        return entity;
    }
    
    //Look for item, return if found
    itemMap::const_iterator item_iter = itemMap.find( eid );
    if ( item_iter != itemMap.end()) {
        entity = item_iter->second;
        return entity;
    }
    
    //NOT FOUND
    return NULL;
}

/*           
            //Access this to see chunks in the Mobiles
            playerMap_t playerMap;    //EID -> Player*
            itemMap_t itemMap;        //EID -> Item*
            entityMap_t entityMap;    //EID -> Entity*
            mc__::World& world; //World info
*/