#include "include/Cpu.h"




Cpu::Cpu(){
	ppu = new Ppu;
	memory = new Memory;
	ppu->setMemory(memory);
	memory->setPPU(ppu);

	//this->powerUpState();
	//this->reset();

	//debug
	value[0] = 0;
	value[1] = 0;
	value[2] = 0;
	value[3] = 0;
}

Cpu::~Cpu(){
	delete ppu;
	delete memory;
}

void Cpu::powerUp(){
	// All internal memory ($0000-$07FF) was consistently set to $ff
	for(uint16_t i = 0; i <= 0xFF; i++)
		memory->write(i, 0xFF);
	memory->write(0x0008, 0xF7);
	memory->write(0x0009, 0xE7);
	memory->write(0x000A, 0xD7);
	memory->write(0x000F, 0xB7);
	// APU register
//	  $4017 = $00 (frame irq enabled)
//    $4015 = $00 (all channels disabled)
//    $4000-$400F = $00 (not sure about $4010-$4013)
	memory->write(0x4017, 0x00);
	memory->write(0x4015, 0x00);
	for(uint16_t i = 0; i <= 0xF; i++)
		memory->write(0x4000 + i, 0x00);
	this->reset();
}

void Cpu::reset(){
	r.pc = memory->read(RST_MSB) * 256 + memory->read(RST_LSB);//0xA200;//0xC000;
	r.sp = 0xFD;
	r.a =  0x0;
	r.x =  0x0;
	r.y =  0x0;
	r.p =  B5_F | BRK_F | DEC_F;// For nestest :B5_F | IRQ_F;//

	ppu->reset();
}

int Cpu::loadCartridge(char * filename){
	return this->memory->loadCartridge(filename);
}

void Cpu::doCycle(uint n){
	for (uint i = 0; i < n * 3; ++i) {
		this->ppu->tick();
	}
}

void Cpu::mW(uint16_t a, uint8_t v){
	memory->write(a, v);
	doCycle();
}

uint8_t Cpu::mR(uint16_t a){
	uint8_t v = memory->read(a);
	doCycle();
	return v;
}

bool Cpu::checkForInt(){
	if(ppu->intNMI()){
		push((r.pc & 0xFF00) >> 8);
		push( r.pc & 0x00FF);
		push( r.p); // Push status register
		r.pc = mR(NVI_MSB) * 256 + mR(NVI_LSB); // fetch interrupt vector

		r.p &= ~IRQ_F; // Set interruption flag to 0

		doCycle(7);
		return true;
	}
	else if(ppu->wasCallDMA()){
		doCycle(ppu->doDMA());
		return true;
	}

	return false;
}

void Cpu::tick(){

	//Save number of cycle for debugging
	oldCycleCount = this->ppu->getCycle();
	oldScanLineLCount = this->ppu->getScreenline();

	// If we do not have blocking interrupt or DMA
	if(!checkForInt()){

		uint8_t opcode = mR(r.pc);
		if(r.pc == 0xc72f){
			inst[0] = 0;
		}

		if(opcodeStr[opcode] == 0){
			printf("Invalid Opcode: %.2X\n", opcode);
			exit(0);
		}

		args_t args = getAddressBaseOnAddressingMode(opcode);

		// Print the current instruction in formated text
		// don't use it for something else than a cpu test ROM
		//this->printCurrentState(opcode);

		// Increment PC, could be replace
		r.pc += addModeLenInstruction[opcodeAddMode[opcode]];

		// Execute Opcode
		(this->*opcodeFunc[opcode])(args);
	}
}

