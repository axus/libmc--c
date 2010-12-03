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
            //Entity();
            
            //Unique entity ID
            uint32_t EID;
            
            //Absolute position (integer)
            int64_t X, Y, Z;

            //Absolute position (double)
            double abs_X, abs_Y, abs_Z;
            
            //Looking in this direction
            float yaw, pitch;
            
            //Velocity vector
            int16_t dX, dY, dZ;
            
            //viewpoint Y-value
            double eyes_Y;
            
            //Health of entity
            uint8_t hitpoints;
            
            //What's it doing
            uint8_t animation;
    };
}

#endif
