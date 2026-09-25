#include <stdio.h>
#include "componentTests.h"

extern "C" void
app_main(void)
{
    FIRMWARE::ComponentTests compTests;
    compTests.runBlinkTest();
    return;
}
