#ifndef __UI__
#define  __UI__
#ifdef __cplusplus
extern "C" {
#endif
#include <stdio.h>
#include <stdbool.h>
#include <pthread.h>
#include <unistd.h>

#include <SDL.h>   // original SDL2/SDL.h
#include <SDL_ttf.h>
#include <SDL_scancode.h>
#include <SDL_image.h>

#include "../machine/appleii.h"

// #define _DEBUG_LOG_
#ifdef _DEBUG_LOG_
#define DEBUG(format, args...) printf("[%s:%d] "format, __FILE__, __LINE__, ##args)
#else
#define DEBUG(format, args...) 
#endif
//video 
int SDLVIDEO_move(int row,int col);
int SDLUI_attrset(int attr);
int SDLUI_move(int row,int col);
int SDLUI_addch(uint8_t ch);
int SDLUI_addtext(int x, int y, uint8_t a, uint8_t ch);
int SDLUI_addlores(int x, int y, uint32_t c1,uint32_t c2);
int SDLUI_addhirespixel(int x, int y, uint32_t c);
int SDLUI_clearscr();


#include "appleii.h"



// #define UI_Initial(device) int UI##device_Initial(SDL_Renderer *renderer)
// #define UI_Event(device)   int UI##device_Event(SDL_Event event)
// #define UI_Release(device) int UI##device_Release()
// #define UI_Refresh(device) int UI##device_Refresh()
extern SDL_Window* window;

/* UI layout VIDEO */
int UIMAIN_Initial(SDL_Renderer *renderer);
int UIMAIN_Event(SDL_Event event);
int UIMAIN_Refresh(int *update);
int UIMAIN_Release();


int UIKEY_Initial(SDL_Renderer *renderer);
int UIKEY_Event(SDL_Event event);
int UIKEY_Refresh();
int UIKEY_Release();
/* UI layout VIDEO */
int UIVIDEO_Initial(SDL_Renderer *renderer);
int UIVIDEO_Event(SDL_Event event);
int UIVIDEO_Refresh(int *update);
int UIVIDEO_Release();
#ifdef __cplusplus
}
#endif
#endif