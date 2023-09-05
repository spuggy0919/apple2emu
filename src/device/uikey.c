
#include "ui.h"
#include <string.h>

#define TILESIZE 50
#define SHADOW_DEPTH 5
#define KB_X 12
#define KB_Y 400
#define FONTSIZE 20
#define SFONTSIZE 14
#define KEYW TILESIZE
#define KEYH TILESIZE
#define KEYBOARDPOSX 
#define KEYBOARDPOSY 23


typedef struct {
    SDL_Rect rect;
    char *labels[2];
    bool ispressed;
    bool ischanged;
    int  scancode; // USB Keyboard scancode
} KEYPAD;

// GUI LAYOUT 
#define ROW0(i) {KB_X+KEYW/2+i*KEYW,0+KB_Y,KEYW,KEYH}
#define ROW1(i) {KB_X+0+i*KEYW,KEYH+KB_Y,KEYW,KEYH}
#define ROW1W(i) {KB_X+0+i*KEYW,KEYH+KB_Y,KEYW*3/2,KEYH}
#define ROW2(i) {KB_X+KEYW/4+i*KEYW,KEYH*2+KB_Y,KEYW,KEYH}
#define ROW3W(i) {KB_X+KEYW/4+i*KEYW,KEYH*3+KB_Y,KEYW*3/2,KEYH}
#define ROW3(i) {KB_X+KEYW*3/4+i*KEYW,KEYH*3+KB_Y,KEYW,KEYH}
#define ROW3W2(i) {KB_X+KEYW*3/4+i*KEYW,KEYH*3+KB_Y,KEYW*3/2,KEYH}
#define ROW4(i) {KB_X+KEYW/4+i*KEYW,KEYH*4+KB_Y,KEYW,KEYH}
#define ROW4B(i) {KB_X+KEYW+i*KEYW+KEYW*3/4,KEYH*4+KB_Y,KEYW*8,KEYH}
#define KEYNUMS 53

