
#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <unistd.h>
#include <stdbool.h>
#include "clock.h"
/* 10HZ repeat timer*/
pthread_mutex_t triggerMutex = PTHREAD_MUTEX_INITIALIZER;
// bool trigger = false;
// bool trig2hz = false;
// int  cnt5=0;


#define DIVFACTOR 1000000

#define SIGNAL_FREQUENCY_1MHZ (1000000/DIVFACTOR)  // 1 microsecond
#define SIGNAL_FREQUENCY_4HZ  4         // 100 million microseconds (10 Hz)
#define SIGNAL_FREQUENCY_10HZ 10       // 100 million microseconds (10 Hz)
#define SIGNAL_FREQUENCY_30HZ 30 //30     // 33.333 million microseconds (30 Hz)

pthread_t thread;

volatile int count_4Hz = 0;
volatile int count_10Hz = 0;
volatile int count_30Hz = 0;
volatile int count_1MHz = 0;
volatile int tickTock_2Hz = 0;
volatile int trigger_10Hz = 0;
volatile int trigger_30Hz = 0;
static    int lastcnt=-1;
static    int tickcnt=0;

volatile  CPUTHREAD cpurunning=(CPUTHREAD)NULL;
void *timerThread(void *arg) {
    int interval_microseconds =  SIGNAL_FREQUENCY_1MHZ; //1/100s 1000000Hz interval 

    while (1) {
        usleep(interval_microseconds); // Wait for the specified interval

        // Set the trigger to true
        pthread_mutex_lock(&triggerMutex);
        // Generate 2 MHz signal
        count_1MHz++;
        count_4Hz++;
        count_10Hz++;
        count_30Hz++;
        tickcnt+= (lastcnt!=-1) ? count_1MHz- lastcnt:100;
        lastcnt = count_1MHz;
        if (cpurunning!=NULL&&tickcnt>10) tickcnt = cpurunning(false,tickcnt*40);
        // Generate 10 Hz signal
        if (count_4Hz >= (1000000/SIGNAL_FREQUENCY_1MHZ)  / SIGNAL_FREQUENCY_4HZ) {
            count_4Hz = 0; 
            // printf("tick-tock\n");
            tickTock_2Hz = !tickTock_2Hz; 

        }
        // Generate 10 Hz signal
        if (count_10Hz >= (1000000/SIGNAL_FREQUENCY_1MHZ) / SIGNAL_FREQUENCY_10HZ) {
            count_10Hz = 0; 

        }
        // Generate 10 Hz signal
        if (count_30Hz >= (1000000/SIGNAL_FREQUENCY_1MHZ)  / SIGNAL_FREQUENCY_30HZ) {
            count_30Hz = 0; 
        }

        trigger_10Hz=(count_10Hz==0);
        trigger_30Hz=(count_30Hz<=700); //1/3 16666
        pthread_mutex_unlock(&triggerMutex);
    }

    return NULL;
}
int HWCLOCK_set_cpucallback(CPUTHREAD func)
{
    pthread_mutex_lock(&triggerMutex);
    cpurunning = func;
    pthread_mutex_unlock(&triggerMutex); 
    return 0;
}

int HWCLOCK_set_tickshrink()
{
    pthread_mutex_lock(&triggerMutex);
    tickcnt &= 0xff;
    pthread_mutex_unlock(&triggerMutex); 
    return 0;
}
int HWCLOCK_get_trigger10Hz()
{
    return trigger_10Hz;
}
int HWCLOCK_clr_trigger10Hz()
{
    pthread_mutex_lock(&triggerMutex);
    trigger_10Hz = false; // Reset the trigger
    pthread_mutex_unlock(&triggerMutex); 
    return 0;   
}

int HWCLOCK_getTicks(int ticks)
{
    tickcnt=ticks;
    tickcnt+=(lastcnt!=-1) ? (count_1MHz-lastcnt):1000; // TODO need to synchonize 1MHZ
    lastcnt = count_1MHz;
    printf("lastcnt %d tickcnt %d count_1MHz %d\n",lastcnt,tickcnt,count_1MHz);
    return (tickcnt);
}

int HALClock_Reset()
{
    // Create the timer thread
    if (pthread_create(&thread, NULL, timerThread, NULL) != 0) {
        // perror("pthread_create");
        return 1;
    }    
    return 0;
}

int HALCLOCK_Release()
{
    pthread_cancel(thread);
    pthread_join(thread,NULL);
    return 0;

}