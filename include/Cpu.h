#ifndef CPU_H
#define CPU_H
#include <stdint.h>

#include "include/Ppu.h"
#include "include/Memory.h"


//N = negative flag (1 when result is negative)
//V = overflow flag (1 on signed overflow)
//# = unused (always 1)
//B = break flag (1 when interupt was caused by a BRK) -- DOES NOT EXIST
//D = decimal flag (1 when CPU in BCD mode)
//I = IRQ flag (when 1, no interupts will occur (exceptions are IRQs forced by BRK and NMIs))
//Z = zero flag (1 when all bits of a result are 0)
//C = carry flag (1 on unsigned overflow)

#define NEG_F	0x80
#define OVER_F	0x40
#define B5_F	0x20
#define BRK_F	0x10
#define DEC_F	0x08
#define IRQ_F	0x04
#define ZERO_F	0x02
#define CARRY_F 0x01

#define RST_LSB 0xFFFC
#define RST_MSB 0xFFFD

#define NVI_LSB 0xFFFA
#define NVI_MSB 0xFFFB


typedef struct{
	uint16_t pc;
	uint8_t sp;
	uint8_t a, x, y;
	uint8_t p;
}reg_t;

typedef struct{
	uint16_t add;
	uint16_t value;
	uint8_t opcode;
}args_t;

class Cpu
{
public:
	Cpu();
	~Cpu();
	void reset();
	void tick();
	int loadCartridge(char * filename);
	void doCycle(uint n = 1);
	void mW(uint16_t a, uint8_t v);
	uint8_t mR(uint16_t a);
	void powerUp();
private:
	bool checkForInt();
	void printCurrentState(uint8_t opcode);
	args_t getAddressBaseOnAddressingMode(uint8_t optcode);


	void push(uint8_t v);
	uint8_t pull();
	void setZero(uint8_t v);
	void setNeg(uint8_t v);
	void setCarry(bool condition);
	void branch(const bool condition, const uint16_t add);

	Memory *memory;
	Ppu *ppu;
	reg_t r;

	// for debugging
	uint8_t inst[3];
	uint8_t value[6];

	int oldCycleCount;
	int oldScanLineLCount;
	int newCycle;

	typedef void (Cpu::*opcodeFunction)(args_t args);
	//8/04 -> 17 todo
	//9/04 -> 8 illegal todo
	void ADC(args_t args);
	void AND(args_t args);
	void ASL(args_t args);

	void BCC(args_t args); void BCS(args_t args); void BEQ(args_t args); void BMI(args_t args); void BNE(args_t args); void BPL(args_t args); void BVC(args_t args); void BVS(args_t args);
	void BIT(args_t args);
	void BRK(args_t args);


	void CLC(args_t args); void CLD(args_t args); void CLI(args_t args); void CLV(args_t args);
	void CMP(args_t args); void CPX(args_t args); void CPY(args_t args);

	void DEC(args_t args); void DEX(args_t args); void DEY(args_t args);

	void EOR(args_t args);

	void INC(args_t args); void INX(args_t args); void INY(args_t args);

	void JMP(args_t args);
	void JSR(args_t args);

	void LDX(args_t args); void LDY(args_t args); void LDA(args_t args);
	void LSR(args_t args);

	void NOP(args_t args);

	void ORA(args_t args);

	void PHA(args_t args);
	void PHP(args_t args);
	void PLA(args_t args);
	void PLP(args_t args);

	void ROL(args_t args);
	void ROR(args_t args);
	void RTI(args_t args);
	void RTS(args_t args);

	void SBC(args_t args);
	void SEC(args_t args); void SED(args_t args); void SEI(args_t args);
	void STX(args_t args); void STY(args_t args); void STA(args_t args);

	void TAX(args_t args);
	void TAY(args_t args);
	void TSX(args_t args);
	void TXA(args_t args);
	void TXS(args_t args);
	void TYA(args_t args);

