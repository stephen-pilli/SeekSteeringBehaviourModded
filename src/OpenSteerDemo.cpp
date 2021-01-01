// ----------------------------------------------------------------------------
//
//
// OpenSteer -- Steering Behaviors for Autonomous Characters
//
// Copyright (c) 2002-2005, Sony Computer Entertainment America
// Original author: Craig Reynolds <craig_reynolds@playstation.sony.com>
//
// Permission is hereby granted, free of charge, to any person obtaining a
// copy of this software and associated documentation files (the "Software"),
// to deal in the Software without restriction, including without limitation
// the rights to use, copy, modify, merge, publish, distribute, sublicense,
// and/or sell copies of the Software, and to permit persons to whom the
// Software is furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
// THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
// FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
// DEALINGS IN THE SOFTWARE.
//
//
// ----------------------------------------------------------------------------
//
//
// OpenSteerDemo
//
// This class encapsulates the state of the OpenSteerDemo application and the
// services it provides to its plug-ins.  It is never instantiated, all of
// its members are static (belong to the class as a whole.)
//
// 10-04-04 bk:  put everything into the OpenSteer namespace
// 11-14-02 cwr: created
//
//
// ----------------------------------------------------------------------------


#include "OpenSteer/OpenSteerDemo.h"
#include "OpenSteer/Annotation.h"
#include "OpenSteer/Color.h"
#include "OpenSteer/Vec3.h"

#include <algorithm>
#include <string>
#include <sstream>
#include <iomanip>

#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "OpenSteer/SimpleVehicle.h"


namespace {


using namespace OpenSteer;

// ----------------------------------------------------------------------------
// This PlugIn uses two vehicle types: MpWanderer and MpPursuer.  They have
// a common base class, MpBase, which is a specialization of SimpleVehicle.


class MpBase : public SimpleVehicle
{
public:

    // constructor
    MpBase () {reset ();}

    // reset state
    void reset (void)
    {
        SimpleVehicle::reset (); // reset the vehicle
        setSpeed (0);            // speed along Forward direction.
        setMaxForce (5.0);       // steering force is clipped to this magnitude
        setMaxSpeed (3.0);       // velocity is clipped to this magnitude
        clearTrailHistory ();    // prevent long streaks due to teleportation
        gaudyPursuitAnnotation = true; // select use of 9-color annotation
    }

    // draw into the scene
    void draw (void)
    {
        drawBasic2dCircularVehicle (this, bodyColor);
        drawTrail ();
    }

    // for draw method
    Color bodyColor;
};


class MpWanderer : public MpBase
{
public:

    // constructor
    MpWanderer () {reset ();}

    // reset state
    void reset (void)
    {
        MpBase::reset ();
        bodyColor.set (0.4f, 0.6f, 0.4f); // greenish
    }

    // one simulation step
    void update (const float elapsedTime)
    {
        const Vec3 wander2d = steerForWander (elapsedTime).setYtoZero ();
        const Vec3 steer = forward() + (wander2d * 3);
        applySteeringForce (steer, elapsedTime);
    }

};


class MpPursuer : public MpBase
{
    MpWanderer* wanderer;
public:

    // constructor
    MpPursuer (MpWanderer* w) {wanderer = w; reset ();}

    // reset state
    void reset (void)
    {
        MpBase::reset ();
        bodyColor.set (0.6f, 0.4f, 0.4f); // redish
        randomizeStartingPositionAndHeading ();
    }

    // one simulation step
    void update (const float elapsedTime)
    {
        // when pursuer touches quarry ("wanderer"), reset its position
        const float d = Vec3::distance (position(), wanderer->position());
        const float r = radius() + wanderer->radius();
        if (d < r) reset ();

        const float maxTime = 20; // xxx hard-to-justify value
        applySteeringForce (steerForPursuit (*wanderer, maxTime), elapsedTime);

    }

    // reset position
    void randomizeStartingPositionAndHeading (void)
    {
        // randomize position on a ring between inner and outer radii
        // centered around the home base
        const float inner = 20;
        const float outer = 30;
        const float radius = frandom2 (inner, outer);
        const Vec3 randomOnRing = RandomUnitVectorOnXZPlane () * radius;
        setPosition (wanderer->position() + randomOnRing);

        // randomize 2D heading
        randomizeHeadingOnXZPlane ();
    }


};


// ----------------------------------------------------------------------------
// PlugIn for OpenSteerDemo


class MpPlugIn{