args_t Cpu::getAddressBaseOnAddressingMode(uint8_t opcode){
	args_t args;
	args.opcode = opcode;
	args.add = 0;
	args.value = 0;

	uint16_t a, lsb, msb, zpa;
	uint8_t addMode = opcodeAddMode[opcode];
	switch(addMode){
		case 0://	0:	Address imply
			args.add = 0;
			args.value = 0;
			break;
		case 1:// Address Immediate				| #$??    | aa			     imm
			args.value = mR(r.pc + 1); // First argument of the instruction
			break;
		case 2:// Address Zero page				| $??     | m[aa]		     zp
			args.add = mR(r.pc + 1);
			break;
		case 3:// Address Zero page X		   *| $??,X   | m[aa + X]	 zpx
			args.add = (mR(r.pc + 1) + r.x) & 0xFF;
			doCycle();
			break;
		case 4:// Address Zero page Y		   *| $??,Y   | m[aa + Y]	 zpy
			args.add = (mR(r.pc + 1) + r.y) & 0xFF;
			doCycle();
			break;
		case 5:// Address Indexed indirect X	| ($??,X) | m[m[aa + X]] izx
			a = (mR(r.pc + 1) + r.x) & 0xFF;
			msb = mR(a);
			// TODO Clean
			if((a & 0xFF) == 0xFF)  // Cross boundary
				lsb = mR(a & 0xFF00);
			else
				lsb = mR(a + 1);

			args.add = (lsb << 8) | msb;

			doCycle(); // This mode of addressing cost an extra cycle

			value[1] = a;
			value[2] = lsb;
			value[3] = msb;
			break;
		case 6:// Address Indexed indirect Y  | ($??),Y | m[m[aa] + Y]   izy
			zpa = mR(r.pc + 1);
			msb = mR(zpa);
			lsb = mR((zpa + 1) & 0xFF);
			a = (lsb << 8) + msb; // 0xaaaa = 0300

			args.add = a + r.y; // m[0xaaaa + 0xYY] 0300

			// Cross boundary *DCP($D3) and other illegal opcode doesn't do it
			if((a & 0xFF00) != (args.add & 0xFF00) &&
			   ((opcode & 0x13) != 0x13 || opcode == 0xB3))
				doCycle();
			// STA need an additionnal cycle for some reason...
			if(opcode == 0x91)
				doCycle();
			value[1] = lsb;
			value[2] = msb;
			value[3] = a;
			break;
		case 7:// Address absolute			  | $????   | m[aaaa]		 abs
			args.add = (mR(r.pc + 2) << 8) | mR(r.pc + 1);
			break;
		case 8:// Address Absolute, X		 *| $????,X | m[aaaa + X]	 abx
			a = (mR(r.pc + 2) << 8) | mR(r.pc + 1);
			args.add = a + r.x;
			// Cross boundary for other than STA and *DCP
			if((a & 0xFF00) != (args.add & 0xFF00)
			   && opcode != 0x99 && opcode != 0x9D && (opcode & 0x1F) != 0x1F)
				doCycle();
			// STA, DEC, INC, ROL, ROR, ASL and LSR need an additionnal cycle
			if(opcode == 0x9D || (opcode & 0x1F) == 0x1E)
				doCycle();
			value[1] = args.add;
			break;
		case 9:// Address Absolute, Y		   *| $????,Y | m[aaaa + Y]	 aby
			a = (mR(r.pc + 2) << 8) | mR(r.pc + 1);
			args.add = a + r.y;
			// Cross boundary for other than STA and *DCP and other illegal opcode doesn't do it
			if((a & 0xFF00) != (args.add & 0xFF00) && (opcode & 0x1B) != 0x1B)
				doCycle();

			// STA need an additionnal cycle for some reason...
			if(opcode == 0x99)
				doCycle();
			value[1] = args.add;
			break;
		case 10:// Address Absolute indirect	| ($????) | m[m[aaaa]]	 ind
			a = (mR(r.pc + 2) << 8) | mR(r.pc + 1);
			if((a & 0xFF) == 0xFF)  // Cross boundary
				lsb = mR(a & 0xFF00);
			else
				lsb = mR(a + 1);
			msb = mR(a);
			args.add = (lsb << 8) | msb;
			value[1] = lsb;
			value[2] = msb;
			break;
		case 11:// Address Relative		   *| $??     | m[pc + aa]	 rel
			args.add = (int8_t)(mR(r.pc + 1)) + r.pc + 2;
			break;
	}

	// If the addressing mode is other than  immediate and imply
	// we load the value
	if(addMode > 1 && addMode < 10){
		// In the case of the STA, JMP and JSR, we don't want to do a read that affect the memory
		if((opcode & 0xF0) != 0x80 &&
		   opcode != 0x20 && opcode != 0x91 && opcode != 0x94 &&
		   opcode != 0x95 && opcode != 0x96 && opcode != 0x99 &&
		   opcode != 0x9D && opcode != 0x0C && opcode != 0x4C)
			args.value = mR(args.add);

		value[0] = args.value;
	}
	return args;
}

