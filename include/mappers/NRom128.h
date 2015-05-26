#ifndef NROM128_MAPPER_H
#define NROM128_MAPPER_H

#include <stdio.h>
#include <string.h>

#include "include/Mapper.h"

class NRom128 : public Mapper
{
public:
	void load(const char * pgrRom, const size_t lenPGR, const char * chrRom, const size_t lenCHR){
		memcpy(PGR_ROM, pgrRom, lenPGR);
		memcpy(CHR_ROM, chrRom, lenCHR);
		bzero(PGR_RAM, sizeof(PGR_RAM));
	}

	uint8_t read(uint16_t a){
		if(a < 0x6000){
			return 0; // Not in cartridge ERROR
		}// 0x6000 - 0x7FFF : Family Basic only: PRG RAM, mirrored as necessary to fill entire 8 KiB window, write protectable with an external switch
		else if(a < 0x8000){
			return PGR_RAM[a % 0x800];
		}// 0x8000 - 0xBFFF : First 16 KB of ROM.
		else// if(a < 0xC000){
			return PGR_ROM[a % 0x4000];
		//}// 0xC000 - 0xFFFF : Last 16 KB of ROM (NROM-256) or mirror of $8000-$BFFF (NROM-128).
		//else{
		//	return PGR_ROM[a % 0x4000];
			//return nesFile.PGR_ROM[a % 0x8000];
		//}
	}

	void write(uint16_t a, uint8_t v){
		if(0x6000 <= a  && a < 0x8000){
			PGR_RAM[a % 0x800] = v;
		}
	}

	uint8_t readCHR(uint16_t a){
		return CHR_ROM[a];
	}

	void writeCHR(uint16_t a, uint8_t v){
		CHR_ROM[a] = v;
	}

private:
	char PGR_ROM[16 * 1024];
	char CHR_ROM[8 * 1024];
	char PGR_RAM[8 * 1024]; // unused
};

#endif // NROM128_MAPPER_H
