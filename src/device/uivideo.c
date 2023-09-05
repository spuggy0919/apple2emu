// #define _MAIN_TEST_

#ifndef _MAIN_TEST_
#include "ui.h"
#else

#include "uivideo.h"

#endif


#define WINDOW_WIDTH 800
#define WINDOW_HEIGHT 600
#define BUTTON_WIDTH 200
#define BUTTON_HEIGHT 50
SDL_Color buttonColor = { 100, 100, 100, 255 };


char *fontpath="./pr-pua-e0-e1.png";
const int charWidth = 8; // Adjust this based on your character size
const int charHeight = 8;
int xoff = 1;
int yoff = 0;
#define PAGE (17*0)
#define CHAR_RECT(c) {(int)12*(((c>>4)&0xf)+PAGE)+12+2+xoff,(int)11*(c&0xf)+28+yoff,charWidth,charHeight}

#define VD_X 70
#define VD_Y 8
#define VD_W 560
#define VD_H 384
#define SCALE 2
SDL_Rect monitor= {VD_X, VD_Y, VD_W, VD_H};
#define ROWCOL(r,c) {(int)c*(charWidth-1)*SCALE+VD_X,(int)r*charHeight*SCALE+VD_Y,(charWidth-1)*SCALE,charHeight*SCALE}
#define RIXELRECT(x,y) {(int)x*SCALE+VD_X,(int)y*SCALE+VD_Y,SCALE,SCALE}

SDL_Color bkColor = { 3, 3, 3, 255 };
/* UI layout VIDEO */

SDL_Renderer *renderer;
SDL_Surface *fontSurface;
SDL_Texture *fontTexture;
SDL_Texture *modifiedTexture;
SDL_Rect charRect = {0,0,charWidth,charHeight};

int pngWidth,pngHeight;

void clearmonitor(SDL_Renderer* renderer,SDL_Rect *rect){
        SDL_SetRenderDrawColor(renderer, bkColor.r, bkColor.g, bkColor.b, bkColor.a);
        SDL_RenderFillRect(renderer, &monitor);
}

int setbackgroundAlpha(SDL_Surface *fontsur){
        // Font png
        SDL_QueryTexture(fontTexture,NULL,NULL,&pngWidth,&pngHeight);
        // Uint32 *pixels = malloc(pngWidth * pngHeight * sizeof(Uint32));
        // memcpy(pixels, fontSurface->pixels, pngWidth * pngHeight * sizeof(Uint32));
        Uint32 *pixels = fontSurface->pixels;
        // Define the color you want to change and the new color
        Uint32 targetColor = SDL_MapRGBA(fontSurface->format, 255, 255, 255,255); // Magenta
        Uint32 newColor = SDL_MapRGBA(fontSurface->format, 255, 255, 255,0); // Green
        for(int i=0;i<pngWidth*pngHeight;i++){
            // SDL_GetRGBA(pixels[i], imageSurface->format, &r, &g, &b, &a);
            if (pixels[i]==targetColor){
                // Modify the pixel's color (e.g., make it red)
                pixels[i] = newColor;
            }    
        }
    // Create a new texture with modified pixels
    // modifiedTexture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA32, SDL_TEXTUREACCESS_STATIC, pngWidth, pngHeight);
    modifiedTexture = SDL_CreateTextureFromSurface(renderer, fontSurface);
    // SDL_UpdateTexture(modifiedTexture, NULL, pixels, pngWidth* pngHeight * sizeof(Uint32));
    // free(pixels);
    return 0;
}