	/* invalid opcode */
	void sNOP(args_t args);
	void LAX(args_t args);
	void SAX(args_t args);
	void AHX(args_t args);
	void DCP(args_t args);
	void sSBC(args_t args);
	void ISB(args_t args); // also know as ISC
	void SLO(args_t args);
	void RLA(args_t args);
	void SRE(args_t args);
	void RRA(args_t args);
	void ANC(args_t args);
	void ALR(args_t args);
	void ARR(args_t args);
	void XAA(args_t args);
	void AXS(args_t args);
	void SHX(args_t args);
	void SHY(args_t args);
	void LAS(args_t args);
	void TAS(args_t args);
	void KIL(args_t args);
	opcodeFunction opcodeFunc[256] =
	{// X0         X1         X2         X3         X4         X5         X6         X7         X8         X9         Xa         Xb         Xc         Xd         Xe         Xf
/* 0X*/ &Cpu::BRK, &Cpu::ORA, &Cpu::KIL, &Cpu::SLO,&Cpu::sNOP, &Cpu::ORA, &Cpu::ASL, &Cpu::SLO, &Cpu::PHP, &Cpu::ORA, &Cpu::ASL, &Cpu::ANC, &Cpu::NOP, &Cpu::ORA, &Cpu::ASL, &Cpu::SLO,
/* 1X*/ &Cpu::BPL, &Cpu::ORA, &Cpu::KIL, &Cpu::SLO,&Cpu::sNOP, &Cpu::ORA, &Cpu::ASL, &Cpu::SLO, &Cpu::CLC, &Cpu::ORA, &Cpu::NOP, &Cpu::SLO,&Cpu::sNOP, &Cpu::ORA, &Cpu::ASL, &Cpu::SLO,
/* 2X*/ &Cpu::JSR, &Cpu::AND, &Cpu::KIL, &Cpu::RLA, &Cpu::BIT, &Cpu::AND, &Cpu::ROL, &Cpu::RLA, &Cpu::PLP, &Cpu::AND, &Cpu::ROL, &Cpu::ANC, &Cpu::BIT, &Cpu::AND, &Cpu::ROL, &Cpu::RLA,
/* 3X*/ &Cpu::BMI, &Cpu::AND, &Cpu::KIL, &Cpu::RLA,&Cpu::sNOP, &Cpu::AND, &Cpu::ROL, &Cpu::RLA, &Cpu::SEC, &Cpu::AND, &Cpu::NOP, &Cpu::RLA,&Cpu::sNOP, &Cpu::AND, &Cpu::ROL, &Cpu::RLA,
/* 4X*/ &Cpu::RTI, &Cpu::EOR, &Cpu::KIL, &Cpu::SRE,&Cpu::sNOP, &Cpu::EOR, &Cpu::LSR, &Cpu::SRE, &Cpu::PHA, &Cpu::EOR, &Cpu::LSR, &Cpu::ALR, &Cpu::JMP, &Cpu::EOR, &Cpu::LSR, &Cpu::SRE,
/* 5X*/ &Cpu::BVC, &Cpu::EOR, &Cpu::KIL, &Cpu::SRE,&Cpu::sNOP, &Cpu::EOR, &Cpu::LSR, &Cpu::SRE, &Cpu::CLI, &Cpu::EOR, &Cpu::NOP, &Cpu::SRE,&Cpu::sNOP, &Cpu::EOR, &Cpu::LSR, &Cpu::SRE,
/* 6X*/ &Cpu::RTS, &Cpu::ADC, &Cpu::KIL, &Cpu::RRA,&Cpu::sNOP, &Cpu::ADC, &Cpu::ROR, &Cpu::RRA, &Cpu::PLA, &Cpu::ADC, &Cpu::ROR, &Cpu::ARR, &Cpu::JMP, &Cpu::ADC, &Cpu::ROR, &Cpu::RRA,
/* 7X*/ &Cpu::BVS, &Cpu::ADC, &Cpu::KIL, &Cpu::RRA,&Cpu::sNOP, &Cpu::ADC, &Cpu::ROR, &Cpu::RRA, &Cpu::SEI, &Cpu::ADC, &Cpu::NOP, &Cpu::RRA,&Cpu::sNOP, &Cpu::ADC, &Cpu::ROR, &Cpu::RRA,
/* 8X*/&Cpu::sNOP, &Cpu::STA, &Cpu::KIL, &Cpu::SAX, &Cpu::STY, &Cpu::STA, &Cpu::STX, &Cpu::SAX, &Cpu::DEY, &Cpu::XAA, &Cpu::TXA, &Cpu::XAA, &Cpu::STY, &Cpu::STA, &Cpu::STX, &Cpu::SAX,
/* 9X*/ &Cpu::BCC, &Cpu::STA, &Cpu::KIL, &Cpu::AHX, &Cpu::STY, &Cpu::STA, &Cpu::STX, &Cpu::SAX, &Cpu::TYA, &Cpu::STA, &Cpu::TXS, &Cpu::TAS, &Cpu::SHY, &Cpu::STA, &Cpu::SHX, &Cpu::AHX,
/* AX*/ &Cpu::LDY, &Cpu::LDA, &Cpu::LDX, &Cpu::LAX, &Cpu::LDY, &Cpu::LDA, &Cpu::LDX, &Cpu::LAX, &Cpu::TAY, &Cpu::LDA, &Cpu::TAX, &Cpu::LAX, &Cpu::LDY, &Cpu::LDA, &Cpu::LDX, &Cpu::LAX,
/* BX*/ &Cpu::BCS, &Cpu::LDA, &Cpu::KIL, &Cpu::LAX, &Cpu::LDY, &Cpu::LDA, &Cpu::LDX, &Cpu::LAX, &Cpu::CLV, &Cpu::LDA, &Cpu::TSX, &Cpu::LAS, &Cpu::LDY, &Cpu::LDA, &Cpu::LDX, &Cpu::LAX,
/* CX*/ &Cpu::CPY, &Cpu::CMP, &Cpu::KIL, &Cpu::DCP, &Cpu::CPY, &Cpu::CMP, &Cpu::DEC, &Cpu::DCP, &Cpu::INY, &Cpu::CMP, &Cpu::DEX, &Cpu::AXS, &Cpu::CPY, &Cpu::CMP, &Cpu::DEC, &Cpu::DCP,
/* DX*/ &Cpu::BNE, &Cpu::CMP, &Cpu::KIL, &Cpu::DCP,&Cpu::sNOP, &Cpu::CMP, &Cpu::DEC, &Cpu::DCP, &Cpu::CLD, &Cpu::CMP, &Cpu::NOP, &Cpu::DCP,&Cpu::sNOP, &Cpu::CMP, &Cpu::DEC, &Cpu::DCP,
/* EX*/ &Cpu::CPX, &Cpu::SBC, &Cpu::KIL, &Cpu::ISB, &Cpu::CPX, &Cpu::SBC, &Cpu::INC, &Cpu::ISB, &Cpu::INX, &Cpu::SBC, &Cpu::NOP,&Cpu::sSBC, &Cpu::CPX, &Cpu::SBC, &Cpu::INC, &Cpu::ISB,
/* FX*/ &Cpu::BEQ, &Cpu::SBC, &Cpu::KIL, &Cpu::ISB,&Cpu::sNOP, &Cpu::SBC, &Cpu::INC, &Cpu::ISB, &Cpu::SED, &Cpu::SBC, &Cpu::NOP, &Cpu::ISB,&Cpu::sNOP, &Cpu::SBC, &Cpu::INC, &Cpu::ISB
	};


