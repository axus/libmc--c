/*
  mc__::Item
    Represent game Item
  
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

#ifndef MC__ITEM
#define MC__ITEM


//mc-- classes
#include "Entity.hpp"

//STL
#include <string>

//Definition
namespace mc__ {
  
    class Item : public mc__::Entity {
        public:
            //Constructor
            Item( uint16_t iid, uint8_t c=1, uint8_t d=0, uint32_t eid=0);

            //Get string for item ID
            std::string getName() const;
            
            //Max number of uses for this item (static)
            uint8_t maxUses() const;
            
            //Entity ID for dropped items
            uint32_t entity_ID;
            
            //type of item
            uint16_t itemID;
            
            //Number of item in stack
            uint8_t count;
            
            //See "hitpoints" derived from Entity
            //  Durability, dye type, wood type

            //Static functions
            static std::string getString(uint16_t iid, uint8_t offset=0);

    };
}

#endif
