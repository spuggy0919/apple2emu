
// #define _VIDEO_TEST_



#ifdef _VIDEO_TEST_
#include "video.h"
#include "uivideo.h"
uint8_t TG_switch[4];
uint8_t memory[0x10000];
#define writeByte(addr,d) memory[(addr)]=(d)
#define writePageBase(addr,d) page[(addr)]=(d)
#define readByte(addr) memory[addr]
#else
#include "appleii.h"
uint8_t *TG_switch;
#endif




#define TextMode    (TG_switch[0])
#define MixMode     (TG_switch[1])
#define PAGE2       (TG_switch[2])
#define HIRESMODE   (TG_switch[3])

uint8_t page[0x400];


int HALVIDEO_Reset(){
    return 0;
}

int HALVIDEO_DisplayMux()
{
    return 0;
}

int setattr=1;
int cachetxt[24][40];
int lastline=24;
int hlastline=192;
int firstline=0;
int offy[3]={0,0x28,0x50};
#define ROWA(i) (0x400+0x80*i)
#define ROWB(i) (0x400+0x80*i+0x28)
#define ROWC(i) (0x400+0x80*i+0x50)
#define HROWA(i) (0x2000+0x80*(i/8)+(i%8)*0x400)
#define HROWB(i) (0x2000+0x80*(i/8)+(i%8)*0x400+0x28)
#define HROWC(i) (0x2000+0x80*(i/8)+(i%8)*0x400+0x50)
int loff[24]={  // text and lores
    ROWA(0),ROWA(1),ROWA(2),ROWA(3),ROWA(4),ROWA(5),ROWA(6),ROWA(7),
    ROWB(0),ROWB(1),ROWB(2),ROWB(3),ROWB(4),ROWB(5),ROWB(6),ROWB(7),
    ROWC(0),ROWC(1),ROWC(2),ROWC(3),ROWC(4),ROWC(5),ROWC(6),ROWC(7),
    };
int hoff[192]={  // hires
    HROWA(0),HROWA(1),HROWA(2),HROWA(3),HROWA(4),HROWA(5),HROWA(6),HROWA(7),
    HROWA(8),HROWA(9),HROWA(10),HROWA(11),HROWA(12),HROWA(13),HROWA(14),HROWA(15),
    HROWA(16),HROWA(17),HROWA(18),HROWA(19),HROWA(20),HROWA(21),HROWA(22),HROWA(13),
    HROWA(24),HROWA(25),HROWA(26),HROWA(27),HROWA(28),HROWA(29),HROWA(30),HROWA(31),
    HROWA(32),HROWA(33),HROWA(34),HROWA(35),HROWA(36),HROWA(37),HROWA(38),HROWA(39),
    HROWA(40),HROWA(41),HROWA(42),HROWA(43),HROWA(44),HROWA(45),HROWA(46),HROWA(47),
    HROWA(48),HROWA(49),HROWA(50),HROWA(51),HROWA(52),HROWA(53),HROWA(54),HROWA(55),
    HROWA(56),HROWA(57),HROWA(58),HROWA(59),HROWA(60),HROWA(61),HROWA(62),HROWA(63),
    HROWB(0),HROWB(1),HROWB(2),HROWB(3),HROWB(4),HROWB(5),HROWB(6),HROWB(7),
    HROWB(8),HROWB(9),HROWB(10),HROWB(11),HROWB(12),HROWB(13),HROWB(14),HROWB(15),
    HROWB(16),HROWB(17),HROWB(18),HROWB(19),HROWB(20),HROWB(21),HROWB(22),HROWB(13),
    HROWB(24),HROWB(25),HROWB(26),HROWB(27),HROWB(28),HROWB(29),HROWB(30),HROWB(31),
    HROWB(32),HROWB(33),HROWB(34),HROWB(35),HROWB(36),HROWB(37),HROWB(38),HROWB(39),
    HROWB(40),HROWB(41),HROWB(42),HROWB(43),HROWB(44),HROWB(45),HROWB(46),HROWB(47),
    HROWB(48),HROWB(49),HROWB(50),HROWB(51),HROWB(52),HROWB(53),HROWB(54),HROWB(55),
    HROWB(56),HROWB(57),HROWB(58),HROWB(59),HROWB(60),HROWB(61),HROWB(62),HROWB(63),
    HROWC(0),HROWC(1),HROWC(2),HROWC(3),HROWC(4),HROWC(5),HROWC(6),HROWC(7),
    HROWC(8),HROWC(9),HROWC(10),HROWC(11),HROWC(12),HROWC(13),HROWC(14),HROWC(15),
    HROWC(16),HROWC(17),HROWC(18),HROWC(19),HROWC(20),HROWC(21),HROWC(22),HROWC(13),
    HROWC(24),HROWC(25),HROWC(26),HROWC(27),HROWC(28),HROWC(29),HROWC(30),HROWC(31),
    HROWC(32),HROWC(33),HROWC(34),HROWC(35),HROWC(36),HROWC(37),HROWC(38),HROWC(39),
    HROWC(40),HROWC(41),HROWC(42),HROWC(43),HROWC(44),HROWC(45),HROWC(46),HROWC(47),
    HROWC(48),HROWC(49),HROWC(50),HROWC(51),HROWC(52),HROWC(53),HROWC(54),HROWC(55),
    HROWC(56),HROWC(57),HROWC(58),HROWC(59),HROWC(60),HROWC(61),HROWC(62),HROWC(63)
    };
