#ifndef __DISK2_H__
#define  __DISK2_H__
#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>

#include "../device/ui.h"
#include "clock.h"
// language card
extern bool LCWR ; // Language Card writable
extern bool LCRD ; // Language Card readable
extern bool LCBK2 ;   // Language Card bank 2 enabled
extern bool LCWFF ;



// disk II
uint8_t diskio(uint16_t address, uint8_t value, bool WRT);

int insertFloppy( char *filename, int drv) ;


#ifdef __cplusplus
}
#endif
#endif