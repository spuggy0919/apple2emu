
#include "disk2.h"
/*
 * Reinette II plus, a french Apple II emulator, using SDL2
 * and powered by puce6502 - a MOS 6502 cpu emulator by the same author
 * Last modified 21st of June 2021
 * Copyright (c) 2020 Arthur Ferreira (arthur.ferreira2@gmail.com)
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
//========================================== MEMORY MAPPED SOFT SWITCHES HANDLER
// this function is called from readMem and writeMem
// it complements both functions when address is in page $C0
// code  from 
// https://github.com/ArthurFerreira2/reinette-II-plus/blob/master/reinetteII%2B.c

//====================================================================== PADDLES

uint8_t PB0 = 0;                                                                // $C061 Push Button 0 (bit 7) / Open Apple
uint8_t PB1 = 0;                                                                // $C062 Push Button 1 (bit 7) / Solid Apple
uint8_t PB2 = 0;                                                                // $C063 Push Button 2 (bit 7) / shift mod !!!
float GCP[2] = { 127.0f, 127.0f };                                              // GC Position ranging from 0 (left) to 255 right
float GCC[2] = { 0.0f };                                                        // $C064 (GC0) and $C065 (GC1) Countdowns
int GCD[2] = { 0 };                                                             // GC0 and GC1 Directions (left/down or right/up)
int GCA[2] = { 0 };                                                             // GC0 and GC1 Action (push or release)
uint8_t GCActionSpeed = 8;                                                      // Game Controller speed at which it goes to the edges
uint8_t GCReleaseSpeed = 8;                                                     // Game Controller speed at which it returns to center
long long int GCCrigger;                                                        // $C070 the tick at which the GCs were reseted

inline static void resetPaddles() {
	GCC[0] = GCP[0] * GCP[0];                                                     // initialize the countdown for both paddles
	GCC[1] = GCP[1] * GCP[1];                                                     // to the square of their actuall values (positions)
	GCCrigger = count_1MHz;                                                            // records the time this was done
}

inline static uint8_t readPaddle(int pdl) {
	const float GCFreq = 6.6;                                                     // the speed at which the GC values decrease

	GCC[pdl] -= (count_1MHz - GCCrigger) / GCFreq;                                     // decreases the countdown
	if (GCC[pdl] <= 0)                                                            // timeout
		return GCC[pdl] = 0;                                                        // returns 0
	return 0x80;                                                                  // not timeout, return something with the MSB set
}

//====================================================================== SPEAKER

#define audioBufferSize 4096                                                    // found to be large enought
Sint8 audioBuffer[2][audioBufferSize] = { 0 };                                  // see in main() for more details
SDL_AudioDeviceID audioDevice;
bool muted = false;                                                             // mute/unmute switch

static void playSound() {
	static long long int lastTick = 0LL;
	static bool SPKR = false;                                                     // $C030 Speaker toggle

	if (!muted) {
		SPKR = !SPKR;      
		                                                         // toggle speaker state
		Uint32 length = (int)((double)(count_1MHz - lastTick) / 10.65625f);              // 1023000Hz / 96000Hz = 10.65625
		lastTick = count_1MHz;
		if (length > audioBufferSize) length = audioBufferSize;
		SDL_QueueAudio(audioDevice, audioBuffer[SPKR], length | 1);                 // | 1 TO HEAR HIGH FREQ SOUNDS
	}
}

//================ LANGUAGE CARD ==================================

bool LCWR  = true;                                                              // Language Card writable
bool LCRD  = false;                                                             // Language Card readable
bool LCBK2 = true;                                                              // Language Card bank 2 enabled
bool LCWFF = false;                                                             // Language Card pre-write flip flop

//================ DISK ][ ==================================

static uint8_t dLatch = 0;                                                    

int curDrv = 0;                                                                 // Current Drive - only one can be enabled at a time

struct drive {
	char		 filename[400];                                                       // the full disk image pathname
	bool		 readOnly;                                                            // based on the image file attributes
	uint8_t	 data[232960];                                                        // nibblelized disk image
	bool		 motorOn;                                                             // motor status
	bool		 writeMode;                                                           // writes to file are not implemented
	uint8_t	 track;                                                               // current track position
	uint16_t nibble;                                                              // ptr to nibble under head position
} disk[2] = { 0 };                                                              // two disk ][ drive units


int insertFloppy(char *filename, int drv) {

	FILE *f = fopen(filename, "rb");                                              // open file in read binary mode
	if (!f || !fread(disk[drv].data, 1, 232960, f) )                      // load it into memory and check size
		return 0;
	fclose(f);

	sprintf(disk[drv].filename, "%s", filename);                                  // update disk filename record

	f = fopen(filename, "ab");                                                    // try to open the file in append binary mode
	if (f) {                                                                      // success, file is writable
		disk[drv].readOnly = false; 
		printf("%s,readOnly false\n",filename);                                               // update the readOnly flag
		fclose(f);                                                                  // and close it untouched
	} else {
		disk[drv].readOnly = true;                                                  // f is NULL, no writable, no need to close it
	}
	int i, a, b;

	i = a = 0;
	while (disk[0].filename[i] != 0)                                              // find start of filename for disk0
		if (disk[0].filename[i++] == '\\') a = i;
	i = b = 0;
	while (disk[1].filename[i] != 0)                                              // find start of filename for disk1
		if (disk[1].filename[i++] == '\\') b = i;

	return 1;
}


int saveFloppy(int drive) {
	if (!disk[drive].filename[0]) return 0;                                       // no file loaded into drive
	if (disk[drive].readOnly) return 0;                                           // file is read only write no aptempted

	FILE *f = fopen(disk[drive].filename, "wb");
	if (!f) return 0;                                                             // could not open the file in write overide binary

	if (fwrite(disk[drive].data, 1, 232960, f) != 232960) {                       // failed to write the full file (disk full ?)
		fclose(f);                                                                  // release the ressource
		return 0;
	}
	fclose(f);                                                                    // success, release the ressource
	return 1;
}


void stepMotor(uint16_t address) {
	static bool phases[2][4] = { 0 };                                             // phases states (for both drives)
	static bool phasesB[2][4] = { 0 };                                            // phases states Before
	static bool phasesBB[2][4] = { 0 };                                           // phases states Before Before
	static int pIdx[2] = { 0 };                                                   // phase index (for both drives)
	static int pIdxB[2] = { 0 };                                                  // phase index Before
	static int halfTrackPos[2] = { 0 };

	address &= 7;
	int phase = address >> 1;

	phasesBB[curDrv][pIdxB[curDrv]] = phasesB[curDrv][pIdxB[curDrv]];
	phasesB[curDrv][pIdx[curDrv]]   = phases[curDrv][pIdx[curDrv]];
	pIdxB[curDrv] = pIdx[curDrv];
	pIdx[curDrv]  = phase;

	if (!(address & 1)) {                                                         // head not moving (PHASE x OFF)
		phases[curDrv][phase] = false;
		return;
	}

	if ((phasesBB[curDrv][(phase + 1) & 3]) && (--halfTrackPos[curDrv] < 0))      // head is moving in
		halfTrackPos[curDrv] = 0;

	if ((phasesBB[curDrv][(phase - 1) & 3]) && (++halfTrackPos[curDrv] > 140))    // head is moving out
		halfTrackPos[curDrv] = 140;

	phases[curDrv][phase] = true;                                                 // update track#
	disk[curDrv].track = (halfTrackPos[curDrv] + 1) / 2;
}


 void setDrv(int drv) {
	disk[drv].motorOn = disk[!drv].motorOn || disk[drv].motorOn;                  // if any of the motors were ON
	disk[!drv].motorOn = false;                                                   // motor of the other drive is set to OFF
	curDrv = drv;                                                                 // set the current drive
}

uint8_t diskio(uint16_t address, uint8_t value, bool WRT) 
{
	// disk ][ I/O register


		switch (address) {
			// audio
    case 0xC033: playSound(); break;    // apple invader uses $C033 to output sound !			// language card 
    case 0xC080:                                                                // LANGUAGE CARD :
  	case 0xC084: LCBK2 = 1; LCRD = 1; LCWR = 0;      LCWFF = 0;    break;       // LC2RD
  	case 0xC081:
  	case 0xC085: LCBK2 = 1; LCRD = 0; LCWR |= LCWFF; LCWFF = !WRT; break;       // LC2WR
  	case 0xC082:
  	case 0xC086: LCBK2 = 1; LCRD = 0; LCWR = 0;      LCWFF = 0;    break;       // ROMONLY2
  	case 0xC083:
  	case 0xC087: LCBK2 = 1; LCRD = 1; LCWR |= LCWFF; LCWFF = !WRT; break;       // LC2RW
  	case 0xC088:
  	case 0xC08C: LCBK2 = 0; LCRD = 1; LCWR = 0;      LCWFF = 0;    break;       // LC1RD
  	case 0xC089:
  	case 0xC08D: LCBK2 = 0; LCRD = 0; LCWR |= LCWFF; LCWFF = !WRT; break;       // LC1WR
  	case 0xC08A:
  	case 0xC08E: LCBK2 = 0; LCRD = 0; LCWR = 0;      LCWFF = 0;    break;       // ROMONLY1
  	case 0xC08B:
  	case 0xC08F: LCBK2 = 0; LCRD = 1; LCWR |= LCWFF; LCWFF = !WRT; break;       // LC1RW

  	case 0xC061: return PB0;                                                    // Push Button 0
  	case 0xC062: return PB1;                                                    // Push Button 1
  	case 0xC063: return PB2;                                                    // Push Button 2
  	case 0xC064: return readPaddle(0);                                          // Paddle 0
  	case 0xC065: return readPaddle(1);                                          // Paddle 1

  	case 0xC070: resetPaddles(); break;


		case 0xC0E0:
		case 0xC0E1:
		case 0xC0E2:
		case 0xC0E3:
		case 0xC0E4:
    case 0xC0E5:
		case 0xC0E6:
		case 0xC0E7: stepMotor(address); break;                                     // MOVE DRIVE HEAD

  	case 0xCFFF:
  	case 0xC0E8: disk[curDrv].motorOn = false; break;                           // MOTOROFF
  	case 0xC0E9: disk[curDrv].motorOn = true;  break;                           // MOTORON

  	case 0xC0EA: setDrv(0); break;                                              // DRIVE0EN
  	case 0xC0EB: setDrv(1); break;                                              // DRIVE1EN

  	case 0xC0EC:                                                                // Shift Data Latch
  		if (disk[curDrv].writeMode)                                               // writting
  			disk[curDrv].data[disk[curDrv].track*0x1A00+disk[curDrv].nibble]=dLatch;// good luck gcc
  		else                                                                      // reading
  			dLatch=disk[curDrv].data[disk[curDrv].track*0x1A00+disk[curDrv].nibble];// easy peasy
  		disk[curDrv].nibble = (disk[curDrv].nibble + 1) % 0x1A00;                 // turn floppy of 1 nibble
  		return dLatch;

  	case 0xC0ED: dLatch = value; break;                                         // Load Data Latch

  	case 0xC0EE:                                                                // latch for READ
  		disk[curDrv].writeMode = false;
  		return disk[curDrv].readOnly ? 0x80 : 0;                                  // check protection

  	case 0xC0EF: disk[curDrv].writeMode = true; break;                          // latch for WRITE
	default:break;
	}
	return 0;                                                          // catch all, gives a 'floating' value
}