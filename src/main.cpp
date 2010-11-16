//SFML test app

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

//Constants
const string imageFilename("terrain.png");

//openGL image
GLuint image;

//Drawing state variables
size_t texmap_X = 1;
size_t texmap_Y = 3;

GLint camera_X = 0;
GLint camera_Y = 0;
GLint camera_Z = -96;

//Input state variables
bool mouse_press[sf::Mouse::ButtonCount];
int mouse_press_X[sf::Mouse::ButtonCount];
int mouse_press_Y[sf::Mouse::ButtonCount];
int mouse_X, mouse_Y;

//DevIL globals
ILuint il_texture_map;
const size_t texmap_SIZE = 16;

//Dump DevIL texture info
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

//draw cube faces for cube at block address (x,y,z)
//Call this inside "glBegin(), glEnd()"
void drawCubes(GLint x, GLint y, GLint z) {
    
    //Draw cube
    
    //x axis: A = -8, B = +8
    //y axis: C = -8, D = +8
    //z axis: E = -8, F = +8
    
    GLint A = (x << 4) - 8;
    GLint B = (x << 4) + 8;
    GLint C = (y << 4) - 8;
    GLint D = (y << 4) + 8;
    GLint E = (z << 4) - 8;
    GLint F = (z << 4) + 8;
    
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

    //Set zoom filters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);  //GL_LINEAR?
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

    ilDeleteImages(1, &il_blocktex);  //free memory used by DevIL
    return il_blocktex;
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
}


//Load the image file using DevIL
ILuint loadImageFile() {
  
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

    //outputRGBAData();

    return il_texture;
}

//Respond to user input
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
                    texmap_Y = (texmap_Y - 1) % 16;
                    blitTexture( il_texture_map, texmap_X * texmap_SIZE,
                        texmap_Y * texmap_SIZE, texmap_SIZE, texmap_SIZE);
                    break;
                case sf::Key::Down:         //Choose texture
                    texmap_Y = (texmap_Y + 1) % 16;
                    blitTexture( il_texture_map, texmap_X * texmap_SIZE,
                        texmap_Y * texmap_SIZE, texmap_SIZE, texmap_SIZE);
                    break;
                case sf::Key::Left:         //Choose texture
                    texmap_X = (texmap_X - 1) % 16;
                    blitTexture( il_texture_map, texmap_X * texmap_SIZE,
                        texmap_Y * texmap_SIZE, texmap_SIZE, texmap_SIZE);
                    break;
                case sf::Key::Right:        //Choose texture
                    texmap_X = (texmap_X + 1) % 16;
                    blitTexture( il_texture_map, texmap_X * texmap_SIZE,
                        texmap_Y * texmap_SIZE, texmap_SIZE, texmap_SIZE);
                    break;
                default:
                    break;
            }
            break;
            
        //Mousewheel
        case sf::Event::MouseWheelMoved: {
            //Change camera position
            GLint move_Z = (Event.MouseWheel.Delta << 4);
            //camera_Z += move_Z;
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


int main()
{
    //Compare devIL DLL version to header version
    if (ilGetInteger(IL_VERSION_NUM) < IL_VERSION) {
        cerr << "devIL wrong DLL version" << endl;
        return 1;
    }
  
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
    
    //Copy alpha channel when blitting, don't blend it.
    ilDisable(IL_BLIT_BLEND);

    //Load terrain image map
    il_texture_map = loadImageFile();
    if (il_texture_map == 0) {
        return 0;   //error, exit program
    }
    
    //Pick a texture
    blitTexture( il_texture_map, texmap_X * texmap_SIZE, texmap_Y * texmap_SIZE,
        texmap_SIZE, texmap_SIZE);
        
    //Change camera to model view mode
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    //Move camera to starting position
    glTranslatef(camera_X, camera_Y, camera_Z);

    //Event loop    
    sf::Event Event;
    bool Running = true;
    while (Running && App.IsOpened()) {
      
        //Set window
        App.SetActive();

        //Erase screen
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

/*        
        //Rotate camera
        glRotatef(Clock.GetElapsedTime() * 2, 1, 0, 0);    //yaw
        glRotatef(Clock.GetElapsedTime() * 30, 0, 1, 0);    //Rotate about Y axis
        //glRotatef(Clock.GetElapsedTime() * 90, 0, 0, 1);    //roll
*/

/*
        // Draw some text on top of our OpenGL object
        sf::String Text("This is a rotating cube");
        Text.SetPosition(250.f, 300.f);
        Text.SetColor(sf::Color(128, 128, 128));
        App.Draw(Text);
*/

        //Draw quads
        glBegin(GL_QUADS);
        drawCubes(0, 0, 0);
        drawCubes(0, 1, 0);
        drawCubes(0, 2, 0);
        drawCubes(0, 3, 0);
        drawCubes(0, 4, 0);
        
        drawCubes(1, 2, 0);
        drawCubes(1, 4, 0);

        drawCubes(2, 0, 0);
        drawCubes(2, 1, 0);
        drawCubes(2, 2, 0);
        drawCubes(2, 3, 0);
        drawCubes(2, 4, 0);

        glEnd();

        //Display the rendered frame
        App.Display();

        //Check events  
        while (App.GetEvent(Event))
        {
            if (!handleSfEvent(Event)) {
                //Stop running when Esc is pressed
                Running = false;
            }
        }
    }
    return 0;
}