/*
 * Print the current instruction in a format suitable for the nestest ROM
 * For example:
 * Head of nestest.log
 * C000  4C F5 C5  JMP $C5F5                       A:00 X:00 Y:00 P:24 SP:FD CYC:  0 SL:241
 * C5F5  A2 00     LDX #$00                        A:00 X:00 Y:00 P:24 SP:FD CYC:  9 SL:241
 * C5F7  86 00     STX $00 = 00                    A:00 X:00 Y:00 P:26 SP:FD CYC: 15 SL:241
 * C5F9  86 10     STX $10 = 00                    A:00 X:00 Y:00 P:26 SP:FD CYC: 24 SL:241
 * */
void Cpu::printCurrentState(uint8_t opcode){
	char asmStr[100];
	char eqStr[100];
	uint8_t addMode = opcodeAddMode[opcode];
	inst[1] = this->memory->read(r.pc + 1);
	inst[2] = this->memory->read(r.pc + 2);
	uint16_t a;
	switch(addMode){
		case 0:
			sprintf(asmStr, addModePrintStr[addMode], opcode, opcodeStr[opcode], ((opcode & 0x9F) == 0x0A ? "A" : " "));
			break;
		case 1:
			sprintf(asmStr, addModePrintStr[addMode], opcode, inst[1], opcodeStr[opcode], inst[1]);
			break;
		case 2:// For Zero page Addressing
			sprintf(asmStr, addModePrintStr[addMode], opcode, inst[1], opcodeStr[opcode], inst[1], value[0]);
			break;
		case 3:
			a = (inst[1] + r.x) & 0xFF;
			sprintf(asmStr, addModePrintStr[addMode], opcode, inst[1], opcodeStr[opcode], inst[1], a, value[0]);
			break;
		case 4:
			a = (inst[1] + r.y) & 0xFF;
			sprintf(asmStr, addModePrintStr[addMode], opcode, inst[1], opcodeStr[opcode], inst[1], a, value[0]);
			break;
		case 5:{// For Indirect Addressing
			sprintf(asmStr, addModePrintStr[addMode], opcode, inst[1], opcodeStr[opcode], inst[1],
					value[1], // add ind
					value[2], // lsb add
					value[3], // msb add
					value[0]);// value
			break;
		}
		case 6:{// For Indirect Addressing
			sprintf(asmStr, addModePrintStr[addMode], opcode, inst[1], opcodeStr[opcode], inst[1],
					value[1], // lsb add
					value[2], // msb add
					value[3], // add  + y
					value[0]);
			break;
		}
		case 7:{
			if(opcode == 0x4C ||opcode == 0x47 || opcode == 0x20)
				strcpy(eqStr, "    ");
			else
				sprintf(eqStr, "= %.2X", value[0]);
			sprintf(asmStr, addModePrintStr[addMode], opcode, inst[1], inst[2], opcodeStr[opcode], inst[2], inst[1], eqStr);
			break;
		}case 8:
			sprintf(asmStr, addModePrintStr[addMode], opcode, inst[1], inst[2], opcodeStr[opcode], inst[2], inst[1], value[1], value[0]);
			break;
		case 9:{
			sprintf(asmStr, addModePrintStr[addMode], opcode, inst[1], inst[2], opcodeStr[opcode], inst[2], inst[1], value[1], value[0]);
			break;
		}case 10:{
			sprintf(asmStr, addModePrintStr[addMode], opcode, inst[1], inst[2], opcodeStr[opcode], inst[2], inst[1], value[1], value[2]);
			break;
		}case 11:// For Relative Addressing
			sprintf(asmStr, addModePrintStr[addMode], opcode, inst[1], opcodeStr[opcode], (int8_t)(inst[1]) + r.pc + 2);
			break;
	}
	printf("%.4X  %s  A:%.2X X:%.2X Y:%.2X P:%.2X SP:%.2X CYC:%3d SL:%d\n", r.pc, asmStr, r.a, r.x, r.y, r.p, r.sp, oldCycleCount, oldScanLineLCount);
}