typedef enum {
    MODE_TEXT=0 ,      //0xC051 X      X
    MODE_LORES ,       //0xC050 0XC052 0xC056  100
    MODE_HIGHRES,      //0xC050 0XC052 0xC057  101
    MODE_LORESMIX ,    //0xC050 0XC053 0xC056  110
    MODE_HIRESMIX     //0xC050 0XC053 0xC057  111
}DISPLAYMODE;
typedef struct {
    int mode;
    int (*displayscandrv)();
}MUX;
MUX Multiplexor[]={

    {MODE_LORES,     HALVIDEO_LoResMode},      //0xC050 0XC052 0xC056  100
    {MODE_HIGHRES,   HALVIDEO_HiResMode},   //0xC050 0XC052 0xC057  101
    {MODE_LORESMIX,  HALVIDEO_LoRes_MixMode},   //0xC050 0XC053 0xC056  110
    {MODE_HIRESMIX,  HALVIDEO_HiRes_MixMode},  //0xC050 0XC053 0xC057  111  
    {MODE_TEXT,      HALVIDEO_Text40Mode},
    {MODE_TEXT,      HALVIDEO_Text40Mode},
    {MODE_TEXT,      HALVIDEO_Text40Mode},
    {MODE_TEXT,      HALVIDEO_Text40Mode}
    };
	const int lorescolor[16] = {  
        0x000000, // Hires 0 & 3  
        0x901740,
        0x402ca5,
        0xd043e5, // Hires 2
        0x006940,
        0x808080,
        0x2f95e5, // Hires 6
        0xbfabff,
        0x405400,
        0xd06a1a, // Hires 5
        0x808080,
        0xff96bf,
        0x2fbc1a, // Hires 1
        0xbfd35a,
        0x6fe8bf,
        0xffffff };// Hires 4 & 7                                                                           // the 16 low res colors
	

	const int hcolor[16] = {  
        0x000000, 0x14f53c, 0xff44fd, 0xffffff,                                                 // the high res colors (2 light levels)
        0x000000, 0xff6a3c, 0x14cffd, 0xffffff,                                                 // the high res colors (2 light levels)
        0x000000, 0xff44fd, 0x14f53c, 0xffffff,                                                 // the high res colors (2 light levels)
        0x000000, 0x14cffd, 0xff6a3c, 0xffffff      
        // 0x000000,0x90A231,0x7e6ead,0xffffff,                                                 // the high res colors (2 light levels)
        // 0x000000,0xE06c15,0x56A8E4,0xffffff,                                                 // the high res colors (2 light levels)
        // 0x000000,0x3f3756,0x485019,0xffffff,                                                 // the high res colors (2 light levels)
        // 0x000000,0x2B5472,0x75360a,0xffffff                                                // the high res colors (2 light levels)
	//	{ 0,   0,   0   }, { 144, 192, 49  }, { 126, 110, 173 }, { 255, 255, 255 },
		// { 0,   0,   0   }, { 234, 108, 21  }, { 86,  168, 228 }, { 255, 255, 255 },
		// { 0,   0,   0   }, { 63,  55,	 86  }, { 72,  96,  25	}, { 255, 255, 255 },
		// { 0,   0,   0   }, { 43,  84,	 114 }, { 117, 54,  10	}, { 255, 255, 255 }
	};
