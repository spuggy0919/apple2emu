#include "appleii.h"
/*
 * apple ii machine
 * author:spuggy0919
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */
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