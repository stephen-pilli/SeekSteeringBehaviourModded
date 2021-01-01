

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
// This class encapsulates the state of the OpenSteerDemo application and
// the services it provides to its plug-ins
//
// 10-04-04 bk:  put everything into the OpenSteer namespace
// 11-14-02 cwr: recast App class as OpenSteerDemo 
// 06-26-02 cwr: App class created 
//
//
// ----------------------------------------------------------------------------


#ifndef OPENSTEER_OPENSTEERDEMO_H
#define OPENSTEER_OPENSTEERDEMO_H



#include "OpenSteer/Clock.h"
#include "OpenSteer/PlugIn.h"
#include "OpenSteer/Camera.h"
#include "OpenSteer/Utilities.h"


namespace OpenSteer {

    class Color;
    class Vec3;
    

    class OpenSteerDemo
    {
    public:
        // ------------------------------------------------------ component objects

        // clock keeps track of both "real time" and "simulation time"
        static Clock clock;

        // camera automatically tracks selected vehicle
        static Camera camera;

        // ------------------------------------------ addresses of selected objects

        // currently selected plug-in (user can choose or cycle through them)
        static PlugIn* selectedPlugIn;

        // currently selected vehicle.  Generally the one the camera follows and
        // for which additional information may be displayed.  Clicking the mouse
        // near a vehicle causes it to become the Selected Vehicle.
        static AbstractVehicle* selectedVehicle;

        // -------------------------------------------- initialize, update and exit

        // initialize OpenSteerDemo
        //     XXX  if I switch from "totally static" to "singleton"
        //     XXX  class structure this becomes the constructor
        static void initialize (void);

        // main update function: step simulation forward and redraw scene
        static void updateSimulationAndRedraw (void);


        static const AVGroup& allVehiclesOfSelectedPlugIn(void);

        // ------------------------------------------------------- camera utilities

        // set a certain initial camera state used by several plug-ins
        static void init2dCamera (AbstractVehicle& selected);
        static void init2dCamera (AbstractVehicle& selected,
                                  float distance,
                                  float elevation);
        static void init3dCamera (AbstractVehicle& selected);
        static void init3dCamera (AbstractVehicle& selected,
                                  float distance,
                                  float elevation);

        // set initial position of camera based on a vehicle
        static void position3dCamera (AbstractVehicle& selected);
        static void position3dCamera (AbstractVehicle& selected,
                                      float distance,
                                      float elevation);
        static void position2dCamera (AbstractVehicle& selected);
        static void position2dCamera (AbstractVehicle& selected,
                                      float distance,
                                      float elevation);

        // camera updating utility used by several (all?) plug-ins
        static void updateCamera (const float currentTime,
                                  const float elapsedTime,
                                  const AbstractVehicle* selected);

        // some camera-related default constants
        static const float camera2dElevation;
        static const float cameraTargetDistance;
        static const Vec3 cameraTargetOffset;

        // ------------------------------------------------ graphics and annotation

        // do all initialization related to graphics
        static void initializeGraphics (void);

        // ground plane grid-drawing utility used by several plug-ins
        static void gridUtility (const Vec3& gridTarget);

    };

    // ----------------------------------------------------------------------------
    // do all initialization related to graphics
    void initializeGraphics (int argc, char **argv);
    // ----------------------------------------------------------------------------
    // run graphics event loop
    void runGraphics (void);
    void runGraphics_CV(void);
    // ----------------------------------------------------------------------------
    // accessors for GLUT's window dimensions
    float drawGetWindowHeight (void);
    float drawGetWindowWidth (void);
} // namespace OpenSteer
// ----------------------------------------------------------------------------
#include "OpenSteer/Draw.h"
// ----------------------------------------------------------------------------
#endif // OPENSTEER_OPENSTEERDEMO_H
