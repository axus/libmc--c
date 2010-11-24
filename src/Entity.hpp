/*
  mc__::Entity
    Represent minecraft mobile entity
  
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
            Entity();
            
            //Unique entity ID
            uint32_t EID;
            
            //Absolute position
            uint64_t X, Y, Z;
            
            //Velocity vector
            int16_t dX, dY, dZ;
            
            //Direction looking in:
            float lookYaw, lookPitch;
    };
}

#endif
