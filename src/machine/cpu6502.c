#include "cpu6502.h"
#include "memory.h"


#define CYCLES(x) {cpu.cycles+=x;}
#define FETCH_BYTE(x)  {(x) = readByte(cpu.PC++); cpu.cycles=1; }
#define FETCH_WORD(x) {(x) = readWord(cpu.PC); cpu.PC+=2;CYCLES(2); } 
#define TOWORD(hi,lo) ((((WORD)(hi))<<8) | ((WORD)(lo)))
#define TOSIGNEDWORD(lo) ((signed)(lo)))
#define aaabbbcc_cc(opcode) (opcode&0x3)
#define WRITE_BYTE(a,d)  {writeByte(a,(d)); CYCLES(1); }
#define WRITE_BYTEN(a,d)  {writeByte(a,(d));  }
#define WRITE_WORD(a,d)  {writeWord(a,(uint16_)(d)); CYCLES(2); }
#define WRITE_WORDN(a,d)  {writeWord(a,(uint16_)(d)); }
#define READ_BYTE(a,d)  {(d)=readByte(a); CYCLES(1); }
#define READ_BYTEN(a,d)  {(d)=readByte(a);  }
#define READ_WORD(a,d)  {(d)=readWord(a); CYCLES(2); }
#define READ_WORDN(a,d)  {(d)=readWord(a);}
#define PUSH(d)  {writeByte(BANK1(cpu.SP),d);cpu.SP--; CYCLES(1); }
#define POP(v)  {cpu.SP++; (v)=readByte(BANK1(cpu.SP));CYCLES(1); }



CPU6502 cpu;
CPU6502 *cpu6502(){
    return &cpu;
}
int HALCPU6502_Reset()
{
    reset_pin(LOW);
    resetExecute();
    reset_pin(HIGH);
    return 0;
}

// pins actions
static BYTE nmisignal=HIGH;
void nmi_pin(BYTE hilo) {
    if (nmisignal>hilo) { //  falling edge
        SET(cpu.PIN,NMI);  // trigger internal NMI
    }
    nmisignal = hilo;
}
void nmiExecute() {
    // Handle NMI (Non-Maskable Interrupt) here
    if (!CHECK(cpu.PIN,NMI)) return;
    CLEAR(cpu.PIN,NMI);   //    falling edge
    //push return address
    // cpu.P &= ~F_B;
    PUSH(LOWBYTE(cpu.PC)); 
    PUSH(HIGHBYTE(cpu.PC)); 
    PUSH(cpu.P & (~F_B)); 
    cpu.P |= F_I;
    READ_WORD(0xFFFA,cpu.PC); // Program Counter 
    cpu.cycles=7; 
}
static BYTE irqsignal=HIGH;
void irq_pin(BYTE hilo) {
    if (irqsignal>hilo) { //  falling edge
        SET(cpu.PIN,IRQ); // trigger internal IRQ
    }
    irqsignal = hilo;
}
void irqExecute() {
    // Handle IRQ (Interrupt Request) here
    if (!CHECK(cpu.PIN,IRQ)) return;
    CLEAR(cpu.PIN,IRQ);  // falling edge
    if (!CHECK(cpu.P,F_I)) { // irq occured
        //push return address
        // cpu.P &= ~F_B;
        PUSH(LOWBYTE(cpu.PC)); //cpu.memory[BANK1(cpu.SP)] = LOWBYTE(cpu.PC); cpu.SP--;
        PUSH(HIGHBYTE(cpu.PC)); // cpu.memory[BANK1(cpu.SP)] = HIGHBYTE(cpu.PC); cpu.SP--;
        PUSH(cpu.P & (~F_B)); //cpu.memory[BANK1(cpu.SP)] = cpu.P & (~F_B); cpu.SP--;
        cpu.P |= F_I;
        READ_WORD(0xfffe,cpu.PC); //cpu.PC = (cpu.memory[0xFFFE]) | ((WORD)cpu.memory[0xFFFF]<<8); // Program Counter   
        cpu.cycles=7;
    }
}
static BYTE resetsignal=HIGH;
void reset_pin(BYTE hilo) {
    resetsignal = hilo;
    if (resetsignal == LOW) {
        CLEAR(cpu.PIN,RESET);   // low active
    }else{
        SET(cpu.PIN,RESET); 
    }
    
}
static int fetch=0;
void dumpReg(int ticks)
{
    /* for test breakpoint */
        // 1. fetch op
    WORD pc[] = {0xfd1b, 0xfd21, 0xfd24, 0xfd26, 0xfd28,  0xfd2b,  0xffff};

    return;
    switch(fetch){
    case  0: // search breakpoint dump fetch first; 
        for(int i=0;i<10;i++){
            if (pc[i]==0xFFFF) return;
            if (cpu.PC==pc[i]) {
                READ_BYTEN(cpu.PC,cpu.IR); 
                printf("PC=%4x,opcode%s[%2x]\n",pc[i],instructions[cpu.IR].mnemonic,instructions[cpu.IR].opcode);
                fetch = 1;
                break;
            }
        }
        printf("************FETCH **************\n"); 
        break;
    case 1: // execute done 
        printf("-----------EXCUTE-DONE----------\n");
        fetch = 0;
        break;
    default: return ;
    }
    printf("cpu6502 registers%d\n",ticks);
    printf("PC=%4x\n",cpu.PC);
    printf("IR=%2x%s\n",cpu.IR,instructions[cpu.IR].mnemonic);
    printf("SP=%2x\n",cpu.SP);
    printf("P=%2x\n",cpu.P);
    printf("A=%2x\n",cpu.A);
    printf("X=%2x\n",cpu.X);
    printf("Y=%2x\n",cpu.Y);
    printf("l=%4x\n",cpu.lo);
    printf("h=%4x\n",cpu.hi);
    printf("w=%4x\n",cpu.tw);
}
void resetExecute() {
    // Handle RESET here low active
    if (CHECK(cpu.PIN,RESET)) return;  
    // printf("CPUReset");
    // SET(cpu.PIN,RESET); // TODO should SET in PIN 
    // cpu.memory=memory;
    cpu.IR = 0; // Instruction Register for debug 
    READ_WORD(0xfffc,cpu.PC);  //cpu.PC = (cpu.memory[0xFFFC]) | ((WORD)cpu.memory[0xFFFD]<<8); // Program Counter
    cpu.A = 0; // Accumulator
    cpu.X = 0; // X Register
    cpu.Y = 0; // Y Register
    SET(cpu.P,F_I); // status 
    cpu.P |= F_r5;
    cpu.SP = 0x00FF; // Stack Pointer
    cpu.cycles = 7; // Cycle count for tracking instructions' execution time
    cpu.PIN = 0; // TODO
}

// instruction dcoder


// addressing mode process, the multiplex select oprands or address
WORD oprand_multiplexor_src(BYTE mode, BYTE *oprand, WORD *address){
    BYTE lo,hi;
    WORD word;
    // BYTE cycles=0;
    cpu.cycletype = 0;
    switch(mode){
        case ACCUMULATOR: //1
            *oprand = cpu.A; 
            break;
        case IMMEDIATE: //2
            FETCH_BYTE(*oprand);  
            cpu.cycletype = CYCLESTYPE_IMMEDIATE; // only 2
            break;
        case ABSOLUTE: //3
            FETCH_WORD(word);   
            *address = word;
            READ_BYTEN(word,*oprand); 
            break;
        case ZEROPAGE: // 2
            FETCH_BYTE(lo); word = (WORD)lo;  
            *address = word;
            READ_BYTEN(word,*oprand); 
            break;
        case ABSOLUTE_X: //3,4
            FETCH_WORD(word);  //3
            word += (WORD)cpu.X; // page increment is possible
            cpu.cycletype=( ((word&0xff)+cpu.X)>255) ? CYCLESTYPE_PENALITY:0; //4?
            *address = word;
            READ_BYTEN(word,*oprand);
            break;
        case ABSOLUTE_Y: //3,4
            FETCH_WORD(word); //3
            word += (WORD)cpu.Y; // page increment is possible
            *address = word;
            READ_BYTEN(word,*oprand);
            cpu.cycletype=( ((word&0xff)+cpu.Y)>255) ? CYCLESTYPE_PENALITY:0; //4?
            break;
       case ZEROPAGE_X://3
            FETCH_BYTE(lo); lo+=cpu.X; //2
            word = (WORD)lo;
            *address = word; CYCLES(1); //3
            READ_BYTEN(word,*oprand);
            break;
        case ZEROPAGE_Y: //3
            FETCH_BYTE(lo); lo+=cpu.Y; //2
            word = (WORD)lo;
            *address = word; CYCLES(1); //3
            READ_BYTEN(word,*oprand); //4
            break;        

        case INDIRECT_X: //5
            FETCH_BYTE(lo); lo+=cpu.X; //2
            word = (WORD)lo;
            READ_BYTE(word,lo); //3lo  =   cpu.memory[word];
            word = (word&0xff00) | ((word+1)&0xff); // not word+1 for 6502 BUG
            READ_BYTE(word,hi); //4 hi  =   cpu.memory[word+1];
            word = TOWORD(hi,lo);  
            *address = word;
            READ_BYTE(word,*oprand); //5 *oprand  =   cpu.memory[word];  cpu.cycles++; //5
            break;
        case INDIRECT_Y://4
            FETCH_BYTE(lo); word = (WORD)lo; //2
            READ_BYTE(word,lo); //3lo  =   cpu.memory[word];
            word = (word&0xff00) | ((word+1)&0xff); // not word+1 for 6502 BUG
            READ_BYTE(word,hi); //4 hi  =   cpu.memory[word+1];
            word = TOWORD(hi,lo);  
            word += (WORD)cpu.Y; // possible page change
            cpu.cycletype=(((lo&0xff)+cpu.Y)>255) ? CYCLESTYPE_PENALITY:0; 
            *address=word;
            READ_BYTE(word,*oprand); //6

            break; 
        case INDIRECT_ADDRESS: //5
            FETCH_WORD(word); //3
            READ_BYTEN(word,lo); 
            word = (word&0xff00) | ((word+1)&0xff); // not word+1 for 6502 BUG
            READ_BYTEN(word,hi); 
            *address = TOWORD(hi,lo); CYCLES(2);  //5
            break;
        case ABSOLUTE_ADDRESS: //3
            FETCH_WORD(*address); //3
            break;    
        case RELATIVE_ADDRESS: //2
            FETCH_BYTE(lo); *address = cpu.PC + (int) ((signed char)lo);  //2
            break;                      
        case IMPLICIT://1
        case NO_OPRAND: //1
            break;
        default:
            // printf("ERROR!")
            break;
    }
    return cpu.cycles;
}

int op_nop_xx(BYTE oprand, WORD addr){
        return 1;

}
/* status flag */
#define SET_SR_BIT(x) (cpu.P|=x)
#define CLR_SR_BIT(x) (cpu.P&=(~(x)))
#define TST_SR_BIT(x) (cpu.P&(x))
#define ASSIGN_F_C(x) if (x)  SET_SR_BIT(F_C); else CLR_SR_BIT(F_C)
#define ASSIGN_F_Z(x) if (x)  SET_SR_BIT(F_Z); else CLR_SR_BIT(F_Z)
#define ASSIGN_F_V(x) if (x)  SET_SR_BIT(F_V); else CLR_SR_BIT(F_V)
#define ASSIGN_F_N(x) if (x)  SET_SR_BIT(F_N); else CLR_SR_BIT(F_N)

