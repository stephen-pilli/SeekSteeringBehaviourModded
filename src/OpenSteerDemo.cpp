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
#include "OpenSteer/Vec3.h"

#include <algorithm>
#include <string>
#include <sstream>
#include <iomanip>

#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "OpenSteer/SimpleVehicle.h"
#include <opencv2/opencv.hpp>


namespace {


using namespace OpenSteer;

// ----------------------------------------------------------------------------
// This PlugIn uses two vehicle types: MpWanderer and MpPursuer.  They have
// a common base class, MpBase, which is a specialization of SimpleVehicle.


class MpBase : public SimpleVehicle
{
public:

    // constructor
    MpBase () {
        reset ();
    }

    // reset state
    void reset (void)
    {
        SimpleVehicle::reset (); // reset the vehicle
        setSpeed (0);            // speed along Forward direction.
        setMaxForce (5.0);       // steering force is clipped to this magnitude
        setMaxSpeed (3.0);       // velocity is clipped to this magnitude
        clearTrailHistory ();    // prevent long streaks due to teleportation
    }

};


class MpWanderer : public MpBase
{
public:

    // constructor
    MpWanderer () {
        reset ();
    }

    // reset state
    void reset (void)
    {
        MpBase::reset ();
    }

    // one simulation step
    void update (const float elapsedTime, Vec3 location)
    {
        const Vec3 wander2d = location;//steerForWander (elapsedTime).setYtoZero ();
        const Vec3 steer = forward() + (wander2d * 3);
        applySteeringForce (steer, elapsedTime);
    }

};


class MpPursuer : public MpBase
{
    MpWanderer* wanderer;
public:

    // constructor
    MpPursuer (MpWanderer* w) {
        wanderer = w;
        reset ();
    }

    // reset state
    void reset (void)
    {
        MpBase::reset ();
        randomizeStartingPositionAndHeading ();
    }

    // one simulation step
    void update (const float elapsedTime, Vec3 location)
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
    }

    void update_hero (const float elapsedTime, Vec3 location)
    {
        // update the wanderer
        wanderer->setPosition(location.x, location.y, location.z);
        //        wanderer->update (elapsedTime, location);
    }

    void update_enemies(const float elapsedTime){

        // update each pursuer
        for (iterator i = pBegin; i != pEnd; i++)
        {

            ((MpPursuer&) (**i)).update (elapsedTime, Vec3(0,0,0));
        }
    }

    void close (void)
    {
        std::cout<<std::endl;
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
    }

    MpWanderer* getWanderer(void){
        return wanderer;
    }

};

}

// ----------------------------------------------------------------------------
// currently selected vehicle.  Generally the one the camera follows and
// for which additional information may be displayed.  Clicking the mouse
// near a vehicle causes it to become the Selected Vehicle.
OpenSteer::AbstractVehicle* OpenSteer::OpenSteerDemo::selectedVehicle = NULL;


MpPlugIn MpObj(8);
float elapsedTime = 0.006;
int world_size = 1000;
float offset = (float)world_size/2;
float multi = 20;
int wanderer_size = 20;
cv::Point Red, Green, Blue, White;
cv::Mat WorldMat;


void genWorld(cv::Mat & world_Mat){

    int rad = 20;
    int thick = 50;
    cv::circle(world_Mat, Red, rad, cv::Scalar(0,0,255), thick);
    cv::circle(world_Mat, Green, rad, cv::Scalar(0,255,0), thick);
    cv::circle(world_Mat, Blue, rad, cv::Scalar(255,0,0), thick);
    cv::circle(world_Mat, White, rad, cv::Scalar(255,255,255), thick);


}

cv::Point getWorldPosition(Vec3 point){
    return cv::Point(point.x*multi+offset, point.z*multi+offset);
}

Vec3 setPlayerPosition(MpPlugIn *mp, int x, int y){
    mp->update_hero (elapsedTime, Vec3((x-offset)/multi, 0, (y-offset)/multi));
}

