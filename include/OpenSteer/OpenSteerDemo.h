

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


#include "OpenSteer/AbstractVehicle.h"
#include "OpenSteer/Utilities.h"


namespace OpenSteer {

    class Vec3;

    class OpenSteerDemo
    {
    public:

        // currently selected vehicle.  Generally the one the camera follows and
        // for which additional information may be displayed.  Clicking the mouse
        // near a vehicle causes it to become the Selected Vehicle.
        static AbstractVehicle* selectedVehicle;
        static void initialize (void);
        static const AVGroup& allVehiclesOfSelectedPlugIn(void);

    };

    // ----------------------------------------------------------------------------
    // run graphics event loop
    void run(void);

} // namespace OpenSteer
// ----------------------------------------------------------------------------
#endif // OPENSTEER_OPENSTEERDEMO_H
