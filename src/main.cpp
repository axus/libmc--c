//Block viewer app
// axus 2010

/*
    OpenGL has specific meaning for x, y, and z axes.
    
    z-axis is distance from the initial viewpoint.  As Z increases, the
    location gets closer to the initial viewpoint. (negative is farther)
    
    y-axis is "up" relative to the initial viewpoint.  As Y increases,
    the location moves further "up".
    
    x-axis is "right" relative to the intial viewpoint.  As X increases,
    the location moves further "right".
*/

/*
    Block faces are always aligned with major axes. So, we can say that each
    face is on an axis, and is "high" or "low" compared to the opposing face.
     
    On the x axis: A = left,    B = right
    On the y axis: C = down,    D = up
    On the z axis: E = farther, F = closer
    
    We can refer to the corners of the cube by which faces they are part of.
    Each of those 3 faces will describe a different axis.  For example:
        ADF is  x=left, y=up, z=close.
    
    You can visualize the cube with those 8 points:
           ADE ---- BDE
           /.       /|
          / .      / |
        ADF ---- BDF |
         | ACE . .| BCE
         | .      | /
         |.       |/
        ACF ---- BCF

    A face will have four points containing its letter.
    
*/

//SFML
#include <SFML/Graphics.hpp>

//DevIL
#include <IL/il.h>
#include <IL/ilu.h>

//STL
#include <iostream>
#include <string>
#include <iomanip>
#include <fstream>
#include <vector>

using std::cout;
using std::cerr;
using std::endl;
using std::string;
using std::hex;
using std::dec;
using std::setw;
using std::setfill;
using std::ofstream;
using std::ios;
using std::vector;
using std::map;

//Compiler specific options
#ifdef _MSC_VER
    #include "ms_stdint.h"
#else
    #include <stdint.h>
#endif

//
// Types
//

//Block in the game world
typedef struct {
    uint8_t blockID;
    uint8_t metadata;
    uint8_t lighting;
} mc__Block;

//Physical properties, to associate with blockID
typedef struct {
    uint16_t textureID[6];  //texture of faces A, B, C, D, E, F
    GLfloat tx[6];          //X coordinate (0.0 - 1.0) of block texture in texture map
    GLfloat ty[6];          //Y coordinate (0.0 - 1.0) of block texture in texture map
    uint8_t  properties;
        //Bytes 0-3: Glow  : 0-7=How much light it emits
        //Byte  4  : Shape : 0=cube, 1=object
        //Byte  5  : Glass : 0=opqaue, 1=see-through
        //Bytes 6-7: State : 0=solid, 1=loose, 2=liquid, 3=gas
} mc__BlockInfo;

//Item properties, on ground or in inventory
typedef struct {
    uint16_t itemID;
    uint8_t  count;
    uint16_t health;
} mc__Item;

//Chunk of blocks (brick shaped, uncompressed)
typedef struct {
    GLint X, Y, Z;
    uint8_t size_X, size_Y, size_Z;
    uint32_t array_length; //array len = size_X * size_Y * size_Z, precomputed
    mc__Block *block_array; 
} mc__Chunk;

//
// Global variables
//

//Constants
const string texture_map_file("terrain.png");
const size_t texmap_TILE_PIXELS = 16;    //pixels in tile (1D)
const size_t texmap_TILES = 16;         //tiles in map (1D)
const unsigned short texmap_TILE_MAX = texmap_TILES * texmap_TILES;
const float tmr = 1.0f/((float)texmap_TILES);   //Texture map ratio:  tile:texmap length

//Currently selected texture for new block
uint16_t texmap_selected = 7;

//Initial camera position
GLint camera_X = 0;
GLint camera_Y = -256;
GLint camera_Z = -160;

//Input state variables
bool mouse_press[sf::Mouse::ButtonCount];
int mouse_press_X[sf::Mouse::ButtonCount];
int mouse_press_Y[sf::Mouse::ButtonCount];
int mouse_X, mouse_Y;

//DevIL textures
ILuint il_texture_map;
ILuint ilTextureList[texmap_TILE_MAX];

//openGL image
GLuint image;

//Map block ID to block information
mc__BlockInfo blockInfo[256];

//Blocks placed in the world
vector<mc__Block> blocks;
vector<mc__Chunk> chunks;

//
// Functions
//

