#include "appleii.h"

#include <stdbool.h>
#include <stdio.h>

#define KEYNUM 53
KEYMAP Keys[KEYNUM]={
    // SCANCODE             Alone CTRL SHIFT BOTH
    {"SCANCODE_SPACE",      {0xA0,0xA0,0xA0,0xA0}},// 0 
    {"SCANCODE_0",          {0xB0,0xB0,0xB0,0xB0}},  // 1 
    {"SCANCODE_1",          {0xB1,0xB1,0xA1,0xA1}},  // 2
    {"SCANCODE_2",          {0xB2,0xB2,0xA2,0xA2}},  // 3 
    {"SCANCODE_3",          {0xB3,0xB3,0xA3,0xA3}},  // 4 
    {"SCANCODE_4",          {0xB4,0xB4,0xA4,0xA4}},  // 5 
    {"SCANCODE_5",          {0xB5,0xB5,0xA5,0xA5}},  // 6 
    {"SCANCODE_6",          {0xB6,0xB6,0xA6,0xA6}},  // 7 
    {"SCANCODE_7",          {0xB7,0xB7,0xA7,0xA7}},  // 8 
    {"SCANCODE_8",          {0xB8,0xB8,0xA8,0xA8}},  // 9 
    {"SCANCODE_9",          {0xB9,0xB9,0xA9,0xA9}},  // 10 
    {"SCANCODE_COLON",      {0xBA,0xBA,0xAA,0xAA}},  // 11
    {"SCANCODE_SEMICOLON",  {0xBB,0xBB,0xAB,0xBB}},  // 13  
    {"SCANCODE_COMMA",      {0xAC,0xAC,0xBC,0xBC}},  // 12
    {"SCANCODE_MINUS",      {0xAD,0xAD,0xBD,0xBD}},  // 14 
    {"SCANCODE_PERIOD",     {0xAE,0xAE,0xBE,0xBE}},  // 15 
    {"SCANCODE_SLASH",      {0xAF,0xAF,0xBF,0xBF}},  // 16
    {"SCANCODE_A",          {0xC1,0x81,0xC1,0x81}},  // 17
    {"SCANCODE_B",          {0xC2,0x82,0xC2,0x82}},  // 18
    {"SCANCODE_C",          {0xC3,0x83,0xC3,0x83}},  // 19
    {"SCANCODE_D",          {0xC4,0x84,0xC4,0x84}},  // 20
    {"SCANCODE_E",          {0xC5,0x85,0xC5,0x85}},  // 21
    {"SCANCODE_F",          {0xC6,0x86,0xC6,0x86}},  // 22
    {"SCANCODE_RETURN",     {0x8D,0x8D,0x8D,0x8D}},  // 23 
    {"SCANCODE_G",          {0xC7,0x87,0xC7,0x88}},  // 24
    {"SCANCODE_H",          {0xC8,0x88,0xC8,0x88}},  // 25
    {"SCANCODE_I",          {0xC9,0x89,0xC9,0x89}},  // 26
    {"SCANCODE_J",          {0xCA,0x8A,0xCA,0x8A}},  // 27
    {"SCANCODE_K",          {0xCB,0x8B,0xCB,0x8B}},  // 28
    {"SCANCODE_L",          {0xCC,0x8C,0xCC,0x8C}},  // 29
    {"SCANCODE_M",          {0xCD,0x8D,0xDD,0x9D}},  // 30
    {"SCANCODE_N",          {0xCE,0x8E,0xDE,0x9E}},  // 31
    {"SCANCODE_O",          {0xCF,0x8F,0xDF,0x8F}},  // 32
    {"SCANCODE_P",          {0xD0,0x90,0xC0,0x80}},  // 33
    {"SCANCODE_Q",          {0xD1,0x91,0xD1,0x91}},  // 34  
    {"SCANCODE_R",          {0xD2,0x92,0xD2,0x92}},  // 35  
    {"SCANCODE_S",          {0xD3,0x93,0xD3,0x93}},  // 36  
    {"SCANCODE_T",          {0xD4,0x94,0xD4,0x94}},  // 37  
    {"SCANCODE_U",          {0xD5,0x95,0xD5,0x95}},  // 38  
    {"SCANCODE_V",          {0xD6,0x96,0xD6,0x96}},  // 39  
    {"SCANCODE_W",          {0xD7,0x97,0xD7,0x97}},  // 40  
    {"SCANCODE_X",          {0xD8,0x98,0xD8,0x98}},  // 41  
    {"SCANCODE_Y",          {0xD9,0x99,0xD9,0x99}},  // 42  
    {"SCANCODE_Z",          {0xDA,0x9A,0xDA,0x9A}},  // 43  
    {"SCANCODE_RIGHT",      {0x88,0x88,0x88,0x88}},  // 44 
    {"SCANCODE_LEFT",       {0x95,0x95,0x95,0x95}},  // 45 
    {"SCANCODE_ESCAPE",     {0x9B,0x9B,0x9B,0x9B}},  // 46 
    {"SCANCODE_REPEAT",     {0x00,0x00,0x00,0x00}},  // 47 
    {"SCANCODE_LCTRL",      {0x00,0x00,0x00,0x00}},  // 48 
    {"SCANCODE_LSHIFT",     {0x00,0x00,0x00,0x00}},  // 49  
    {"SCANCODE_RSHIFT",     {0x00,0x00,0x00,0x00}},  // 50 
    {"SCANCODE_RESET",      {0x00,0x00,0x00,0x00}},  // 51
    {"SCANCODE_POWER",      {0x00,0x00,0x00,0x00}},  // 52    
};

