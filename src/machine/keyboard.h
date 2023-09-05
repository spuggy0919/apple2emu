#ifndef __KEYBOARD_H__
#define  __KEYBOARD_H__
#ifdef __cplusplus
extern "C" {
#endif

/* below code seqence is same as SDL defined*/
typedef enum { // keyboar layout scancode
    SCANCODE_SPACE=0,   // 0 
    SCANCODE_0,         // 1 
    SCANCODE_1,         // 2
    SCANCODE_2,         // 3 
    SCANCODE_3,         // 4 
    SCANCODE_4,         // 5 
    SCANCODE_5,         // 6 
    SCANCODE_6,         // 7 
    SCANCODE_7,         // 8 
    SCANCODE_8,         // 9 
    SCANCODE_9,         // 10 
    SCANCODE_COLON,     // 11
    SCANCODE_COMMA,     // 12
    SCANCODE_SEMICOLON, // 13  
    SCANCODE_MINUS,     // 14 
    SCANCODE_PERIOD,    // 15 
    SCANCODE_SLASH,     // 16
    SCANCODE_A,         // 17
    SCANCODE_B,         // 18
    SCANCODE_C,         // 19
    SCANCODE_D,         // 20
    SCANCODE_E,         // 21
    SCANCODE_F,         // 22
    SCANCODE_RETURN,    // 23 
    SCANCODE_G,         // 24
    SCANCODE_H,         // 25
    SCANCODE_I,         // 26
    SCANCODE_J,         // 27
    SCANCODE_K,         // 28
    SCANCODE_L,         // 29
    SCANCODE_M,         // 30
    SCANCODE_N,         // 31
    SCANCODE_O,         // 32
    SCANCODE_P,         // 33
    SCANCODE_Q,         // 34  
    SCANCODE_R,         // 35  
    SCANCODE_S,         // 36  
    SCANCODE_T,         // 37  
    SCANCODE_U,         // 38  
    SCANCODE_V,         // 39  
    SCANCODE_W,         // 40  
    SCANCODE_X,         // 41  
    SCANCODE_Y,         // 42  
    SCANCODE_Z,         // 43  
    SCANCODE_LEFT,      // 44 
    SCANCODE_RIGHT,     // 45 
    SCANCODE_ESCAPE,    // 46 
    SCANCODE_REPEAT,    // 47 
    SCANCODE_LCTRL,     // 48 
    SCANCODE_LSHIFT,    // 49  
    SCANCODE_RSHIFT,    // 50 
    SCANCODE_RESET,     // 51
    SCANCODE_POWER,     // 52 
    SCANCODE_END,       // 53
}SCANCODE;

typedef struct keymap
{
    // SCANCODE    code;
    char *      codestr;
    char        ascii[4]; // along, ctrl, shift, both
}KEYMAP;


// SDL Interfaces called by UIKey.c
int SDLHW_Key_PlugIn();
int SDLHW_Key_PlugOut();
int SDLHW_Key_Polling();// Repeat Key 
int SDLHW_Key_Input(int kbscancode, int ispress);

// HAL machine 
int HALKey_Reset();
int HALKey_Input(char keyascii);
uint8_t HALKey_Read(); //0xC000
int HALKey_Strobe(); //0xC010

#ifdef __cplusplus
}
#endif
#endif