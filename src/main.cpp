#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <SDL/SDL.h>

#include "include/Cpu.h"

using namespace std;




// TODO load from args
int main(){
	char filename[] = "nestest.nes";//"SuperMarioBros.nes";//"sprite_overflow_tests/2.Details.nes";//"palette_ram.nes";//"donkeykong.nes";//"NEStress.NES";////
	Cpu cpu;
	if(cpu.loadCartridge(filename) < 0){
		printf("Fail to load cartridge");
		return 1;
	}

	cpu.powerUp();

	bool flag = false;
	int s = 10000;
	for(int i = 0; i < s; i++){// For tests: 5 000 000 // 07-abs_xy.nes 5000000// 15-brk fail at 36100 // 16-special 36000
		/*uint8_t kk = m.read(0x6000);
		if(kk == 0x81){
			printf("RESET!\n");
			exit(1);
			//cpu.reset();
		}
		else if(kk != 0x80 && flag){
			//printf("Test complete!\n");
			/*for(int j = 0; j < 1000; j++)
				cpu.tick();
			//break;
		}
		else if(kk == 0x80 && !flag){
			flag = true;
		}
		*/
		cpu.tick();
	}
	/*
	char c;
	uint16_t i = 0x6004;
	do{
		c = m.read(i);
		i++;
		putchar(c);
	}while(i <= 0x6100);*/
	return 0;
}

