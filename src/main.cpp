#ifdef _WIN32
#include <Windows.h>
#endif

#include "sphere.h"
#include "YarnBall.h"
#include "AnimatedGifSaver.h"

#include <GL/glut.h>
#include <IL/il.h>
#include <IL/ilu.h>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>
#include <glm/mat4x4.hpp> // glm::mat4
#include <glm/ext/matrix_transform.hpp> // glm::translate, glm::rotate, glm::scale
#include <glm/ext/matrix_clip_space.hpp> // glm::perspective
#include <glm/ext/scalar_constants.hpp> // glm::pi

#include <fstream>
#include <getopt.h>
#include <iomanip>
#include <iostream>
#include <sstream>

// constants
const int   SCREEN_WIDTH    = 500;
const int   SCREEN_HEIGHT   = 500;
const float CAMERA_DISTANCE = 4.0f;
const int   TEXT_WIDTH      = 8;
const int   TEXT_HEIGHT     = 13;

// global variables
int screenWidth;
int screenHeight;
bool mouseLeftDown;
bool mouseRightDown;
bool mouseMiddleDown;
int mouseX, mouseY;
float cameraAngleX;
float cameraAngleY;
float cameraDistance;
int drawMode;
int imageWidth;
int imageHeight;
bool ballRotating = false;

GLfloat modelViewMatrix[16]; 

std::string yarnballFile;
int subdivLevel = 0;
int lineWidth = 1;
YarnBall yarnBall;

float rad(float deg)
{
    return deg/180.0f*glm::pi<float>();
}

float deg(float rad)
{
    return rad*180.0f/glm::pi<float>();
}

glm::mat4 getPerspectiveMat()
{
    return glm::perspective(rad(40.0f), (float)(screenWidth)/screenHeight, 1.0f, 1000.0f);
}

glm::mat4 getViewMat()
{
	glm::mat4 view = glm::translate(glm::mat4(1.0f), glm::vec3(0, 0, -cameraDistance));
	view = glm::rotate(view, rad(cameraAngleX), glm::vec3(1.0f, 0.0f, 0.0f));
	view = glm::rotate(view, rad(cameraAngleY), glm::vec3(0.0f, 1.0f, 0.0f));
	view = glm::rotate(view, rad(-90), glm::vec3(1.0f, 0.0f, 0.0f));
    return view;
}

void displayCB()
{
    // clear buffer
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

    // save the initial ModelView matrix before modifying ModelView matrix
    glPushMatrix();

    // transform modelview matrix
    glTranslatef(0, 0, -cameraDistance);

    // set material
    float ambient[] = {0.0f, 0.0f, 0.0f, 1};
    float diffuse[] = {1.0f, 1.0f, 1.0f, 1};
    float specular[] = {1.0f, 1.0f, 1.0f, 1};
    float shininess = 128;
    glMaterialfv(GL_FRONT, GL_AMBIENT, ambient);
    glMaterialfv(GL_FRONT, GL_DIFFUSE, diffuse);
    glMaterialfv(GL_FRONT, GL_SPECULAR, specular);
    glMaterialf(GL_FRONT, GL_SHININESS, shininess);

    // draw right sphere with texture
    glRotatef(cameraAngleX, 1, 0, 0);
    glRotatef(cameraAngleY, 0, 1, 0);
    glRotatef(-90, 1, 0, 0);
    yarnBall.draw();

    glPopMatrix();
    glutSwapBuffers();
}

/**
  * set the projection matrix as perspective
 **/
void toPerspective()
{
    // set viewport to be the entire window
    glViewport(0, 0, (GLsizei)screenWidth, (GLsizei)screenHeight);

    // set perspective viewing frustum
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(40.0f, (float)(screenWidth)/screenHeight, 1.0f, 1000.0f); // FOV, AspectRatio, NearClip, FarClip

    // switch to modelview matrix in order to set scene
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}


void reshapeCB(int w, int h)
{
    screenWidth = w;
    screenHeight = h;
    toPerspective();
    std::cout << "window resized: " << w << " x " << h << std::endl;
}


AnimatedGifSaver saver(1,1);
bool savingAnimation = false;


void saveAnimationScreenShot()
{
    std::vector<unsigned char> frame(screenWidth*screenHeight*3);
    glReadPixels(0, 0, screenWidth, screenHeight, GL_RGB, GL_UNSIGNED_BYTE, &frame[0]);
    saver.AddFrame(&frame[0],0.033);
}