int curmode = MODE_TEXT;
int currentsw = -1;
int lastsw = -2; 
int HALVIDEO_SwitchPageMode()
{
    int page;
#ifndef _VIDEO_TEST_
    TG_switch = HALMemory_GetVideoSwitches();
#endif
    int idx= (TG_switch[0]<<2)|(TG_switch[1]<<1)|(TG_switch[3]);
    int currentsw = (TG_switch[0]<<2)|(TG_switch[1]<<1)|(TG_switch[2])|(TG_switch[3]);
    firstline=0; lastline=24; hlastline=192;
    // printf("HALVIDEO_SwitchPageMode (idx=%d)(currentsw=%d,lastsw=%d\n",idx,currentsw,lastsw);
    if (currentsw==lastsw) {
        return idx&7;
    }
    lastsw = currentsw;
    // int cmode = Multiplexor[idx&7].mode;
    // recalulate base; 
    page = TG_switch[2];
    int curbase = page ? 0x800:0x400;
    if (loff[0]!=curbase) {
        int oldbase = loff[0];
        for(int row=0;row<24;row++)
            loff[row]+=curbase-oldbase;
    }
    curbase = page ? 0x4000:0x2000;
    if (hoff[0]!=curbase) {
        int oldbase = hoff[0];
        for(int row=0;row<192;row++){
            hoff[row]+=curbase-oldbase;
            // printf("page=%d,hoff[%d]=%x\n",page,row,hoff[row]);
        }
    }
    // int mode=Multiplexor[idx].mode;
    return idx&7;
}
int lastmode=0;
int cachedisable = 5;

int HALVIDEO_DisplayModeMUX()
{
    int mode=HALVIDEO_SwitchPageMode();
    if (lastmode!=mode) {
        lastmode =  mode;
        cachedisable = 5;
    }
    // if (cachedisable) cachedisable--;
    DEBUG("HALVIDEO_DisplayModeMUX%d\n",mode);
    (*Multiplexor[mode].displayscandrv)();
    return 0;
}
#ifdef _VIDEO_TEST_
int HALVIDEO_DispMODETest(){
    uint8_t *page=&memory[0]; 
    int pageoff=0;
    TG_switch[0]=0; // 1 0 0 0 text 1 graphics 0
    TG_switch[1]=0; // 1 0 1 0 none 0 mix 1
    TG_switch[2]=0; // 1 0 1 0 page1 0 Page2 1
    TG_switch[3]=0; // 1 0 1 0 Lores 0 HiRes 1     
    // page1
    pageoff = (loff[0]==0x400) ? 0 : 0x400-0x800; // force page 0
    printf("loff[0]=%x\n",loff[0]);
    for(int r=0;r<24;r++)
        for(int i=0;i<32;i++) 
            writeByte(pageoff+loff[r]+i,i+(7-((r/2)%8))*32);
    // page 2
    printf("loff[0]=%x\n",loff[0]);
    pageoff = (loff[0]==0x400) ? 0x800-0x400:0; // force page 1
    for(int r=0;r<24;r++)
        for(int i=0;i<32;i++) {
            int c= i&0xf;
            c = (c&0xf)|((c&0xf)<<4);
            writeByte(pageoff+loff[r]+i,c);
        }
    pageoff = (hoff[0]==0x2000) ? 0:0x2000-0x4000; // force page 1
    printf("hoff[0]=%x\n",hoff[0]);
    for(int r=0;r<32;r++){
        for(int i=0;i<40;i+=2) {
            writeByte(hoff[r]+i,0x2A);    // green
            writeByte(hoff[r]+i+1,0x55);    // green
        }
    }
    for(int r=32;r<64;r++){
        for(int i=0;i<40;i+=2) {
            writeByte(hoff[r]+i,0x55);    // green
            writeByte(hoff[r]+i+1,0x2A);    // green
        }
    }
    for(int r=64;r<96;r++){
        for(int i=0;i<40;i+=2) {
            writeByte(hoff[r]+i,0xd5);    // green
            writeByte(hoff[r]+i+1,0x8A);    // green
        }
    }
    for(int r=96;r<128;r++){
        for(int i=0;i<40;i+=2) {
            page[hoff[r]+i]=0x8a;    // green
            page[hoff[r]+i+1]=0xd5;    // green
            writeByte(hoff[r]+i,0x8a);    // green
            writeByte(hoff[r]+i+1,0xd5);    // green     
        }
    }
    for(int r=128;r<192;r++){
        uint8_t c=0;
        for(int i=0;i<40;i++) {
            c=(r%4)&3;
            writeByte(hoff[r]+i,((c<<6)&0x40)|(c<<4)|(c<<2)|c|0x80);    // green
        }
    }
    return 0;
}