    // a group (STL vector) of all vehicles
    std::vector<MpBase*> allMP;
    typedef std::vector<MpBase*>::const_iterator iterator;
    iterator pBegin, pEnd;

    MpWanderer* wanderer;

    int pursuerCount;

public:

    AVGroup& allVehicles (void) {
        return (AVGroup&) allMP;
    }


    MpPlugIn(int n){
        pursuerCount = n;
    }
    virtual ~MpPlugIn() {} // be more "nice" to avoid a compiler warning

    void open (void)
    {
        // create the wanderer, saving a pointer to it
        wanderer = new MpWanderer;
        allMP.push_back (wanderer);

        // create the specified number of pursuers, save pointers to them
        for (int i = 0; i < pursuerCount; i++)
            allMP.push_back (new MpPursuer (wanderer));
        pBegin = allMP.begin() + 1;  // iterator pointing to first pursuer
        pEnd = allMP.end();          // iterator pointing to last pursuer

        // initialize camera
        OpenSteerDemo::selectedVehicle = wanderer;
        OpenSteerDemo::camera.mode = Camera::cmStraightDown;
        OpenSteerDemo::camera.fixedDistDistance = OpenSteerDemo::cameraTargetDistance;
        OpenSteerDemo::camera.fixedDistVOffset = OpenSteerDemo::camera2dElevation;
    }

    void update (const float elapsedTime)
    {
        // update the wanderer
        wanderer->update (elapsedTime);

        // update each pursuer
        for (iterator i = pBegin; i != pEnd; i++)
        {
            ((MpPursuer&) (**i)).update (elapsedTime);
        }
    }

    void redraw (const float currentTime, const float elapsedTime)
    {
        // selected vehicle (user can mouse click to select another)
        AbstractVehicle* selected = OpenSteerDemo::selectedVehicle;

        // update camera
        OpenSteerDemo::updateCamera (currentTime, elapsedTime, selected);

        // draw "ground plane"
        OpenSteerDemo::gridUtility (selected->position());

        // draw each vehicles
        for (iterator i = allMP.begin(); i != pEnd; i++) (**i).draw ();
    }

    void close (void)
    {
        // delete wanderer, all pursuers, and clear list
        delete (wanderer);
        for (iterator i = pBegin; i != pEnd; i++) delete ((MpPursuer*)*i);
        allMP.clear();
    }

    void reset (void)
    {
        // reset wanderer and pursuers
        wanderer->reset ();
        for (iterator i = pBegin; i != pEnd; i++) ((MpPursuer&)(**i)).reset ();

        // immediately jump to default camera position
        OpenSteerDemo::camera.doNotSmoothNextMove ();
        OpenSteerDemo::camera.resetLocalSpace ();
    }

    MpWanderer* getWanderer(void){
        return wanderer;
    }

};

}



GLFWwindow* demo_window = nullptr;
// ----------------------------------------------------------------------------
// keeps track of both "real time" and "simulation time"
OpenSteer::Clock OpenSteer::OpenSteerDemo::clock;
// ----------------------------------------------------------------------------
// camera automatically tracks selected vehicle
OpenSteer::Camera OpenSteer::OpenSteerDemo::camera;
// ----------------------------------------------------------------------------
//// currently selected plug-in (user can choose or cycle through them)
//OpenSteer::PlugIn* OpenSteer::OpenSteerDemo::selectedPlugIn = NULL;
// ----------------------------------------------------------------------------
// currently selected vehicle.  Generally the one the camera follows and
// for which additional information may be displayed.  Clicking the mouse
// near a vehicle causes it to become the Selected Vehicle.
OpenSteer::AbstractVehicle* OpenSteer::OpenSteerDemo::selectedVehicle = NULL;
// ----------------------------------------------------------------------------
// graphical annotation: master on/off switch
bool OpenSteer::enableAnnotation = true;


// ----------------------------------------------------------------------------
// handler for window resizing

void reshape( GLFWwindow* window, int width, int height )
{
    GLfloat h = (GLfloat) height / (GLfloat) width;
    GLfloat xmax, znear, zfar;

    znear = 1.0f;
    zfar  = 400.0f;
    xmax  = znear * 0.5f;

    glViewport( 0, 0, (GLint) width, (GLint) height );
    glMatrixMode( GL_PROJECTION );
    glLoadIdentity();
    glFrustum( -xmax, xmax, -xmax*h, xmax*h, znear, zfar );
    glMatrixMode( GL_MODELVIEW );
}