//Dump DevIL texture info to "texture.log"
void outputRGBAData() {
    
    //Return if wrong format
    if (ilGetInteger(IL_IMAGE_FORMAT) != IL_RGBA) {
        cerr << "Not RGBA" << endl;
        return;
    }
    
    //Get image information
    size_t width = ilGetInteger(IL_IMAGE_WIDTH);
    size_t height = ilGetInteger(IL_IMAGE_HEIGHT);
    size_t bpp = ilGetInteger(IL_IMAGE_BPP);
    size_t bytes_used = width*height*bpp;
    ILubyte *data = ilGetData();

    //Open output file (append)
    ofstream outfile;
    outfile.open("texture.log", ios::app);
    
    //Texture information
    outfile << "Size=" << bytes_used << " BPP=" << bpp << " Width="
        << width << " Height=" << height << endl;
    
    //Repeat for each byte
    size_t bytenum, x, y, index;
    index = 0;    
    for (y = 0; y < height; y++) {
        outfile << setw(4) << setfill(' ') << y << ": ";
        for (x = 0; x < width; x++) {
            for (bytenum = 0; bytenum < bpp; bytenum++, index++) {
                outfile << hex << setw(2) << setfill('0') << (short)data[index];
            }
            outfile << " ";
        }
        outfile << endl << dec;
    }
    
    //Close file
    outfile.close();
}

/*
//draw cube faces for cube at block address (x,y,z)
//Call this inside "glBegin(), glEnd()"
void drawCubes(GLint x, GLint y, GLint z) {
    
    //Draw cube
    GLint A = (x << 4) - 8;
    GLint B = (x << 4) + 8;
    GLint C = (y << 4) - 8;
    GLint D = (y << 4) + 8;
    GLint E = (z << 4) - 8;
    GLint F = (z << 4) + 8;


//       ADE ---- BDE
//       /.       /|
//      / .      / |
//    ADF ---- BDF |
//     | ACE . .| BCE
//     | .      | /
//     |.       |/
//    ACF ---- BCF


    //A
    glTexCoord2i(0,0); glVertex3i( A, C, E);  //Lower left:  ACE
    glTexCoord2i(1,0); glVertex3i( A, C, F);  //Lower right: ACF
    glTexCoord2i(1,1); glVertex3i( A, D, F);  //Top right:   ADF
    glTexCoord2i(0,1); glVertex3i( A, D, E);  //Top left:    ADE

    //B
    glTexCoord2i(0,0); glVertex3i( B, C, F);  //Lower left:  BCF
    glTexCoord2i(1,0); glVertex3i( B, C, E);  //Lower right: BCE
    glTexCoord2i(1,1); glVertex3i( B, D, E);  //Top right:   BDE
    glTexCoord2i(0,1); glVertex3i( B, D, F);  //Top left:    BDF
    
    //C
    glTexCoord2i(0,0); glVertex3i( A, C, E);  //Lower left:  ACE
    glTexCoord2i(1,0); glVertex3i( B, C, E);  //Lower right: BCE
    glTexCoord2i(1,1); glVertex3i( B, C, F);  //Top right:   BCF
    glTexCoord2i(0,1); glVertex3i( A, C, F);  //Top left:    ACF
    
    //D
    glTexCoord2i(0,0); glVertex3i( A, D, F);  //Lower left:  ADF
    glTexCoord2i(1,0); glVertex3i( B, D, F);  //Lower right: BDF
    glTexCoord2i(1,1); glVertex3i( B, D, E);  //Top right:   BDE
    glTexCoord2i(0,1); glVertex3i( A, D, E);  //Top left:    ADE
    
    //E
    glTexCoord2i(0,0); glVertex3i( B, C, E);  //Lower left:  BCE
    glTexCoord2i(1,0); glVertex3i( A, C, E);  //Lower right: ACE
    glTexCoord2i(1,1); glVertex3i( A, D, E);  //Top right:   ADE
    glTexCoord2i(0,1); glVertex3i( B, D, E);  //Top left:    BDE
    
    //F
    glTexCoord2i(0,0); glVertex3i( A, C, F);  //Lower left:  ACF
    glTexCoord2i(1,0); glVertex3i( B, C, F);  //Lower right: BCF
    glTexCoord2i(1,1); glVertex3i( B, D, F);  //Top right:   BDF
    glTexCoord2i(0,1); glVertex3i( A, D, F);  //Top left:    ADF
}
*/


/*
//Blit from texture map to current OpenGL texture
ILuint blitTexture( ILuint texmap, ILuint SrcX, ILuint SrcY, ILuint Width, ILuint Height) {
    //Take out a specific block from the texture grid (Blit)
    ILuint il_blocktex;
    ilGenImages(1, &il_blocktex);
    ilBindImage(il_blocktex);
    ilTexImage( Width, Height, 1, 4, IL_RGBA, IL_UNSIGNED_BYTE, NULL);

    //Blit the rectangle from texmap to il_blocktex
    ilBlit( texmap, 0, 0, 0, SrcX, SrcY, 0, Width, Height, 1);

    //Use single OpenGL "image"
    glBindTexture(GL_TEXTURE_2D, image);

    //Copy current DevIL image to OpenGL image
    glTexImage2D(GL_TEXTURE_2D, 0, ilGetInteger(IL_IMAGE_BPP),
        ilGetInteger(IL_IMAGE_WIDTH), ilGetInteger(IL_IMAGE_HEIGHT), 0,
        ilGetInteger(IL_IMAGE_FORMAT), GL_UNSIGNED_BYTE, ilGetData());

    //free memory used by DevIL
    ilDeleteImages(1, &il_blocktex);  
    return il_blocktex;
}
*/