int HALVIDEO_Text40Test()
{
    uint8_t glyph;
    // int offsetsForRows= 0x80;
   //                  0123456789012345678901234567890123456789
      char *text[4] ={"@ABCDEFGHIJKLMNO",
                      "PQRSTUVWXYZ[\\]^_",
                      " !\"#$%&',()*+,-.",
                      "0123456789:;<=>?"};
      int attr[]={1,1,1,1,2,2,2,2,0,0,0,0,0,0,0,0};           
      // video controller - page 1 text mode only
    //   HALMemory_GetVideoFrameData(PAGE2 ? 0x700:0x400, 0x400, page);
      for (int row=0; row<16; row++){       //TODO 24             // for each row
          SDLUI_attrset(attr[row]);
        for (int col=0;text[row%4][col]!=0; col++){    
        //   if (setattr) attr[row][col]=(rand()%3);
              // for each column
          SDLUI_move(row,col);
          glyph = text[row%4][col%16]; //page[offsetsForRows*row + col];        // read video memory
          if (glyph == '`') glyph = '_';                 // change cursor shape
        //   if (glyph < 0x40) SDLUI_attrset(A_REVERSE);          // is REVERSE ?
        //   else if (glyph > 0x7F) SDLUI_attrset(A_NORMAL);      // is NORMAL ?
        //   else SDLUI_attrset(A_BLINK);                         // is FLASHING ?
        //   glyph &= 0x7F;                                 // unset bit 7
        //   if (glyph > 0x5F) glyph &= 0x3F;               // shifts to match
        //   if (glyph < 0x20) glyph |= 0x40;               // the ASCII codes
          SDLUI_addch(glyph);                                  // print the glyph
        }
      }
      setattr=0;
      return 0;
}
#endif 
#define SDLBUFFNUMS 3
uint8_t tcache[SDLBUFFNUMS][24][40]; //[2] for sdl dual buffer
uint8_t dualbuff = 0;
int HALVIDEO_Text40Mode()
{
    uint8_t c,a=0;
    dualbuff=(dualbuff+1)%SDLBUFFNUMS;
    // if (!trigger_10Hz) return 0; //for blink
    // printf("HALVIDEO_Text40Mode%x, firstline=%d,lastline%d\n",loff[0],firstline,lastline);
    for(int y=firstline; y<lastline;y++){
        int addr = loff[y]; 
        // printf("loff(%d)=%x\n",y,addr);
        for(int x=0;x<40;x++) {
           c=readByte(addr+x);
           if (cachedisable||c!=tcache[dualbuff][y][x]||(c>=0x40&&c<=0x7f)){
                tcache[dualbuff][y][x]=c;
                if (c < 0x40) SDLUI_attrset(A_INVERSE);          // is REVERSE ?
                else if (c > 0x7F) SDLUI_attrset(A_NORMAL);      // is NORMAL ?
                else SDLUI_attrset(A_BLINK);                         // is FLASHING ?
                if (c=='`') c='_';
                c &= 0x7F;                                 // unset bit 7
                if (c > 0x5F) c &= 0x3F;               // shifts to match
                if (c < 0x20) c |= 0x40;               // the ASCII codes
                SDLUI_addtext(x,y,a,c);
           }
        }
    }
    return 0;
}

