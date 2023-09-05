#ifndef __APPLEII_H__
#define  __APPLEII_H__
#ifdef __cplusplus
extern "C" {
#endif
#include <stdio.h>
#include <stdbool.h>
#include <pthread.h>
#include <unistd.h>
#include "../device/ui.h"

// HAL layers
#include "clock.h"
#include "cpu6502.h"
#include "keyboard.h"
#include "memory.h"
#include "video.h"
#include "disk2.h"

void HALAPPLEII_Reset();
void HALAPPLEII_Running();
void HALAPPLEII_PowerOff();


#ifdef __cplusplus
}
#endif
#endif