/*
//choose OpenGL drawing texture from image list
ILuint chooseTexture( ILuint* ilImages, size_t index) {
  
    //ilBind image from list
    ILuint ilImage = ilImages[index];
    ilBindImage( ilImage );
    
    //glBind texture
    glBindTexture(GL_TEXTURE_2D, image);
    glTexImage2D(GL_TEXTURE_2D, 0, ilGetInteger(IL_IMAGE_BPP),
        ilGetInteger(IL_IMAGE_WIDTH), ilGetInteger(IL_IMAGE_HEIGHT), 0,
        ilGetInteger(IL_IMAGE_FORMAT), GL_UNSIGNED_BYTE, ilGetData());
        
    return ilImage;
}

//Chop texture map into ilImages array (don't bind anything to GL)
bool splitTextureMap( ILuint texmap, size_t tiles_x, size_t tiles_y, ILuint* ilImages)
{
    ILuint SrcX, SrcY, Width, Height;
    Width = texmap_TILE_PIXELS;
    Height = texmap_TILE_PIXELS;
    
    //User should have allocated ilImages array
    if (ilImages == NULL) {
        return false;
    }
    
    //Create blank images in the array
    ilGenImages(tiles_x * tiles_y, ilImages);
    
    //Copy alpha channel when blitting, don't blend it.
    ilDisable(IL_BLIT_BLEND);

    //For each tile...
    size_t x, y, index;
    for (y = 0, index=0; y < tiles_y; y++) {
        for (x = 0; x < tiles_x; x++, index++) {
            //Bind the image
            ilBindImage(ilImages[index]);
            ilTexImage( Width, Height, 1, 4, IL_RGBA, IL_UNSIGNED_BYTE, NULL);

            //Blit the rectangle from texmap to il_blocktex
            SrcX = x * Width; SrcY = y * Height;
            ilBlit( texmap, 0, 0, 0, SrcX, SrcY, 0, Width, Height, 1);
        }
    }
    
    return true;
}
*/