	uint8_t addModeLenInstruction[12] = { 1, 2, 2, 2, 2, 2, 2 ,3, 3, 3, 3, 2};

	char* addModePrintStr[12] = {
		"%.2X       %s %s                         ",			//	0:	Address imply
		"%.2X %.2X    %s #$%.2X                      ",			//	1:	Address Immediate			| #$??    | aa			 imm
		"%.2X %.2X    %s $%.2X = %.2X                  ",		//	2:	Address Zero page			| $??     | m[aa]		 zp
		"%.2X %.2X    %s $%.2X,X @ %.2X = %.2X           ",		//	3:	Address Zero page X		   *| $??,X   | m[aa + X]	 zpx
		"%.2X %.2X    %s $%.2X,Y @ %.2X = %.2X           ",		//	4:	Address Zero page Y		   *| $??,Y   | m[aa + Y]	 zpy
		"%.2X %.2X    %s ($%.2X,X) @ %.2X = %.2X%.2X = %.2X  ",	//	5:	Address Indexed indirect X	| ($??,X) | m[m[aa + X]] izx
		"%.2X %.2X    %s ($%.2X),Y = %.2X%.2X @ %.4X = %.2X",	//	6:	Address Indexed indirect Y  | ($??),Y | m[m[aa] + Y] izy
		"%.2X %.2X %.2X %s $%.2X%.2X %s                ",		//	7:	Address absolute			| $????   | m[aaaa]		 abs
		"%.2X %.2X %.2X %s $%.2X%.2X,X @ %.4X = %.2X       ",	//	8:	Address Absolute, X		   *| $????,X | m[aaaa + X]	 abx
		"%.2X %.2X %.2X %s $%.2X%.2X,Y @ %.4X = %.2X       ",	//	9:	Address Absolute, Y		   *| $????,Y | m[aaaa + Y]	 aby
		"%.2X %.2X %.2X %s ($%.2X%.2X) = %.2X%.2X            ",	// 10:	Address Absolute indirect	| ($????) | m[m[aaaa]]	 ind
		"%.2X %.2X    %s $%.4X                     "			// 11:	Address Relative		   *| $??     | m[pc + aa]	 rel
	};

