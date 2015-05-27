#include "include/Memory.h"
#include "include/Ppu.h"

Memory::Memory():
	mapper(0),
	lastStrobeEqual1(false),
	ctrlBitShift(0),
	bitShiftPlayer1Enable(false){

	memset(&keyState, 0, sizeof(keyState));

}


Memory::~Memory(){
	delete mapper;
}

void Memory::setPPU(Ppu *p){
	ppu = p;
}


void Memory::printNesFileInfo(iNesFile_t &nesFile){
	printf("File header:\n");
	printf("\t0-3 :Header: %s\n", nesFile.header.constant);
	printf("\t 4  :Length PRG ROM: %d * 16 KB = %d Bytes\n", nesFile.header.len_PRG_ROM,  nesFile.header.len_PRG_ROM * 16 * 1024);
	printf("\t 5  :Length CHR ROM: %d * 8 KB = %d Bytes\n", nesFile.header.len_CHR_ROM, nesFile.header.len_CHR_ROM * 8 * 1024);
	printf("\t 6  :flag 6: 0x%x\n", nesFile.header.flag_6);
	printf("\t 7  :flag 7: 0x%x\n", nesFile.header.flag_7);
	printf("\t 8  :Length PRG RAM (unused): %d\n", nesFile.header.len_PRG_RAM);
	printf("\t 9  :flag 9: 0x%x\n", nesFile.header.flag_9);
	printf("\t10  :flag 10: 0x%x\n", nesFile.header.flag_10);
	printf("\t11-15:Zero if iNES or NES 2.0: 0x%x 0x%x 0x%x 0x%x 0x%x\n\n",  nesFile.header.unused[0],
																			 nesFile.header.unused[1],
																			 nesFile.header.unused[2],
																			 nesFile.header.unused[3],
																			 nesFile.header.unused[4]);

	printf("\tMapper Nbr: %d\n", this->mapperNb);
	printf("\tNametable mirroiring: %s\n", nametabMirroring == FOUR_SCREEN ? "4 Screens": ( nametabMirroring == VERTICAL ? "V" : "H"));

}

// TODO use exception Godamit
int Memory::loadCartridge(char * filename){
	FILE *pFile;
	iNesFile_t nesFile;

	pFile = fopen(filename,"rb");
	if (!pFile){
		printf("Unable to open file!");
		return -1;
	}
	// Load the header before loading the data
	size_t lenHeader = sizeof(header_t);
	fread(&(nesFile.header), lenHeader, 1, pFile);

	// Get Mapper number
	mapperNb = (nesFile.header.flag_7 & 0xF0);       // MSB mapper
	mapperNb |= (nesFile.header.flag_6 & 0xF0) >> 4; // LSB mapper

	// Get nametab mirroring
	if((nesFile.header.flag_6 & 0x04) == 0x4)
		this->nametabMirroring = FOUR_SCREEN;
	else if((nesFile.header.flag_6 & 0x01) == 1)
		this->nametabMirroring = HORIZONTAL;
	else
		this->nametabMirroring = VERTICAL;

	this->printNesFileInfo(nesFile);

	// Check the Nes file type
	if((nesFile.header.flag_7 & 0x0C) == 0x08){
		printf("This is a NES 2.0 file!\n");
	}
	else if((nesFile.header.flag_7 & 0x0C) == 0x00 &&
			nesFile.header.unused[1] == 0 &&
			nesFile.header.unused[2] == 0 &&
			nesFile.header.unused[3] == 0 &&
			nesFile.header.unused[4] == 0){
		//printf("This is an iNES file!\n");
	}
	else
		printf("This is an archaic iNES!\n");

	// Load PGR ROM
	size_t lenPGR = nesFile.header.len_PRG_ROM * 16 * 1024;
	nesFile.PGR_ROM = (char *)malloc(lenPGR);
	fread(nesFile.PGR_ROM, lenPGR, 1, pFile);

	// Load CHR ROM
	size_t lenCHR = nesFile.header.len_CHR_ROM * 8 * 1024;
	nesFile.CHR_ROM = (char *)malloc(lenCHR);
	fread(nesFile.CHR_ROM, lenCHR, 1, pFile);

	fclose(pFile);

	switch(mapperNb){
		case 0:
			// NROM-128
			if(nesFile.header.len_PRG_ROM == 1){
				this->mapper = new NRom128;
				printf("\tMapper: NRom128\n");
			}// NROM-256
			else{
				this->mapper = new NRom256;
				printf("\tMapper: NRom256\n");
			}
			break;
		case 1:
			this->mapper = new MMC1;
			printf("\tMapper:MMC1\n");
			break;
		default:
			printf("Error: Mapper not supported!\n");
			exit(1);
	}
	this->mapper->load(nesFile.PGR_ROM, lenPGR, nesFile.CHR_ROM, lenCHR);

	free(nesFile.PGR_ROM);
	free(nesFile.CHR_ROM);

	return 0;
}