// PSR Status Register 
void setADDV(BYTE oprand,BYTE result){
    if (~(cpu.A^oprand)&(cpu.A^result)&0x80) 
        SET_SR_BIT(F_V); 
    else 
        CLR_SR_BIT(F_V); 
}
void setV(BYTE oprand,BYTE result){
    if ((cpu.A^oprand)&(cpu.A^result)&0x80) 
        SET_SR_BIT(F_V); 
    else 
        CLR_SR_BIT(F_V); 
}
void setC(WORD vw){
    if (vw>255) SET_SR_BIT(F_C); else CLR_SR_BIT(F_C);
}
void setCbyCompare(BYTE r, BYTE d){
    if (r>=d) SET_SR_BIT(F_C); else CLR_SR_BIT(F_C);
}
void setNZ(BYTE v){
    if (v==0) SET_SR_BIT(F_Z); else CLR_SR_BIT(F_Z);
    if (v&F_N) SET_SR_BIT(F_N); else CLR_SR_BIT(F_N);
}
/* mos6502 instruction set sorted by decoder */
// ALU Arithematic instructions 
int op_ALU_01(BYTE oprand, WORD addr){
    BYTE c;
    //TODO aaabbbcc cc = 01
    switch((cpu.IR)&0b11100000){
     //   aaa	opcode
        case 0b00000000: //	ORA  A OR M -> A
            cpu.A |= oprand; 
            cpu.cycles=cpu.cycletype + cpu.cycles; 
            cpu.cycles++;
            break;
        case 0b00100000://	AND A AND M -> A
            cpu.A &= oprand;  
            cpu.cycles=cpu.cycletype + cpu.cycles; 
            cpu.cycles++;
            break;
        case 0b01000000://	EOR A EOR M -> A
            cpu.A ^= oprand;  
            cpu.cycles=cpu.cycletype + cpu.cycles; 
            cpu.cycles++;
            break;

        case 0b01100000://	ADC A + M + C -> A, C // F_N|F_V|F_Z|F_C            
            if (TST_SR_BIT(F_D)) { //decimal mode
                cpu.lo = (cpu.A&0xf)+(oprand&0xf)+TST_SR_BIT(F_C);
                cpu.hi = (cpu.A&0xf0)+(oprand&0xf0);
                if (cpu.lo >= 0xa){
                    cpu.lo+=0x6;
                    cpu.hi+=0x10;
                }
                if (cpu.hi >= 0xa0){
                    cpu.hi += 0x60;
                }
                setC(cpu.hi);
                setADDV(oprand,(BYTE)cpu.hi);
                cpu.A = (cpu.hi & 0xf0) | (cpu.lo & 0xf);              
            }else{
                cpu.tw = (WORD)cpu.A + (WORD)oprand + TST_SR_BIT(F_C);
                setADDV(oprand,(BYTE)cpu.tw);
                setC(cpu.tw);
                cpu.A = cpu.tw & 0xff;
            }
            cpu.cycles=cpu.cycletype + cpu.cycles; 
            CYCLES(1);
            break;
        case 0b11100000://	SBC  A - M - not C -> A
            c = TST_SR_BIT(F_C) ? 0:1;
            if (TST_SR_BIT(F_D)) { //decimal mode
                cpu.tw = cpu.A - oprand - c;
                cpu.lo = (cpu.A&0xf) - (oprand&0xf) - c;
		        cpu.hi = (WORD)(cpu.A & 0xF0) - (oprand & 0xF0);
                cpu.P &= ~(F_V | F_C | F_Z | F_N);
                if (cpu.lo & 0x10)		      {cpu.lo -= 6; cpu.hi--;}
		        if (cpu.hi & 0x0100) cpu.hi -= 0x60;
                cpu.A = (cpu.lo & 0xf) | (cpu.hi&0xf0);
                // carry clear when result <= 0 
                if  (!(cpu.tw >> 8)) SET_SR_BIT(F_C); 
                setADDV(oprand,(BYTE)cpu.hi);
            }else{
                oprand = oprand ^ 0xff;
                cpu.tw = (WORD)cpu.A + (WORD)oprand + TST_SR_BIT(F_C);
                setADDV(oprand,(BYTE)cpu.tw);
                setC(cpu.tw);
                cpu.A = cpu.tw & 0xff;
            }
            cpu.cycles=cpu.cycletype + cpu.cycles; 
            CYCLES(1);
            break;
        case 0b10000000://	STA // A -> M
            WRITE_BYTE(addr,cpu.A);
            cpu.cycletype = CYCLESTYPE_TABLE; 
            return cpu.cycles;
            break;
        case 0b10100000://	LDA
            cpu.A = oprand; // M -> A
            cpu.cycles=cpu.cycletype + cpu.cycles; 
            CYCLES(1);
            break;
        case 0b11000000://	CMP A-M 
            cpu.lo = (WORD)cpu.A - (int) ((signed char)oprand); // A-M
            // cpu.lo = cpu.A - oprand; // A-M
            setCbyCompare(cpu.A,oprand);
            setNZ(cpu.lo&0xff);  
            cpu.cycles=cpu.cycletype + cpu.cycles + 1; 
            return cpu.cycles;
            break;
    }
    setNZ(cpu.A);
    return cpu.cycles;
}
// SHITE ROTATE instructions 
int op_ASL_10(BYTE oprand, WORD addr){
    //aaabbbcc cc = 10
    switch((cpu.IR)&0b11100000){
        case 0b00000000://	ASL C <- [76543210] <- 0  F_N|F_Z|F_C 
            cpu.lo =  oprand << 1; cpu.cycles++;
            cpu.cycletype = CYCLESTYPE_TABLE; 
            if (cpu.IR== 0x0A) {            
                cpu.A = cpu.lo; 
            }else{
                WRITE_BYTE(addr,cpu.lo); cpu.cycles+=1;
            }
            ASSIGN_F_C(oprand&0x80);
            setNZ(cpu.lo);
            break;
        case 0b00100000://	ROL C <- [76543210] <- C  F_N|F_Z|F_C  
            cpu.lo = (oprand << 1) | TST_SR_BIT(F_C); cpu.cycles++;
            if (cpu.IR== 0x2A) {            
                cpu.A = cpu.lo;
            }else{
                WRITE_BYTEN(addr,cpu.lo);
            }
            ASSIGN_F_C(oprand&0x80);
            setNZ(cpu.lo);
            cpu.cycletype = CYCLESTYPE_TABLE; 
            break;
        case 0b01000000://	LSR 0 -> [76543210] -> C  F_N|F_Z|F_C 
            cpu.lo = ((oprand >> 1)&0x7F);  cpu.cycles++;
            if (cpu.IR== 0x4A) {            
                cpu.A = cpu.lo;
            }else{
                WRITE_BYTEN(addr,cpu.lo);
            }
            ASSIGN_F_C(oprand&0x01);
            setNZ(cpu.lo);
            cpu.cycletype = CYCLESTYPE_TABLE; 
            break;
        case 0b01100000://	ROR C -> [76543210] -> C  F_N|F_Z|F_C  
            cpu.lo = ((oprand >> 1)&0x7F) | (BYTE)TST_SR_BIT(F_C)<<7; cpu.cycles++;
            if (cpu.IR== 0x6A) {            
                cpu.A = cpu.lo;
            }else{
                WRITE_BYTEN(addr,cpu.lo);
            }
            ASSIGN_F_C(oprand&0x01);
            setNZ(cpu.lo);
            cpu.cycletype = CYCLESTYPE_TABLE; 
            break;
        case 0b10000000://	STX X -> M                F_S|F_Z 
            WRITE_BYTE(addr,cpu.X);
            cpu.cycletype = CYCLESTYPE_TABLE; 
            break;
        case 0b10100000://	LDX M -> X                F_S|F_Z  
            cpu.X = oprand;
            setNZ(cpu.X);
            cpu.cycles+=cpu.cycletype; 
            cpu.cycles++;
            break;
        case 0b11000000://	DEC M - 1 -> M            F_S|F_Z 
            READ_BYTE(addr,cpu.lo);
            cpu.lo--; cpu.cycles++;
            WRITE_BYTE(addr,cpu.lo);
            setNZ(cpu.lo);
            cpu.cycletype = CYCLESTYPE_TABLE;
            break;
        case 0b11100000://	INC M + 1 -> M            F_S|F_Z 
            READ_BYTE(addr,cpu.lo);
            cpu.lo++; cpu.cycles++;
            WRITE_BYTE(addr,cpu.lo);
            setNZ(cpu.lo);
            cpu.cycletype = CYCLESTYPE_TABLE;
            break;

    }
    return cpu.cycles;
}
// SHITE ROTATE instructions 
int op_BIT_00(BYTE oprand, WORD addr){
    //aaabbbcc cc = 0b11100000 
    switch((cpu.IR)&0b11100000){
        case 0b00000000://  NOP
            cpu.cycles++;
            break;
        case 0b00100000://	BIT	 A AND M, M7 -> N, M6 -> V  N	Z	C	I	D	V
                        //                                  M7	+	-	-	-	M6
            READ_BYTE(addr,cpu.lo); // cycles++
            cpu.hi  =  cpu.lo & cpu.A ; 
            setNZ(cpu.hi);
            ASSIGN_F_V(cpu.lo&0x40);
            ASSIGN_F_N(cpu.lo&0x80);
            break;
        case 0b01000000://	JMP	ind (PC+1) -> PCL (PC+2) -> PCH
            cpu.PC= addr;      
            break;
        case 0b01100000://	JMP abs	(PC+1) -> PCL (PC+2) -> PCH
            cpu.PC= addr;
            break;
        case 0b10000000://	STY	Y -> M
            WRITE_BYTE(addr,cpu.Y);
            cpu.cycletype = CYCLESTYPE_TABLE;             
            break;
        case 0b10100000://	LDY	M -> Y  
            cpu.Y = oprand;
            setNZ(cpu.Y);
            cpu.cycles+=cpu.cycletype; 
            cpu.cycles++;
            break;
        case 0b11000000://	CPY	Y - M F_N	F_Z	F_C	
            // cpu.lo = (WORD)cpu.Y - (int) ((signed char)oprand); 
                        cpu.lo = cpu.Y - oprand; // A-M

            setNZ(cpu.lo);
            setCbyCompare(cpu.Y,oprand);
            cpu.cycles=cpu.cycletype + cpu.cycles; 
            cpu.cycles++; 
            break;
        case 0b11100000://	CPX  X - M F_N	F_Z	F_C	
            // cpu.lo = (WORD)cpu.X - (int) ((signed char)oprand); 
            cpu.lo = cpu.X - oprand; // A-M
            setNZ(cpu.lo);
            setCbyCompare(cpu.X,oprand);
            cpu.cycles=cpu.cycletype + cpu.cycles; 
            cpu.cycles++; 
            break;
    }
    return cpu.cycles;
}
// SHITE ROTATE instructions 
int op_BPL_00(BYTE oprand, WORD addr){
    // aaabbbcc cc = 10
    BYTE condjmp = 0;
    switch((cpu.IR)&0b11110000){
        case 0b00010000://  BPL	branch on N = 0
            condjmp =  (!TST_SR_BIT(F_N)) ;
            break;
        case 0b00110000://	BMI	branch N = 1
            condjmp =  (TST_SR_BIT(F_N));
            break;
        case 0b01010000://	BVC	branch on V = 0
            condjmp =  (!TST_SR_BIT(F_V));
            break;
        case 0b01110000://	BVS	 branch on V = 1
            condjmp =  TST_SR_BIT(F_V);
            break;
        case 0b10010000://	BCC	 branch on C = 0
            condjmp =  !TST_SR_BIT(F_C);
            break;
        case 0b10110000://	BCS	 branch on C = 1
            condjmp =  TST_SR_BIT(F_C);
            break;
        case 0b11010000://	BNE	 branch on Z = 0
            condjmp =  !TST_SR_BIT(F_Z);
            break;
        case 0b11110000://	BEQ  branch on Z = 1
            condjmp =  TST_SR_BIT(F_Z);
            break;
    }
    if (condjmp) {
        if ((cpu.PC&0xFF00)!=(addr&0xff00)) cpu.cycles++;
        cpu.cycles++;
        cpu.PC = addr; 
    }
    return cpu.cycles;
}
int op_JSR_00(BYTE oprand, WORD addr){
    //TODO aaabbbcc cc = 10
    switch((cpu.IR)&0b11110000){
        case 0b00000000://  BRK	"interrupt,push PC+2, push SR"

            PUSH(HIGHBYTE(cpu.PC+1));//cpu.memory[BANK1(cpu.SP)] = ((cpu.PC+1)&0xFF00)>>8; cpu.SP--; cpu.cycles++;// TODO: memory write cycle
            PUSH(LOWBYTE(cpu.PC+1));//cpu.memory[BANK1(cpu.SP)] = ((cpu.PC+1)&0xFF);cpu.SP--; cpu.cycles++;// TODO: memory write cycle
            PUSH(cpu.P |F_r5| F_B);//cpu.memory[BANK1(cpu.SP)] = cpu.P |F_r5| F_B; cpu.SP--; cpu.cycles++;// TODO: memory write cycle
            cpu.P |= F_r5|F_I; // disable interrupt race around 
            READ_WORD(0xfffe,addr); //addr =  (WORD)cpu.memory[0xFFFE] | ((WORD)cpu.memory[0xFFFF] << 8); cpu.cycles+=2;
            cpu.PC = addr; cpu.cycles++;
            break;
        case 0b00100000://	JSR	"push (PC+2),(PC+1) -> PCL(PC+2) -> PCH"
            PUSH(HIGHBYTE(cpu.PC-1));//cpu.memory[BANK1(cpu.SP)] = ((cpu.PC-1)&0xFF00)>>8; cpu.SP--; cpu.cycles++;/// TODO: memory write cycle
            PUSH(LOWBYTE(cpu.PC-1));//cpu.memory[BANK1(cpu.SP)] = ((cpu.PC-1)&0xFF); cpu.SP--; cpu.cycles++;// TODO: memory write cycle
            cpu.PC =  addr; cpu.cycles++;
            break;
        case 0b01000000://	RTI	pull SR, pull PC F_S|F_Z|F_C|F_B|F_D|F_V   
            POP(cpu.P); cpu.P&=~F_B; cpu.cycles++; //cpu.P = cpu.memory[BANK1(++cpu.SP)]&(~F_B); cpu.cycles+=2;
            cpu.P |= F_r5;
            POP(cpu.PC); POP(cpu.hi);
            cpu.PC |=cpu.hi<<8; cpu.cycles++;
            //cpu.PC =  ((WORD)cpu.memory[BANK1(++cpu.SP)]) | ((WORD)cpu.memory[BANK1(++cpu.SP)]<<8); cpu.cycles+=3;

            break;
        case 0b01100000://	RTS	pull PC, PC+1 -> PC
            POP(cpu.PC); POP(cpu.hi);
            cpu.PC |=cpu.hi<<8; cpu.cycles+=3;
//            cpu.PC =  ((WORD)cpu.memory[BANK1(++cpu.SP)]) | ((WORD)cpu.memory[BANK1(++cpu.SP)]<<8); cpu.cycles+=5;
            cpu.PC++;
            break;
        default:
            break;
    }
    return cpu.cycles;
}
// condition branch instructions 
int op_SINGLE(BYTE oprand, WORD addr){
    // xxy10000 
    // all implied mode
    switch((cpu.IR)){
        case 0x08: // PHP push SR
        PUSH(cpu.P | (F_r5|F_B));
           // cpu.memory[BANK1(cpu.SP)] = cpu.P | (F_r5|F_B); cpu.SP--; cpu.cycles+=1;
            break;
        case 0x18: // CLC 0 -> C
            CLR_SR_BIT(F_C); 
            break;
        case 0x28: // PLP
            POP(cpu.lo); cpu.cycles++;
            // cpu.lo = cpu.memory[BANK1(++cpu.SP)] ; cpu.cycles+=2; //& (~F_B)
            cpu.hi = cpu.P & ( F_r5|F_B);
            cpu.P = (cpu.lo & ~(F_r5|F_B))|cpu.hi;
            // cpu.P = cpu.memory[BANK1(++cpu.SP)] ; cpu.cycles++;
            // cpu.P |= (F_r5|F_B);
            break;
        case 0x38: // SEC 1 -> C 
            SET_SR_BIT(F_C); 
            break;
        case 0x48: // PHA push A
            PUSH(cpu.A); // cpu.memory[BANK1(cpu.SP)] = cpu.A; cpu.SP--; cpu.cycles++;
            break;
        case 0x58: // CLI 0 -> I
            CLR_SR_BIT(F_I); 
            break;
        case 0x68: // PLA PULL A cycles 4 F_S}F_Z
            POP(cpu.A); cpu.cycles++;//cpu.A = cpu.memory[BANK1(++cpu.SP)]; cpu.cycles+=2; 
            setNZ(cpu.A);
            break;
        case 0x78: // SEI 1 -> I
            SET_SR_BIT(F_I); 
            break;
        case 0x88: // DEY Y - 1 -> Y
            cpu.Y--;
            setNZ(cpu.Y);
            break;
        case 0x98: // TYA Y -> A
            cpu.A = cpu.Y;
            setNZ(cpu.A);
            break;
        case 0xA8: // TAY  A -> Y
            cpu.Y = cpu.A;
            setNZ(cpu.Y);
            break;
        case 0xB8: // CLV 0 -> V
            CLR_SR_BIT(F_V); 
            break;
        case 0xC8: // INY
            cpu.Y++;
            setNZ(cpu.Y);            
            break;
        case 0xD8: // CLD 0 -> D
            CLR_SR_BIT(F_D); 
            break;
        case 0xE8: // INX
            cpu.X++;
            setNZ(cpu.X);
            break;
        case 0xF8: // SED 1 -> D
            SET_SR_BIT(F_D); 
            break;
        case 0x8A: // TXA X -> A  F_S}F_Z
            cpu.A = cpu.X;
            setNZ(cpu.A);
            break;
        case 0x9A: // TXS X -> P
            cpu.SP = cpu.X;
            break;
        case 0xAA: // TAX A -> X
            cpu.X = cpu.A;
            setNZ(cpu.A);
            break;
        case 0xBA: // TSX SP -> X  F_S}F_Z
            cpu.X = cpu.SP;
            setNZ(cpu.X);
            break;
        case 0xCA: // DEX
            cpu.X--;
            setNZ(cpu.X);
            break;
        case 0xEA: // NOP
            break;
    }
    cpu.cycles++;
    return cpu.cycles;
}
/* illegal or undocument instructions*/
int illop_nop(BYTE oprand, WORD addr){
    cpu.cycles=cpu.cycletype + cpu.cycles; 
    cpu.cycles++; 
    return cpu.cycles;
}
int illop_alr(BYTE oprand, WORD addr){ // 0x4b
    cpu.lo = cpu.A & oprand; 
    cpu.A = cpu.lo >> 1;
    ASSIGN_F_C(cpu.lo&0x1);
    setNZ(cpu.A);
    cpu.cycles=cpu.cycletype + cpu.cycles; 
    cpu.cycles++; 
    return cpu.cycles;
}
int illop_anc(BYTE oprand, WORD addr){ // 0x0B 0x2B
    cpu.A = cpu.A & oprand; 
    ASSIGN_F_C(cpu.A&0x80);
    setNZ(cpu.A);
    cpu.cycles=cpu.cycletype + cpu.cycles; 
    cpu.cycles++; 
    return cpu.cycles;
}
int illop_ane(BYTE oprand, WORD addr){ // 0x8B unstable
    cpu.A = cpu.A & cpu.X & oprand; 
    setNZ(cpu.A);
    cpu.cycles=cpu.cycletype + cpu.cycles; 
    cpu.cycles++; 
    return cpu.cycles;
}
int illop_arr(BYTE oprand, WORD addr){ // 0x6B
    cpu.lo = (cpu.A & oprand); 
    ASSIGN_F_C((cpu.A & oprand)&0x01);
    cpu.tw = cpu.A + cpu.lo;
    setADDV(cpu.lo,(BYTE)cpu.tw);
    cpu.A = ((cpu.A)>>1) | (TST_SR_BIT(F_C)<<7); 
    setNZ(cpu.A);
    cpu.cycles=cpu.cycletype + cpu.cycles; 
    cpu.cycles++; 
    return cpu.cycles;
}
int illop_dcp(BYTE oprand, WORD addr){ // TODO 0xC7 d7 cf df db c3 d3 
    cpu.cycletype = CYCLESTYPE_TABLE;             
    return cpu.cycles;
}
int illop_isc(BYTE oprand, WORD addr){ // TODO 0xe7 f7 ef ff fb e3 f3
    cpu.cycletype = CYCLESTYPE_TABLE;             
    return cpu.cycles;
}
int illop_las(BYTE oprand, WORD addr){ // TODO 0xBB
    cpu.cycles=cpu.cycletype + cpu.cycles; 
    cpu.cycles++; 
    return cpu.cycles;
}
int illop_lax(BYTE oprand, WORD addr){ // TODO 0xa7 b7 af bf a3 b3
    cpu.cycles=cpu.cycletype + cpu.cycles; 
    cpu.cycles++; 
    return cpu.cycles;
}
int illop_lxa(BYTE oprand, WORD addr){ // TODO 0xAB
    cpu.cycles=cpu.cycletype + cpu.cycles; 
    cpu.cycles++; 
    return cpu.cycles;
}
int illop_rla(BYTE oprand, WORD addr){ // TODO 0x27 37 2f 3f 3b 23 33 
    cpu.cycletype = CYCLESTYPE_TABLE;             
    return cpu.cycles;
}
int illop_rra(BYTE oprand, WORD addr){ // TODO 0x67 77 6f 7f 7b 63 73 
    cpu.cycletype = CYCLESTYPE_TABLE;             
    return cpu.cycles;
}
int illop_sax(BYTE oprand, WORD addr){ // TODO 0x87 97 8f 83 
    cpu.cycletype = CYCLESTYPE_TABLE;             
    return cpu.cycles;
}
int illop_sbx(BYTE oprand, WORD addr){ // TODO 0xCB
    cpu.cycletype = CYCLESTYPE_TABLE;             
    return cpu.cycles;
}
int illop_sha(BYTE oprand, WORD addr){ // TODO 0x9F 3F
    cpu.cycletype = CYCLESTYPE_TABLE;             
    return cpu.cycles;
}
int illop_shx(BYTE oprand, WORD addr){ // TODO 0x9E
    cpu.cycletype = CYCLESTYPE_TABLE;             
    return cpu.cycles;
}
int illop_shy(BYTE oprand, WORD addr){ // TODO 0x9C
    cpu.cycletype = CYCLESTYPE_TABLE;             
    return cpu.cycles;
}
int illop_slo(BYTE oprand, WORD addr){ // TODO 0x07 17 0f 1F 1B 03 13 
    cpu.cycletype = CYCLESTYPE_TABLE;             
    return cpu.cycles;
}
int illop_sre(BYTE oprand, WORD addr){ // TODO 0x47 57 4f 5F 5B 43 53 
    cpu.cycletype = CYCLESTYPE_TABLE;             
    return cpu.cycles;
}
int illop_tas(BYTE oprand, WORD addr){ // TODO 0x9B 
    cpu.cycletype = CYCLESTYPE_TABLE;             
    return cpu.cycles;
}
int illop_usbc(BYTE oprand, WORD addr){ // TODO 0xEB same as SBC 
    cpu.cycletype = CYCLESTYPE_TABLE;             
    return cpu.cycles;
}
int illop_hlt(BYTE oprand, WORD addr){ // TODO  0x02 12 32 42 52 62 72 92 b2 d2 f2
    // must reset
    cpu.PC = cpu.PC - 1; // HALT lock
    cpu.cycles = 0;             
    return cpu.cycles;
}
// BYTE testop[256];
int HALCPU6502_Running(BYTE forever, int ticks) {
    // Fetch the instruction pointed by PC into IR
    // fetch op code
    BYTE oprand;
    WORD address;
    // BYTE opcode = 0x61;
    // WORD pc[] = {0x5cf, 0x37c4, 0x3308, 0x3368, 0x340c, 0x3469, 0xffff};
    // // WORD lasttw = 0x8;
    // WORD lastpc = 0x0;
    // int repeatcnt = 0;
    // int cycles=0;
    // printf("cpurun%d\n",ticks);
    while(forever||(ticks > 0)){
        // reset 
        resetExecute();
        // NMI
        nmiExecute();
        // irq 
        irqExecute();

    /* for test breakpoint */
        // 1. fetch op
        // if (cpu.tw==0xffff || (cpu.PC==0x335f && cpu.A==lasttw)) {
        //     cpu.IR=cpu.memory[cpu.PC];
        //     printf("opcode%s[%2x]\n",instructions[cpu.IR].mnemonic,instructions[cpu.IR].opcode);
        // }
        // for(int i=0;i<10;i++){
        //     if (pc[i]==0xFFFF) break;
        //     if (cpu.PC==pc[i]) {
        //         READ_BYTEN(cpu.PC,cpu.IR); 
        //         printf("PC=%4x,opcode%s[%2x]\n",pc[i],instructions[cpu.IR].mnemonic,instructions[cpu.IR].opcode);
        //         break;
        //     }
        // }
        // if (lastpc==cpu.PC) {
        //     repeatcnt++;
        //     if (repeatcnt > 10) {
        //         if (cpu.PC==0x3469) {
        //             printf("Tesing PASS\n");
        //             break;
        //         }
        //         printf("fail:PC=%4x,opcode%s[%2x]\n",cpu.PC,instructions[cpu.IR].mnemonic,instructions[cpu.IR].opcode);
        //         break;
        //     }
              
        // }else{
        //     repeatcnt = 0;
        // }
        // lastpc = cpu.PC;
        // dumpReg(ticks);
        FETCH_BYTE(cpu.IR); 

        // if (cpu.IR == opcode){
        //         printf("PC=%4x,opcode%s[%2x]\n",cpu.PC-1,instructions[cpu.IR].mnemonic,instructions[cpu.IR].opcode);
        // }
        // 2. fetch oprand and process addressing mode
        cpu.cycles = oprand_multiplexor_src(instructions[cpu.IR].mode, &oprand, &address);
        // printf("oprand=%x, addr=%x\n",oprand,address);
        // execute the instruction based on the opcode in IR
        cpu.cycles = (*(instructions[cpu.IR].opexec))(oprand,address);

        // Increment the cycle count based on the instruction's execution time
        if (cpu.cycletype==CYCLESTYPE_TABLE) {
            cpu.cycles = instructions[cpu.IR].cycles;
        }
        // if (cpu.cycles&0x80) {
        //     printf("Fail1*%s[%2x][%2x]",instructions[cpu.IR].mnemonic,instructions[cpu.IR].opcode,cpu.cycles);
        //     printf("(%1d,%2d])\n",instructions[cpu.IR].length,instructions[cpu.IR].cycles);
        //     break;
        // }
        ticks -= cpu.cycles;
        // dumpReg(ticks);
        // cycles+= cpu.cycles;
        // if (cycles >1000) {
        //     cycles=0;
        // }
        // /* test cycles*/
        // testop[cpu.IR] = 1;
        // if (instructions[cpu.IR].cycles&0x80) {
        //     // printf("opcode%s[%2x]",instructions[cpu.IR].mnemonic,instructions[cpu.IR].opcode);
        //     // printf("(%1d,%2d])\n",instructions[cpu.IR].length,instructions[cpu.IR].cycles);
        // }else{
        //     if (cpu.cycles != instructions[cpu.IR].cycles) {
        //         // printf("Fail2*%s[%2x][%2x]",instructions[cpu.IR].mnemonic,instructions[cpu.IR].opcode,cpu.cycles);
        //         // printf("(%1d,%2d])\n",instructions[cpu.IR].length,instructions[cpu.IR].cycles);
        //     }
        // }
    }
    // for(int i=0;i<256;i++) {
    //     if ( testop[i]!=1){
    //         cpu.IR=i;
    //         printf("opUntest%s[%2x]",instructions[cpu.IR].mnemonic,instructions[cpu.IR].opcode);
    //         printf("(%1d,%2d])\n",instructions[cpu.IR].length,instructions[cpu.IR].cycles);            
    //     }
    // }
    return ticks;
}

