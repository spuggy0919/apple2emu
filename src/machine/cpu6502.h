#ifndef __CPU6502_H_
#define __CPU6502_H_
#ifdef __cplusplus
extern "C" {
#endif
#include <stdio.h>
#include <stdint.h>
#include <string.h>

typedef  uint8_t  BYTE;
typedef  uint16_t WORD;
typedef struct {
    // Function pointers for memory read and write operations
    WORD bytes; // Cycle count for tracking instructions' execution time
    WORD cycles; // Cycle count for tracking instructions' execution time
    int  cycletype; // Cycle count for tracking instructions' execution time
    // BYTE *memory; 

    // Internal Registers
    BYTE IR; // Instruction Register
    WORD PC; // Program Counter
    BYTE P; // Status Register
    BYTE SP; // Stack Pointer
    BYTE A; // Accumulator
    BYTE X; // X Register
    BYTE Y; // Y Register
    WORD tw; // temp value for compare
    WORD lo; // temp value for compare
    WORD hi; // temp value for compare

    //pins 
    BYTE PIN; 


} CPU6502;

// addressing mode
enum{
    ACCUMULATOR = 0,
    IMMEDIATE,
    ABSOLUTE,  
    ZEROPAGE,
    ABSOLUTE_X,
    ABSOLUTE_Y,
    ZEROPAGE_X,
    ZEROPAGE_Y,
    INDIRECT_X,
    INDIRECT_Y,
    INDIRECT_ADDRESS,
    ABSOLUTE_ADDRESS,
    RELATIVE_ADDRESS,
    IMPLICIT,
    NO_OPRAND,
}ADDR_MODE;
#define CYCLESTYPE_PENALITY     1
#define CYCLESTYPE_TABLE        -2
#define CYCLESTYPE_STORE        -2
#define CYCLESTYPE_IMMEDIATE    -1

typedef  int (*OP_FUN)(BYTE oprand, WORD addr);
// instruction table
typedef struct {
    BYTE     opcode;
    char *      mnemonic;
    OP_FUN      opexec;
    BYTE     mode;
    BYTE     flag;
    BYTE     length;
    BYTE     cycles;
}INSTRUCTION_ITEM;

extern INSTRUCTION_ITEM instructions[];

#define F_N         (1<<7) // Negative
#define F_V         (1<<6) // Overflow
#define F_r5        (1<<5) // reserved
#define F_B         (1<<4) // Break
#define F_D         (1<<3) // Decimal
#define F_I         (1<<2) // Interrupt disable
#define F_Z         (1<<1) // Zero
#define F_C         (1)    // Carry
#define SET(u,v)    (u)=((u)|(v))
#define CLEAR(u,v)  (u)=((u)&(~(v)))
#define CHECK(u,v)  ((u)&(v))
#define HIGH        (1)
#define LOW         (0)

// define PINS
#define RESET  (1)
#define IRQ    (1<<1)
#define NMI    (1<<2)

#define LOWBYTE(pc)  ((pc)&(0xFF))
#define HIGHBYTE(pc)  (((pc)>>8)&0xFF)
#define BANK0(s)  (0x0000|((WORD)s))
#define BANK1(s)  (0x0100|((WORD)s))


extern CPU6502 cpu;


CPU6502 *cpu6502();
void wire_cpu_pins(BYTE *mem);
int  getInstructionSize(BYTE opcode) ;
void reset_pin( BYTE hilo);
void irq_pin( BYTE hilo);
void nmi_pin(BYTE hilo);
void resetExecute();

int getInstructionCycles(BYTE opcode) ;
int getInstructionSize(BYTE opcode) ;
void dumpReg(int ticks);



// instruction excution
int HALCPU6502_Running(BYTE forever, int ticks);
int HALCPU6502_Reset();

#ifdef __cplusplus
}
#endif
#endif // !__CPU6502__