MpPlugIn gMpPlugIn(8);

void
OpenSteer::OpenSteerDemo::initialize (void)
{

    //    selectedPlugIn = &gMpPlugIn;
    camera.reset ();
    selectedVehicle = NULL;
    gMpPlugIn.open ();

}
// ----------------------------------------------------------------------------
// main update function: step simulation forward and redraw scene
void
OpenSteer::OpenSteerDemo::updateSimulationAndRedraw (void)
{
    // update global simulation clock
    clock.update ();

    float currentTime = 0;//clock.getTotalSimulationTime();
    float elapsedTime = 0.006;//clock.getElapsedSimulationTime();

    // run selected PlugIn (with simulation's current time and step size)
    // if no vehicle is selected, and some exist, select the first one
    if (selectedVehicle == NULL)
    {
        const AVGroup& vehicles = gMpPlugIn.allVehicles ();
        if (vehicles.size() > 0) selectedVehicle = vehicles.front();
    }

    // invoke selected PlugIn's Update method
    gMpPlugIn.update (elapsedTime);

    // redraw selected PlugIn (based on real time)
    // invoke selected PlugIn's Draw method
    gMpPlugIn.redraw (currentTime, elapsedTime);

    // draw any annotation queued up during selected PlugIn's Update method
    drawAllDeferredLines ();
    drawAllDeferredCirclesOrDisks ();

}



// ------------------------------------------------------------------------
// Main drawing function for OpenSteerDemo application,
// drives simulation as a side effect


void
displayFunc (void)
{
    // clear color and depth buffers
    glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // run simulation and draw associated graphics
    OpenSteer::OpenSteerDemo::updateSimulationAndRedraw ();

    // double buffering, swap back and front buffers
    glFlush ();
}


// ----------------------------------------------------------------------------
// do all initialization related to graphics


void
OpenSteer::initializeGraphics (int argc, char **argv)
{
    if (!glfwInit())
    {
        fprintf( stderr, "Failed to initialize GLFW\n" );
        exit( EXIT_FAILURE );
    }

    demo_window = glfwCreateWindow( 1200, 900, "OpenSteer", NULL, NULL );
    if (!demo_window)
    {
        fprintf( stderr, "Failed to open GLFW window\n" );
        glfwTerminate();
        exit( EXIT_FAILURE );
    }

    // Set callback functions
    glfwSetFramebufferSizeCallback(demo_window, reshape);

    glfwMakeContextCurrent(demo_window);
    gladLoadGLLoader((GLADloadproc) glfwGetProcAddress);
    glfwSwapInterval( 1 );

    int width, height;
    glfwGetFramebufferSize(demo_window, &width, &height);
    reshape(demo_window, width, height);
}


// ----------------------------------------------------------------------------
// run graphics event loop

#include <opencv2/opencv.hpp>

void foo(){
    float elapsedTime = 0.006;//clock.getElapsedSimulationTime();

    // run selected PlugIn (with simulation's current time and step size)
    // if no vehicle is selected, and some exist, select the first one
    const AVGroup& vehicles = gMpPlugIn.allVehicles ();

    if (vehicles.size() > 0) OpenSteer::OpenSteerDemo::selectedVehicle = vehicles.front();

    gMpPlugIn.update (elapsedTime);
    std::cout<<gMpPlugIn.getWanderer()->position()<<std::endl;

    Vec3 point = gMpPlugIn.getWanderer()->position();

    cv::Mat mat = cv::Mat(1000, 1000, CV_8UC3);

    cv::circle(mat, cv::Point(point.x*10+500, point.z*10+500), 5, cv::Scalar(0,255,0));

    for (int i = 1; i < vehicles.size() ; ++i){
        Vec3 pos = vehicles[i]->position();
        cv::circle(mat, cv::Point(pos.x*10+500, pos.z*10+500), 5, cv::Scalar(255,0,0));

    }

    cv::imshow("Window", mat);
    mat.deallocate();
    cv::waitKey(10);
}

