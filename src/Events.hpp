/*
  mc__::Events
  Keep track of things that have happened outside of the user interface

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

#ifndef MC__EVENTS_H
#define MC__EVENTS_H

//STL
#include <deque>

//Compiler specific options
#ifdef _MSC_VER
    #include "ms_stdint.h"
#else
    #include <stdint.h>
#endif


namespace mc__ {
    class Events {
        public:
            //Enumarated types
            enum type_t {
                //Events from game world/server
                GAME_KEEPALIVE=          0x00,
                GAME_LOGIN=              0x01,
                GAME_HANDSHAKE=          0x02,
                GAME_CHAT_MESSAGE=       0x03,
                GAME_TIME_UPDATE=        0x04,
                GAME_INVENTORY=          0x05,
                GAME_SPAWN_POS=          0x06,
                GAME_PLAYER_VEHICLE=     0x07,
                GAME_PLAYER_HEALTH=      0x08,
                GAME_PLAYER_RESPAWN=     0x09,
                GAME_PLAYER=             0x0A,
                GAME_PLAYER_POS=         0x0B,
                GAME_PLAYER_LOOK=        0x0C,
                GAME_PLAYER_POSLOOK=     0x0D,
                GAME_PLAYER_DIG=         0x0E,
                GAME_PLAYER_PLACE=       0x0F,
                GAME_PLAYER_HOLD=        0x10,
                GAME_ADD_INVENTORY=      0x11,
                GAME_ARM_ANIMATION=      0x12,
                GAME_NAMED_SPAWN=        0x14,
                GAME_ITEM_SPAWN=         0x15,
                GAME_COLLECT_ITEM=       0x16,
                GAME_ADD_OBJECT=         0x17,
                GAME_MOB_SPAWN=          0x18,
                GAME_ENT_VELOCITY=       0x1C,
                GAME_DESTROY_ENTITY=     0x1D,
                GAME_ENTITY=             0x1E,
                GAME_ENT_REL_MOVE=       0x1F,
                GAME_ENT_LOOK=           0x20,
                GAME_ENT_LOOK_MOVE=      0x21,
                GAME_ENT_TELEPORT=       0x22,
                GAME_ENT_DESTROYED=      0x26,
                GAME_ENT_VEHICLE=        0x27,
                GAME_PRECHUNK=           0x32,
                GAME_MAPCHUNK=           0x33,
                GAME_MULTI_BLOCK_CHANGE= 0x34,
                GAME_BLOCK_CHANGE=       0x35,
                GAME_COMPLEX_ENTITY=     0x3B,
                GAME_KICK=               0xFF,
                
                //Actions by player
                ACTION_CONFIG,
                ACTION_CHAT,
                ACTION_MOVE,
                ACTION_EQUIP,
                ACTION_POS,
                ACTION_LOOK,
                ACTION_POSLOOK,
                ACTION_RESPAWN,
                ACTION_DIG,
                ACTION_PLACE,
                ACTION_ANIMATION,
                ACTION_PICKUP,
                ACTION_QUIT,
                
                //Input actions
                KEY_BACK,
                KEY_FORWARD,
                KEY_LEFT,
                KEY_RIGHT,
                KEY_DOWN,
                KEY_UP,
                KEY_RESET,
                KEY_RED,
                KEY_GREEN,
                KEY_BLUE,
                KEY_SAVE,
                KEY_QUIT,
                
                //Max event ID
                MC__EVENTS_MAX
            };
            
            //Generic data for event callback
            typedef struct {
                type_t type;
                const void *data;
            } Event_t;
            
            //Data types for callback data
            typedef struct {
                float yaw;
                float pitch;
                uint8_t animation;  //0=ground
            } dataLook;
            
            //Function pointers
            typedef uint8_t(*CB)(type_t t, const void* d);
            
            Events();
            
            //Add event
            void put(type_t t, const void *d);
            
            //Get next unhandled event
            bool get(Event_t& result);
            
            //Empty/not empty
            bool isEmpty;
            
        protected:
            std::deque<Event_t> myQueue;

    };
}
#endif
