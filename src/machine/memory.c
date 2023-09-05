#include "cpu6502.h"
#include "memory.h"
/*
 * memory and io mapping
 * author:spuggy0919
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */
#define ROMSTART  0xD000
#define ROMSIZE   0x3000    // 12KB
#define IOSTART   0xC000
#define IOSIZE    0x1000    // TODO 4K 
#define S16START  0xC600
#define S16SIZE   0x100    // 256

#define RAMSIZE   0xC000    // 48KB
// MEMORY AND I/O with static decode APPLE II IOMAP
uint8_t memory[0x10000];
uint8_t *ram=memory;            // 48KB 0x0000
uint8_t *rom=&memory[ROMSTART]; // 12KB 0xC000
// language card 12K memory 
#define LGCSTART 0xD000
#define LGCSIZE  0x3000
#define BK2START 0xD000
#define BK2SIZE  0x1000
uint8_t lgc[LGCSIZE];   // Language Card 12K in $D000-$FFFF
uint8_t bk2[BK2SIZE];   // bank 2 of Language Card 4K in $D000-$DFFF

// disk ][ prom
#define SL6START 0xC600
#define SL6SIZE  0x0100
uint8_t sl6[SL6SIZE];   // P5A disk ][ prom in slot 6


// IO map Table 
// TODO IORegistry will be modified for realtime respone by registry function

typedef void (*readf)(uint16_t addr);
typedef uint8_t (*writef)(uint16_t addr);
// typedef struct {
//     uint16_t addr;
//     readf   r;
//     writef  w;
// }IO;
// IO  ioport[IOSIZE];


// keyboard static io map
uint8_t key=0; 
uint8_t video_sw[4]={true, false, false, false}; 
uint8_t videoNeedsRefresh=true;

uint8_t swSwitches(uint16_t address,uint8_t value, uint8_t rw){
  if (address == 0xC000)  {
//    if (key&0x80) printf("Key:=%x\n",key);
    return(key);          // KBD
  }
  if (address == 0xC010){                        // KBDSTRB
    // if (key&0x80) printf("Keystrobe:=%x\n",key);
    key &= 0x7F;                                 // unset bit 7
    return(key);
  }
  
  if ((address&0xFFF0) == 0xC050) {
    //   printf("IOAddress=%4x %4x ",address,address&0x7);
      video_sw[(address&0x6)>>1] = address&1; 
    //   printf("-IOAddress=%4x %4x ",address,address&0x7);

  }
  if ((address&0xFFF0) == 0xC0E0) {
        return  diskio(address, 0, rw) ;

  }
  return 0;
}

uint8_t readByte(uint16_t address){
  if (address >= ROMSTART) {
		if (!LCRD) // ram disable read rom
            return(memory[address]);
		if (LCBK2 && (address < 0xE000)) // BANK 2 enable
			return bk2[address - BK2START];  
		return lgc[address - LGCSTART];    // bank 0 ;     
  }      
  if (address <  RAMSIZE)  return(ram[address]);
  if ((address & 0xFF00) == SL6START) //0xc600 
		return sl6[address - SL6START];                                             // disk][

  // Key IO
  if ((address&0xFF00) == IOSTART){
		  return swSwitches(address, 0, false); // Soft Switches
  }
  //Language card
                                     
        //  HWCLOCK_set_tickshrink();

                                  

//   if ((address&0xFFF0) == 0xC0E0) {
//         return  diskio(address, 0, false);

//   }
  return 0;                                    // catch all
}
uint16_t readWord(uint16_t address)
{
    return ((readByte(address)) | ((uint16_t)readByte(address+1)<<8));
}

void copyBlock(uint16_t address, uint16_t len, uint8_t *dst)// for video 
{
    memcpy(&ram[address],dst,len);
}


void writeByte(uint16_t address, uint8_t value){
    if (address & 0x400) videoNeedsRefresh = true; // a change in text page 1
    if (address < RAMSIZE) ram[address] = value;
//   else if (address == 0xC010) key &= 0x7F;       // KBDSTRB, as in readMem
	if (LCWR && (address >= ROMSTART)) { // language card bank2 
		if (LCBK2 && (address < 0xE000)) {
			bk2[address - BK2START] = value;                                          // BK2
			return;
		}
		lgc[address - LGCSTART] = value;                                            // LC
		return;
	}
	if ((address & 0xFF00) == IOSTART) {
		  swSwitches(address, value, true);                                         // Soft Switches
		  return;
	}
    //   if ((address&0xFFF0) == 0xC0E0) {
    //       diskio(address, 0, true);
    //   }
      return;
}

void writeWord(uint16_t address,uint16_t value)
{
    writeByte(address,value&0xff);
    writeByte(address+1,(value>>8)&0xff);
}

// Example custom function to convert integer to hexadecimal string
const char* intToHex(int value) {
    static char hexString[9]; // Assuming a 32-bit integer (8 characters + null-terminator)
    snprintf(hexString, sizeof(hexString), "0x%X", value);
    return hexString;
}
#define TARGET_EXTENSION ".rom"
int  load_File_to_memory(char *filename, char *ext, BYTE memory[])
{

    // Check if the filename has the required extension
    char *extension = strrchr(filename, '.');

    if (ext != NULL && strcmp(extension, ext) != 0) {
        printf("Invalid file extension. The file must have the (%s) \"%s\" %d extension.\n", extension,TARGET_EXTENSION,strcmp(extension, ext));
        return 1;
    }

    // Open the file as a binary file for reading
    FILE *file = fopen(filename, "rb");
    if (file == NULL) {
        perror("Error opening the file");
        return 1;
    }

    // Read and process the contents of the file here
    // ...
    int n=fread(memory,1,65536,file);

    // Close the file after reading
    fclose(file);

    printf("File \"%s\" successfully read(size=%x).\n", filename,n);
    return n;
}

uint8_t * HALMemory_point()
{
    return memory;
}
// Memory HAL Interface
void HALMemory_Reset(){
    key=0;
    // 0xC050 0      0xC052 1 FULL 0xC054  page 1 0xC056 1 LORES
    // 0xC051 1 TEXT 0xC053 0      0xC055  0      0xC057 0
    // load Rom from file
    load_File_to_memory("appleII+.rom", NULL, rom);   
    load_File_to_memory("diskii.rom", NULL, sl6);   
    
    insertFloppy("DOS 3.3.nib", 0) ; 

    insertFloppy("diskwrite.nib", 1) ; 
    return ;
}

// Keyboard IO functions
void HALMemory_SetIOKey(uint8_t keyin)
{
    key = keyin|0x80;
}
uint8_t HALMemory_GetIOKey()
{
    return key;
}
// Video IO functions
uint8_t* HALMemory_GetVideoSwitches()
{
    return video_sw;
}
void HALMemory_GetVideoFrameData(uint16_t addr, uint16_t len, uint8_t *dstmem){
     copyBlock(addr, len, dstmem);// for video 
    //   char c = 0xB1;
    //  for (int i=0;i<0x100;i++){
    //      dstmem[i]=c++; // TODO DEBUG 
    //      if (c==0x7d) c=0x30;
    //  }
}


// SDL Interfaces
int SDLHW_MemoryReset()
{
    return 0;
}
int SDLHW_Memory_Polling()//
{
    return 0;
}
int SDLHW_Memory_Refresh(int kbscancode, int ispress)
{
    return 0;
}
int SDLHW_Memory_Release(int kbscancode, int ispress)
{
    return 0;
}