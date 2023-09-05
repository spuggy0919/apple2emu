


#ifndef __VIDEO_H__
#define  __VIDEO_H__
#ifdef __cplusplus
extern "C" {
#endif
#include <stdio.h>
#include <stdlib.h>


#define A_NORMAL  0
#define A_INVERSE 1
#define A_BLINK   2
// SDL Interfaces
int SDLHW_VIDEO_Initial();
int SDLHW_VIDEO_Polling();// Repeat Key 
int SDLHW_VIDEO_Refresh();
int SDLHW_VIDEO_Release();
int HALVIDEO_Text40Mode();
// int HALVIDEO_Text40Test();
int HALVIDEO_LoResMode();
int HALVIDEO_HiResMode();
int HALVIDEO_LoRes_MixMode();
int HALVIDEO_HiRes_MixMode();

// HAL machine 
// int HALVIDEO_Text40Mode();
int HALVIDEO_Reset();

// HAL Exchange with memory io



#ifdef __cplusplus
}
#endif
#endif