	uint8_t opcodeAddMode[256] =
	{// X0 X1  X2  X3  X4  X5  X6  X7  X8  X9  XA  XB  XC  XD  XE  XF
/* 0X*/ 0,  5,  0,  5,  2,  2,  2,  2,  0,  1,  0,  1,  7,  7,  7,  7,
/* 1X*/11,  6,  0,  6,  3,  3,  3,  3,  0,  9,  0,  9,  8,  8,  8,  8,
/* 2X*/ 7,  5,  0,  5,  2,  2,  2,  2,  0,  1,  0,  1,  7,  7,  7,  7,
/* 3X*/11,  6,  0,  6,  3,  3,  3,  3,  0,  9,  0,  9,  8,  8,  8,  8,
/* 4X*/ 0,  5,  0,  5,  2,  2,  2,  2,  0,  1,  0,  1,  7,  7,  7,  7,
/* 5X*/11,  6,  0,  6,  3,  3,  3,  3,  0,  9,  0,  9,  8,  8,  8,  8,
/* 6X*/ 0,  5,  0,  5,  2,  2,  2,  2,  0,  1,  0,  1, 10,  7,  7,  7,
/* 7X*/11,  6,  0,  6,  3,  3,  3,  3,  0,  9,  0,  9,  8,  8,  8,  8,
/* 8X*/ 1,  5,  1,  5,  2,  2,  2,  2,  0,  0,  0,  1,  7,  7,  7,  7,
/* 9X*/11,  6,  0,  6,  3,  3,  4,  4,  0,  9,  0,  9,  8,  8,  9,  9,
/* AX*/ 1,  5,  1,  5,  2,  2,  2,  2,  0,  1,  0,  1,  7,  7,  7,  7,
/* BX*/11,  6,  0,  6,  3,  3,  4,  4,  0,  9,  0,  9,  8,  8,  9,  9,
/* CX*/ 1,  5,  1,  5,  2,  2,  2,  2,  0,  1,  0,  1,  7,  7,  7,  7,
/* DX*/11,  6,  0,  6,  3,  3,  3,  3,  0,  9,  0,  9,  8,  8,  8,  8,
/* EX*/ 1,  5,  1,  5,  2,  2,  2,  2,  0,  1,  0,  1,  7,  7,  7,  7,
/* FX*/11,  6,  0,  6,  3,  3,  3,  3,  0,  9,  0,  9,  8,  8,  8,  8
	};

	uint8_t opcodeCycle[256] =
	{// X0 X1  X2  X3  X4  X5  X6  X7  X8  X9  XA  XB  XC  XD  XE  XF
/* 0X*/ 2,  6,  0,  8,  3,  3,  5,  5,  3,  2,  2,  2,  4,  4,  6,  6,
/* 1X*/ 2,  5,  0,  8,  4,  4,  6,  6,  2,  4,  2,  7,  4,  4,  7,  7,
/* 2X*/ 6,  6,  0,  8,  3,  3,  5,  5,  4,  2,  2,  2,  4,  4,  6,  6,
/* 3X*/ 2,  5,  0,  8,  4,  4,  6,  6,  2,  4,  2,  7,  4,  4,  7,  7,
/* 4X*/ 6,  6,  0,  8,  3,  3,  5,  5,  3,  2,  2,  2,  3,  4,  6,  6,
/* 5X*/ 2,  5,  0,  8,  4,  4,  6,  6,  2,  4,  2,  7,  4,  4,  7,  7,
/* 6X*/ 6,  6,  0,  8,  3,  3,  5,  5,  4,  2,  2,  2,  5,  4,  6,  6,
/* 7X*/ 2,  5,  0,  8,  4,  4,  6,  6,  2,  4,  2,  7,  4,  4,  7,  7,
/* 8X*/ 2,  6,  2,  6,  3,  3,  3,  3,  2,  2,  2,  2,  4,  4,  4,  4,
/* 9X*/ 2,  6,  0,  6,  4,  4,  4,  4,  2,  5,  2,  5,  5,  5,  5,  5,
/* AX*/ 2,  6,  2,  6,  3,  3,  3,  3,  2,  2,  2,  2,  4,  4,  4,  4,
/* BX*/ 2,  5,  0,  5,  4,  4,  4,  4,  2,  4,  2,  4,  4,  4,  4,  4,
/* CX*/ 2,  6,  2,  8,  3,  3,  5,  5,  2,  2,  2,  2,  4,  4,  6,  6,
/* DX*/ 2,  5,  0,  8,  4,  4,  6,  6,  2,  4,  2,  7,  4,  4,  7,  7,
/* EX*/ 2,  6,  2,  8,  3,  3,  5,  5,  2,  2,  2,  2,  4,  4,  6,  6,
/* FX*/ 2,  5,  0,  8,  4,  4,  6,  6,  2,  4,  2,  7,  4,  4,  7,  7
	};

