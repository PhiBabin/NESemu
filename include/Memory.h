#ifndef MEMORY_H
#define MEMORY_H
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <SDL/SDL.h>

#include "include/Mapper.h"
#include "include/mappers/NRom128.h"
#include "include/mappers/NRom256.h"
#include "include/mappers/MMC1.h"


/*
   Ref for iNES file :
		http://wiki.nesdev.com/w/index.php/INES
 */
typedef struct{
	char constant[4]; // 0-3: $4E $45 $53 $1A ("NES" followed by MS-DOS end-of-file)
	char len_PRG_ROM; // 4:   Size of PRG ROM in 16 KB units
	char len_CHR_ROM; // 5:   Size of CHR ROM in 8 KB units (Value 0 means the board uses CHR RAM)
	char flag_6;      // 6:   Flags 6
	char flag_7;      // 7:   Flags 7
	char len_PRG_RAM; // 8:   Size of PRG RAM in 8 KB units (Value 0 infers 8 KB for compatibility; see PRG RAM circuit)
	char flag_9;      // 9:   Flags 9
	char flag_10;     //10:   Flags 10 (unofficial)
	char unused[5];   //11-15: Zero filled
}header_t;

typedef struct {
	header_t header;          // Header (16 bytes)
	char *PGR_ROM;            // PRG ROM data (16384 * x bytes)
	char *CHR_ROM;            // CHR ROM data, if present (8192 * y bytes)

	char *PlayChoice_INST_ROM;// PlayChoice INST-ROM, if present (0 or 8192 bytes)
	char *PlayChoice_PROM;    // PlayChoice PROM, if present (16 bytes Data, 16 bytes CounterOut)
	char title[128];          // Some ROM-Images additionally contain a 128-byte (or sometimes 127-byte) title at the end of the file.

}iNesFile_t;



typedef enum {
	VERTICAL,	 // vertical arrangement/horizontal mirroring (CIRAM A10 = PPU A11)
	HORIZONTAL,  // horizontal arrangement/vertical mirroring (CIRAM A10 = PPU A10)
	FOUR_SCREEN // four-screen VRAM
}nametab_mirroring_t;

// The $4016/$4017 register return back in that order the button state
typedef enum {
	BTN_A = 0,
	BTN_B,
	BTN_SELECT,
	BTN_START,
	BTN_UP,
	BTN_DOWN,
	BTN_LEFT,
	BTN_RIGHT
}controller_input_t;

class Ppu;

class Memory
{
public:
	Memory();
	void setPPU(Ppu *p);
	~Memory();
	int loadCartridge(char * filename);
	uint8_t read(uint16_t a);
	void write(uint16_t a, uint8_t v);

	uint8_t readCHR(uint16_t a);
	void writeCHR(uint16_t a, uint8_t v);

	nametab_mirroring_t getNameTableMirroring();

	void fetchKeyboardEvent();
	void setPlayerInput(const SDL_Event event);
	uint8_t readControllerInput();
	void writeControllerInput(uint8_t v);
private:
	void printNesFileInfo(iNesFile_t &nesFile);

	uint8_t ram[2048];
	//char ppu_reg[8];
	uint8_t sram[512];

	Ppu *ppu;

	uint8_t apu_reg[24];

	uint8_t keyState[SDLK_LAST];
	const uint16_t controller_player1[8] = {
										SDLK_x,		// Button A
										SDLK_z,		// Button B
										SDLK_RSHIFT,// Button SELECT
										SDLK_RETURN,// Button START
										SDLK_UP,	// Button UP
										SDLK_DOWN,	// Button DOWN
										SDLK_LEFT,	// Button LEFT
										SDLK_RIGHT	// Button RIGHT
										};

	uint8_t ctrlBitShift;
	bool lastStrobeEqual1, bitShiftPlayer1Enable;

	// Meta Data
	Mapper *mapper;
	nametab_mirroring_t nametabMirroring;
	uint8_t mapperNb;



};

#endif // MEMORY_H