//Draw a mc__Block in openGL
void drawBlock( const mc__Block& block, GLint x, GLint y, GLint z)
{

    GLint A, B, C, D, E, F;     //Face coordinates (in pixels)
    GLint G, H;                 //non-cube quad offsets

    //Texture map coordinates (0.0 - 1.0)
    GLfloat tx_0, tx_1, ty_0, ty_1;

    //Start putting quads in memory
    glBegin(GL_QUADS);

    //For each face, load the appropriate texture for the block ID, and draw square
    //       ADE ---- BDE
    //       /.       /|
    //      / .      / |
    //    ADF ---- BDF |
    //     | ACE . .| BCE
    //     | .      | /
    //     |.       |/
    //    ACF ---- BCF

    //   Texture map was loaded upside down...
    // 0.0 -------------> 1.0 (X)
    // |
    // |
    // |
    // |
    // |
    // |
    // v
    // 1.0
    // (Y)
    
    //
    // (tx_0, ty_1)      (tx_1, ty_1)
    //
    // (tx_0, ty_0)      (tx_1, ty_0)

  if (blockInfo[block.blockID].properties & 0x8) {

    A = (x << 4) + 0;
    B = (x << 4) + texmap_TILE_PIXELS;
    C = (y << 4) + 0;
    D = (y << 4) + texmap_TILE_PIXELS;
    E = (z << 4) + 0;
    F = (z << 4) + texmap_TILE_PIXELS;
    G = (z << 4) + (texmap_TILE_PIXELS/2);
    H = (x << 4) + (texmap_TILE_PIXELS/2);

    //Draw a planted object, using only 1 texture
    tx_0 = blockInfo[block.blockID].tx[0];
    tx_1 = blockInfo[block.blockID].tx[0] + tmr;
    ty_0 = blockInfo[block.blockID].ty[0] + tmr;    //flip y
    ty_1 = blockInfo[block.blockID].ty[0];
    glTexCoord2f(tx_0,ty_0); glVertex3i( A, C, G);  //Lower left:  ACG
    glTexCoord2f(tx_1,ty_0); glVertex3i( B, C, G);  //Lower right: BCG
    glTexCoord2f(tx_1,ty_1); glVertex3i( B, D, G);  //Top right:   BDG
    glTexCoord2f(tx_0,ty_1); glVertex3i( A, D, G);  //Top left:    ADG

    glTexCoord2f(tx_0,ty_0); glVertex3i( H, C, F);  //Lower left:  HCF
    glTexCoord2f(tx_1,ty_0); glVertex3i( H, C, E);  //Lower right: HCE
    glTexCoord2f(tx_1,ty_1); glVertex3i( H, D, E);  //Top right:   HDE
    glTexCoord2f(tx_0,ty_1); glVertex3i( H, D, F);  //Top left:    HDF

  } else {
    //Draw a solid cube
    A = (x << 4) + 0;
    B = (x << 4) + texmap_TILE_PIXELS;
    C = (y << 4) + 0;
    D = (y << 4) + texmap_TILE_PIXELS;
    E = (z << 4) + 0;
    F = (z << 4) + texmap_TILE_PIXELS;

    //A
    tx_0 = blockInfo[block.blockID].tx[0];
    tx_1 = blockInfo[block.blockID].tx[0] + tmr;
    ty_0 = blockInfo[block.blockID].ty[0] + tmr;    //flip y
    ty_1 = blockInfo[block.blockID].ty[0];
    glTexCoord2f(tx_0,ty_0); glVertex3i( A, C, E);  //Lower left:  ACE
    glTexCoord2f(tx_1,ty_0); glVertex3i( A, C, F);  //Lower right: ACF
    glTexCoord2f(tx_1,ty_1); glVertex3i( A, D, F);  //Top right:   ADF
    glTexCoord2f(tx_0,ty_1); glVertex3i( A, D, E);  //Top left:    ADE

    //B
    tx_0 = blockInfo[block.blockID].tx[1];
    tx_1 = blockInfo[block.blockID].tx[1] + tmr;
    ty_0 = blockInfo[block.blockID].ty[1] + tmr;
    ty_1 = blockInfo[block.blockID].ty[1];
    glTexCoord2f(tx_0,ty_0); glVertex3i( B, C, F);  //Lower left:  BCF
    glTexCoord2f(tx_1,ty_0); glVertex3i( B, C, E);  //Lower right: BCE
    glTexCoord2f(tx_1,ty_1); glVertex3i( B, D, E);  //Top right:   BDE
    glTexCoord2f(tx_0,ty_1); glVertex3i( B, D, F);  //Top left:    BDF
    
    //C
    tx_0 = blockInfo[block.blockID].tx[2];
    tx_1 = blockInfo[block.blockID].tx[2] + tmr;
    ty_0 = blockInfo[block.blockID].ty[2] + tmr;
    ty_1 = blockInfo[block.blockID].ty[2];
    glTexCoord2f(tx_0,ty_0); glVertex3i( A, C, E);  //Lower left:  ACE
    glTexCoord2f(tx_1,ty_0); glVertex3i( B, C, E);  //Lower right: BCE
    glTexCoord2f(tx_1,ty_1); glVertex3i( B, C, F);  //Top right:   BCF
    glTexCoord2f(tx_0,ty_1); glVertex3i( A, C, F);  //Top left:    ACF
    
    //D
    tx_0 = blockInfo[block.blockID].tx[3];
    tx_1 = blockInfo[block.blockID].tx[3] + tmr;
    ty_0 = blockInfo[block.blockID].ty[3] + tmr;
    ty_1 = blockInfo[block.blockID].ty[3];
    glTexCoord2f(tx_0,ty_0); glVertex3i( A, D, F);  //Lower left:  ADF
    glTexCoord2f(tx_1,ty_0); glVertex3i( B, D, F);  //Lower right: BDF
    glTexCoord2f(tx_1,ty_1); glVertex3i( B, D, E);  //Top right:   BDE
    glTexCoord2f(tx_0,ty_1); glVertex3i( A, D, E);  //Top left:    ADE
    
    //E
    tx_0 = blockInfo[block.blockID].tx[4];
    tx_1 = blockInfo[block.blockID].tx[4] + tmr;
    ty_0 = blockInfo[block.blockID].ty[4] + tmr;
    ty_1 = blockInfo[block.blockID].ty[4];
    glTexCoord2f(tx_0,ty_0); glVertex3i( B, C, E);  //Lower left:  BCE
    glTexCoord2f(tx_1,ty_0); glVertex3i( A, C, E);  //Lower right: ACE
    glTexCoord2f(tx_1,ty_1); glVertex3i( A, D, E);  //Top right:   ADE
    glTexCoord2f(tx_0,ty_1); glVertex3i( B, D, E);  //Top left:    BDE
    
    //F
    tx_0 = blockInfo[block.blockID].tx[5];
    tx_1 = blockInfo[block.blockID].tx[5] + tmr;
    ty_0 = blockInfo[block.blockID].ty[5] + tmr;
    ty_1 = blockInfo[block.blockID].ty[5];
    glTexCoord2f(tx_0,ty_0); glVertex3i( A, C, F);  //Lower left:  ACF
    glTexCoord2f(tx_1,ty_0); glVertex3i( B, C, F);  //Lower right: BCF
    glTexCoord2f(tx_1,ty_1); glVertex3i( B, D, F);  //Top right:   BDF
    glTexCoord2f(tx_0,ty_1); glVertex3i( A, D, F);  //Top left:    ADF
  }
    //Draw
    glEnd();
}

