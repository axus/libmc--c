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

#include "Events.hpp"

//C
//#include <cstring>

//STL
using std::list;

//mc--
using mc__::Events;


//Add events
void Events::put(type_t t, void *d)
{
    Event_t event = { t, d};
    myQueue.push_back( event );
}

//Pop the next event off the list, return false if not found
bool Events::get(Event_t& result) {
    
    if (myQueue.empty()) {
        return false;
    }
    
    result = myQueue.back();
    myQueue.pop_back();
    
    return true;
}

/*
using mc__::Viewer;
using mc__::World;

//Constructor
Events::Events(mc__::World& w, mc__::Viewer& v): world(w), viewer(v)
{
    //Clear viewer callbacks and data
    uint16_t index;
    for ( index=0; index < sizeof(uint8_t); index++) {
        viewerCallbacks[index] = nullCB;
        viewerCBdata[index] = NULL;
    }
    
}

//Process all event callbacks
 Events::run()
{
    list<Event_t>::const_iterator iter;
    for (iter = myQueue.begin(); iter != myQueue.end(); iter++) {
        viewerCallbacks[iter->type](iter->type, iter->data);
        //TODO: viewerCBdata
    }
    
    //Processed all events
    myQueue.clear();
}

//Callback setter
void Events::setViewerCB(type_t t, CB cb, void* d)
{
    viewerCallbacks[t] = cb;
    viewerCBdata[t] = d;
}

//Unsetter
void Events::unsetViewerCB(type_t t)
{
    viewerCallbacks[t] = NULL;
    viewerCBdata[t] = NULL;
}

uint8_t Events::nullCB(type_t t, void* d)
{
    return 0;
}
*/