INSTRUCTION_ITEM instructions[]= {
//opcode,MNEMONIC       ,OPEXEC     ,Address Mode       ,FLAGS                      ,LENGTH ,CYCLES  //  ,OPERANDS   ,FUNCTION                       ,CATALOG
 {0x00, "BRK"            ,op_JSR_00 ,IMPLICIT           ,F_I                        ,1      ,7     },// ,           ,"interrupt,push PC+2, push SR" ,BRE
 {0x01, "ORA"            ,op_ALU_01 ,INDIRECT_X         ,F_N|F_Z                    ,2      ,6     },// ,"(ind,X)    ",A OR M -> A,ALU (L) or
 {0x02, "HLT"            ,illop_hlt ,NO_OPRAND          ,0                          ,1      ,1     },// ,,,
 {0x03, "SLO"            ,illop_slo ,INDIRECT_X         ,F_N|F_Z|F_C                ,2      ,8     },// ,"(ind,X)    ",M = (0>>[76543210]>>C) A | M -> A,ALU
 {0x04, "NOP"            ,illop_nop ,ZEROPAGE           ,0                          ,2      ,3     },// ,zpg        ,,
 {0x05, "ORA"            ,op_ALU_01 ,ZEROPAGE           ,F_N|F_Z                    ,2      ,3     },// ,zpg        ,A OR M -> A,ALU (L) or
 {0x06, "ASL"            ,op_ASL_10 ,ZEROPAGE           ,F_N|F_Z|F_C                ,2      ,5     },// ,zpg        ,C <- [76543210] <- 0,ALU (L) SHIFT
 {0x07, "SLO"            ,illop_slo ,ZEROPAGE           ,F_N|F_Z|F_C                ,2      ,5     },// ,zpg        ,M = (0>>[76543210]>>C) A | M -> A,ALU(L
 {0x08, "PHP"            ,op_SINGLE ,IMPLICIT           ,0                          ,1      ,3     },// ,           ,push SR,MEMORY LOAD STORE
 {0x09, "ORA"            ,op_ALU_01 ,IMMEDIATE          ,F_N|F_Z                    ,2      ,2     },// ,#          ,A OR M -> A,ALU (L) or
 {0x0A, "ASL"            ,op_ASL_10 ,ACCUMULATOR        ,F_N|F_Z|F_C                ,1      ,2     },// ,A          ,C <- [76543210] <- 0,ALU (L) SHIFT
 {0x0B, "ANC"            ,illop_anc ,IMMEDIATE          ,F_N|F_Z|F_C                ,2      ,2     },// ,#,"A & oper, bit(7)->c","ALU (L) SHIFT, PS"
 {0x0C, "NOP"            ,illop_nop ,ABSOLUTE           ,0                          ,3      ,4     },// ,Abs,,
 {0x0D, "ORA"            ,op_ALU_01 ,ABSOLUTE           ,F_N|F_Z                    ,3      ,4     },// ,abs        ,A OR M -> A,ALU (L) or
 {0x0E, "ASL"            ,op_ASL_10 ,ABSOLUTE           ,F_N|F_Z|F_C                ,3      ,6     },// ,abs        ,C <- [76543210] <- 0,ALU (L) SHIFT
 {0x0F, "SLO"            ,illop_slo ,ABSOLUTE           ,F_N|F_Z|F_C                ,3      ,6     },// ,abs      ,M = (0>>[76543210]>>C) A | M -> A,ALU(L
 {0x10, "BPL"            ,op_BPL_00 ,RELATIVE_ADDRESS   ,0                          ,2      ,-2    },// ,rel        ,branch on N = 0,COND BRANCH
 {0x11, "ORA"            ,op_ALU_01 ,INDIRECT_Y         ,F_N|F_Z                    ,2      ,-5    },// ,"(ind),Y    ",A OR M -> A,ALU (L) or
 {0x12, "HLT"            ,illop_hlt ,NO_OPRAND          ,0                          ,1      ,1     },// ,,,
 {0x13, "SLO"            ,illop_slo ,INDIRECT_Y         ,F_N|F_Z|F_C                ,2      ,8     },// ,"(ind),Y    ",M = (0>>[76543210]>>C) A | M -> A,ALU
 {0x14, "NOP"            ,illop_nop ,ZEROPAGE_X         ,0                          ,2      ,4     },// ,"zpg,X      ",,
 {0x15, "ORA"            ,op_ALU_01 ,ZEROPAGE_X         ,F_N|F_Z                    ,2      ,4     },// ,"zpg,X      ",A OR M -> A,ALU (L) or
 {0x16, "ASL"            ,op_ASL_10 ,ZEROPAGE_X         ,F_N|F_Z|F_C                ,2      ,6     },// ,"zpg,X      ",C <- [76543210] <- 0,ALU (L) SHIFT
 {0x17, "SLO"            ,illop_slo ,ZEROPAGE_X         ,F_N|F_Z|F_C                ,2      ,6     },// ,"zpg,X      ",M = (0>>[76543210]>>C) A | M -> A,ALU
 {0x18, "CLC"            ,op_SINGLE ,IMPLICIT           ,F_C                        ,1      ,2     },// ,           ,0 -> C,SR flag
 {0x19, "ORA"            ,op_ALU_01 ,ABSOLUTE_Y         ,F_N|F_Z                    ,3      ,-4    },// ,"abs,Y      ",A OR M -> A,ALU (L) or
 {0x1A, "NOP"            ,illop_nop ,IMPLICIT           ,0                          ,1      ,2     },// ,,,
 {0x1B, "SLO"            ,illop_slo ,ABSOLUTE_Y         ,F_N|F_Z|F_C                ,3      ,7     },// ,"abs,Y      ",M = (0>>[76543210]>>C) A | M -> A,ALU
 {0x1C, "NOP"            ,illop_nop ,ABSOLUTE_X         ,0                          ,3      ,-4    },// ,"Abs,X",,
 {0x1D, "ORA"            ,op_ALU_01 ,ABSOLUTE_X         ,F_N|F_Z                    ,3      ,-4    },// ,"abs,X      ",A OR M -> A,ALU (L) or
 {0x1E, "ASL"            ,op_ASL_10 ,ABSOLUTE_X         ,F_N|F_Z|F_C                ,3      ,7     },// ,"abs,X      ",C <- [76543210] <- 0,ALU (L) SHIFT
 {0x1F, "SLO"            ,illop_slo ,ABSOLUTE_X         ,F_N|F_Z|F_C                ,3      ,7     },// ,"abs,X      ",M = (0>>[76543210]>>C) A | M -> A,ALU
 {0x20, "JSR"            ,op_JSR_00 ,ABSOLUTE           ,0                          ,3      ,6     },// ,abs        ,"push (PC+2),(PC+1) -> PCL(PC+2) ->
 {0x21, "AND"            ,op_ALU_01 ,INDIRECT_X         ,F_N|F_Z                    ,2      ,6     },// ,"(ind,X)    ",A AND M -> A,ALU (L) AND
 {0x22, "HLT"            ,illop_hlt ,NO_OPRAND          ,0                          ,1      ,1     },// ,,,
 {0x23, "RLA"            ,illop_rla ,INDIRECT_X         ,F_N|F_Z|F_C                ,2      ,8     },// ,"(ind,X)    ","M = C <- [76543210] <- C, A AND M 
 {0x24, "BIT"            ,op_BIT_00 ,ZEROPAGE           ,F_N|F_V|F_Z                ,2      ,3     },// ,zpg        ,"A AND M, M7 -> N, M6 -> V",SR flag
 {0x25, "AND"            ,op_ALU_01 ,ZEROPAGE           ,F_N|F_Z                    ,2      ,3     },// ,zpg        ,A AND M -> A,ALU (L) AND
 {0x26, "ROL"            ,op_ASL_10 ,ZEROPAGE           ,F_N|F_Z|F_C                ,2      ,5     },// ,zpg        ,C <- [76543210] <- C,ALU (L) ROT
 {0x27, "RLA"            ,illop_rla ,ZEROPAGE           ,F_N|F_Z|F_C                ,2      ,5     },// ,zpg        ,"M = C <- [76543210] <- C, A AND M -> 
 {0x28, "PLP"            ,op_SINGLE ,IMPLICIT           ,F_N|F_Z|F_C|F_D|F_V|F_I    ,1      ,4     },// ,           ,pull SR,MEMORY LOAD STORE
 {0x29, "AND"            ,op_ALU_01 ,IMMEDIATE          ,F_N|F_Z                    ,2      ,2     },// ,#          ,A AND M -> A,ALU (L) AND
 {0x2A, "ROL"            ,op_ASL_10 ,ACCUMULATOR        ,F_N|F_Z|F_C                ,1      ,2     },// ,A          ,C <- [76543210] <- C,ALU (L) ROT
 {0x2B, "ANC2"           ,illop_anc ,IMMEDIATE          ,F_N|F_Z|F_C                ,2      ,2     },// ,#,"A & oper, bit(7)->c","ALU (L) SHIFT, PS"
 {0x2C, "BIT"            ,op_BIT_00 ,ABSOLUTE           ,F_N|F_V|F_Z                ,3      ,4     },// ,abs        ,"A AND M, M7 -> N, M6 -> V",SR flag
 {0x2D, "AND"            ,op_ALU_01 ,ABSOLUTE           ,F_N|F_Z                    ,3      ,4     },// ,abs        ,A AND M -> A,ALU (L) AND
 {0x2E, "ROL"            ,op_ASL_10 ,ABSOLUTE           ,F_N|F_Z|F_C                ,3      ,6     },// ,abs        ,C <- [76543210] <- C,ALU (L) ROT
 {0x2F, "RLA"            ,illop_rla ,ABSOLUTE           ,F_N|F_Z|F_C                ,3      ,6     },// ,abs      ,"M = C <- [76543210] <- C, A AND M -> A"
 {0x30, "BMI"            ,op_BPL_00 ,RELATIVE_ADDRESS   ,0                          ,2      ,-2    },// ,rel        ,branch on N = 1,COND BRANCH
 {0x31, "AND"            ,op_ALU_01 ,INDIRECT_Y         ,F_N|F_Z                    ,2      ,-5    },// ,"(ind),Y    ",A AND M -> A,ALU (L) AND
 {0x32, "HLT"            ,illop_hlt ,NO_OPRAND          ,0                          ,1      ,1     },// ,,,
 {0x33, "RLA"            ,illop_rla ,INDIRECT_Y         ,F_N|F_Z|F_C                ,3      ,8     },// ,"(ind),Y    ","M = C <- [76543210] <- C, A AND M 
 {0x34, "NOP"            ,illop_nop ,ZEROPAGE_X         ,0                          ,2      ,4     },// ,"zpg,X      ",,
 {0x35, "AND"            ,op_ALU_01 ,ZEROPAGE_X         ,F_N|F_Z                    ,2      ,4     },// ,"zpg,X      ",A AND M -> A,ALU (L) AND
 {0x36, "ROL"            ,op_ASL_10 ,ZEROPAGE_X         ,F_N|F_Z|F_C                ,2      ,6     },// ,"zpg,X      ",C <- [76543210] <- C,ALU (L) ROT
 {0x37, "RLA"            ,illop_rla ,ZEROPAGE_X         ,F_N|F_Z|F_C                ,2      ,6     },// ,"zpg,X      ","M = C <- [76543210] <- C, A AND M 
 {0x38, "SEC"            ,op_SINGLE ,IMPLICIT           ,F_C                        ,1      ,2     },// ,           ,1 -> C,SR flag
 {0x39, "AND"            ,op_ALU_01 ,ABSOLUTE_Y         ,F_N|F_Z                    ,3      ,-4    },// ,"abs,Y      ",A AND M -> A,ALU (L) AND
 {0x3A, "NOP"            ,illop_nop ,IMPLICIT           ,0                          ,1      ,2     },// ,,,
 {0x3B, "RLA"            ,illop_rla ,ABSOLUTE_Y         ,F_N|F_Z|F_C                ,3      ,7     },// ,"abs,Y     ","M = C <- [76543210] <- C, A AND M -> 
 {0x3C, "NOP"            ,illop_nop ,ABSOLUTE_X         ,0                          ,3      ,-4    },// ,"Abs,X",,
 {0x3D, "AND"            ,op_ALU_01 ,ABSOLUTE_X         ,F_N|F_Z                    ,3      ,-4    },// ,"abs,X      ",A AND M -> A,ALU (L) AND
 {0x3E, "ROL"            ,op_ASL_10 ,ABSOLUTE_X         ,F_N|F_Z|F_C                ,3      ,7     },// ,"abs,X      ",C <- [76543210] <- C,ALU (L) ROT
 {0x3F, "RLA"            ,illop_rla ,ABSOLUTE_X         ,F_N|F_Z|F_C                ,3      ,7     },// ,"abs,X      ","M = C <- [76543210] <- C, A AND M 
 {0x40, "RTI"            ,op_JSR_00 ,IMPLICIT           ,F_N|F_Z|F_C|F_I|F_D|F_V    ,1      ,6     },// ,           ,"pull SR, pull PC",BRANCH
 {0x41, "EOR"            ,op_ALU_01 ,INDIRECT_X         ,F_N|F_Z                    ,2      ,6     },// ,"(ind,X)    ",A EOR M -> A,ALU (L) XOR
 {0x42, "HLT"            ,illop_hlt ,NO_OPRAND          ,0                          ,1      ,1     },// ,,,
 {0x43, "SRE"            ,illop_sre ,INDIRECT_X         ,F_N|F_Z|F_C                ,2      ,8     },// ,"(ind,X)    ",M = (0>>[76543210]>>C) A ^ M -> A,ALU
 {0x44, "NOP"            ,illop_nop ,ZEROPAGE           ,0                          ,2      ,3     },// ,zpg        ,,
 {0x45, "EOR"            ,op_ALU_01 ,ZEROPAGE           ,F_N|F_Z                    ,2      ,3     },// ,zpg        ,A EOR M -> A,ALU (L) XOR
 {0x46, "LSR"            ,op_ASL_10 ,ZEROPAGE           ,F_N|F_Z|F_C                ,2      ,5     },// ,zpg        ,0 -> [76543210] -> C,ALU (L) shift
 {0x47, "SRE"            ,illop_sre ,ZEROPAGE           ,F_N|F_Z|F_C                ,2      ,5     },// ,zpg        ,M = (0>>[76543210]>>C) A ^ M -> A,ALU(L
 {0x48, "PHA"            ,op_SINGLE ,IMPLICIT           ,0                          ,1      ,3     },// ,           ,push A,MEMORY LOAD STORE
 {0x49, "EOR"            ,op_ALU_01 ,IMMEDIATE          ,F_N|F_Z                    ,2      ,2     },// ,#          ,A EOR M -> A,ALU (L) XOR
 {0x4A, "LSR"            ,op_ASL_10 ,ACCUMULATOR        ,F_N|F_Z|F_C                ,1      ,2     },// ,A          ,0 -> [76543210] -> C,ALU (L) shift
 {0x4B, "ALR"            ,illop_alr ,IMMEDIATE          ,F_N|F_Z|F_C                ,2      ,3     },// ,#,"A & oper, 0->[76543210]->C",ALU (L) shift
 {0x4C, "JMP"            ,op_BIT_00 ,ABSOLUTE           ,0                          ,3      ,3     },// ,abs        ,"(PC+1) -> PCL(PC+2) -> PCH",BRANCH
 {0x4D, "EOR"            ,op_ALU_01 ,ABSOLUTE           ,F_N|F_Z                    ,3      ,4     },// ,abs        ,A EOR M -> A,ALU (L) XOR
 {0x4E, "LSR"            ,op_ASL_10 ,ABSOLUTE           ,F_N|F_Z|F_C                ,3      ,6     },// ,abs        ,0 -> [76543210] -> C,ALU (L) shift
 {0x4F, "SRE"            ,illop_sre ,ABSOLUTE           ,F_N|F_Z|F_C                ,3      ,6     },// ,Abs,M = (0>>[76543210]>>C) A ^ M -> A,ALU(L)
 {0x50, "BVC"            ,op_BPL_00 ,RELATIVE_ADDRESS   ,0                          ,2      ,-2    },// ,rel        ,branch on V = 0,COND BRANCH
 {0x51, "EOR"            ,op_ALU_01 ,INDIRECT_Y         ,F_N|F_Z                    ,2      ,-5    },// ,"(ind),Y    ",A EOR M -> A,ALU (L) XOR
 {0x52, "HLT"            ,illop_hlt ,NO_OPRAND          ,0                          ,1      ,1     },// ,,,
 {0x53, "SRE"            ,illop_sre ,INDIRECT_Y         ,F_N|F_Z|F_C                ,2      ,8     },// ,"(ind),Y    ",M = (0>>[76543210]>>C) A ^ M -> A,ALU
 {0x54, "NOP"            ,illop_nop ,ZEROPAGE_X         ,0                          ,2      ,4     },// ,"zpg,X      ",,
 {0x55, "EOR"            ,op_ALU_01 ,ZEROPAGE_X         ,F_N|F_Z                    ,2      ,4     },// ,"zpg,X      ",A EOR M -> A,ALU (L) XOR
 {0x56, "LSR"            ,op_ASL_10 ,ZEROPAGE_X         ,F_N|F_Z|F_C                ,2      ,6     },// ,"zpg,X      ",0 -> [76543210] -> C,ALU (L) shift
 {0x57, "SRE"            ,illop_sre ,ZEROPAGE_X         ,F_N|F_Z|F_C                ,2      ,6     },// ,"zpg,X      ",M = (0>>[76543210]>>C) A ^ M -> A,ALU
 {0x58, "CLI"            ,op_SINGLE ,IMPLICIT           ,F_I                        ,1      ,2     },// ,        ,0 -> I,SR flag
 {0x59, "EOR"            ,op_ALU_01 ,ABSOLUTE_Y         ,F_N|F_Z                    ,3      ,-4    },// ,"abs,Y      ",A EOR M -> A,ALU (L) XOR
 {0x5A, "NOP"            ,illop_nop ,IMPLICIT           ,0                          ,1      ,2     },// ,,,
 {0x5B, "SRE"            ,illop_sre ,ABSOLUTE_Y         ,F_N|F_Z|F_C                ,3      ,7     },// ,"abs,Y      ",M = (0>>[76543210]>>C) A ^ M -> A,ALU
 {0x5C, "NOP"            ,illop_nop ,ABSOLUTE_X         ,0                          ,3      ,-4    },// ,"Abs,X",,
 {0x5D, "EOR"            ,op_ALU_01 ,ABSOLUTE_X         ,F_N|F_Z                    ,3      ,-4    },// ,"abs,X      ",A EOR M -> A,ALU (L) XOR
 {0x5E, "LSR"            ,op_ASL_10 ,ABSOLUTE_X         ,F_N|F_Z|F_C                ,3      ,7     },// ,"abs,X      ",0 -> [76543210] -> C,ALU (L) shift
 {0x5F, "SRE"            ,illop_sre ,ABSOLUTE_X         ,F_N|F_Z|F_C                ,3      ,7     },// ,"abs,X      ",M = (0>>[76543210]>>C) A ^ M -> A,ALU
 {0x60, "RTS"            ,op_JSR_00 ,IMPLICIT           ,0                          ,1      ,6     },// ,           ,"pull PC, PC+1 -> PC",BRANCH
 {0x61, "ADC"            ,op_ALU_01 ,INDIRECT_X         ,F_N|F_V|F_Z|F_C            ,2      ,6     },// ,"(ind,X)    ","A + M + C -> A, C",ALU (A) ADD + C
 {0x62, "HLT"            ,illop_hlt ,NO_OPRAND          ,0                          ,1      ,1     },// ,,,
 {0x63, "RRA"            ,illop_rra ,INDIRECT_X         ,F_N|F_Z|F_C|F_V            ,2      ,8     },// ,"(ind,X)    ","M = C -> [76543210] -> C, A + M + IFT
 {0x64, "NOP"            ,illop_nop ,ZEROPAGE           ,0                          ,2      ,3     },// ,zpg        ,,
 {0x65, "ADC"            ,op_ALU_01 ,ZEROPAGE           ,F_N|F_V|F_Z|F_C            ,2      ,3     },// ,zpg        ,"A + M + C -> A, C",ALU (A) ADD + C
 {0x66, "ROR"            ,op_ASL_10 ,ZEROPAGE           ,F_N|F_Z|F_C                ,2      ,5     },// ,zpg        ,C -> [76543210] -> C,ALU (L) ROT
 {0x67, "RRA"            ,illop_rra ,ZEROPAGE           ,F_N|F_Z|F_C                ,2      ,5     },// ,zpg,"M = C -> [76543210] -> C, A + M + C -> A, C"
 {0x68, "PLA"            ,op_SINGLE ,IMPLICIT           ,F_N|F_Z                    ,1      ,4     },// ,           ,pull A,MEMORY LOAD STORE
 {0x69, "ADC"            ,op_ALU_01 ,IMMEDIATE          ,F_N|F_V|F_Z|F_C            ,2      ,2     },// ,#          ,"A + M + C -> A, C",ALU (A) ADD + C
 {0x6A, "ROR"            ,op_ASL_10 ,ACCUMULATOR        ,F_N|F_Z|F_C                ,1      ,2     },// ,A          ,C -> [76543210] -> C,ALU (L) ROT
 {0x6B, "ARR"            ,illop_arr ,IMMEDIATE          ,F_N|F_Z|F_C                ,2      ,2     },// ,#,"A & oper, C->[76543210]->C",
 {0x6C, "JMP"            ,op_BIT_00 ,INDIRECT_ADDRESS   ,0                          ,3      ,5     },// ,(ind)      ,"(PC+1) -> PCL(PC+2) -> PCH",BRANCH
 {0x6D, "ADC"            ,op_ALU_01 ,ABSOLUTE           ,F_N|F_V|F_Z|F_C            ,3      ,4     },// ,abs        ,"A + M + C -> A, C",ALU (A) ADD + C
 {0x6E, "ROR"            ,op_ASL_10 ,ABSOLUTE           ,F_N|F_Z|F_C                ,3      ,6     },// ,abs        ,C -> [76543210] -> C,ALU (L) ROT
 {0x6F, "RRA"            ,illop_rra ,ABSOLUTE           ,F_N|F_Z|F_C                ,3      ,6     },// ,Abs,"M = C -> [76543210] -> C, A + M + C -> A, C"
 {0x70, "BVS"            ,op_BPL_00 ,RELATIVE_ADDRESS   ,0                          ,2      ,-2    },// ,rel        ,branch on V = 1,COND BRANCH
 {0x71, "ADC"            ,op_ALU_01 ,INDIRECT_Y         ,F_N|F_V|F_Z|F_C            ,2      ,-5    },// ,"(ind),Y    ","A + M + C -> A, C",ALU (A) ADD + C
 {0x72, "HLT"            ,illop_hlt ,NO_OPRAND          ,0                          ,1      ,1     },// ,,,
 {0x73, "RRA"            ,illop_rra ,INDIRECT_Y         ,F_N|F_Z|F_C                ,2      ,8     },// ,"(ind),Y    ","M = C -> [76543210] -> C, A + M + IFT
 {0x74, "NOP"            ,illop_nop ,ZEROPAGE_X         ,0                          ,2      ,4     },// ,"zpg,X      ",,
 {0x75, "ADC"            ,op_ALU_01 ,ZEROPAGE_X         ,F_N|F_V|F_Z|F_C            ,2      ,4     },// ,"zpg,X      ","A + M + C -> A, C",ALU (A) ADD + C
 {0x76, "ROR"            ,op_ASL_10 ,ZEROPAGE_X         ,F_N|F_Z|F_C                ,2      ,6     },// ,"zpg,X      ",C -> [76543210] -> C,ALU (L) ROT
 {0x77, "RRA"            ,illop_rra ,ZEROPAGE_X         ,F_N|F_Z|F_C                ,2      ,6     },// ,"zpg,X      ","M = C -> [76543210] -> C, A + M + IFT
 {0x78, "SEI"            ,op_SINGLE ,IMPLICIT           ,F_I                        ,1      ,2     },// , ,1 -> I,SR flag
 {0x79, "ADC"            ,op_ALU_01 ,ABSOLUTE_Y         ,F_N|F_V|F_Z|F_C            ,3      ,-4    },// ,"abs,Y      ","A + M + C -> A, C",ALU (A) ADD + C
 {0x7A, "NOP"            ,illop_nop ,IMPLICIT           ,0                          ,1      ,2     },// ,,,
 {0x7B, "RRA"            ,illop_rra ,ABSOLUTE_Y         ,F_N|F_Z|F_C                ,3      ,7     },// ,"abs,y","M = C -> [76543210] -> C, A + M + C -> A,
 {0x7C, "NOP"            ,illop_nop ,ABSOLUTE_X         ,0                          ,3      ,-4    },// ,"Abs,X",,
 {0x7D, "ADC"            ,op_ALU_01 ,ABSOLUTE_X         ,F_N|F_V|F_Z|F_C            ,3      ,-4    },// ,"abs,X      ","A + M + C -> A, C",ALU (A) ADD + C
 {0x7E, "ROR"            ,op_ASL_10 ,ABSOLUTE_X         ,F_N|F_Z|F_C                ,3      ,7     },// ,"abs,X      ",C -> [76543210] -> C,ALU (L) ROT
 {0x7F, "RRA"            ,illop_rra ,ABSOLUTE_X         ,F_N|F_Z|F_C                ,3      ,7     },// ,"Abs,X","M = C -> [76543210] -> C, A + M + C -> A,
 {0x80, "NOP"            ,illop_nop ,IMMEDIATE          ,0                          ,2      ,2     },// ,#,,
 {0x81, "STA"            ,op_ALU_01 ,INDIRECT_X         ,0                          ,2      ,6     },// ,"(ind,X)    ",A -> M,MOVE
 {0x82, "NOP"            ,illop_nop ,IMMEDIATE          ,0                          ,2      ,2     },// ,#,,
 {0x83, "SAX"            ,illop_sax ,INDIRECT_X         ,0                          ,2      ,6     },// ,"(ind,X)    ",A AND X -> M,ALU (L)
 {0x84, "STY"            ,op_BIT_00 ,ZEROPAGE           ,0                          ,2      ,3     },// ,zpg        ,Y -> M,MOVE
 {0x85, "STA"            ,op_ALU_01 ,ZEROPAGE           ,0                          ,2      ,3     },// ,zpg        ,A -> M,MOVE
 {0x86, "STX"            ,op_ASL_10 ,ZEROPAGE           ,0                          ,2      ,3     },// ,zpg        ,X -> M,MOVE
 {0x87, "SAX"            ,illop_sax ,ZEROPAGE           ,0                          ,2      ,3     },// ,zpg,A AND X -> M,ALU (L)
 {0x88, "DEY"            ,op_SINGLE ,IMPLICIT           ,F_N|F_Z                    ,1      ,2     },// ,,Y - 1 -> Y,ALU(A) DEC
 {0x89, "NOP"            ,illop_nop ,IMMEDIATE          ,0                          ,2      ,2     },// ,#,,
 {0x8A, "TXA"            ,op_SINGLE ,IMPLICIT           ,F_N|F_Z                    ,1      ,2     },// ,           ,X -> A,MOVE
 {0x8B, "ANE"            ,illop_ane ,IMMEDIATE          ,F_N|F_Z                    ,2      ,2     },// ,#,(A | const) & X & oper ->A,Unstable
 {0x8C, "STY"            ,op_BIT_00 ,ABSOLUTE           ,0                          ,3      ,4     },// ,abs        ,Y -> M,MOVE
 {0x8D, "STA"            ,op_ALU_01 ,ABSOLUTE           ,0                          ,3      ,4     },// ,abs        ,A -> M,MOVE
 {0x8E, "STX"            ,op_ASL_10 ,ABSOLUTE           ,0                          ,3      ,4     },// ,abs        ,X -> M,MOVE
 {0x8F, "SAX"            ,illop_sax ,ABSOLUTE           ,0                          ,3      ,4     },// ,Abs,A AND X -> M,ALU (L)
 {0x90, "BCC"            ,op_BPL_00 ,RELATIVE_ADDRESS   ,0                          ,2      ,-2    },// ,rel        ,branch on C = 0,COND BRANCH
 {0x91, "STA"            ,op_ALU_01 ,INDIRECT_Y         ,0                          ,2      ,6     },// ,"(ind),Y    ",A -> M,MOVE
 {0x92, "HLT"            ,illop_hlt ,NO_OPRAND          ,0                          ,1      ,1     },// ,,,
 {0x93, "SHA"            ,illop_sha ,INDIRECT_Y         ,0                          ,2      ,6     },// ,"(ind),Y    ",A AND X AND (H+1) -> M,ALU (L UNSTATB
 {0x94, "STY"            ,op_BIT_00 ,ZEROPAGE_X         ,0                          ,2      ,4     },// ,"zpg,X      ",Y -> M,MOVE
 {0x95, "STA"            ,op_ALU_01 ,ZEROPAGE_X         ,0                          ,2      ,4     },// ,"zpg,X      ",A -> M,MOVE
 {0x96, "STX"            ,op_ASL_10 ,ZEROPAGE_Y         ,0                          ,2      ,4     },// ,"zpg,Y      ",X -> M,MOVE
 {0x97, "SAX"            ,illop_sax ,ZEROPAGE_Y         ,0                          ,2      ,4     },// ,"zpg,Y      ",A AND X -> M,ALU (L)
 {0x98, "TYA"            ,op_SINGLE ,IMPLICIT           ,F_N|F_Z                    ,1      ,2     },// ,     ,Y -> A,MOVE
 {0x99, "STA"            ,op_ALU_01 ,ABSOLUTE_Y         ,0                          ,3      ,5     },// ,"abs,Y      ",A -> M,MOVE
 {0x9A, "TXS"            ,op_SINGLE ,IMPLICIT           ,0                          ,1      ,2     },// ,  ,X -> SP,MOVE
 {0x9B, "TAS"            ,illop_tas ,ABSOLUTE_Y         ,0                          ,3      ,5     },// ,"Abs,Y    ","A & X -> S, A &X &(H+1)->M",ALU(L)
 {0x9C, "SHY"            ,illop_shy ,ABSOLUTE_X         ,0                          ,3      ,5     },// ,"abs,X      ",Y AND (H+1) -> M,ALU (L UNSTATBLE
 {0x9D, "STA"            ,op_ALU_01 ,ABSOLUTE_X         ,0                          ,3      ,5     },// ,"abs,X      ",A -> M,MOVE
 {0x9E, "SHX"            ,illop_shx ,ABSOLUTE_Y         ,0                          ,3      ,5     },// ,"abs,Y    ",X AND (H+1) -> M,ALU (L UNSTATBLE
 {0x9F, "SHA"            ,illop_sha ,ABSOLUTE_X         ,0                          ,3      ,5     },// ,"abs,X      ",A AND X AND (H+1) -> M,ALU (L UNSTATB
 {0xA0, "LDY"            ,op_BIT_00 ,IMMEDIATE          ,F_N|F_Z                    ,2      ,2     },// ,#          ,M -> Y,ALU (A) LOAD 
 {0xA1, "LDA"            ,op_ALU_01 ,INDIRECT_X         ,F_N|F_Z                    ,2      ,6     },// ,"(ind,X)    ",M -> A,ALU (A) LOAD 
 {0xA2, "LDX"            ,op_ASL_10 ,IMMEDIATE          ,F_N|F_Z                    ,2      ,2     },// ,#          ,M -> X,ALU (A) LOAD 
 {0xA3, "LAX"            ,illop_las ,INDIRECT_X         ,F_N|F_Z                    ,2      ,6     },// ,"(ind,X)    ",M->A->X,LOAD STORE
 {0xA4, "LDY"            ,op_BIT_00 ,ZEROPAGE           ,F_N|F_Z                    ,2      ,3     },// ,zpg        ,M -> Y,ALU (A) LOAD 
 {0xA5, "LDA"            ,op_ALU_01 ,ZEROPAGE           ,F_N|F_Z                    ,2      ,3     },// ,zpg        ,M -> A,ALU (A) LOAD 
 {0xA6, "LDX"            ,op_ASL_10 ,ZEROPAGE           ,F_N|F_Z                    ,2      ,3     },// ,zpg        ,M -> X,ALU (A) LOAD 
 {0xA7, "LAX"            ,illop_las ,ZEROPAGE           ,F_N|F_Z                    ,2      ,3     },// ,zpg        ,M->A->X,op_lax_illegal
 {0xA8, "TAY"            ,op_SINGLE ,IMPLICIT           ,F_N|F_Z                    ,1      ,2     },// ,          ,A -> Y,MOVE
 {0xA9, "LDA"            ,op_ALU_01 ,IMMEDIATE          ,F_N|F_Z                    ,2      ,2     },// ,#          ,M -> A,ALU (A) LOAD 
 {0xAA, "TAX"            ,op_SINGLE ,IMPLICIT           ,F_N|F_Z                    ,1      ,2     },// ,          ,A -> X,MOVE
 {0xAB, "LXA"            ,illop_lxa ,NO_OPRAND          ,0                          ,0      ,0     },//  ,,,
 {0xAC, "LDY"            ,op_BIT_00 ,ABSOLUTE           ,F_N|F_Z                    ,3      ,4     },// ,abs        ,M -> Y,ALU (A) LOAD 
 {0xAD, "LDA"            ,op_ALU_01 ,ABSOLUTE           ,F_N|F_Z                    ,3      ,4     },// ,abs        ,M -> A,ALU (A) LOAD 
 {0xAE, "LDX"            ,op_ASL_10 ,ABSOLUTE           ,F_N|F_Z                    ,3      ,4     },// ,abs        ,M -> X,ALU (A) LOAD 
 {0xAF, "LAX"            ,illop_las ,ABSOLUTE           ,F_N|F_Z                    ,3      ,4     },// ,abs      ,M->A->X,op_lax_illegal
 {0xB0, "BCS"            ,op_BPL_00 ,RELATIVE_ADDRESS   ,0                          ,2      ,-2    },// ,rel        ,branch on C = 1,COND BRANCH
 {0xB1, "LDA"            ,op_ALU_01 ,INDIRECT_Y         ,F_N|F_Z                    ,2      ,-5    },// ,"(ind),Y    ",M -> A,ALU (A) LOAD 
 {0xB2, "HLT"            ,illop_hlt ,NO_OPRAND          ,0                          ,1      ,1     },// ,,,
 {0xB3, "LAX"            ,illop_las ,INDIRECT_Y         ,F_N|F_Z                    ,2      ,-5    },// ,"(ind),Y    ",M->A->X,op_lax_illegal
 {0xB4, "LDY"            ,op_BIT_00 ,ZEROPAGE_X         ,F_N|F_Z                    ,2      ,4     },// ,"zpg,X      ",M -> Y,ALU (A) LOAD 
 {0xB5, "LDA"            ,op_ALU_01 ,ZEROPAGE_X         ,F_N|F_Z                    ,2      ,4     },// ,"zpg,X      ",M -> A,ALU (A) LOAD 
 {0xB6, "LDX"            ,op_ASL_10 ,ZEROPAGE_Y         ,F_N|F_Z                    ,2      ,4     },// ,"zpg,Y      ",M -> X,ALU (A) LOAD 
 {0xB7, "LAX"            ,illop_las ,ZEROPAGE_Y         ,F_N|F_Z                    ,2      ,4     },// ,"zpg,Y      ",M->A->X,op_lax_illegal
 {0xB8, "CLV"            ,op_SINGLE ,IMPLICIT           ,F_V                        ,1      ,2     },// ,          ,0 -> V,SR flag
 {0xB9, "LDA"            ,op_ALU_01 ,ABSOLUTE_Y         ,F_N|F_Z                    ,3      ,-4    },// ,"abs,Y      ",M -> A,ALU (A) LOAD 
 {0xBA, "TSX"            ,op_SINGLE ,IMPLICIT           ,F_N|F_Z                    ,1      ,2     },// ,          ,SP -> X,MOVE
 {0xBB, "LAS"            ,illop_las ,ABSOLUTE_Y         ,F_N|F_Z                    ,3      ,-4    },// ,"abs,Y    ","M & SP ->A, X, SP",ALU(L)
 {0xBC, "LDY"            ,op_BIT_00 ,ABSOLUTE_X         ,F_N|F_Z                    ,3      ,-4    },// ,"abs,X      ",M -> Y,ALU (A) LOAD 
 {0xBD, "LDA"            ,op_ALU_01 ,ABSOLUTE_X         ,F_N|F_Z                    ,3      ,-4    },// ,"abs,X      ",M -> A,ALU (A) LOAD 
 {0xBE, "LDX"            ,op_ASL_10 ,ABSOLUTE_Y         ,F_N|F_Z                    ,3      ,-4    },// ,"abs,Y      ",M -> X,ALU (A) LOAD 
 {0xBF, "LAX"            ,illop_las ,ABSOLUTE_Y         ,F_N|F_Z                    ,3      ,-4    },// ,"abs,Y      ",M->A->X,op_lax_illegal
 {0xC0, "CPY"            ,op_BIT_00 ,IMMEDIATE          ,F_N|F_Z|F_C                ,2      ,2     },// ,#          ,Y - M,ALU(L) COMPARE
 {0xC1, "CMP"            ,op_ALU_01 ,INDIRECT_X         ,F_N|F_Z|F_C                ,2      ,6     },// ,"(ind,X)    ",A - M,ALU(L) COMPARE
 {0xC2, "NOP"            ,illop_nop ,IMMEDIATE          ,0                          ,2      ,2     },// ,#,,
 {0xC3, "DCP"            ,illop_dcp ,INDIRECT_X         ,F_N|F_Z|F_C                ,2      ,8     },// ,"(ind,X)    ","M-1->M, A-M",CMP
 {0xC4, "CPY"            ,op_BIT_00 ,ZEROPAGE           ,F_N|F_Z|F_C                ,2      ,3     },// ,zpg        ,Y - M,ALU(L) COMPARE
 {0xC5, "CMP"            ,op_ALU_01 ,ZEROPAGE           ,F_N|F_Z|F_C                ,2      ,3     },// ,zpg        ,A - M,ALU(L) COMPARE
 {0xC6, "DEC"            ,op_ASL_10 ,ZEROPAGE           ,F_N|F_Z                    ,2      ,5     },// ,zpg        ,M - 1 -> M,ALU(A) DEC
 {0xC7, "DCP"            ,illop_dcp ,ZEROPAGE           ,F_N|F_Z|F_C                ,2      ,5     },// ,zpg        ,"M-1->M, A-M",CMP
 {0xC8, "INY"            ,op_SINGLE ,IMPLICIT           ,F_N|F_Z                    ,1      ,2     },// ,          ,Y + 1 -> Y,ALU (A) INC
 {0xC9, "CMP"            ,op_ALU_01 ,IMMEDIATE          ,F_N|F_Z|F_C                ,2      ,2     },// ,#          ,A - M,ALU(L) COMPARE
 {0xCA, "DEX"            ,op_SINGLE ,IMPLICIT           ,F_N|F_Z                    ,1      ,2     },// ,          ,X - 1 -> X,ALU(A) DEC
 {0xCB, "SBX"            ,illop_sbx ,IMMEDIATE          ,F_N|F_Z|F_C                ,2      ,2     },// ,#,(A AND X) - oper -> X,ALU (L)
 {0xCC, "CPY"            ,op_BIT_00 ,ABSOLUTE           ,F_N|F_Z|F_C                ,3      ,4     },// ,abs        ,Y - M,ALU(L) COMPARE
 {0xCD, "CMP"            ,op_ALU_01 ,ABSOLUTE           ,F_N|F_Z|F_C                ,3      ,4     },// ,abs        ,A - M,ALU(L) COMPARE
 {0xCE, "DEC"            ,op_ASL_10 ,ABSOLUTE           ,F_N|F_Z                    ,3      ,6     },// ,abs        ,M - 1 -> M,ALU(A) DEC
 {0xCF, "DCP"            ,illop_dcp ,ABSOLUTE           ,F_N|F_Z|F_C                ,3      ,6     },// ,abs      ,"M-1->M, A-M",CMP
 {0xD0, "BNE"            ,op_BPL_00 ,RELATIVE_ADDRESS   ,0                          ,2      ,-2    },// ,rel        ,branch on Z = 1,COND BRANCH
 {0xD1, "CMP"            ,op_ALU_01 ,INDIRECT_Y         ,F_N|F_Z|F_C                ,2      ,-5    },// ,"(ind),Y    ",A - M,ALU(L) COMPARE
 {0xD2, "HLT"            ,illop_hlt ,NO_OPRAND          ,0                          ,1      ,1     },// ,,,
 {0xD3, "DCP"            ,illop_dcp ,INDIRECT_Y         ,F_N|F_Z|F_C                ,2      ,8     },// ,"(ind),Y    ","M-1->M, A-M",CMP
 {0xD4, "NOP"            ,illop_nop ,ZEROPAGE_X         ,0                          ,2      ,4     },// ,"zpg,X      ",,
 {0xD5, "CMP"            ,op_ALU_01 ,ZEROPAGE_X         ,F_N|F_Z|F_C                ,2      ,4     },// ,"zpg,X      ",A - M,ALU(L) COMPARE
 {0xD6, "DEC"            ,op_ASL_10 ,ZEROPAGE_X         ,F_N|F_Z                    ,2      ,6     },// ,"zpg,X      ",M - 1 -> M,ALU(A) DEC
 {0xD7, "DCP"            ,illop_dcp ,ZEROPAGE_X         ,F_N|F_Z|F_C                ,2      ,6     },// ,"zpg,X      ","M-1->M, A-M",CMP
 {0xD8, "CLD"            ,op_SINGLE ,IMPLICIT           ,F_D                        ,1      ,2     },// ,          ,0 -> D,SR flag
 {0xD9, "CMP"            ,op_ALU_01 ,ABSOLUTE_Y         ,F_N|F_Z|F_C                ,3      ,-4    },// ,"abs,Y      ",A - M,ALU(L) COMPARE
 {0xDA, "NOP"            ,illop_nop ,IMPLICIT           ,0                          ,1      ,2     },// ,,,
 {0xDB, "DCP"            ,illop_dcp ,ABSOLUTE_Y         ,F_N|F_Z|F_C                ,3      ,7     },// ,"abs,Y    ","M-1->M, A-M",CMP
 {0xDC, "NOP"            ,illop_nop ,ABSOLUTE_X         ,0                          ,3      ,-4    },// ,"Abs,X",,
 {0xDD, "CMP"            ,op_ALU_01 ,ABSOLUTE_X         ,F_N|F_Z|F_C                ,3      ,-4    },// ,"abs,X      ",A - M,ALU(L) COMPARE
 {0xDE, "DEC"            ,op_ASL_10 ,ABSOLUTE_X         ,F_N|F_Z                    ,3      ,7     },// ,"abs,X      ",M - 1 -> M,ALU(A) DEC
 {0xDF, "DCP"            ,illop_dcp ,ABSOLUTE_X         ,F_N|F_Z|F_C                ,3      ,7     },// ,"abs,X      ","M-1->M, A-M",CMP
 {0xE0, "CPX"            ,op_BIT_00 ,IMMEDIATE          ,F_N|F_Z|F_C                ,2      ,2     },// ,#          ,X - M,ALU(L) COMPARE
 {0xE1, "SBC"            ,op_ALU_01 ,INDIRECT_X         ,F_N|F_V|F_Z|F_C            ,2      ,6     },// ,"(ind,X)    ",A - M - not C -> A,ALU (A) 
 {0xE2, "NOP"            ,illop_nop ,IMMEDIATE          ,0                          ,2      ,2     },// ,#,,
 {0xE3, "ISC"            ,illop_isc ,INDIRECT_X         ,F_N|F_Z|F_C                ,2      ,8     },// ,"(ind,X)    ","M+1->M,A-M-!C->A",ALU (A)
 {0xE4, "CPX"            ,op_BIT_00 ,ZEROPAGE           ,F_N|F_Z|F_C                ,2      ,3     },// ,zpg        ,X - M,ALU(L) COMPARE
 {0xE5, "SBC"            ,op_ALU_01 ,ZEROPAGE           ,F_N|F_V|F_Z|F_C            ,2      ,3     },// ,zpg        ,A - M - not C -> A,ALU (A) 
 {0xE6, "INC"            ,op_ASL_10 ,ZEROPAGE           ,F_N|F_Z                    ,2      ,5     },// ,zpg        ,M + 1 -> M,ALU (A) INC
 {0xE7, "ISC"            ,illop_isc ,ZEROPAGE           ,F_N|F_Z|F_C                ,2      ,5     },// ,zpg        ,"M+1->M,A-M-!C->A",ALU (A)
 {0xE8, "INX"            ,op_SINGLE ,IMPLICIT           ,F_N|F_Z                    ,1      ,2     },// ,          ,X + 1 -> X,ALU (A) INC
 {0xE9, "SBC"            ,op_ALU_01 ,IMMEDIATE          ,F_N|F_V|F_Z|F_C            ,2      ,2     },// ,#          ,A - M - not C -> A,ALU (A) 
 {0xEA, "NOP"            ,op_SINGLE ,NO_OPRAND          ,0                          ,1      ,2     },// ,Nop,---,Nop
 {0xEB, "USBC"           ,op_ALU_01 ,IMMEDIATE          ,F_N|F_V|F_Z|F_C            ,2      ,2     },// ,#          ,A - M - ! C -> A,ALU (A) 
 {0xEC, "CPX"            ,op_BIT_00 ,ABSOLUTE           ,F_N|F_Z|F_C                ,3      ,4     },// ,abs        ,X - M,ALU(L) COMPARE
 {0xED, "SBC"            ,op_ALU_01 ,ABSOLUTE           ,F_N|F_V|F_Z|F_C            ,3      ,4     },// ,abs        ,A - M - not C -> A,ALU (A) 
 {0xEE, "INC"            ,op_ASL_10 ,ABSOLUTE           ,F_N|F_Z                    ,3      ,6     },// ,abs        ,M + 1 -> M,ALU (A) INC
 {0xEF, "ISC"            ,illop_isc ,ABSOLUTE           ,F_N|F_Z|F_C                ,3      ,6     },// ,abs      ,"M+1->M,A-M-!C->A",ALU (A)
 {0xF0, "BEQ"            ,op_BPL_00 ,RELATIVE_ADDRESS   ,0                          ,2      ,-2    },// ,rel        ,branch on Z = 1,COND BRANCH
 {0xF1, "SBC"            ,op_ALU_01 ,INDIRECT_Y         ,F_N|F_V|F_Z|F_C            ,2      ,-5    },// ,"(ind),Y    ",A - M - not C -> A,ALU (A) 
 {0xF2, "HLT"            ,illop_hlt ,NO_OPRAND          ,0                          ,1      ,1     },// ,,,
 {0xF3, "ISC"            ,illop_isc ,INDIRECT_Y         ,F_N|F_Z|F_C                ,2      ,8     },// ,"(ind),Y    ","M+1->M,A-M-!C->A",ALU (A)
 {0xF4, "NOP"            ,illop_nop ,ZEROPAGE_X         ,0                          ,2      ,4     },// ,"zpg,X      ",,
 {0xF5, "SBC"            ,op_ALU_01 ,ZEROPAGE_X         ,F_N|F_V|F_Z|F_C            ,2      ,4     },// ,"zpg,X      ",A - M - not C -> A,ALU (A) 
 {0xF6, "INC"            ,op_ASL_10 ,ZEROPAGE_X         ,F_N|F_Z                    ,2      ,6     },// ,"zpg,X      ",M + 1 -> M,ALU (A) INC
 {0xF7, "ISC"            ,illop_isc ,ZEROPAGE_X         ,F_N|F_Z|F_C                ,2      ,6     },// ,"zpg,X      ","M+1->M,A-M-!C->A",ALU (A)
 {0xF8, "SED"            ,op_SINGLE ,IMPLICIT           ,F_D                        ,1      ,2     },// ,          ,1 -> D,SR flag
 {0xF9, "SBC"            ,op_ALU_01 ,ABSOLUTE_Y         ,F_N|F_V|F_Z|F_C            ,3      ,-4    },// ,"abs,Y      ",A - M - not C -> A,ALU (A) 
 {0xFA, "NOP"            ,illop_nop ,IMPLICIT           ,0                          ,1      ,2     },// ,,,
 {0xFB, "ISC"            ,illop_isc ,ABSOLUTE_Y         ,F_N|F_Z|F_C                ,3      ,7     },// ,"abs,Y    ","M+1->M,A-M-!C->A",ALU (A)
 {0xFC, "NOP"            ,illop_nop ,ABSOLUTE_X         ,0                          ,3      ,-4    },// ,"Abs,X",,
 {0xFD, "SBC"            ,op_ALU_01 ,ABSOLUTE_X         ,F_N|F_V|F_Z|F_C            ,3      ,-4    },// ,"abs,X      ",A - M - not C -> A,ALU (A) 
 {0xFE, "INC"            ,op_ASL_10 ,ABSOLUTE_X         ,F_N|F_Z                    ,3      ,7     },// ,"abs,X      ",M + 1 -> M,ALU (A) INC
 {0xFF, "ISC"            ,illop_isc ,ABSOLUTE_X         ,F_N|F_Z|F_C                ,3      ,7     }// ,"abs,X      ","M+1->M,A-M-!C->A",ALU (A)
};