//Draw blocks in chunk list
void drawChunks()
{

    //Iterator for moving through chunk list
    vector<mc__Chunk>::const_iterator iter;
    
    //Offset in block array of chunk
    size_t index;
    
    //Keep track of block coordinates for each chunk in block
    GLint off_x, off_y, off_z, x, y, z;
    
    //For each chunk...
    for (iter = chunks.begin(); iter != chunks.end(); iter++)
    {
        //Get the chunk
        const mc__Chunk& myChunk = *iter;
        index=0;
        
        //Draw every block in chunk.  x,y,z determined by position in array.
        for (off_z=0, z=myChunk.Z; off_z <= myChunk.size_Z; off_z++, z++) {
        for (off_y=0, y=myChunk.Y; off_y <= myChunk.size_Y; off_y++, y++) {
        for (off_x=0, x=myChunk.X; off_x <= myChunk.size_X; off_x++, x++)
        {
            drawBlock( myChunk.block_array[index], x, y, z);
            index++;
        }}}
    }
}



//Set up buffer, perspective, blah blah blah
void startOpenGL() {
    //OPENGL setup in SFML
    //Set color and depth clear value
    glClearDepth(1.0f);
    glClearColor(0.f, 0.f, 0.f, 1.f);

    // Enable Z-buffer read and write
    glEnable(GL_DEPTH_TEST);
    glDepthMask(GL_TRUE);

    //Enable texture mapping
    glViewport(0, 0, 800, 600);
    glEnable(GL_TEXTURE_2D);
    
    //Transparency in textures
    glEnable(GL_ALPHA_TEST);
    glAlphaFunc(GL_GREATER, 0.5f);

    // Setup perspective projection
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    //gluPersective( Field of View Y, aspect ratio, near clip distance, far clip distance)
    gluPerspective(90.f, 1.f, 1.f, 512.f);

    //Move camera
    glMatrixMode(GL_MODELVIEW);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
    glLoadIdentity();

    glPushMatrix(); //Save the original viewpoint

    //Create OpenGL texture
    glGenTextures(1, &image);
    glBindTexture(GL_TEXTURE_2D, image);    //bind empty texture
    
    //Set texture zoom filters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);  //GL_LINEAR?
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

}

//Load image file using DevIL
ILuint loadImageFile( const string &imageFilename) {
  
    ILuint il_texture;
  
    ilGenImages(1, &il_texture);
    ilBindImage(il_texture);

    if (ilLoad(IL_PNG, imageFilename.c_str())) {
      
        //Convert image to RGBA
        if (!ilConvertImage(IL_RGBA, IL_UNSIGNED_BYTE)) {
            cerr << "Error converting " << imageFilename << endl;
            ILenum ilErrorCode = 0;
            while ( (ilErrorCode = ilGetError()) != IL_NO_ERROR) {
                cerr << "Error code " << ilErrorCode << endl;
            }
            return 0;
        }
    
    } else {
        cerr << "Error loading " << imageFilename << endl;
        ILenum ilErrorCode = 0;
        while ( (ilErrorCode = ilGetError()) != IL_NO_ERROR) {
            cerr << "Error code " << ilErrorCode << endl;
        }
        return 0;
    }

    return il_texture;
}