SDL_Texture *getBitmapFont(char c, char attr, int w, int h){
    SDL_Rect clipRect  = CHAR_RECT(c); // from fontTexture
    //
    // Render the texture onto surface A
        // Create a target texture for the character
    // Create a new char surface
    SDL_Surface *charSurface = SDL_CreateRGBSurfaceWithFormat(0, w, h, 32, SDL_PIXELFORMAT_RGBA32);
    SDL_BlitSurface(fontSurface, &clipRect, charSurface, NULL);

    // Lock the char surface for direct pixel access
    SDL_LockSurface(charSurface); 
    // modified char attribute
    // Access pixel data of surface A
    Uint32 fgColor = SDL_MapRGBA(charSurface->format, 0, 0, 0,255); // Magenta
    Uint32 bkColor = SDL_MapRGBA(charSurface->format, 0, 0, 0,0); // Magenta
    Uint32 nfgColor = SDL_MapRGBA(charSurface->format, 255, 255, 255,255); // foreground
    Uint32 nbkColor = SDL_MapRGBA(charSurface->format, 1, 1, 1,255); // background
    Uint32 newfgColor  = (attr==A_NORMAL) ? nfgColor:nbkColor;
    Uint32 newbkColor  = (attr==A_NORMAL) ? nbkColor:nfgColor;
 // Modify pixel data in the char surface
    Uint32 *pixels = (Uint32 *)charSurface->pixels;
    int charPitch = charSurface->pitch / sizeof(Uint32);
    for (int y = 0; y < h; y++) {
        for (int x = 0; x < w; x++) {
            int i = y * charPitch + x;
            if (pixels[i]==bkColor) pixels[i]=newbkColor;
            if (pixels[i]==fgColor) pixels[i]=newfgColor;
        }
    }

    // Unlock the char surface
    SDL_UnlockSurface(charSurface);
    // Convert the char surface to a char texture
    SDL_Texture *charTexture = SDL_CreateTextureFromSurface(renderer, charSurface);
    return charTexture;

}

int r,c,a=0,b=0,bb=0,cnt=0;
int SDLUI_move(int row,int col)
{
    r=row;
    c=col;
    return 0;
}
int SDLUI_attrset(int attr){
    a=attr;
    if (a==A_BLINK) b=(bb)? A_NORMAL:A_INVERSE;
    else b = a;
    return 0;
}
int blink;
int change=1;
int SDLUI_frameSyncBlink(){
    change=1;
#ifndef _MAIN_TEST_
    bb=tickTock_2Hz;
#else 
    bb=blink++;
    if (blink==20) blink=0;
#endif
    return 0;
}
int SDLUI_clearscr()
{
    change=1;
    clearmonitor(renderer, &monitor);
    return 0;
}

int SDLUI_addch(uint8_t ch)
{
    change=1;
    SDL_Texture *charTexture = getBitmapFont(ch, b, charWidth, charHeight); // get bitmapfont
    SDL_Rect dstRect=ROWCOL(r,c);
    SDL_RenderCopy(renderer, charTexture, &charRect, &dstRect);
    SDL_DestroyTexture(charTexture);        
    return 0;
}
int SDLUI_addtext(int x, int y, uint8_t a, uint8_t ch){
    // printf("SDLUI_addtext(%d,%d)-%d-%d\n",x,y,a,ch);
    change=1;
    SDL_Texture *charTexture = getBitmapFont(ch, b, charWidth, charHeight); // get bitmapfont
    SDL_Rect dstRect=ROWCOL(y,x); 
    SDL_RenderCopy(renderer, charTexture, &charRect, &dstRect);
    SDL_DestroyTexture(charTexture);   
    return 0;

}
int SDLUI_addlores(int x, int y, uint32_t c1,uint32_t c2){
    SDL_Rect dstRect=ROWCOL(y,x);
    // upprer half
    change=1;
    dstRect.h/=2; 
    SDL_SetRenderDrawColor(renderer, (c1&0xFF0000)>>16,  (c1&0xFF00)>>8,  (c1&0xFF),  255);
    SDL_RenderFillRect(renderer, &dstRect);
    dstRect.y+=dstRect.h;
    SDL_SetRenderDrawColor(renderer, (c2&0xFF0000)>>16,  (c2&0xFF00)>>8,  (c2&0xFF),  255);
    SDL_RenderFillRect(renderer, &dstRect);

    return 0;

}
int SDLUI_addhirespixel(int x, int y, uint32_t c){
    change=1;
    SDL_Rect dstRect=RIXELRECT(x,y);
    SDL_SetRenderDrawColor(renderer, (c&0xFF0000)>>16,  (c&0xFF00)>>8,  (c&0xFF),  255);
    SDL_RenderFillRect(renderer, &dstRect);
    return 0;
}

