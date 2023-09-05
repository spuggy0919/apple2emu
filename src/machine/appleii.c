#include "appleii.h"

void HALAPPLEII_Reset()
{
    HALMemory_Reset(); // load rom first
    HALClock_Reset();
    HALCPU6502_Reset();
    HALVIDEO_Reset();
    HALKey_Reset();
    HWCLOCK_set_cpucallback(HALCPU6502_Running);

}

int tickcnt=0;
void HALAPPLEII_Running()
{
    // DEBUG("HALAPPLEII_Running\n");
    // tickcnt += HWCLOCK_getTicks(tickcnt); // TODO need to synchonize 1MHZ
    // tickcnt = HALCPU6502_Running(false, tickcnt);

}
void HALAPPLEII_PowerOff()
{

}