//Process user input
bool handleSfEvent( const sf::Event& Event )
{
    bool result=true;
    
    switch( Event.Type) {
      
        //Window resize
        case sf::Event::Resized:
            glViewport(0,0, Event.Size.Width, Event.Size.Height);
            break;
        
        //Key pressed
        case sf::Event::KeyPressed:
            switch ( Event.Key.Code) {
                case sf::Key::Escape:       //Quit
                    result = false;
                    break;
                case sf::Key::Up:           //Choose texture
                    //texmap_selected = (texmap_selected - texmap_TILES) % texmap_TILE_MAX;
                    //chooseTexture(ilTextureList, texmap_selected);
                    break;
                case sf::Key::Down:         //Choose texture
                    //texmap_selected = (texmap_selected + texmap_TILES) % texmap_TILE_MAX;
                    //chooseTexture(ilTextureList, texmap_selected);
                    break;
                case sf::Key::Left:         //Choose texture
                    //texmap_selected = (texmap_selected - 1) % texmap_TILE_MAX;
                    //chooseTexture(ilTextureList, texmap_selected);
                    break;
                case sf::Key::Right:        //Choose texture
                    //texmap_selected = (texmap_selected + 1) % texmap_TILE_MAX;
                    //chooseTexture(ilTextureList, texmap_selected);
                    break;
                default:
                    break;
            }
            break;
            
        //Mousewheel scroll
        case sf::Event::MouseWheelMoved: {
            //Change camera position (in increments of 16).  Down zooms out.
            GLint move_Z = (Event.MouseWheel.Delta << 4);
            glTranslatef(0, 0, move_Z);
            break;
        }
        
        //Mouse clicks
        case sf::Event::MouseButtonPressed:
        
            //Remember where this mouse button was pressed
            mouse_press[Event.MouseButton.Button] = true;
            mouse_press_X[Event.MouseButton.Button] = mouse_X;
            mouse_press_Y[Event.MouseButton.Button] = mouse_Y;
            
            
            //Reset camera if middle mouse button pressed
            switch( Event.MouseButton.Button ) {
                case sf::Mouse::Middle:
                    glLoadIdentity();
                    glTranslatef( camera_X, camera_Y, camera_Z);
                    break;
                default:
                    break;
            }
            break;
            
        //Mouse un-clicked
        case sf::Event::MouseButtonReleased:
        
            //Forget that this mouse button was pressed
            mouse_press[Event.MouseButton.Button] = false;
            break;
        
        //Mouse move
        case sf::Event::MouseMoved:

            //Get latest position from event
            mouse_X = Event.MouseMove.X;
            mouse_Y = Event.MouseMove.Y;

            //Translate camera if moved while holding left button
            if (mouse_press[sf::Mouse::Left]) {

                //Move camera if total X motion was >= 16
                GLint diff_X = (mouse_press_X[sf::Mouse::Left] - mouse_X)/16;
                if (diff_X != 0) {
                    glTranslatef((diff_X << 4), 0, 0);
                    
                    //Save new mouse position
                    mouse_press_X[sf::Mouse::Left] = mouse_X;
                }
                
                //Move camera if total Y motion was >= 16
                GLint diff_Y = (mouse_Y - mouse_press_Y[sf::Mouse::Left])/16;
                if (diff_Y != 0) {
                    glTranslatef(0, diff_Y << 4,0);
                    
                    //Save new mouse position
                    mouse_press_Y[sf::Mouse::Left] = mouse_Y;
                }
            }

            //Rotate camera if mouse moved while holding right button
            if (mouse_press[sf::Mouse::Right]) {
              
                //Use change in X position to rotate about Y-axis
                glRotatef(mouse_press_X[sf::Mouse::Right] - mouse_X, 0, 1, 0);

                //Don't rotate about X-axis
                //glRotatef(mouse_Y - mouse_press_Y[sf::Mouse::Right], 1, 0, 0);
                
                //Update mouse press position for right mouse button
                mouse_press_X[sf::Mouse::Right] = mouse_X;
                mouse_press_Y[sf::Mouse::Right] = mouse_Y;
            }
            break;
        //Unhandled events
        default:
            break;
    }
    
    return result;
}

//OpenGL rendering of cubes.  No update to camera.
bool drawWorld()
{
    //Erase openGL world
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

    //Draw the loaded chunks
    drawChunks();
    
    mc__Block block1 = {2, 0, 0};   //Grass
    drawBlock( block1, 0, 0, 2);
    mc__Block block2 = {19, 0, 0};    //Leaves
    drawBlock( block2, 2, 2, 2);

    return true;
}

//Copy block info to struct
void setBlockInfo( uint8_t index,
    uint8_t A, uint8_t B,
    uint8_t C, uint8_t D,
    uint8_t E, uint8_t F,
    uint8_t properties)
{
    //A - F contain texture ID of the corresponding face (textureID = 0 - 255)

    //For each face,  precompute OpenGL texture offsets in texture map
    // (Remember that texture map was loaded upside down!)
    blockInfo[index].textureID[0] = A;
    blockInfo[index].tx[0] = float(A & (texmap_TILES-1))/((float)texmap_TILES);
    blockInfo[index].ty[0] = float(A/texmap_TILES)/((float)texmap_TILES);

    blockInfo[index].textureID[1] = B;
    blockInfo[index].tx[1] = float(B & (texmap_TILES-1))/((float)texmap_TILES);
    blockInfo[index].ty[1] = float(B/texmap_TILES)/((float)texmap_TILES);

    blockInfo[index].textureID[2] = C;
    blockInfo[index].tx[2] = float(C & (texmap_TILES-1))/((float)texmap_TILES);
    blockInfo[index].ty[2] = float(C/texmap_TILES)/((float)texmap_TILES);

    blockInfo[index].textureID[3] = D;
    blockInfo[index].tx[3] = float(D & (texmap_TILES-1))/((float)texmap_TILES);
    blockInfo[index].ty[3] = float(D/texmap_TILES)/((float)texmap_TILES);

    blockInfo[index].textureID[4] = E;
    blockInfo[index].tx[4] = float(E & (texmap_TILES-1))/((float)texmap_TILES);
    blockInfo[index].ty[4] = float(E/texmap_TILES)/((float)texmap_TILES);
    
    blockInfo[index].textureID[5] = F;
    blockInfo[index].tx[5] = float(F & (texmap_TILES-1))/((float)texmap_TILES);
    blockInfo[index].ty[5] = float(F/texmap_TILES)/((float)texmap_TILES);
    
    //Assign properties
        //Bytes 0-3: Glow  : 0-7=How much light it emits
        //Byte  4  : Shape : 0=cube, 1=object
        //Byte  5  : Glass : 0=opqaue, 1=see-through
        //Bytes 6-7: State : 0=solid, 1=loose, 2=liquid, 3=gas
    blockInfo[index].properties = properties;
}

