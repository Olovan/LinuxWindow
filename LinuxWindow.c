#include <stdio.h>      //console output
#include <stdlib.h>     //exit function
#include <X11/X.h>      //Window Stuff
//include <X11/Xlib.h>   //More Window Stuff
#include <GL/gl.h>      //OpenGL core functions
#include <GL/glu.h>     //OpenGL utility functions
#include <GL/glx.h>     //OpenGL Window Stuff

Display               *display;    //Keep track of which computer we're targetting
Window                 root;       //Base Window which has the exit button and Titlebar
Window                 win;        //The Window we draw inside of
XVisualInfo           *vi;         //Visual Info that matches the attributes we want
Colormap               cmap;       //Window's color map
XSetWindowAttributes   swa;        //Struct filled with Window attribs we can assign
GLXContext             glContext;  //Our OpenGL context
XWindowAttributes      winAttribs; //Struct we fill when we query the state of the Window
XEvent                 xevent;     //Used to query window for events

//OpenGL Context Attributes we want
GLint     desiredAttribs[] = { GLX_RGBA,    
                               GLX_DEPTH_SIZE, 24, 
                               GLX_DOUBLEBUFFER, 
                               None }; 

void renderTriangle()
{
    glBegin(GL_TRIANGLES); //Draw Triangle
        glColor3f(1, 0, 0);
        glVertex2f(-1, -1);

        glColor3f(0, 1, 0);
        glVertex2f(0, 1);

        glColor3f(0, 0, 1);
        glVertex2f(1, -1);
    glEnd();
}

int main(int argc, char *argv[])
{
    display = XOpenDisplay(NULL); //Get our current display
    if(display == NULL) { printf("X Server Crapped Out\n"); exit(0);} //Make sure it worked

    root = DefaultRootWindow(display); //Make our Base window

    vi = glXChooseVisual(display, 0, desiredAttribs); //Find a Visual ID that has what we want
    if( vi == NULL ) { printf("Desired Visual Attributes not supported\n"); exit(0); } //Make sure it worked

    cmap = XCreateColormap(display, root, vi->visual, AllocNone);
    swa.colormap    =  cmap;
    swa.event_mask  =  ExposureMask | KeyPressMask; //Only Events we care about

    win = XCreateWindow(display,                   //Which Computer
                        root,                      //Parent Window
                        0, 0,                      //Topleft corner pos (relative)
                        800, 600,                  //Width and Height
                        0,                         //Border Width
                        vi->depth,                 //Window Depth
                        InputOutput,               //Window Type
                        vi->visual,                //Visual Info Stuff like Color Depth and such
                        CWColormap | CWEventMask,  //Tell it which fields we have filled out ourselves
                        &swa);                     //Pointer to attributes

    XMapWindow(display, win); //Show the Window
    XStoreName(display, win, "Linux Window Example"); //Set Window Title
    
    glContext = glXCreateContext(display, vi, NULL, GL_TRUE); //Create OpenGL Context
    glXMakeCurrent(display, win, glContext); //Make it our current OpenGL Context

    int windowShouldClose = 0;
    while( !windowShouldClose ) //Event Loop
    {
        XNextEvent(display, &xevent); //Blocks until we get an event

        switch(xevent.type)
        {
            case Expose: //Called Once Every Monitor Refresh
                XGetWindowAttributes(display, win, &winAttribs);
                glViewport(0, 0, winAttribs.width, winAttribs.height); //Fill up full window size
                glClear(GL_COLOR_BUFFER_BIT);
                renderTriangle();
                glXSwapBuffers(display, win);
                break;

            case KeyPress: //Key was Pressed
                if( xevent.xkey.keycode == 0x09 )   //Escape Key Pressed
                    windowShouldClose = 1;

                printf("Pressed Keycode: %x\n", xevent.xkey.keycode);
                break;

            default:
                break;
        }
    }

    //CLEANUP
    glXMakeCurrent(display, None, NULL); //Unassign OpenGL Context
    glXDestroyContext(display, glContext); //Destroy OpenGL Context
    XDestroyWindow(display, win); //Destroy Window
    XCloseDisplay(display); //Close Display

    return 0;
}