void
OpenSteer::runGraphics (void)
{
    cv::namedWindow("Window");

    // Main loop
    while( !glfwWindowShouldClose(demo_window) )
    {
        displayFunc();

        foo();

        // Swap buffers
        glfwSwapBuffers(demo_window);
        glfwPollEvents();

    }
    glfwTerminate();
}



void
OpenSteer::runGraphics_CV (void)
{

    // Main loop
    while(true)
    {


        cv::waitKey(30);

        // invoke selected PlugIn's Update method


    }
}







//CAM
void
OpenSteer::OpenSteerDemo::init3dCamera (AbstractVehicle& selected)
{
    init3dCamera (selected, cameraTargetDistance, camera2dElevation);
}

void
OpenSteer::OpenSteerDemo::init3dCamera (AbstractVehicle& selected,
                                        float distance,
                                        float elevation)
{
    position3dCamera (selected, distance, elevation);
    camera.fixedDistDistance = distance;
    camera.fixedDistVOffset = elevation;
    camera.mode = Camera::cmFixedDistanceOffset;
}


void
OpenSteer::OpenSteerDemo::init2dCamera (AbstractVehicle& selected)
{
    init2dCamera (selected, cameraTargetDistance, camera2dElevation);
}

void
OpenSteer::OpenSteerDemo::init2dCamera (AbstractVehicle& selected,
                                        float distance,
                                        float elevation)
{
    position2dCamera (selected, distance, elevation);
    camera.fixedDistDistance = distance;
    camera.fixedDistVOffset = elevation;
    camera.mode = Camera::cmFixedDistanceOffset;
}


void
OpenSteer::OpenSteerDemo::position3dCamera (AbstractVehicle& selected)
{
    position3dCamera (selected, cameraTargetDistance, camera2dElevation);
}

void
OpenSteer::OpenSteerDemo::position3dCamera (AbstractVehicle& selected,
                                            float distance,
                                            float /*elevation*/)
{
    selectedVehicle = &selected;
    if (&selected)
    {
        const Vec3 behind = selected.forward() * -distance;
        camera.setPosition (selected.position() + behind);
        camera.target = selected.position();
    }
}


void
OpenSteer::OpenSteerDemo::position2dCamera (AbstractVehicle& selected)
{
    position2dCamera (selected, cameraTargetDistance, camera2dElevation);
}

void
OpenSteer::OpenSteerDemo::position2dCamera (AbstractVehicle& selected,
                                            float distance,
                                            float elevation)
{
    // position the camera as if in 3d:
    position3dCamera (selected, distance, elevation);

    // then adjust for 3d:
    Vec3 position3d = camera.position();
    position3d.y += elevation;
    camera.setPosition (position3d);
}


// ----------------------------------------------------------------------------
// camera updating utility used by several plug-ins


void
OpenSteer::OpenSteerDemo::updateCamera (const float currentTime,
                                        const float elapsedTime,
                                        const AbstractVehicle* selected)
{
    camera.vehicleToTrack = selected;
    camera.update (currentTime, elapsedTime, clock.getPausedState ());
}


// ----------------------------------------------------------------------------
// some camera-related default constants


const float OpenSteer::OpenSteerDemo::camera2dElevation = 8;
const float OpenSteer::OpenSteerDemo::cameraTargetDistance = 13;
const OpenSteer::Vec3 OpenSteer::OpenSteerDemo::cameraTargetOffset (0, OpenSteer::OpenSteerDemo::camera2dElevation,
                                                                    0);


// ----------------------------------------------------------------------------
// ground plane grid-drawing utility used by several plug-ins


void
OpenSteer::OpenSteerDemo::gridUtility (const Vec3& gridTarget)
{
    // round off target to the nearest multiple of 2 (because the
    // checkboard grid with a pitch of 1 tiles with a period of 2)
    // then lower the grid a bit to put it under 2d annotation lines
    const Vec3 gridCenter ((round (gridTarget.x * 0.5f) * 2),
                           (round (gridTarget.y * 0.5f) * 2) - .05f,
                           (round (gridTarget.z * 0.5f) * 2));

    // colors for checkboard
    const Color gray1(0.27f);
    const Color gray2(0.30f);

    // draw 50x50 checkerboard grid with 50 squares along each side
    drawXZCheckerboardGrid (50, 50, gridCenter, gray1, gray2);

    // alternate style
    // drawXZLineGrid (50, 50, gridCenter, gBlack);
}