void Cpu::push(uint8_t v){
	mW( 0x100 | (uint16_t)(r.sp), v);
	r.sp--;
}

uint8_t Cpu::pull(){
	r.sp++;
	return mR( 0x100 | (uint16_t)(r.sp));
}


/*
 * Check if number zero and change Z flag in consequence
 */
void Cpu::setZero(uint8_t v){
	if(v == 0)
		r.p |= ZERO_F;
	else
		r.p &= ~ZERO_F;
}
/*
 * Check b7 of the number and change N flag in consequence
 */
void Cpu::setNeg(uint8_t v){
	if(v & 0x80)// Check bit 7
		r.p |= NEG_F;
	else
		r.p &= ~NEG_F;
}

/*
 * If condition true set C flag to 1
 */
void Cpu::setCarry(bool condition){
	if(condition)
		r.p |= CARRY_F;
	else
		r.p &= ~CARRY_F;
}

void Cpu::branch(const bool condition, const uint16_t add){
	if(condition){
		if(((r.pc + 2) & 0xFF00) != (add & 0xFF00))  // Cross boundary
			doCycle(2);
		else
			doCycle(1);
		r.pc = add;
	}
}

/**
  Opcode impletmentation
*/

void Cpu::JMP(args_t args){
	r.pc = args.add;
}

void Cpu::LDX(args_t args){
	r.x = args.value;
	setZero(args.value);
	setNeg(args.value);
}
void Cpu::LDY(args_t args){
	r.y = args.value;
	setZero(args.value);
	setNeg(args.value);
}
void Cpu::LDA(args_t args){
	r.a = args.value;
	setZero(args.value);
	setNeg(args.value);
}

void Cpu::STX(args_t args){ mW(args.add , r.x); }
void Cpu::STY(args_t args){ mW(args.add , r.y); }
void Cpu::STA(args_t args){ mW(args.add , r.a); }


void Cpu::JSR(args_t args){
	r.pc--;//r.pc -= 4; // address - 1
	push((r.pc & 0xFF00) >> 8);//MSB
	push( r.pc & 0x00FF); //lSB
	doCycle();
	r.pc = args.add;
}

void Cpu::RTS(args_t args){
	uint16_t lsb = pull();
	uint16_t msb = pull();
	r.pc = (msb << 8) | lsb;
	r.pc++; // address +1
	doCycle(3);
}

void Cpu::NOP(args_t args){
	doCycle(1);
}

void Cpu::CLC(args_t args){ r.p &= ~CARRY_F;doCycle();}
void Cpu::CLI(args_t args){ r.p &= ~IRQ_F;	doCycle();}
void Cpu::CLV(args_t args){ r.p &= ~OVER_F; doCycle();}
void Cpu::CLD(args_t args){ r.p &= ~DEC_F;	doCycle();}

void Cpu::SEC(args_t args){ r.p |= CARRY_F; doCycle();}
void Cpu::SEI(args_t args){ r.p |= IRQ_F;	doCycle();}
void Cpu::SED(args_t args){ r.p |= DEC_F;	doCycle();}



void Cpu::BIT(args_t args){
	setZero(args.value & r.a); // Z = v AND a == 0
	r.p = (args.value & 0xC0) | (r.p & 0x3F); // Bit 7 = N flag and Bit 6 = V flag
}

void Cpu::BCC(args_t args){
	branch((r.p & CARRY_F) == 0,
		   args.add);
}

void Cpu::BCS(args_t args){
	branch(r.p & CARRY_F,
		   args.add);
}

void Cpu::BEQ(args_t args){
	branch(r.p & ZERO_F,
		   args.add);
}

