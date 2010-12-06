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

//mc--
#include "Events.hpp"
using mc__::Events;

//C
//#include <cstring>

//STL
//using std::deque;


//Constructor
Events::Events(): isEmpty(true)
{
}

//Add events
void Events::put(type_t t, const void *d)
{
    Event_t event = { t, d};
    myQueue.push_back( event );
    isEmpty = false;
}

//Pop the next event off the list, return false if not found
bool Events::get(Event_t& result) {
    
    if (myQueue.empty()) {
        isEmpty = true;
        return false;
    }
    
    result = myQueue.back();
    myQueue.pop_back();

    if (myQueue.empty()) {
        isEmpty = true;
        return false;
    }

    return true;
}