// below Table sequence should be same as HAL_Keyboard.c
KEYPAD keys[KEYNUMS]= {
    // rect         print       notebook mac keyboard
  { ROW4B( 1), {"    "},  0,1,SDL_SCANCODE_SPACE},// 1 
  { ROW0( 9), {"0"," "},  0,1,SDL_SCANCODE_0},// 9 
  { ROW0( 0), {"1","!"},  0,1,SDL_SCANCODE_1},// 0 
  { ROW0( 1),{"2","\""},  0,1,SDL_SCANCODE_2},// 1 
  { ROW0( 2), {"3","#"},  0,1,SDL_SCANCODE_3},// 2
  { ROW0( 3), {"4","$"},  0,1,SDL_SCANCODE_4},// 3 
  { ROW0( 4), {"5","%"},  0,1,SDL_SCANCODE_5},// 4 
  { ROW0( 5), {"6","&"},  0,1,SDL_SCANCODE_6},// 5 
  { ROW0( 6), {"7","'"},  0,1,SDL_SCANCODE_7},// 6 
  { ROW0( 7), {"8","("},  0,1,SDL_SCANCODE_8},// 7 
  { ROW0( 8), {"9",")"},  0,1,SDL_SCANCODE_9},// 8 
  { ROW0(10), {":","*"},  0,1,SDL_SCANCODE_MINUS},// 10 
  { ROW2(10), {";","+"},  0,1,SDL_SCANCODE_SEMICOLON},// 10 
  { ROW3( 8), {",","<"},  0,1,SDL_SCANCODE_COMMA},// 8 
  { ROW0(11), {"-","="},  0,1,SDL_SCANCODE_EQUALS},// 11
  { ROW3( 9), {".",">"},  0,1,SDL_SCANCODE_PERIOD},// 9 
  { ROW3(10), {"/","?"},  0,1,SDL_SCANCODE_SLASH},// 10 
  { ROW2( 1), {"A"," "},  0,1,SDL_SCANCODE_A},// 1 
  { ROW3( 5), {"B"," "},  0,1,SDL_SCANCODE_B},// 5 
  { ROW3( 3), {"C"," "},  0,1,SDL_SCANCODE_C},// 3 
  { ROW2( 3), {"D"," "},  0,1,SDL_SCANCODE_D},// 3 
  { ROW1( 3), {"E"," "},  0,1,SDL_SCANCODE_E},// 3 
  { ROW2( 4), {"F"," "},  0,1,SDL_SCANCODE_F},// 4 
  { ROW1W(12),{"RETURN"}, 0,1,SDL_SCANCODE_RETURN},// 12
  {ROW2( 5),{"G","BELL"}, 0,1,SDL_SCANCODE_G},// 5 
  { ROW2( 6), {"H"," "},  0,1,SDL_SCANCODE_H},// 6 
  { ROW1( 8), {"I"," "},  0,1,SDL_SCANCODE_I},// 8 
  { ROW2( 7), {"J"," "},  0,1,SDL_SCANCODE_J},// 7 
  { ROW2( 8), {"K"," "},  0,1,SDL_SCANCODE_K},// 8 
  { ROW2( 9), {"L"," "},  0,1,SDL_SCANCODE_L},// 9 
  { ROW3( 7), {"M"," "},  0,1,SDL_SCANCODE_M},// 7 
  { ROW3( 6), {"N","^"},  0,1,SDL_SCANCODE_N},// 6 
  { ROW1( 9), {"O"," "},  0,1,SDL_SCANCODE_O},// 9 
  { ROW1(10), {"P","@"},  0,1,SDL_SCANCODE_P},// 10 
  { ROW1( 1), {"Q"," "},  0,1,SDL_SCANCODE_Q},// 1 
  { ROW1( 4), {"R"," "},  0,1,SDL_SCANCODE_R},// 4 
  { ROW2( 2), {"S"," "},  0,1,SDL_SCANCODE_S},// 2
  { ROW1( 5), {"T"," "},  0,1,SDL_SCANCODE_T},// 5 
  { ROW1( 7), {"U"," "},  0,1,SDL_SCANCODE_U},// 7 
  { ROW3( 4), {"V"," "},  0,1,SDL_SCANCODE_V},// 4 
  { ROW1( 2), {"W"," "},  0,1,SDL_SCANCODE_W},// 2
  { ROW3( 2), {"X"," "},  0,1,SDL_SCANCODE_X},// 2
  { ROW1( 6), {"Y"," "},  0,1,SDL_SCANCODE_Y},// 6 
  { ROW3( 1), {"Z"," "},  0,1,SDL_SCANCODE_Z},// 1 
  { ROW2(11), {"<-"},     0,1,SDL_SCANCODE_LEFTBRACKET},// 11
  { ROW2(12), {"->"},     0,1,SDL_SCANCODE_RIGHTBRACKET},// 12
  { ROW1( 0), {"ESC"}  ,  0,1,SDL_SCANCODE_ESCAPE},// 0 
  { ROW1(11), {"REPT"} ,  0,1,SDL_SCANCODE_APOSTROPHE},// 11
  { ROW2( 0), {"CTRL"},   0,1,SDL_SCANCODE_LCTRL},// 0 
  { ROW3W( 0),{"SHIFT"},  0,1,SDL_SCANCODE_LSHIFT},// 0 
  {ROW3W2(11),{"SHIFT"},  0,1,SDL_SCANCODE_RSHIFT},// 11
  { ROW0(12), {"RESET"},  0,1,SDL_SCANCODE_BACKSPACE},// 12
  { ROW4( 0), {"POWER"},  0,1,SDL_SCANCODE_LALT},// 0 
};
#define KEYMAP 256
int keyLUT[KEYMAP];
SDL_Color keyColors[KEYNUMS];
SDL_Color releaseColor = { 90, 90, 0, 255 };
SDL_Color pressColor = { 70, 70, 70, 255 };
SDL_Color textColor = { 255, 255, 255, 255 };