void Cpu::BMI(args_t args){
	branch(r.p & NEG_F,
		   args.add);
}

void Cpu::BNE(args_t args){
	branch((r.p & ZERO_F) == 0,
		   args.add);
}

void Cpu::BPL(args_t args){
	branch((r.p & NEG_F) == 0,
		   args.add);
}

void Cpu::BVC(args_t args){
	branch((r.p & OVER_F) == 0,
		   args.add);
}

void Cpu::BVS(args_t args){
	branch(r.p & OVER_F,
		   args.add);
}

void Cpu::TXS(args_t args){
	r.sp = r.x;
	doCycle();
}
void Cpu::TSX(args_t args){
	r.x = r.sp;

	doCycle();
	setZero(r.x);
	setNeg(r.x);
}
void Cpu::TAX(args_t args){
	r.x = r.a;

	doCycle();
	setZero(r.x);
	setNeg(r.x);
}

void Cpu::TAY(args_t args){
	r.y = r.a;

	doCycle();
	setZero(r.y);
	setNeg(r.y);
}
void Cpu::TXA(args_t args){
	r.a = r.x;

	doCycle();
	setZero(r.a);
	setNeg(r.a);
}

void Cpu::TYA(args_t args){
	r.a = r.y;

	doCycle();
	setZero(r.a);
	setNeg(r.a);
}

void Cpu::PHA(args_t args){
	push(r.a);
	doCycle();
	//printf("A est pusher avec valeur de %.2X\n", r.a);
}
void Cpu::PLA(args_t args){
	r.a = pull();
	setZero(r.a);
	doCycle();
	setNeg(r.a);
	doCycle();
}
void Cpu::PHP(args_t args){
//	 B flag doesn't exit, set to 1 when push
//	Instruction 	Bits 5 and 4 	Side effects after pushing
//	PHP					11				None
//	BRK					11				I is set to 1
//	/IRQ				10				I is set to 1
//	/NMI				10				I is set to 1
	push(r.p | BRK_F);
	doCycle();
}
void Cpu::PLP(args_t args){
	r.p = (pull() & ~BRK_F) | B5_F; // BRK always 0, Bit 5 always 1
	doCycle(2);
}

void Cpu::AND(args_t args){
	r.a &= args.value;
	setZero(r.a);
	setNeg(r.a);
}

void Cpu::ORA(args_t args){
	r.a |= args.value;
	setZero(r.a);
	setNeg(r.a);
}

void Cpu::EOR(args_t args){
	r.a ^= args.value;
	setZero(r.a);
	setNeg(r.a);
}

void Cpu::ADC(args_t args){
	uint16_t v = r.a + args.value + (r.p & CARRY_F);
	if((r.a ^ v) & (args.value ^ v) & 0x80)
		r.p |= OVER_F;
	else
		r.p &= ~OVER_F;
	r.a = v;
	setCarry(v & 0x100);
	setZero(r.a);
	setNeg(r.a);
}

void Cpu::SBC(args_t args){
	uint8_t cn = 255 - args.value;
	uint16_t v = r.a + cn + (r.p & CARRY_F);
	if((r.a ^ v) & (cn ^ v) & 0x80)
		r.p |= OVER_F;
	else
		r.p &= ~OVER_F;
	r.a = v;

	setCarry(v & 0x100);
	setZero(r.a);
	setNeg(r.a);
}

void Cpu::INC(args_t args){
	mW(args.add, args.value + 1);

	doCycle();
	setZero(args.value + 1);
	setNeg(args.value + 1);
}

void Cpu::INX(args_t args){
	r.x++;

	doCycle();
	setZero(r.x);
	setNeg(r.x);
}

void Cpu::INY(args_t args){
	r.y++;

	doCycle();
	setZero(r.y);
	setNeg(r.y);
}

void Cpu::DEC(args_t args){
	mW(args.add, args.value - 1);

	doCycle();
	setZero(args.value - 1);
	setNeg(args.value - 1);
}

void Cpu::DEX(args_t args){
	r.x--;

	doCycle();
	setZero(r.x);
	setNeg(r.x);
}

