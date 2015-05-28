#include <string.h>

#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <SDL/SDL.h>

#include "include/Cpu.h"

using namespace std;

void printUsage(char* exec) {
	printf("Usage : %s (play|help) <filename>\n", exec);
}


//list of rom : "SuperMarioBros.nes,sprite_overflow_tests/2.Details.nes";//"palette_ram.nes";//"nestest.nes";//"donkeykong.nes";//"NEStress.NES";////

int main(int argc, char* argv[]){

	char* filename;

	if (argc < 2) {
		printf("NESemu needs at least one arguments, %d was given\n", argc);
		printUsage(argv[0]);
		return 0;
	}

	if (!strcmp(argv[1], "play") and argc > 2)
		filename = argv[2];
	else if (!strcmp(argv[1], "help")) {
		printUsage(argv[0]);
		return 0;
	}
	else {
		printf("Wrong argument : %s\n", argv[1]);
		printUsage(argv[0]);
		return 0;
	}

	printf("Starting %s ...\n", filename);

	Cpu cpu;
	if(cpu.loadCartridge(filename) < 0){
		printf("Fail to load cartridge\n");
		return 1;
	}

	cpu.powerUp();

	//for(long int i = 0; i < 1000*1000; i++){// For tests: 5 000 000 // 07-abs_xy.nes 5000000// 15-brk fail at 36100 // 16-special 36000
	while(1){
		cpu.tick();
	}
	return 0;
}