//Map block ID to block type information
bool loadBlockInfo()
{
    uint8_t blockID;

    //Set all block information to defaults
    for (blockID = 0; blockID != 0xFF; blockID++) {
        setBlockInfo( blockID, blockID, blockID, blockID, blockID, blockID, blockID, 0);
    }

/*  Block geometry

   ADE ---- BDE
   /.       /|
  / .      / |
ADF ---- BDF |
 | ACE . .| BCE
 | .      | /
 |.       |/
ACF ---- BCF

//Properties
0xF0: Shape : 0=cube, 1=stairs, 2=half-cube, 3=fence, 4=door, 5=wall, 6=floor
0x08: Type  : 0=cube, 1=object
0x04: Vision: 0=opqaue, 1=see-through
0x03: State : 0=solid, 1=loose, 2=liquid, 3=gas

Planted item = 0x0F: object, see-through, move-through
Normal block = 0x00: cube, opaque, solid
*/

    //Set specific blocks
    setBlockInfo( 0, 11, 11, 11, 11, 11, 11, 0x07);         //Air
    setBlockInfo( 1, 1, 1, 1, 1, 1, 1, 0);                  //Stone
    setBlockInfo( 2, 3, 3, 2, 0, 3, 3, 0);                  //Grass
    setBlockInfo( 3, 2, 2, 2, 2, 2, 2, 0);                  //Dirt
    setBlockInfo( 4, 16, 16, 16, 16, 16, 16, 0);            //Cobblestone
    setBlockInfo( 5, 4, 4, 4, 4, 4, 4, 0);                  //Wood
    setBlockInfo( 6, 15, 15, 15, 15, 15, 15, 0x0F);         //Sapling
    setBlockInfo( 7, 17, 17, 17, 17, 17, 17, 0);            //Bedrock
    setBlockInfo( 8, 0xCD, 0xCD, 0xCD, 0xCD, 0xCD, 0xCD, 0x06);     //Water
    setBlockInfo( 9, 0xCD, 0xCD, 0xCD, 0xCD, 0xCD, 0xCD, 0x06);     //Water
    setBlockInfo( 10, 0xED, 0xED, 0xED, 0xED, 0xED, 0xED, 0x02);    //Lava
    setBlockInfo( 11, 0xED, 0xED, 0xED, 0xED, 0xED, 0xED, 0x02);    //Lava
    setBlockInfo( 12, 18, 18, 18, 18, 18, 18, 0x01);        //Sand
    setBlockInfo( 13, 19, 19, 19, 19, 19, 19, 0x01);        //Gravel
    setBlockInfo( 14, 32, 32, 32, 32, 32, 32, 0);           //GoldOre
    setBlockInfo( 15, 33, 33, 33, 33, 33, 33, 0);           //IronOre
    setBlockInfo( 16, 34, 34, 34, 34, 34, 34, 0);           //CoalOre
    setBlockInfo( 17, 20, 20, 21, 21, 20, 20, 0);           //Log
    setBlockInfo( 18, 48, 48, 48, 48, 48, 48, 0);           //Sponge
    setBlockInfo( 19, 52, 52, 52, 53, 52, 52, 0x04);        //Leaves
    setBlockInfo( 20, 49, 49, 49, 49, 49, 49, 0x04);        //Glass
    for (blockID = 21; blockID < 37; blockID++) {
        setBlockInfo( blockID, 64, 64, 64, 64, 64, 64, 0);  //Cloth (color?)
    }
    setBlockInfo( 37, 13, 0, 0, 0, 0, 0, 0x0F);         //Flower
    setBlockInfo( 38, 12, 0, 0, 0, 0, 0, 0x0F);         //Rose
    setBlockInfo( 39, 29, 0, 0, 0, 0, 0, 0x0F);         //BrownShroom
    setBlockInfo( 40, 28, 0, 0, 0, 0, 0, 0x0F);         //RedShroom
    setBlockInfo( 41, 39, 39, 55, 23, 39, 39, 0);       //GoldBlock
    setBlockInfo( 42, 38, 38, 54, 22, 38, 38, 0);       //IronBlock
    setBlockInfo( 43, 5, 5, 6, 6, 5, 5, 0);       //DoubleStep
    setBlockInfo( 44, 5, 5, 6, 6, 5, 5, 0x20);       //Step
    setBlockInfo( 45, 7, 7, 7, 7, 7, 7, 0);       //Brick
    setBlockInfo( 46, 8, 8, 10, 9, 8, 8, 0);       //TNT
    setBlockInfo( 47, 35, 35, 4, 4, 35, 35, 0);       //Bookshelf
    setBlockInfo( 48, 36, 36, 16, 36, 36, 36, 0);      //Mossy
    setBlockInfo( 49, 37, 37, 37, 37, 37, 37, 0);            //Obsidian
    setBlockInfo( 50, 80, 80, 0, 0, 0, 0, 0x0F);   //Torch
    
    return true;
}

