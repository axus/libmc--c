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

#ifndef MC__TEXTUREINFO_H
#define MC__TEXTUREINFO_H

//OpenGL
#include <GL/gl.h>


namespace mc__ {

    enum tex_t { TEX_TERRAIN, TEX_ITEM, TEX_SIGN, TEX_MAX};
  
    class TextureInfo {
      public:
        //Constructors
        TextureInfo( tex_t tt, GLfloat tx0, GLfloat tx1,
            GLfloat ty0, GLfloat ty1);
        TextureInfo( tex_t tt, GLsizei max_width, GLsizei max_height,
            GLsizei x0, GLsizei y0, GLsizei tex_width, GLsizei tex_height);
        
        //Get values
        void getCoords( GLfloat& tx0, GLfloat& tx1,
            GLfloat& ty0, GLfloat& ty1) const;
        
        //Calculate texture coordinates (utility function)
        static void setTexCoords(GLsizei max_width, GLsizei max_height,
            GLsizei x0, GLsizei y0, GLsizei tex_width, GLsizei tex_height,
            GLfloat& tx0, GLfloat& tx1,
            GLfloat& ty0, GLfloat& ty1);

        //Variables
        tex_t texType;
        GLfloat tx_0, tx_1, ty_0, ty_1;
    };
}


#endif