#ifndef MMC1_H
#define MMC1_H

#include <stdio.h>
#include <string.h>

#include "include/Mapper.h"

#define REG_CONTROL		0x00
#define REG_CHR_BANK_0  0x01
#define REG_CHR_BANK_1  0x02
#define REG_PRG			0x03

#define WRAM_F			0x10

#define SLOT_F			0x08
#define PGR_MOD1_F		0x08
#define PGR_MOD2_F		0x0C


#define BANK_LEN		(16 * 1024)

//4bit0
//-----
//CSPMM
//|||||
//|||++- Mirroring (0: one-screen, lower bank; 1: one-screen, upper bank;
//|||               2: vertical; 3: horizontal)
//|++--- PRG ROM bank mode (0, 1: switch 32 KB at $8000, ignoring low bit of bank number;
//|                         2: fix first bank at $8000 and switch 16 KB bank at $C000;
//|                         3: fix last bank at $C000 and switch 16 KB bank at $8000)
//+----- CHR ROM bank mode (0: switch 8 KB at a time; 1: switch two separate 4 KB banks)

// For the SxROM variant of MMC1
class MMC1 : public Mapper
{
public:
	void load(const char * pgrRom, const size_t lenPGR, const char * chrRom, const size_t lenCHR){
		// Large PGR => SUROM
		if(lenPGR > 256 * 1024){
			// When PRG ROM is large, the highest CHR line (CHR A16) switches 256 KiB PRG ROM banks as in SUROM.
			printf("Doesn't support SUROM\n");
			exit(0);
		}

		// Both CHR and PGR are small
		else{
			reg[REG_PRG] = 0;   // WRAM Disable
		}
		PGR_ROM = (char *)malloc(lenPGR);
		CHR_ROM = (char *)malloc(lenCHR);
		memcpy(PGR_ROM, pgrRom, lenPGR);
		memcpy(CHR_ROM, chrRom, lenCHR);
		bzero(PGR_RAM, sizeof(PGR_RAM)); // Clear RAM

		reg[REG_CONTROL] = PGR_MOD2_F; // PRG ROM bank mode 3
		reg[REG_CHR_BANK_0] = 0;
		reg[REG_CHR_BANK_1] = 0;
		reg[REG_PRG] = WRAM_F;   // WRAM Enable

		shiftReg = 1;

		nbrBankPgr = lenPGR / BANK_LEN;
		printf("Bank nbr: %d\n", nbrBankPgr);
	}

	uint8_t read(uint16_t a){
		if(a < 0x6000){
			return 0; // Not in cartridge ERROR
		}// 0x6000 - 0x7FFF : 8 KB PRG RAM bank, fixed
		else if(a < 0x8000){
			if(reg[REG_PRG] & WRAM_F) // If WRAM is enable
				return PGR_RAM[a % 0x800];
			else
				return 0;
		}// 0x8000 - 0xFFFF PGR_ROM
		else {
			// 32 KB bank
			if((reg[REG_CONTROL] & SLOT_F) == 0){
				// Ignore low bit of bank number
				return PGR_ROM[a % 0x8000 + (reg[REG_PRG] & 0x0E) * BANK_LEN];
			}// First, Switchable
			else if((reg[REG_CONTROL] & PGR_MOD1_F) == PGR_MOD1_F){
				// 0x8000 - 0xBFFF : 16 KB PRG ROM bank, first bank
				if(a < 0xC000){
					return PGR_ROM[a - 0x8000];
				}// 0xC000 - 0xFFFF : 16 KB PRG ROM bank, switchable bank
				else{
					return PGR_ROM[a - 0xC000 + (reg[REG_PRG] & 0x0F) * BANK_LEN];
				}
			}// Switchable, last
			else{
				// 0x8000 - 0xBFFF : 16 KB PRG ROM bank, switchable bank
				if(a < 0xC000){
					return PGR_ROM[a - 0x8000 + (reg[REG_PRG] & 0x0F) * BANK_LEN];
				}// 0xC000 - 0xFFFF : 16 KB PRG ROM bank, last bank
				else{
					return PGR_ROM[a - 0xC000 + (nbrBankPgr - 1) * BANK_LEN];
				}
			}
		}
	}
	//       PPU $0000-$0FFF: 4 KB switchable CHR bank
	//       PPU $1000-$1FFF: 4 KB switchable CHR bank

	void write(uint16_t a, uint8_t v){
		if(0x6000 <= a  && a < 0x8000){
			if(reg[REG_PRG] & WRAM_F) // If WRAM is enable
				PGR_RAM[a % 0x800] = v;
		}// 0x8000 - 0xFFFF : Config register
		else{
			// Writing with b7 = 1, clear the register
			if(v & 0x80){
				shiftReg = 1;
			}// Write with b7 = 0, shift b0 into shift register
			else{
				if((shiftReg & 0x10) == 0){
					shiftReg = (shiftReg << 1) | (v & 0x1);
				}// On fifth write: Copy shift register into internal register selected by b14 and b13 of the address
				 // Clear the shift register after that
				else{
					shiftReg = (shiftReg << 1) | (v & 0x1);
					reg[ (a & 0x6000) >> 13] = shiftReg & 0x1F; // Copy only the first 5 bit
					shiftReg = 1;
				}

			}
		}
	}

	uint8_t readCHR(uint16_t a){
		return 0;
	}

	void writeCHR(uint16_t a, uint8_t v){
	}


private:
	uint8_t shiftReg;
	uint8_t reg[4];
	uint8_t nbrBankPgr;
	char *PGR_ROM;
	char *CHR_ROM;
	char PGR_RAM[8 * 1024]; // unused
};

#endif // MMC1_H
