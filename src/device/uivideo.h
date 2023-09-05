#ifndef __UIVIDEO_H__
#define  __UIVIDEO_H__
#ifdef __cplusplus
extern "C" {
#endif
#include <stdio.h>
#include <stdbool.h>
#include <pthread.h>
#include <unistd.h>
#define SDL_MAIN_HANDLED

#include <SDL.h>   // original SDL2/SDL.h
// #include <SDL_ttf.h>
#include <SDL_scancode.h>
#include <SDL_image.h>

#include "video.h"

//video 
int SDLVIDEO_move(int row,int col);
int SDLUI_attrset(int attr);
int SDLUI_move(int row,int col);
int SDLUI_addch(uint8_t ch);
int SDLUI_addtext(int x, int y, uint8_t a, uint8_t ch);
int SDLUI_addlores(int x, int y, uint32_t c1,uint32_t c2);
int SDLUI_addhirespixel(int x, int y, uint32_t c);
int SDLUI_clearscr();






// #define UI_Initial(device) int UI##device_Initial(SDL_Renderer *renderer)
// #define UI_Event(device)   int UI##device_Event(SDL_Event event)
// #define UI_Release(device) int UI##device_Release()
// #define UI_Refresh(device) int UI##device_Refresh()


/* UI layout VIDEO */
int UIVIDEO_Initial(SDL_Renderer *renderer);
int UIVIDEO_Event(SDL_Event event);
int UIVIDEO_Refresh(int *update);
int UIVIDEO_Release();
#ifdef __cplusplus
}
#endif
#endif