uint8_t Memory::read(uint16_t a){
	// 0x0 - 0x7FF : Internal RAM
	if(a < 0x2000){
		return ram[a % 0x800]; // Mirroiring every 2kB
	}
	// 0x2000 à 0x3FFF: : PPU Registres
	else if(a < 0x4000){
		return ppu->readReg(a);
	}// 0x4000 - 0x4016 : Register of APU and controller
	else if(a < 0x4017){
		if(a == 0x4016 || a == 0x4017)
			readControllerInput();
	}// 0x4017 - 0x4FFF : Unused
	else if(a < 0x5000){
		return 0;
	}// 0x5000 - 0x5FFF : Expansion
	else if(a < 0x6000){
		return 0;
	}// 0x6000 - 0xFFFF : SRAM for save
	else if(a < 0x8000){
		return sram[a - 0x6000];
	}
	else if(a < 0xFFFF){
		return mapper->read(a);
	}

}

void Memory::write(uint16_t a, uint8_t v){
	// 0x0 - 0x7FF : Internal RAM
	if(a < 0x2000){
		ram[a % 0x800] = v; // Mirroiring every 2kB
	} // 0x2000 à 0x3FFF: : PPU Registres
	else if(a < 0x4000){
		ppu->writeReg(a % 8, v);
	}// 0x4000 - 0x4016 : Register of APU and controller
	else if(a < 0x4017){
		if(a == 0x4014){
			ppu->writeReg(a, v);
		}
		else if(a == 0x4016)
			writeControllerInput(v);
//			apu_reg[a - 0x4000] = v;
	}// 0x4017 - 0x4FFF : Unused
	else if(a < 0x5000){
		return;
	}// 0x5000 - 0x5FFF : Expansion
	else if(a < 0x6000){
		return;
	}// 0x6000 - 0x7FFF : SRAM for save
	else if(a < 0x8000){
		sram[a - 0x6000] = v;
	}// 0x8000 - 0xFFF9 : Cartridge
	else if(a < 0xFFFF){
		mapper->write(a, v);
	}// 0xFFFA - 0xFFFF : interrupt vector

}


uint8_t Memory::readCHR(uint16_t a){
	return mapper->readCHR(a);
}

void Memory::writeCHR(uint16_t a, uint8_t v){
	mapper->writeCHR(a, v);
}

nametab_mirroring_t Memory::getNameTableMirroring(){
	return nametabMirroring;
}

void Memory::fetchKeyboardEvent(){
	SDL_Event event;


	while(SDL_PollEvent(&event)){
		switch (event.type){
		case SDL_KEYDOWN:
			keyState[event.key.keysym.sym] = 1;
			break;
		case SDL_KEYUP:
			keyState[event.key.keysym.sym] = 0;
			break;
		case SDL_QUIT:
			exit(0);
			break;
		default:
			break;
		}
	}
}

uint8_t Memory::readControllerInput(){
	// The 3 MSB are not driven so usually this is the most significant
	// byte of the address of the controller port—0x40
	if(keyState[SDLK_RIGHT] != 0 && ctrlBitShift == 7){
		readCHR(0);
	}
	if(ctrlBitShift < 8){
		uint8_t v = 0x40 | keyState[controller_player1[ctrlBitShift]];
		if(bitShiftPlayer1Enable)
			ctrlBitShift++;
		return v;
	}// Bitshifter return 1 after the 8 first read
	else{
		bitShiftPlayer1Enable = false;
		return 0x40 | 1;
	}
}

void Memory::writeControllerInput(uint8_t v){
	bool currentStrobeEqual1 = v & 1;
	if(lastStrobeEqual1 && !currentStrobeEqual1){
		ctrlBitShift = 0;
		bitShiftPlayer1Enable = true;
	}
	lastStrobeEqual1 = currentStrobeEqual1;
}