	char* opcodeStr[256] =
	{//		 X0      X1      X2      X3      X4      X5      X6      X7      X8      X9      Xa      Xb      Xc      Xd      Xe      Xf
	/* 0X*/ " BRK", " ORA", "*KIL", "*SLO", "*NOP", " ORA", " ASL", "*SLO", " PHP", " ORA", " ASL", "*ANC", "*NOP", " ORA", " ASL", "*SLO",
	/* 1X*/ " BPL", " ORA", "*KIL", "*SLO", "*NOP", " ORA", " ASL", "*SLO", " CLC", " ORA", "*NOP", "*SLO", "*NOP", " ORA", " ASL", "*SLO",
	/* 2X*/ " JSR", " AND", "*KIL", "*RLA", " BIT", " AND", " ROL", "*RLA", " PLP", " AND", " ROL", "*ANC", " BIT", " AND", " ROL", "*RLA",
	/* 3X*/ " BMI", " AND", "*KIL", "*RLA", "*NOP", " AND", " ROL", "*RLA", " SEC", " AND", "*NOP", "*RLA", "*NOP", " AND", " ROL", "*RLA",
	/* 4X*/ " RTI", " EOR", "*KIL", "*SRE", "*NOP", " EOR", " LSR", "*SRE", " PHA", " EOR", " LSR", "*ALR" ," JMP", " EOR", " LSR", "*SRE",
	/* 5X*/ " BVC", " EOR", "*KIL", "*SRE", "*NOP", " EOR", " LSR", "*SRE", " CLI", " EOR", "*NOP", "*SRE", "*NOP", " EOR", " LSR", "*SRE",
	/* 6X*/ " RTS", " ADC", "*KIL", "*RRA", "*NOP", " ADC", " ROR", "*RRA", " PLA", " ADC", " ROR", "*ARR", " JMP", " ADC", " ROR", "*RRA",
	/* 7X*/ " BVS", " ADC", "*KIL", "*RRA", "*NOP", " ADC", " ROR", "*RRA", " SEI", " ADC", "*NOP", "*RRA", "*NOP", " ADC", " ROR", "*RRA",
	/* 8X*/ "*NOP", " STA", "*KIL", "*SAX", " STY", " STA", " STX", "*SAX", " DEY", "*NOP", " TXA", "*XAA", " STY", " STA", " STX",  "*SAX",
	/* 9X*/ " BCC", " STA", "*KIL", "*AHS", " STY", " STA", " STX", "*SAX", " TYA", " STA", " TXS", "*TAS", "*NOP", " STA", "*SHX", "*AHX",
	/* AX*/ " LDY", " LDA", " LDX", "*LAX", " LDY", " LDA", " LDX", "*LAX", " TAY", " LDA", " TAX", "*LAX", " LDY", " LDA", " LDX", "*LAX",
	/* BX*/ " BCS", " LDA", "*KIL", "*LAX", " LDY", " LDA", " LDX", "*LAX", " CLV", " LDA", " TSX", "*LAS", " LDY", " LDA", " LDX", "*LAX",
	/* CX*/ " CPY", " CMP", "*KIL", "*DCP", " CPY", " CMP", " DEC", "*DCP", " INY", " CMP", " DEX", "*AXS", " CPY", " CMP", " DEC", "*DCP",
	/* DX*/ " BNE", " CMP", "*KIL", "*DCP", "*NOP", " CMP", " DEC", "*DCP", " CLD", " CMP", "*NOP", "*DCP", "*NOP", " CMP", " DEC", "*DCP",
	/* EX*/ " CPX", " SBC", "*KIL", "*ISB", " CPX", " SBC", " INC", "*ISB", " INX", " SBC", " NOP", "*SBC", " CPX", " SBC", " INC", "*ISB",
	/* FX*/ " BEQ", " SBC", "*KIL", "*ISB", "*NOP", " SBC", " INC", "*ISB", " SED", " SBC", "*NOP", "*ISB", "*NOP", " SBC", " INC", "*ISB"
	};

};

#endif // CPU_H
