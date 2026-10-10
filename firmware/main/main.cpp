#include <stdio.h>
//#include "componentTests.h"
#include "TestFirmware.h"

extern "C" void
app_main(void)
{
    //FIRMWARE::ComponentTests compTests;
    //compTests.runBlinkTest();

    FIRMWARE::TEST::TestFirmware tests;
    tests.opt()._runComponentTest = true;
    tests.runFirmwareTest();

    return;
}