void Cpu::DEY(args_t args){
	r.y--;

	doCycle();
	setZero(r.y);
	setNeg(r.y);
}

void Cpu::ROL(args_t args){
	if((args.opcode & 0x0F) == 0x0A){// Check if we are in Accumulator mode
		uint8_t b7 = r.a & 0x80;
		r.a = (r.a << 1) | (r.p & CARRY_F);
		setCarry(b7);
		setZero(r.a);
		setNeg(r.a);
	}
	else{
		uint8_t b7 = args.value & 0x80;
		args.value = (args.value << 1) | (r.p & CARRY_F);
		setCarry(b7);
		setZero(args.value);
		setNeg(args.value);
		mW(args.add, args.value);
	}

	doCycle();
}

void Cpu::ROR(args_t args){
	if((args.opcode & 0x0F) == 0x0A){// Check if we are in Accumulator mode
		uint8_t b0 = r.a & 0x01;
		r.a = (r.a >> 1) + (r.p & CARRY_F) * 128;
		setCarry(b0);
		setZero(r.a);
		setNeg(r.a);
	}
	else{
		uint8_t b0 = args.value & 0x01;
		args.value = (args.value >> 1) | ((r.p & CARRY_F) << 7);
		setCarry(b0);
		setZero(args.value);
		setNeg(args.value);
		mW(args.add, args.value);
	}

	doCycle();
}

void Cpu::LSR(args_t args){
	if((args.opcode & 0x0F) == 0x0A){// Check if we are in Accumulator mode
		setCarry(r.a & 0x01); // C = B0
		r.a = r.a / 2;
		setZero(r.a);
		setNeg(r.a);
	}
	else{
		setCarry(args.value & 0x01); // C = B0
		args.value = args.value / 2;
		setZero(args.value);
		setNeg(args.value);
		mW(args.add, args.value);
	}

	doCycle();
}

void Cpu::ASL(args_t args){
	if((args.opcode & 0x0F) == 0x0A){// Check if we are in Accumulator mode
		setCarry(r.a & 0x80); // C = B7
		r.a = r.a * 2;
		setZero(r.a);
		setNeg(r.a);
	}
	else{
		setCarry(args.value & 0x80); // C = B7
		args.value = args.value * 2;
		setZero(args.value);
		setNeg(args.value);
		mW(args.add, args.value);
	}

	doCycle();
}

void Cpu::BRK(args_t args){// NOP on NES
	push((r.pc & 0xFF00) >> 8);//MSB
	push( r.pc & 0x00FF); //lSB

	push(r.p);

	r.pc = 0xFFFE;

	r.p |= BRK_F | IRQ_F;
}

void Cpu::RTI(args_t args){
	PLP(args); // Pull p from stack
	uint16_t lsb = pull();
	uint16_t msb = pull();
	r.pc = (msb << 8) | lsb;
}



void Cpu::CMP(args_t args){
	setCarry(r.a >= args.value);
	setZero(r.a - args.value);
	setNeg(r.a - args.value);
}

void Cpu::CPX(args_t args){
	setCarry(r.x >= args.value);
	setZero(r.x - args.value);
	setNeg(r.x - args.value);
}

void Cpu::CPY(args_t args){
	setCarry(r.y >= args.value);
	setZero(r.y - args.value);
	setNeg(r.y - args.value);
}
/*
  Illegal opcode
*/
void Cpu::sNOP(args_t args){}

void Cpu::LAX(args_t args){
	//LDA(args);
	//LDX(args);
	LDA(args);
	//TAX without an idle cycle
	r.x = r.a;
	setZero(r.x);
	setNeg(r.x);
}