/* SDL Functions*/
SDL_Color randomColor() {
    SDL_Color color;
    color.r = rand() % 256;
    color.g = rand() % 256;
    color.b = rand() % 256;
    color.a = 255;  // Opaque
    return color;
}
TTF_Font *font,*fonts; // for different size
int initialfont()
{
    if (TTF_Init() != 0) {
        printf("TTF initialization error: %s\n", TTF_GetError());
        // SDL_Quit();
        return 1;
    }
    font = TTF_OpenFont("./AGFOR26.TTF", FONTSIZE); 
    fonts = TTF_OpenFont("./AGFOR26.TTF", SFONTSIZE); 
    // Replace with the actual path to your font
    if (!font) {
        printf("Font load error: %s\n", TTF_GetError());
        // SDL_DestroyRenderer(renderer);
        // SDL_DestroyWindow(window);
        // TTF_Quit();
        // SDL_Quit();
        return 1;
    }
    return 0;
}
void printLabel(SDL_Renderer* renderer,int kidx, SDL_Rect *rect){
    // Draw button label
    if (strlen(keys[kidx].labels[0])>1) { 
        // String for RESET RETURN ... center align
        SDL_Surface* textSurface = TTF_RenderText_Solid(fonts, keys[kidx].labels[0], textColor);
        SDL_Texture* textTexture = SDL_CreateTextureFromSurface(renderer, textSurface);
        int textWidth = textSurface->w;
        int textHeight = textSurface->h;
        SDL_Rect textRect = { rect->x + (rect->w - textWidth) / 2, rect->y + (rect->h - textHeight) / 2,
                                textWidth, textHeight };
        SDL_RenderCopy(renderer, textTexture, NULL, &textRect);
        SDL_DestroyTexture(textTexture);
        SDL_FreeSurface(textSurface);
        
    }
    else
    {
        if (kidx!=31) { // not G BELL 
            SDL_Surface* textSurface = TTF_RenderText_Solid(font, keys[kidx].labels[0], textColor);
            SDL_Texture* textTexture = SDL_CreateTextureFromSurface(renderer, textSurface);
            int textWidth = textSurface->w;
            int textHeight = textSurface->h;
            SDL_Rect textRect = { rect->x + (rect->w - textWidth) / 2, rect->y + (rect->h - textHeight) * 7 / 8,
                                    textWidth, textHeight };
            SDL_RenderCopy(renderer, textTexture, NULL, &textRect);
        SDL_DestroyTexture(textTexture);
        SDL_FreeSurface(textSurface);
            textSurface = TTF_RenderText_Solid(font, keys[kidx].labels[1], textColor);
            textTexture = SDL_CreateTextureFromSurface(renderer, textSurface);
            textWidth = textSurface->w;
            textHeight = textSurface->h;
            textRect.y = rect->y + (rect->h - textHeight) * 1 / 8;
            SDL_RenderCopy(renderer, textTexture, NULL, &textRect);
        SDL_DestroyTexture(textTexture);
        SDL_FreeSurface(textSurface);
        }else  {
            SDL_Surface* textSurface = TTF_RenderText_Solid(font, keys[kidx].labels[0], textColor);
            SDL_Texture* textTexture = SDL_CreateTextureFromSurface(renderer, textSurface);
            int textWidth = textSurface->w;
            int textHeight = textSurface->h;
            SDL_Rect textRect = { rect->x + (rect->w - textWidth) / 2, rect->y + (rect->h - textHeight) * 7 / 8,
                                    textWidth, textHeight };
            SDL_RenderCopy(renderer, textTexture, NULL, &textRect);
        SDL_DestroyTexture(textTexture);
        SDL_FreeSurface(textSurface);
            textSurface = TTF_RenderText_Solid(fonts, keys[kidx].labels[1], textColor);
            textTexture = SDL_CreateTextureFromSurface(renderer, textSurface);
            textWidth = textSurface->w;
            textHeight = textSurface->h;
            textRect.y = rect->y + (rect->h - textHeight) * 1 / 8;
            SDL_RenderCopy(renderer, textTexture, NULL, &textRect);
        SDL_DestroyTexture(textTexture);
        SDL_FreeSurface(textSurface);
        }
    }
}
int cache =1000;
void drawButton(SDL_Renderer* renderer,int kidx){
    if (keys[kidx].ischanged){ 
        SDL_Color *color = (keys[kidx].ispressed) ? &pressColor : &releaseColor;
        int shadow = (keys[kidx].ispressed) ? SHADOW_DEPTH : 2;
        SDL_Rect rect = {keys[kidx].rect.x + shadow, keys[kidx].rect.y + shadow, keys[kidx].rect.w - shadow , keys[kidx].rect.h - shadow};
        SDL_SetRenderDrawColor(renderer, color->r, color->g, color->b, color->a);
        SDL_RenderFillRect(renderer, &rect);
        // print label
        printLabel(renderer, kidx, &rect);
        // keys[kidx].ischanged = false;// dispaper by dual display buffer

    }
}
SDL_Renderer* renderer;
int  UIKEY_Initial(SDL_Renderer* irenderer)
{
    // font
    renderer = irenderer;
    if (initialfont()){
        return 1;
    }
    // Draw grid cells with their colors
    for (int i = 0; i < KEYNUMS; ++i) {
        keys[i].ischanged = true; // force drawing
        drawButton(renderer, i);
    }
    for (int i = 0; i < KEYMAP; ++i){
        keyLUT[i] = -1;
        for (int j = 0; j < KEYNUMS; j++){
            if (i==keys[j].scancode) {
                keyLUT[i] = j;
                break;
            }
        }
    }

        return 0;
}
static int keyFound=1;
int KeyMouseEventCheck(SDL_Event event)
{
        int mouseX = event.button.x;
        int mouseY = event.button.y;
        // ROW 0 
        int i;
        for (i=0;i<KEYNUMS;i++) {
            // printf("keyckeck %d mouseXY=(%d,%d) keysY(%d,%d)\n",i,mouseX,mouseY,keys[i].rect.y,keys[i].rect.y + keys[i].rect.h);
            if (!(mouseY >= keys[i].rect.y && mouseY <= keys[i].rect.y + keys[i].rect.h)){
                 continue; // no row 0~3
            }
            if (mouseX >= keys[i].rect.x && mouseX <= keys[i].rect.x + keys[i].rect.w ) {
                keys[i].ispressed = event.type == SDL_MOUSEBUTTONDOWN;
                keys[i].ischanged = true;
                SDLHW_Key_Input(i, keys[i].ispressed );
                keyFound=1;
                return i;
            }
        }
        return -1;
}
int UIKEY_Event(SDL_Event event)
{
    if (event.type == SDL_MOUSEBUTTONDOWN) {
        KeyMouseEventCheck(event);
        return 0;
    }
    if (event.type == SDL_MOUSEBUTTONUP) {
        KeyMouseEventCheck(event);
        return 0;
    }
    if (event.type == SDL_KEYDOWN) {
        SDL_Scancode scancode = event.key.keysym.scancode;
        const char* keyName = SDL_GetKeyName(SDL_GetKeyFromScancode(scancode));
        printf("Down Scancode: %d,[%d], Key: %s\n", scancode, keyLUT[scancode],keyName);
        if (keyLUT[scancode]!=-1){
                keys[keyLUT[scancode]].ispressed = true;
                keys[keyLUT[scancode]].ischanged = true; 
                SDLHW_Key_Input(keyLUT[scancode], true);
                keyFound=1;
        }
        return 0;
    }
    if (event.type == SDL_KEYUP) {
        SDL_Scancode scancode = event.key.keysym.scancode;
        const char* keyName = SDL_GetKeyName(SDL_GetKeyFromScancode(scancode));
        printf("UP Scancode: %d,[%d], Key: %s\n", scancode, keyLUT[scancode],keyName);
        if (keyLUT[scancode]!=-1){
                keys[keyLUT[scancode]].ispressed = false;
                keys[keyLUT[scancode]].ischanged = true;  
                SDLHW_Key_Input(keyLUT[scancode], false);
            
        }
    }
    return 0;
}

int UIKEY_Refresh()
{
    DEBUG("UIKEY_Refresh\n");
    SDLHW_Key_Polling();
    // Draw grid cells with their colors
    if (keyFound){
        for (int i = 0; i < KEYNUMS; ++i) {
            drawButton(renderer, i);    
        }
        // keyFound=0;
    }
    
    return 0;
}
int UIKEY_Release()
{
    SDLHW_Key_PlugOut();

    // Cleanup and shutdown SDL
    TTF_CloseFont(font);
    TTF_CloseFont(fonts);
    return 0;
}
