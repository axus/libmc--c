/*
  mc__::TextureInfo
  Store texture ID and offset information for one face

  Copyright 2011 axus

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

#include "TextureInfo.hpp"

using mc__::TextureInfo;

//Constructor
TextureInfo::TextureInfo( tex_t tt, GLfloat tx0, GLfloat tx1,
            GLfloat ty0, GLfloat ty1):
                texType(tt), tx_0(tx0), tx_1(tx1), ty_0(ty0), ty_1(ty1)
{
}

//Other constructor
TextureInfo::TextureInfo( tex_t tt, GLsizei max_width, GLsizei max_height,
            GLsizei x0, GLsizei y0, GLsizei tex_width, GLsizei tex_height):
                texType(tt)
{
    //Calculate values
    setTexCoords(max_width, max_height, x0, y0, tex_width, tex_height,
        tx_0, tx_1, ty_0, ty_1);
}

void TextureInfo::getCoords( GLfloat& tx0, GLfloat& tx1,
    GLfloat& ty0, GLfloat& ty1) const
{
    tx0 = tx_0; tx1 = tx_1; ty0 = ty_0; ty1 = ty_1;
}


//Utility function
void TextureInfo::setTexCoords(GLsizei max_width, GLsizei max_height,
    GLsizei x0, GLsizei y0, GLsizei tex_width, GLsizei tex_height,
    GLfloat& tx0, GLfloat& tx1,
    GLfloat& ty0, GLfloat& ty1)
{
    //Safety check
    if (max_width == 0 || max_height == 0) {
        return;
    }
    
    //Calculations
    tx0 = (x0)/GLfloat(max_width);
    tx1 = (x0 + tex_width)/GLfloat(max_width);
    ty0 = (y0)/GLfloat(max_height);
    ty1 = (y0 + tex_height)/GLfloat(max_height);
}