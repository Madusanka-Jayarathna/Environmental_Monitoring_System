// Copyright (c) 2026 Madusanka Jayarathna. All rights reserved.
// Use of this source code is governed by the MIT license.

#include <stdio.h>
#include "components/libComponentHandler/public/ComponentHandler.h"
#include "tests/componentTests/ESP32Tests.h"

extern "C" void
app_main(void)
{
    /*
    RUN COMPONENT HARDWARE TESTS (Check peripherals & connection sanity)
    */

    FIRMWARE::TEST::COMPTEST::ESP32Tests componentTests;
    componentTests.opt()._runBlinkTest      = false;
    componentTests.opt()._runBME280Test     = true;
    componentTests.opt()._runSoilSensorTest = true;
    componentTests.opt()._runSDCardTest     = true;
    componentTests.opt()._runOLEDTest       = true;
    componentTests.run();

    /*
    MAIN FIRMWARE PIPELINE
    */
    //FIRMWARE::ComponentHandler componentHandler;
    ////componentHandler.opt()._serialRun = true;
    //componentHandler.run();
}