//Load the list of chunks to vector
bool loadChunks(vector<mc__Chunk>& chunkList) {
    
    const GLint X=-8, Y=-8, Z=0;
    const uint8_t size_X=15, size_Y=31, size_Z=0;
    const uint32_t array_len = (size_X+1)*(size_Y+1)*(size_Z+1);
    
    //Allocate array of blocks
    mc__Block *firstBlockArray = new mc__Block[array_len];


    //Assign blocks
    size_t index, x, y;
    index=0;
    for (y=0; y <= size_Y; y++) {
        for (x=0; x <= size_X; x++) {
            firstBlockArray[index].blockID = ( (y&1) ? 0 : ((((y>>1)<<4)^0xF0)| (x&0x0F)) );
            index++;
        }
    }
    /*
    for (index = 0; index < array_len; index++) {
        if ((index >> 4)& 1) {
            firstBlockArray[index].blockID = ((index ^ 0xF0)|(index&0x0F));
        } else {
            firstBlockArray[index].blockID = 0;
        }
    }
    */
    
    //Create chunk
    mc__Chunk firstChunk = { X, Y, Z, size_X, size_Y, size_Z, array_len, firstBlockArray};
    chunkList.push_back(firstChunk);
    
    return true;
}

int main()
{
    //Compare devIL DLL version to header version
    if (ilGetInteger(IL_VERSION_NUM) < IL_VERSION) {
        cerr << "devIL wrong DLL version" << endl;
        return 1;
    }


    //Load game block information
    loadBlockInfo();
    
    //Load game chunk(s)
    loadChunks( chunks);
    
    //Extract chunks to individual blocks?
  
    //SFML variables
    sf::Clock Clock;
    sf::WindowSettings Settings;
    Settings.DepthBits = 24;
    Settings.StencilBits = 8;
    Settings.AntialiasingLevel = 2;

    //Create the program window, with settings
    sf::RenderWindow App(sf::VideoMode(800, 600, 32),
        "SFML OpenGL", sf::Style::Close, Settings);

    //Initialize OpenGL
    startOpenGL();

    //Start DevIL
    ilInit();

    //Load terrain texture map, bind it to current DevIL image
    il_texture_map = loadImageFile(texture_map_file);
    if (il_texture_map == 0) {
        return 0;   //error, exit program
    }
    //The PNG file is "flipped" in memory relative to OpenGL coordinates.
    
    //glBind texture before assigning it
    glBindTexture(GL_TEXTURE_2D, image);
    
    //Copy current DevIL image to OpenGL image.  
    glTexImage2D(GL_TEXTURE_2D, 0, ilGetInteger(IL_IMAGE_BPP),
        ilGetInteger(IL_IMAGE_WIDTH), ilGetInteger(IL_IMAGE_HEIGHT), 0,
        ilGetInteger(IL_IMAGE_FORMAT), GL_UNSIGNED_BYTE, ilGetData());


    //Change camera to model view mode
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    //Move camera to starting position
    glTranslatef(camera_X, camera_Y, camera_Z);

    //Initial drawing
    App.SetActive();
    drawWorld();
    App.Display();
    sf::Sleep(0.01f);

    //Cube location variables: TODO

    //Event loop    
    sf::Event Event;
    bool Running = true;
    bool something_happened;
    while (Running && App.IsOpened()) {
      
        //Set window
        App.SetActive();

        //Check events  
        something_happened=false;
        while (App.GetEvent(Event))
        {
            if (!handleSfEvent(Event)) {
                //Stop running when Esc is pressed
                Running = false;
            }
            something_happened=true;
        }
        
        //Update the world if something happened
        if (something_happened) {
            drawWorld();
        }

        //Display the rendered frame
        App.Display();
        
        //Sleep some to decrease CPU usage
        sf::Sleep(0.01f);
    }
    return 0;
}