void killEnemy(){

}


void CallBackFunc(int event, int x, int y, int flags, void* userdata)
{
    if  ( event == cv::EVENT_LBUTTONDOWN )
    {
        std::cout << "Left button of the mouse is clicked - position (" << x << ", " << y << ")" << std::endl;
        setPlayerPosition(&MpObj, Red.x, Red.y);
    }
    else if  ( event == cv::EVENT_RBUTTONDOWN )
    {
        std::cout << "Right button of the moue is clicked - position (" << x << ", " << y << ")" << std::endl;
        setPlayerPosition(&MpObj, White.x, White.y);

    }
    else if  ( event == cv::EVENT_MBUTTONDOWN )
    {
        std::cout << "Middle button of the mouse is clicked - position (" << x << ", " << y << ")" << std::endl;
        setPlayerPosition(&MpObj, x, y);
    }
    else if ( event == cv::EVENT_MOUSEMOVE )
    {
        std::cout << "Mouse move over the window - position (" << x << ", " << y << ")" << std::endl;

    }
    else if ( event == cv::EVENT_MOUSEHWHEEL )
    {

        std::cout << "Mouse move over the window - position (" << x << ", " << y << ")" << std::endl;


    }


}


void foo(){


    // run selected PlugIn (with simulation's current time and step size)
    // if no vehicle is selected, and some exist, select the first one
    const AVGroup& vehicles = MpObj.allVehicles ();
    if (vehicles.size() > 0) OpenSteer::OpenSteerDemo::selectedVehicle = vehicles.front();

    //Update Enemies
    MpObj.update_enemies(elapsedTime);

    //Draw hero Position
    cv::circle(WorldMat, cv::Point(getWorldPosition(MpObj.getWanderer()->position())), wanderer_size, cv::Scalar(0,255,0), 5);
    //Draw Enemies position
    for (int i = 1; i < vehicles.size() ; ++i){
        Vec3 pos = vehicles[i]->position();
        cv::circle(WorldMat, cv::Point(getWorldPosition(pos)), wanderer_size, cv::Scalar(0,0,255), 5);
    }

}




void
OpenSteer::run(void)
{
    while(true)
    {
        //INIT World
        WorldMat = cv::Mat(world_size, world_size, CV_8UC3);
        genWorld(WorldMat);

        foo();

        cv::imshow("Window", WorldMat);
        WorldMat.deallocate();
        char keypress = cv::waitKey(1);
        Vec3 position = MpObj.getWanderer()->position();
        if(keypress == 27){
            break;
        }else if (keypress == 'w') {
            MpObj.getWanderer()->setPosition(position.x,0.f,position.z - 0.3);
        }else if (keypress == 'a') {
            MpObj.getWanderer()->setPosition(position.x - 0.3,0.f,position.z);

        }else if (keypress == 's') {
            MpObj.getWanderer()->setPosition(position.x,0.f,position.z + 0.3);

        }else if (keypress == 'd') {
            MpObj.getWanderer()->setPosition(position.x + 0.3,0.f,position.z);

        }else if (keypress == 'q') {

            setPlayerPosition(&MpObj, Green.x, Green.y);

        }else if (keypress == 'e') {

            setPlayerPosition(&MpObj, Blue.x, Blue.y);

        }
    }
}

void
OpenSteer::OpenSteerDemo::initialize (void)
{
    Red = cv::Point(world_size*0.25, world_size*0.25);
    Green = cv::Point(world_size*0.75, world_size*0.25);
    Blue = cv::Point(world_size*0.25, world_size*0.75);
    White= cv::Point(world_size*0.75, world_size*0.75);




    selectedVehicle = NULL;
    MpObj.open ();

    //set the callback function for any mouse event
    cv::namedWindow("Window", 1);
    cv::setMouseCallback("Window", CallBackFunc, NULL);

}