void Cpu::SAX(args_t args){
	// Use cycle-less write, because the write operation is done
	// at the same time than a fetch operation
	this->memory->write(args.add, r.a & r.x);
	if(args.opcode == 0x83 || args.opcode == 0x87 || args.opcode == 0x8F)
		doCycle();
}
void Cpu::AHX(args_t args){ mW(args.add, r.a & r.x);}
void Cpu::sSBC(args_t args){ SBC(args); }
void Cpu::DCP(args_t args){
	DEC(args);
	if(args.opcode == 0xD3 || args.opcode == 0xDB || args.opcode == 0xDF)
		args.value = mR(args.add);
	else
		args.value = this->memory->read(args.add); // Cycle-less read
	CMP(args);
}
void Cpu::ISB(args_t args){
	INC(args);
	if(args.opcode == 0xE3 || args.opcode == 0xE7 || args.opcode == 0xEF || args.opcode == 0xF7)
		args.value = this->memory->read(args.add); // Cycle-less read
	else
		args.value = mR(args.add);
	SBC(args);
}
void Cpu::SLO(args_t args){
	ASL(args);
	if(args.opcode == 0x03 || args.opcode == 0x07 || args.opcode == 0x0F || args.opcode == 0x17)
		args.value = this->memory->read(args.add); // Cycle-less read
	else
		args.value = mR(args.add);
	ORA(args);
}
void Cpu::RLA(args_t args){
	ROL(args);
	if(args.opcode == 0x23 || args.opcode == 0x27 || args.opcode == 0x2F || args.opcode == 0x37)
		args.value = this->memory->read(args.add); // Cycle-less read
	else
		args.value = mR(args.add);

	//if(args.opcode == 0x33)
	//	doCycle();
	AND(args);
}
void Cpu::SRE(args_t args){
	LSR(args);
	if(args.opcode == 0x43 || args.opcode == 0x47 || args.opcode == 0x4F || args.opcode == 0x57)
		args.value = this->memory->read(args.add); // Cycle-less read
	else
		args.value = mR(args.add);
	EOR(args);
}
void Cpu::RRA(args_t args){
	ROR(args);
	if(args.opcode == 0x63 || args.opcode == 0x67 || args.opcode == 0x6F || args.opcode == 0x77)
		args.value = this->memory->read(args.add); // Cycle-less read
	else
		args.value = mR(args.add);
	ADC(args);
}
/*
 not tested
*/

void Cpu::ANC(args_t args){
	AND(args);
	setCarry((r.p  & NEG_F)  == NEG_F); // N => C
	//ASL(args);
}
void Cpu::ALR(args_t args){
	AND(args);
	args.add = 0x0A;
	LSR(args);
}
void Cpu::ARR(args_t args){
	AND(args);
	args.add = 0x0A;
	ROR(args);
	setCarry((r.a  & 0x40)  == 0x40); // B6 => C
	if((((r.a  & 0x40) >> 6)^((r.a  & 0x20) >> 5)) == 1) // B6^B7 => V
		r.p |= OVER_F;
	else
		r.p &= ~OVER_F;

}
void Cpu::XAA(args_t args){
	TXA(args);
	AND(args);
}

void Cpu::AXS(args_t args){
	r.x = (r.a & r.x) - args.value;
	setNeg(r.x);
	setZero(r.x);
	setCarry(((r.a & r.x) - args.value) > 0xFF);
}
//[1005] => addr = PB();
//[1020] => d = Y;
//[1050] => addr = u8(addr) + 256 * PB();
//[1080] => RB(wrap(addr, addr+d));
//[1262] => WB(wrap(addr, addr+d), X & ((addr+d) >> 8));
void Cpu::SHX(args_t args){
	mW(args.add, (r.x & (((args.add & 0xFF00) >> 8) + 1)));
//	if (low + Y > 0xFF)
//	   low += (((high & X) << 8) + Y);
//	else
//	   low += ((high << 8) + Y);

//	DataBus = (X & (high + 1));
}
void Cpu::SHY(args_t args){
	mW(args.add, (r.y & (((args.add & 0xFF00) >> 8) + 1)));
}
void Cpu::LAS(args_t args){
	r.a = args.value & r.sp;
	r.x = args.value & r.sp;
	r.sp = args.value & r.sp;
}
void Cpu::TAS(args_t args){
	r.sp = r.a & r.x;
	mW(args.add, r.a & r.x);
}

void Cpu::KIL(args_t args){}