int keyStatus = 0;
char lastkey = 0;

#define KSTATUS_STROBE      (1<<7)
#define KSTATUS_POWER       (1<<6)
#define KSTATUS_RESET       (1<<5)
#define KSTATUS_KEYIN       (1<<3)
#define KSTATUS_REPEAT      (1<<2)
#define KSTATUS_SHIFT       (1<<1)
#define KSTATUS_CTRL        (1)
#define SetStatusBit(x) {keyStatus|=(x);}
#define ClrStatusBit(x) {keyStatus&=~(x);}
#define TstStatusBit(x) (keyStatus&(x))


//TODO process repeat Key 
/* HAL emulator for apple II*/  
char HALKey_Queue(char key)
{
    return 0;
}
int HALKey_Reset(){
    return 0;
}
int HALKey_Input(char keyascii){
    // TODO for queue in Keyboard buffer if keyboard can not send

    if (!(HALMemory_GetIOKey()&0x80)) { // strobe clear 
        //  printf("HAL_KeyInput[%x]\n",keyascii);
         HWCLOCK_set_tickshrink();
         HALMemory_SetIOKey(keyascii|0x80); //0xc000 0xc010
    }
    return 0;
}
uint8_t HALKey_Read(){ //0xC000
    return readByte(0xC000);
}
int HALKey_Strobe(){ //0xC010
    return 0;
}






/* call by SDL UI Laysers */

int SDLHW_Key_PlugIn(){

    return 0;

}
int SDLHW_Key_PlugOut(){

    return 0;
}
// Repeat Key 
int SDLHW_Key_Polling(){
    if (TstStatusBit(KSTATUS_REPEAT)){
        if (!TstStatusBit(KSTATUS_KEYIN)){ // no keyin Status
            HALKey_Input(lastkey); // one time
            ClrStatusBit(KSTATUS_REPEAT); 
        }else{ // repeat + KeyIN until KEYINOFF 10HZ
            if (HWCLOCK_get_trigger10Hz()) {
                HALKey_Input(lastkey); // one time
                HWCLOCK_clr_trigger10Hz();
            }
        }
    }
    return 0;
}

int SDLHW_Key_Input(int kbscancode, int ispress){
    if (kbscancode>=SCANCODE_END) return 1; // out of range
    if (ispress) {
        switch(kbscancode){
        case   SCANCODE_REPEAT:  
                SetStatusBit(KSTATUS_REPEAT);
                break;
        case   SCANCODE_LCTRL:  
                SetStatusBit(KSTATUS_CTRL);
                break;
        case   SCANCODE_LSHIFT:  
        case   SCANCODE_RSHIFT:  
                SetStatusBit(KSTATUS_SHIFT);
                break;
        case   SCANCODE_RESET:  
                // SetStatusBit(KSTATUS_RESET);
                printf("Reset Key\n");
                reset_pin(0); // mod6502 low reset
                break;
        case   SCANCODE_POWER:  
                SetStatusBit(KSTATUS_POWER);
                break;
        default:
                SetStatusBit(KSTATUS_KEYIN);
                lastkey = Keys[kbscancode].ascii[keyStatus&3];
                HALKey_Input(lastkey);
                SetStatusBit(KSTATUS_POWER);
                break;
        }
    }else{
        switch(kbscancode){
        case   SCANCODE_REPEAT:  
                ClrStatusBit(KSTATUS_REPEAT);
                break;
        case   SCANCODE_LCTRL:  
                ClrStatusBit(KSTATUS_CTRL);
                break;
        case   SCANCODE_LSHIFT:  
        case   SCANCODE_RSHIFT:  
                ClrStatusBit(KSTATUS_SHIFT);
                break;
        case   SCANCODE_RESET:  
                printf("Reset Key Off\n");
               reset_pin(1); // mod6502 low reset
                break;
        case   SCANCODE_POWER:  
        default:
                ClrStatusBit(KSTATUS_KEYIN);
                break;
        }        
    }
    return 0;
}