void saveSingleScreenShot(std::string fileName)
{
    std::cout << "Saving single screenshot" << std::endl;
    std::vector<unsigned char> frame(screenWidth*screenHeight*3);
    glReadPixels(0, 0, screenWidth, screenHeight, GL_RGB, GL_UNSIGNED_BYTE, &frame[0]);
    ILuint imgID;
    ilEnable(IL_FILE_OVERWRITE);
    ilGenImages(1, &imgID);
    ilBindImage(imgID);
    ilTexImage(screenWidth, screenHeight, 1, 3, IL_RGB, IL_UNSIGNED_BYTE, &frame[0]);
    ilSave(IL_PNG, fileName.c_str());
    ilDisable(IL_FILE_OVERWRITE);

    if(ilGetError() != IL_NO_ERROR) {
        std::cerr << "Error saving file " << fileName << ": " << iluErrorString(ilGetError()) << std::endl;
    }
}


void timerCB(int millisec)
{
    glutTimerFunc(millisec, timerCB, millisec);

    if (ballRotating) {
        cameraAngleY += 1.0;
        //        cameraAngleX += 0.5;
    }
    if (cameraAngleX > 360)
        cameraAngleX -= 360;
    if (cameraAngleX < 0)
        cameraAngleX += 360;
    if (cameraAngleY > 360)
        cameraAngleY -= 360;
    if (cameraAngleY < 0)
        cameraAngleY += 360;

    if (savingAnimation)
        saveAnimationScreenShot();


    glutPostRedisplay();
}


void keyboardCB(unsigned char key, int, int)
{
    switch (key) {
    case 27: // ESCAPE
//        clearSharedMem();
        exit(0);
        break;
    case 'r':
    case 'R':
        // rotate ball
        ballRotating = !ballRotating;
        break;
    case 's':
    case 'S':
        if (ballRotating) { 
            if (savingAnimation) {
                saver.Save("animation.gif");
                savingAnimation = false;
            } else {
                saver = AnimatedGifSaver(screenWidth, screenWidth);
                savingAnimation = true;
            }
        } else {
            saveSingleScreenShot("screenshot.png");
        }
        break;
    case 'd': // switch rendering modes (fill -> wire -> point)
    case 'D':
        ++drawMode;
        drawMode %= 3;
        if (drawMode == 0) {       // fill mode
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
            glEnable(GL_DEPTH_TEST);
            glEnable(GL_CULL_FACE);
        } else if (drawMode == 1) { // wireframe mode
            glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
            glDisable(GL_DEPTH_TEST);
            glDisable(GL_CULL_FACE);
        } else {                   // point mode
            glPolygonMode(GL_FRONT_AND_BACK, GL_POINT);
            glDisable(GL_DEPTH_TEST);
            glDisable(GL_CULL_FACE);
        }
        break;

    default:
        ;
    }
}

void resetCamera()
{
    cameraAngleX = 0.0f;
    cameraAngleY = 90.0f;
    cameraDistance = CAMERA_DISTANCE;
}

void loadColoredTris(std::string inFile, int subdivLevel, int lineWidth)
{
    yarnBall = YarnBall::fromFile(inFile, subdivLevel, lineWidth);
}

void mouseCB(int button, int state, int x, int y)
{
    mouseX = x;
    mouseY = y;

    if (button == GLUT_LEFT_BUTTON) {
        if (state == GLUT_DOWN) {
            mouseLeftDown = true;
        } else if (state == GLUT_UP) {
            mouseLeftDown = false;
        }
    }

    else if (button == GLUT_RIGHT_BUTTON) {
        if (state == GLUT_DOWN) {
            mouseRightDown = true;
        } else if (state == GLUT_UP) {
            mouseRightDown = false;
            loadColoredTris(yarnballFile, subdivLevel, lineWidth);
        }
    }

    else if (button == GLUT_MIDDLE_BUTTON) {
        if (state == GLUT_DOWN) {
            mouseMiddleDown = true;
        } else if (state == GLUT_UP) {
            mouseMiddleDown = false;
            resetCamera();
        }
    }
}


void mouseMotionCB(int x, int y)
{
    if (mouseLeftDown) {
        cameraAngleY += 0.2*(x - mouseX);
        cameraAngleX += 0.2*(y - mouseY);
        mouseX = x;
        mouseY = y;
    }

    // if (mouseRightDown) {
    //     cameraDistance -= (y - mouseY) * 0.2f;
    //     mouseY = y;
    // }
}

/**
 * initialize GLUT for windowing
 **/
int initGLUT(int &argc, char **argv)
{
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH | GLUT_STENCIL | GLUT_MULTISAMPLE);   // display mode
    glutInitWindowSize(screenWidth, screenHeight);  // window size
    glutInitWindowPosition(100, 100);               // window location

    // finally, create a window with openGL context
    // Window will not displayed until glutMainLoop() is called
    // it returns a unique ID
    int handle = glutCreateWindow(argv[0]);     // param is the title of window

    // register GLUT callback functions
    glutDisplayFunc(displayCB);
    glutTimerFunc(33, timerCB, 33);             // redraw only every given millisec
    glutReshapeFunc(reshapeCB);
    glutKeyboardFunc(keyboardCB);
    glutMouseFunc(mouseCB);
    glutMotionFunc(mouseMotionCB);

    return handle;
}