int HALVIDEO_LoResMode()
{
    uint8_t c;
    uint32_t c1,c2;
    // printf("HALVIDEO_LoResMode%x lastline=%d\n",loff[0],lastline);
    dualbuff=(dualbuff+1)%SDLBUFFNUMS;
    for(int y=0; y<lastline;y++){
        int addr = loff[y];
        for(int x=0;x<40;x++) {
           c=readByte(addr+x); c1=c&0xf; c2=(c&0xf0)>>4;
           if (cachedisable||c!=tcache[dualbuff][y][x]){
                tcache[dualbuff][y][x]=c;
                SDLUI_addlores(x,y,lorescolor[c1],lorescolor[c2]); // 8x8 char
           }
        }
    }
    return 0;
}
// 40x48
uint16_t hcache[192][40]={0};
uint8_t pbit[192][40]={0};

int HALVIDEO_HiResMode()
{
    uint16_t w;
    int artifact=0;
    int color,colorBit7;
    // printf("HALVIDEO_HiResMode%x, firstline=%d,hlastline%d\n",hoff[0],firstline,hlastline);
    for(int y=0; y<hlastline;y++){
        int addr = hoff[y]; 
        for(int c=0;c<40;c++) {
           w=readByte(addr+c);
        //  if (y==160&&c==20)   printf("y,c=(%d,%d) w=%d addr=%x,col=%d\n",y,c,w,addr,c);
           colorBit7=w&0x80 ? 4:0;
           hcache[y][c]=w;
           if (c!=39) pbit[y][c+1]=(w&0x40) ? 1:0;
           else 
             pbit[(y+1)%192][0]=(w&0x40) ? 1:0;
           w=((w&0x7f)<<1)|pbit[y][c];
           for(int x=c*7;x<(c+1)*7; x++){ 
            int cidx=artifact + colorBit7 +(w&3);
              color = hcolor[cidx];
            //   if (y==0) printf("y=%d,x=%d,artifact=%d,colorBit7=%d,[%x][%x],cidx=%d,color=%x\n",y,x,artifact,colorBit7,w,w&3,cidx,color);
              SDLUI_addhirespixel(x,y,color); //2  4x8 
               artifact ^= 8; w>>=1;
           } //x
        }
    }//y
    return 0;
}

int HALVIDEO_LoRes_MixMode()
{
    lastline=20;
    HALVIDEO_LoResMode();
    firstline=20; lastline=24;
    HALVIDEO_Text40Mode();
    return 0;
}
int HALVIDEO_HiRes_MixMode()
{
    hlastline=160;
    HALVIDEO_HiResMode();
    firstline=20; 
    HALVIDEO_Text40Mode();
    return 0;
}
// SDL Interfaces
int SDLHW_VIDEO_Initial()
{
#ifdef _VIDEO_TEST_
    HALVIDEO_DispMODETest();
#endif
    return 0;
}
int SDLHW_VIDEO_Polling()//
{
    return 0;
}
int SDLHW_VIDEO_Refresh()
{

#ifdef _VIDEO_TEST_
    HALVIDEO_DispMODETest();
#endif
    HALVIDEO_DisplayModeMUX();
    return 0;
}
int SDLHW_VIDEO_Release()
{
    return 0;
}