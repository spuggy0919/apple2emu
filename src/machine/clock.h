#ifndef __CLOCK_H__
#define  __CLOCK_H__
#ifdef __cplusplus
extern "C" {
#endif

#include "appleii.h"

typedef int (*CPUTHREAD)(uint8_t forever, int ticks);

//CLOCK functions & variables
extern volatile int count_1MHz ;

extern volatile int tickTock_2Hz;
extern volatile int trigger_10Hz;
extern volatile int trigger_30Hz;
extern volatile CPUTHREAD cpurunning;

int HWCLOCK_getTicks(int ticks);
int HWCLOCK_set_tickshrink();

int HWCLOCK_clr_trigger10Hz();
int HWCLOCK_get_trigger10Hz();
int HWCLOCK_set_cpucallback(CPUTHREAD func);


//CLOCK functions

int HALClock_Reset();
int HALCLOCK_Release();


#ifdef __cplusplus
}
#endif
#endif