/**
 * initialize lights
**/
void initLights()
{
    // set up light colors (ambient, diffuse, specular)
    GLfloat lightKa[] = {0.0f, 0.0f, 0.0f, 1.0f};  // ambient light
    GLfloat lightKd[] = {1, 1, 1, 1};  // diffuse light
    GLfloat lightKs[] = {0.5f, 0.5f, 0.5f, 1};           // specular light
    glLightfv(GL_LIGHT0, GL_AMBIENT, lightKa);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, lightKd);
    glLightfv(GL_LIGHT0, GL_SPECULAR, lightKs);

    // position the light
    float lightPos[4] = {5, 5, 10, 0}; // directional light
    glLightfv(GL_LIGHT0, GL_POSITION, lightPos);

    glEnable(GL_LIGHT0);                        // MUST enable each light source after configuration
}


/**
 * initialize OpenGL
 * disable unused features
**/
void initGL()
{
//    glShadeModel(GL_SMOOTH);                    // shading mathod: GL_SMOOTH or GL_FLAT
    glPixelStorei(GL_UNPACK_ALIGNMENT, 4);      // 4-byte pixel alignment

    // enable /disable features
    glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
    glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
    glHint(GL_POLYGON_SMOOTH_HINT, GL_NICEST);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_LIGHTING);
    glEnable(GL_MULTISAMPLE);
    glEnable(GL_CULL_FACE);
    glEnable(GL_NORMALIZE);

    // track material ambient and diffuse from surface color, call it before glEnable(GL_COLOR_MATERIAL)
    glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);
    glEnable(GL_COLOR_MATERIAL);

    glClearColor(0, 0, 0, 0);                   // background color
    glClearStencil(0);                          // clear stencil buffer
    glClearDepth(1.0f);                         // 0 is near, 1 is far
    glDepthFunc(GL_LEQUAL);

    initLights();
}


/**
 * initialize global variables
**/
bool initGlobalVars()
{
    screenWidth = SCREEN_WIDTH;
    screenHeight = SCREEN_HEIGHT;

    mouseLeftDown = mouseRightDown = mouseMiddleDown = false;
    mouseX = mouseY = 0;

    cameraAngleX = 0.0f;
    cameraAngleY = 90.0f;
    cameraDistance = CAMERA_DISTANCE;

    drawMode = 0; // 0:fill, 1: wireframe, 2:points

    return true;
}


void printUsage()
{
    std::cout << "Usage:" << std::endl;
    std::cout << " yarnball [options] <input_file>" << std::endl;
    std::cout << std::endl;
    std::cout << "Dispay an spherical mosaic file." << std::endl;
    std::cout << std::endl;
    std::cout << "Options:" << std::endl;
    std::cout << " -s <subdivisions>\tSet the number of subdivision steps." << std::endl;
    std::cout << " \t\t\tDefault is 0 for no subdivision" << std::endl;
    std::cout << " -w <line_width>\tSet the line width, in pixels" << std::endl;
    std::cout << " \t\t\tDefault is 1 pixel" << std::endl;
    std::cout << " -h\t\t\tDisplay this help" << std::endl;
}

int main(int argc, char **argv)
{
    initGlobalVars();

    // init GLUT and GL
    initGLUT(argc, argv);
    initGL();
    ilInit ();

    // Retrieve the options:
    int opt;
    while ( (opt = getopt(argc, argv, "s:w:h")) != -1 ) {  // for each option...
        switch ( opt ) {
            case 's':
                subdivLevel = atoi(optarg);
                break;
            case 'w':
                lineWidth = atoi(optarg);
                break;
            case 'h':
                printUsage();
                exit(0);
                break;
            case '?':  // unknown option...
                std::cerr << "Unknown option: '" << char(optopt) << "'" << std::endl;
                printUsage();
                exit(1);
                break;
        }
    }

    if ( (argc <= 1) || (argv[argc-1] == NULL) || (argv[argc-1][0] == '-') ) {  // there is NO input...
        std::cerr << "No input file provided!" << std::endl;
        printUsage();
        exit(1);
    }  else {  // there is an input...
        yarnballFile = argv[argc-1];
        // Load color and geometry
        loadColoredTris(yarnballFile, subdivLevel, lineWidth);
    }

    glutMainLoop();
    return 0;
}

