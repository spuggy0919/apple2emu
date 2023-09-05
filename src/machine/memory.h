


#ifndef __MEMORY_H__
#define  __MEMORY_H__
#ifdef __cplusplus
extern "C" {
#endif
#include <stdio.h>
#include <stdbool.h>
#include <string.h>

#include "disk2.h"

extern uint8_t memory[0x10000];
// SDL Interfaces
int SDLHW_Memory_Initial();
int SDLHW_Memory_Polling();// Repeat Key 
int SDLHW_Memory_Refresh(int kbscancode, int ispress);
int SDLHW_Memory_Release(int kbscancode, int ispress);

// HAL machine 
uint8_t readByte(uint16_t address);
uint16_t readWord(uint16_t address);
void copyBlock(uint16_t address, uint16_t len, uint8_t *det); // for video 
void writeByte(uint16_t address, uint8_t value);
void writeWord(uint16_t address,uint16_t value);
const char* intToHex(int value); // for VSCODE Debug
int  load_File_to_memory(char *filename, char *ext, uint8_t memory[]);

// HAL Exchange with memory io
void HALMemory_Reset();
    // Keyboard
void HALMemory_SetIOKey(uint8_t keyin); //0xc000 0xc010
uint8_t HALMemory_GetIOKey(); 
    // video 
uint8_t* HALMemory_GetVideoSwitches();
void HALMemory_GetVideoFrameData(uint16_t addr, uint16_t len, uint8_t *dstmem);

#ifdef __cplusplus
}
#endif
#endif