int UIVIDEO_Initial(SDL_Renderer *irenderer){
        renderer = irenderer;
        clearmonitor(renderer, &monitor);
        IMG_Init(IMG_INIT_PNG);
        // Load font image
        fontSurface = IMG_Load(fontpath); // Replace with your font image file
        fontTexture = SDL_CreateTextureFromSurface(renderer, fontSurface);
    // Load the full font texture (PNG image)
        // fontTexture = IMG_LoadTexture(renderer,fontpath);
        setbackgroundAlpha(fontSurface);
        // Font png
        SDL_QueryTexture(fontTexture,NULL,NULL,&pngWidth,&pngHeight);

        return 0;
}

int UIVIDEO_Event(SDL_Event event){

        return 0;
}
int UIVIDEO_Refresh(int *update){
    DEBUG("UIVIDEO_Refresh\n");
        // printf("UIVIDEO_Refresh\n ");
        // clearmonitor(renderer, &monitor);
        // SDLUI_frameSyncBlink();
        SDLHW_VIDEO_Refresh();  
        *update=1; change=0;
        // SDL_SetTextureColorMod(fontTexture, 0, 255, 255);
        // SDL_SetTextureAlphaMod(fontTexture, 128);
        // SDL_Rect pngRect = {200, 200, pngWidth*2, pngHeight*2};
        // SDL_RenderCopy(renderer, fontTexture, NULL, &pngRect);
  
        // SDL_SetTextureColorMod(fontTexture, 0, 255, 0);

         // Iterate through the characters in the string
        // String to render
        // int scale = 2;
        // const char *text = "AHello, World!";
        // SDL_Rect destRect = {100, 50, charWidth*scale, charHeight*scale};
        // SDL_Rect dest2Rect = {100, 120, charWidth*scale, charHeight*scale};

        // for (int i = 0; text[i] != '\0'; ++i) {
        //     char asciiChar = text[i];
        //         SDL_Rect srcRect  = CHAR_RECT(asciiChar); // from fontTexture
        //         SDL_RenderCopy(renderer, fontTexture, &srcRect, &destRect); //no attribute

        //         SDL_Texture *charTexture = getBitmapFont(asciiChar, 0, charWidth, charHeight);
        //         // Calculate the source rectangle based on character index
        //         // printf("%c(%d,%d)-(%d,%d)\n",text[i],charRect.x,charRect.y,charRect.w,charRect.h);
        //         // Destination rectangle for rendering
        //         // Render the character &sourceRect
        //         SDL_RenderCopy(renderer, charTexture, &charRect, &dest2Rect);
        //         SDL_DestroyTexture(charTexture);

        //         // Increment x-coordinate for the next character
        //         destRect.x += (charWidth-1)*scale;
        //         dest2Rect.x += (charWidth-1)*scale;

        // }
        return 0;
}
int UIVIDEO_Release(){
        SDL_FreeSurface(fontSurface);
        IMG_Quit();
        return 0;
    }
#ifdef _MAIN_TEST_
int main() 
{
    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        printf("SDL initialization error: %s\n", SDL_GetError());
        return 1;
    }

    SDL_Window* window = SDL_CreateWindow("Bitmap Font Example", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                                          WINDOW_WIDTH, WINDOW_HEIGHT, SDL_WINDOW_SHOWN);
    if (!window) {
        printf("Window creation error: %s\n", SDL_GetError());
        SDL_Quit();
        return 1;
    }

    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (!renderer) {
        printf("Renderer creation error: %s\n", SDL_GetError());
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    UIVIDEO_Initial(renderer);

    SDL_Event event;
    int quit = 0;


    while (!quit) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                quit = 1;
            }
            UIVIDEO_Event(event);
        }
        // Clear the screen
        // SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
        // SDL_RenderClear(renderer);
        int update=0;
        UIVIDEO_Refresh(&update);


        if (update) SDL_RenderPresent(renderer);
    }

    UIVIDEO_Release();

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);

    SDL_Quit();

    return 0;
}
#endif