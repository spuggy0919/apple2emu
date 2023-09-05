#include "ui.h"

// this function is call from top layer SDL main loop HiApple.c
/* UI layout VIDEO */
int UIMAIN_Initial(SDL_Renderer *renderer){
    HALAPPLEII_Reset();

    UIKEY_Initial(renderer);
    UIVIDEO_Initial(renderer);

    return 0;
}
int UIMAIN_Event(SDL_Event event){
    UIKEY_Event(event);
    return 0;
}
int UIMAIN_Refresh(int *update){
    
    UIVIDEO_Refresh(update);
    HALAPPLEII_Running();
    UIKEY_Refresh();

    return 0;
}
int UIMAIN_Release(){
    UIKEY_Release();    
    UIVIDEO_Release();    
    return 0;
}
