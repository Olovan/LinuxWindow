#include <stdio.h>      //console output
#include <stdlib.h>     //exit function
#include <time.h>       //nanosleep
#include <math.h>       //sin, cos
#include <X11/X.h>      //Window Stuff
#include <X11/Xlib.h>   //More Window Stuff
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



float modelMatrix[16] = {1, 0, 0, 0,
                         0, 1, 0, 0,
                         0, 0, 1, 0,
                         0, 0, 0, 1};

//OpenGL Context Attributes we want
GLint     desiredAttribs[] = { GLX_RGBA,    
    GLX_DEPTH_SIZE, 24, 
    GLX_DOUBLEBUFFER, 
    None }; 

void renderTriangle(float angle)
{
    glMatrixMode(GL_MODELVIEW);

    //Update Model Matrix
    glPushMatrix(); 
    glLoadMatrixf(modelMatrix);
    glRotatef(sin(angle / 180), 0, 0, 1);
    glRotatef(cos(angle / 120), 0, 1, 0);
    glRotatef(sin(angle / 60), 1, 0, 0);
    glGetFloatv(GL_MODELVIEW_MATRIX, modelMatrix);
    glPopMatrix();

    glPushMatrix();
    glMultMatrixf(modelMatrix);

    glBegin(GL_TRIANGLE_FAN); //Draw Pyramid
    glColor3f(cos(angle / 40), sin(angle / 30) / 2 + .5, 1); //Vert 1
    glVertex3f(-.5, -.333, -.333);

    glColor3f(1, sin((angle + 70)/45), cos(angle / 80)); //Vert 2
    glVertex3f(0, .533, -.333);

    glColor3f(sin(angle / 30), cos(angle / 50), 1);  //Vert 3
    glVertex3f(.5, -.333, -.333);

    glColor3f(sin((angle + 90)/40), 1, cos(angle / 60));  //Vert 4
    glVertex3f(0, 0, .533);

    glColor3f(1, sin((angle + 70)/45), cos(angle / 80)); //Vert 2
    glVertex3f(0, .533, -.333);
    glEnd();

    glBegin(GL_TRIANGLES); //Cover up the Open Side of the Triangle
    glColor3f(1, sin((angle + 70)/45), cos(angle / 80)); //Vert 2
    glVertex3f(0, .533, -.333);

    glColor3f(sin(angle / 30), cos(angle / 50), 1);  //Vert 3
    glVertex3f(.5, -.333, -.333);

    glColor3f(sin((angle + 90)/40), 1, cos(angle / 60));  //Vert 4
    glVertex3f(0, 0, .533);
    glEnd();
}

int main(int argc, char *argv[])
{
    display = XOpenDisplay(NULL); //Get our current display
    if(display == NULL) { printf("X Server Crapped Out\n"); exit(0);} //Make sure it worked

    root = DefaultRootWindow(display); //Make our Base window

    //Non-Transparent Test
    //vi = glXChooseVisual(display, 0, desiredAttribs); //Find a Visual ID that has what we want

    //TRANSPARENT WINDOW TEST
    XVisualInfo transparentInfo;
    vi = &transparentInfo;
    XMatchVisualInfo(display, DefaultScreen(display), 32, TrueColor, vi);

    if( vi == NULL ) { printf("Desired Visual Attributes not supported\n"); exit(0); } //Make sure it worked

    cmap = XCreateColormap(display, root, vi->visual, AllocNone);
    swa.colormap    =  cmap;
    swa.event_mask  =  ExposureMask | KeyPressMask; //Only Events we care about
    swa.background_pixmap = None;                   //Required for Transparency
    swa.border_pixel = 0;                           //Required for Transparency

    win = XCreateWindow(display,                   //Which Computer
            root,                      //Parent Window
            0, 0,                      //Topleft corner pos (relative)
            800, 800,                  //Width and Height
            0,                         //Border Width
            vi->depth,                 //Window Depth
            InputOutput,               //Window Type
            vi->visual,                //Visual Info Stuff like Color Depth and such
            CWColormap | CWEventMask | CWBackPixmap | CWBorderPixel,  //Tell it which fields we have filled out ourselves
            &swa);                     //Pointer to attributes

    XMapWindow(display, win); //Show the Window
    XStoreName(display, win, "Linux Window Example"); //Set Window Title

    glContext = glXCreateContext(display, vi, NULL, GL_TRUE); //Create OpenGL Context
    glXMakeCurrent(display, win, glContext); //Make it our current OpenGL Context

    //Tell the Window Manager that we want to know about close events
    Atom wmCloseEvent = XInternAtom(display, "WM_DELETE_WINDOW", False); //Find the Atom identify the close event
    XSetWMProtocols(display, win, &wmCloseEvent, 1); //Register that we care about the close event

    //OpenGL Settings
    glEnable(GL_BLEND);
    glEnable(GL_DEPTH_TEST);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glClearColor(0, 0, 0, 0);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    gluLookAt(0, 0, 4.5, 0, 0, 0, 0, 1, 0);

    int windowShouldClose = 0;
    float angle = 0;
    while( !windowShouldClose ) //Event Loop
    {

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        renderTriangle(angle++);
        glXSwapBuffers(display, win);

        //Process Events if there are any to process
        while(XPending(display) > 0)
        {
            XNextEvent(display, &xevent); //Blocks until we get an event
            switch(xevent.type)
            {
                case Expose: //Called When Window Resized or damaged
                    XGetWindowAttributes(display, win, &winAttribs);
                    glViewport(0, 0, winAttribs.width, winAttribs.height); //Fill up full window size
                    glMatrixMode(GL_PROJECTION);
                    glLoadIdentity();
                    gluPerspective(30, (double)winAttribs.width/winAttribs.height, .01, 10);
                    break;

                case KeyPress: //Key was Pressed
                    if( xevent.xkey.keycode == 0x09 )   //Escape Key Pressed
                        windowShouldClose = 1;

                    //printf("Pressed Keycode: %x\n", xevent.xkey.keycode);
                    break;

                case ClientMessage:
                    if((Atom)xevent.xclient.data.l[0] == wmCloseEvent)
                        windowShouldClose = 1;
                default:
                    break;
            }
        }

        //Sleep
        struct timespec waitTime, remainingTime;
        waitTime.tv_nsec = (double)1/60 * 1e9; //Limit to 60 FPS
	waitTime.tv_sec = 0;
        nanosleep(&waitTime, &remainingTime);
    }

    //CLEANUP
    glXMakeCurrent(display, None, NULL); //Unassign OpenGL Context
    glXDestroyContext(display, glContext); //Destroy OpenGL Context
    XDestroyWindow(display, win); //Destroy Window
    XCloseDisplay(display); //Close Display

    return 0;
}
