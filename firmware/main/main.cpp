// Copyright (c) 2026 Madusanka Jayarathna. All rights reserved.
// Use of this source code is governed by the MIT license.

#include <stdio.h>
#include "TestFirmware.h"

extern "C" void
app_main(void)
{
    FIRMWARE::TEST::TestFirmware tests;
    tests.opt()._runComponentTest = true;
    tests.runFirmwareTest();

    return;
}
