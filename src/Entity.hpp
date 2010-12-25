/*
  mc__::Entity
    Represent mobile entity in a 3D game
  
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

#ifndef MC__ENTITY
#define MC__ENTITY

//Compiler specific options
#ifdef _MSC_VER
    #include "ms_stdint.h"
#else
    #include <stdint.h>
#endif


namespace mc__ {
  
    class Entity {
        public:
        
            //Default player height
            static const double humanHeight=1.62;
        
            //Constructor with lots of defaults
            Entity(uint32_t eid, double x=0, double y=0, double z=0,
                float YAW=0, float PITCH=0):
                EID(eid), abs_X(x), abs_Y(y), abs_Z(z), yaw(YAW), pitch(PITCH),
                on_ground(0), dX(0), dY(0), dZ(0), eyes_Y(y + humanHeight),
                hitpoints(0), animation(0)
            {
                X = abs_X*32;
                Y = abs_Y*32;
                Z = abs_Z*32;
            };
            
            //Unique entity ID
            uint32_t EID;
            
            //Type:
            uint8_t typeID;
            
            //Absolute position (integer)
            int64_t X, Y, Z;

            //Absolute position (double)
            double abs_X, abs_Y, abs_Z;
            
            //Looking in this direction (or orientation of item)
            float yaw, pitch;
            
            //Entity on ground (1), or in the air (0)
            //  Item on ground (1), or in inventory (0)
            uint8_t on_ground;
            
            //Velocity vector
            int16_t dX, dY, dZ;
            
            //viewpoint Y-value
            double eyes_Y;
            
            //Health of entity (or times item used)
            uint8_t hitpoints;
            
            //What's it doing
            uint8_t animation;
    };
}

#endif

//typeID    Description
//------    -----------
//0         Player
            
//1         Boat
//10        MineCart
//11        ChestCart
//12        FurnaceCart

//50        Creeper
//51        Skeleton
//52        Spider
//53        Giant Zombie
//54        Zombie
//55        Slime
//56        Ghast
//57        Zombie Pigman

//90        Pig
//91        Sheep
//92        Cow
//93        Chicken
