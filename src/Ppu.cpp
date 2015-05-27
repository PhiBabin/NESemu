#include "include/Ppu.h"


void drawPixel32( SDL_Surface *surface, int x, int y, Uint32 pixel )
{
	//Convert the pixels to 32 bit
	Uint32 *pixels = (Uint32 *)surface->pixels;

	//Set the pixel
	pixels[ ( y * surface->w ) + x ] = pixel;
}

Ppu::Ppu():
	memory(NULL),
	SL(-1),
	cycle(0),
	bytesLeft(0){
	ppu_reg.ctrl = 0;
	ppu_reg.mask = 0;
	ppu_reg.status = 0;
	ppu_reg.oma_add = 0;

	ppu_io_latch = 0;

	ppu_reg.scrollx = 0;
	ppu_reg.scrolly = 0;
	ppu_reg.ppu_add = 0;

	ppu_reg_enable_after_rst = false;

	add_cpu_ppu = 0;

	toggle2005 = true;

	nmi_enable = false;

	bzero(ppu_pallette, 32);

	// SDL stuff todo put in class
	screen = SDL_SetVideoMode( CYC_VISIBLE + 256, SL_PER_SCREEN, 32, SDL_SWSURFACE );
	/*SDL_PixelFormat& fmt = *(screen->format);
	surface = SDL_CreateRGBSurface( 0, CYC_PER_SL, SL_PER_SCREEN,
									fmt.BitsPerPixel,
									fmt.Rmask,
									fmt.Gmask,
									fmt.Bmask,
									fmt.Amask );*/
	nesColor[0]  = SDL_MapRGB(screen->format, 0x75, 0x75, 0x75);
	nesColor[1]  = SDL_MapRGB(screen->format, 0x27, 0x1b, 0x8f);
	nesColor[2]  = SDL_MapRGB(screen->format, 0x00, 0x00, 0xAB);
	nesColor[3]  = SDL_MapRGB(screen->format, 0x47, 0x00, 0x9f);
	nesColor[4]  = SDL_MapRGB(screen->format, 0x8f, 0x00, 0x77);
	nesColor[5]  = SDL_MapRGB(screen->format, 0xab, 0x00, 0x13);
	nesColor[6]  = SDL_MapRGB(screen->format, 0xa7, 0x00, 0x00);
	nesColor[7]  = SDL_MapRGB(screen->format, 0x7f, 0x0b, 0x00);
	nesColor[8]  = SDL_MapRGB(screen->format, 0x43, 0x2f, 0x00);
	nesColor[9]  = SDL_MapRGB(screen->format, 0x00, 0x47, 0x00);
	nesColor[10] = SDL_MapRGB(screen->format, 0x00, 0x51, 0x00);
	nesColor[11] = SDL_MapRGB(screen->format, 0x00, 0x3f, 0x17);
	nesColor[12] = SDL_MapRGB(screen->format, 0x1b, 0x3f, 0x5f);
	nesColor[13] = SDL_MapRGB(screen->format, 0x00, 0x00, 0x00);
	nesColor[14] = SDL_MapRGB(screen->format, 0x00, 0x00, 0x00);
	nesColor[15] = SDL_MapRGB(screen->format, 0x00, 0x00, 0x00);
	nesColor[16] = SDL_MapRGB(screen->format, 0xbc, 0xbc, 0xbc);
	nesColor[17] = SDL_MapRGB(screen->format, 0x00, 0x73, 0xef);
	nesColor[18] = SDL_MapRGB(screen->format, 0x23, 0x3b, 0xef);
	nesColor[19] = SDL_MapRGB(screen->format, 0x83, 0x00, 0xf3);
	nesColor[20] = SDL_MapRGB(screen->format, 0xbf, 0x00, 0xbf);
	nesColor[21] = SDL_MapRGB(screen->format, 0xe7, 0x00, 0x5b);
	nesColor[22] = SDL_MapRGB(screen->format, 0xdb, 0x2b, 0x00);
	nesColor[23] = SDL_MapRGB(screen->format, 0xcb, 0x4f, 0x0f);
	nesColor[24] = SDL_MapRGB(screen->format, 0x8b, 0x73, 0x00);
	nesColor[25] = SDL_MapRGB(screen->format, 0x00, 0x97, 0x00);
	nesColor[26] = SDL_MapRGB(screen->format, 0x00, 0xab, 0x00);
	nesColor[27] = SDL_MapRGB(screen->format, 0x00, 0x93, 0x3b);
	nesColor[28] = SDL_MapRGB(screen->format, 0x00, 0x83, 0x8b);
	nesColor[29] = SDL_MapRGB(screen->format, 0x00, 0x00, 0x00);
	nesColor[30] = SDL_MapRGB(screen->format, 0x00, 0x00, 0x00);
	nesColor[31] = SDL_MapRGB(screen->format, 0x00, 0x00, 0x00);
	nesColor[32] = SDL_MapRGB(screen->format, 0xFF, 0xFF, 0xFF);
	nesColor[33] = SDL_MapRGB(screen->format, 0x3f, 0xbf, 0xff);
	nesColor[34] = SDL_MapRGB(screen->format, 0x5f, 0x97, 0xff);
	nesColor[35] = SDL_MapRGB(screen->format, 0xa7, 0x8b, 0xfd);
	nesColor[36] = SDL_MapRGB(screen->format, 0xf7, 0x7b, 0xff);
	nesColor[37] = SDL_MapRGB(screen->format, 0xff, 0x77, 0xb7);
	nesColor[38] = SDL_MapRGB(screen->format, 0xff, 0x77, 0x63);
	nesColor[39] = SDL_MapRGB(screen->format, 0xff, 0x9b, 0x3b);
	nesColor[40] = SDL_MapRGB(screen->format, 0xf3, 0xbf, 0x3f);
	nesColor[41] = SDL_MapRGB(screen->format, 0x83, 0xd3, 0x13);
	nesColor[42] = SDL_MapRGB(screen->format, 0x4f, 0xdf, 0x4b);
	nesColor[43] = SDL_MapRGB(screen->format, 0x58, 0xf8, 0x98);
	nesColor[44] = SDL_MapRGB(screen->format, 0x00, 0xeb, 0xdb);
	nesColor[45] = SDL_MapRGB(screen->format, 0x00, 0x00, 0x00);
	nesColor[46] = SDL_MapRGB(screen->format, 0x00, 0x00, 0x00);
	nesColor[47] = SDL_MapRGB(screen->format, 0x00, 0x00, 0x00);
	nesColor[48] = SDL_MapRGB(screen->format, 0xff, 0xff, 0xFF);
	nesColor[49] = SDL_MapRGB(screen->format, 0xab, 0xe7, 0xFF);
	nesColor[50] = SDL_MapRGB(screen->format, 0xc7, 0xd7, 0xFF);
	nesColor[51] = SDL_MapRGB(screen->format, 0xd7, 0xcb, 0xFF);
	nesColor[52] = SDL_MapRGB(screen->format, 0xff, 0xc7, 0xFF);
	nesColor[53] = SDL_MapRGB(screen->format, 0xff, 0xc7, 0xdb);
	nesColor[54] = SDL_MapRGB(screen->format, 0xff, 0xbf, 0xb3);
	nesColor[55] = SDL_MapRGB(screen->format, 0xff, 0xdb, 0xab);
	nesColor[56] = SDL_MapRGB(screen->format, 0xff, 0xe7, 0xa3);
	nesColor[57] = SDL_MapRGB(screen->format, 0xe3, 0xff, 0xa3);
	nesColor[58] = SDL_MapRGB(screen->format, 0xab, 0xf3, 0xbf);
	nesColor[59] = SDL_MapRGB(screen->format, 0xb3, 0xff, 0xcf);
	nesColor[60] = SDL_MapRGB(screen->format, 0x9f, 0xff, 0xf3);
	nesColor[61] = SDL_MapRGB(screen->format, 0x00, 0x00, 0x00);
	nesColor[62] = SDL_MapRGB(screen->format, 0x00, 0x00, 0x00);
	nesColor[63] = SDL_MapRGB(screen->format, 0x00, 0x00, 0x00);

	frame = 0;
}

Ppu::~Ppu(){
	SDL_Quit();
}

void Ppu::setMemory(Memory *m){
	memory = m;
}

void Ppu::reset(){
	SL = 241;
	cycle = 0;

	ppu_reg.ctrl = 0;
	ppu_reg.mask = 0;

	ppu_io_latch = 0;

	ppu_reg.scrollx = 0;
	ppu_reg.scrolly = 0;

	ppu_reg_enable_after_rst = false;
}

bool Ppu::intNMI(){
	// If the nmi was already throw, don't retrow it
	if(nmi_enable)
		return false;
	if((ppu_reg.status & STAT_NMI_OCCU_F) &&
	   (ppu_reg.ctrl & CTRL_NMI_EN_F)){
		nmi_enable = true;
		return true;
	}
	return false;
}



void Ppu::incrementX(){
	if((cycle + ppu_reg.x) % 8 == 7){
		//ppu_reg.x = 0;
		// Increment coarse x
		if((ppu_reg.v & 0x001F) == 31){		// if coarse X == 31
			ppu_reg.v &= ~0x001F;			// coarse X = 0
			ppu_reg.v ^= 0x0400;			// switch horizontal nametable
		}
		else
			ppu_reg.v += 1;					// increment coarse X
	}
	//else
		//ppu_reg.x += 1;
}

void Ppu::incrementY(){
	if((ppu_reg.v & 0x7000) != 0x7000)		// if fine Y < 7
		ppu_reg.v += 0x1000;				// increment fine Y
	else{
		ppu_reg.v &= ~0x7000;				// fine Y = 0
		int y = (ppu_reg.v & 0x03E0) >> 5;	// let y = coarse Y
		if(y == 29){
			y = 0;							// coarse Y = 0
			ppu_reg.v ^= 0x0800;			// switch vertical nametable
		}
		else if(y == 31)
			y = 0;							// coarse Y = 0, nametable not switched
		else
			y += 1;							// increment coarse Y
		ppu_reg.v = (ppu_reg.v & ~0x03E0) | (y << 5);// put coarse Y back into v
	}
}

uint16_t Ppu::getBackgroundColor(){
	uint16_t ay, ti, atfxf, msb, lsb, attributeAddress, tileAddress;

	//uint16_t xPartOfT = ppu_reg.t & 0x1F;
	//uint16_t realX = (xPartOfT << 3) | ppu_reg.x;
	//uint16_t nametabPartOfT = (ppu_reg.t & 0xC000) >> 11;

	/* v register:
	 *	yyy NN YYYYY XXXXX
	 *	||| || ||||| +++++-- coarse X scroll
	 *	||| || +++++-------- coarse Y scroll
	 *	||| ++-------------- nametable select
	 *	+++----------------- fine Y scroll
	 */
	uint16_t xCoarse = (ppu_reg.v & 0x1F);
	//uint16_t xReal = (xCoarse << 3) | ppu_reg.x;
	uint16_t xFine = (ppu_reg.x + cycle) % 8;
	uint16_t yCoarse = (ppu_reg.v & 0b1111100000) >> 5;
	//uint16_t yReal = (yCoarse << 3) | ((ppu_reg.v & 0x7000) >> 12);
	uint16_t yFine = (ppu_reg.v & 0x7000) >> 12;

	uint16_t xFineo = (cycle + ppu_reg.scrollx) % 8;
	uint16_t yFineo = SL % 8;
	uint16_t xCoarseo = (cycle + ppu_reg.scrollx) / 8;
	uint16_t yCoarseo = SL / 8;

	tileAddress = 0x2000 | (ppu_reg.v & 0x0FFF);
	ti = readPPU(tileAddress);


	ay = (ppu_reg.v & 0x7000) >> 12;
	// TODO put v and t in a separeted class
	if(xFineo != xFine
	   || xCoarseo != xCoarse
	   || yFineo != yFine
	   || yCoarseo != yCoarse){
		flagDMA = false;
	}

	uint16_t patterntable = 0x1000 * ((ppu_reg.ctrl & CTRL_PAT_TAB_BACKG_F) >> 4);
	lsb = ((readPPU(patterntable + ti * 16 + yFine	   ) >> (7 - xFine)) & 0x1) +
		  ((readPPU(patterntable + ti * 16 + yFine	+ 8) >> (7 - xFine)) & 0x1) * 2;


	// NN 1111 YYY XXX
	// || |||| ||| +++-- high 3 bits of coarse X (x/4)
	// || |||| +++------ high 3 bits of coarse Y (y/4)
	// || ++++---------- attribute offset (960 bytes)
	// ++--------------- nametable select
	attributeAddress = 0x23C0
						| (ppu_reg.v & 0x0C00)
						| ((ppu_reg.v >> 4) & 0x38)
						| ((ppu_reg.v >> 2) & 0x07);
	atfxf = readPPU(attributeAddress);
	msb = (atfxf >> ( 2 * ((xCoarse/2) & 1) + 4 * ((yCoarse/2) & 1))) & 3;

	uint16_t ac = 0x3F00 + (msb << 2) + lsb;


	return ac;
}

void Ppu::drawTilset(){
	//uint16_t patterntable = 0x1000 * ((ppu_reg.ctrl & 0x10) >> 4);
	for(uint32_t p = 0; p < 2; p++){
		for(uint32_t ti = 0; ti < 256; ti++){
			for(uint32_t ax = 0; ax < 8; ax++){
				for(uint32_t ay = 0; ay < 8; ay++){
					// Retrieve the lsb
					uint8_t lsb = ((readPPU(p * 0x1000 + ti * 16 + ay	   ) >> (7 - ax)) & 0x1) +
								  ((readPPU(p * 0x1000 + ti * 16 + ay	+ 8) >> (7 - ax)) & 0x1) * 2;

					uint16_t msb = 0;
					uint16_t ac = 0x3F00 + (msb << 2) + lsb;
					uint8_t color = readPPU(ac);

					drawPixel32(screen,
								CYC_VISIBLE + (p * 128) + (ti % 16) * 8 + ax,
								ay  + (ti / 16) * 8, nesColor[color]);
				}
			}
		}
	}
}

void Ppu::spriteEvaluation(){
	// Fill oam2 with 0xFF
	memset(ppu_oam2, 0xFF, 32);
	uint8_t y;
	uint16_t n = ppu_reg.oma_add; // The first sprite evaluate is base on %2003 OMA_ADD
	uint8_t nbr_sprite_f = 0;
	while(nbr_sprite_f < 8 && n < 256){
		y = ppu_oam1[n];
		if(y <= SL && SL < y + 8){
			memcpy(ppu_oam2 + 4 * nbr_sprite_f, ppu_oam1 + n, 4);
			nbr_sprite_f++;
		}
		n += 4;
	}

	// not enough sprite on this scanline then we are finish
	if(n >= 256)
		return;
	// Overflow:
	while(n < 256){
		y = ppu_oam1[n];
		if(y <= SL && SL < y + 8){
			// Set overflow flag
			ppu_reg.status |= STAT_SPRITE_OVERF_F;
		}
		n += 4;
	}
	// Overflow with obscure behavior:
	//	uint8_t m = 0;
	//	while(n < 256){
	//		y = ppu_oam1[n + m];
	//		if(y <= SL && SL < y + 8){
	//			// Set overflow flag
	//			ppu_reg.status |= STAT_SPRITE_OVERF_F;
	//			//Read next 3 entries
	//			m += 3;
	//			if(m > 3){
	//				m %= 4;
	//			}
	//		}
	//		n += 4;
	//		m++;
	//	}


}


uint16_t Ppu::getSpriteColor(bool bgTransparent){
	uint8_t  x, y, ti, attr, offx, offy;
	uint16_t pAdd, lsb, msb;
	uint16_t color = 0x3F00;
	uint8_t pixelX = cycle;
	uint8_t pixelY = SL - 1;

	for(int n = 0; n < 8; n++){
		y =  ppu_oam2[n * 4];
		x =  ppu_oam2[n * 4 + 3];
		if(x <= pixelX && pixelX < x + 8){
			// TODO use struct
			ti =  ppu_oam2[n * 4 + 1];
			attr =  ppu_oam2[n * 4 + 2];

			// If the sprite doesn't have priority over background and it's transparent.
			//if((attr & OAM_SPRITE_PRIORITE_F) && bgTransparent && n != 0)
			//	continue;
			// 8x16
			if(ppu_reg.ctrl & CTRL_8x16_SPRITE_F){
				pAdd = 0x1000 * (ti & 0x1);
				ti = (ti & 0xFE) >> 1;
			}// 8x8
			else{
				pAdd = 0x1000 * ((ppu_reg.ctrl & CTRL_PAT_TAB_SPRITE_F) >> 3);
			}

			offx = pixelX - x;
			offy = pixelY - y;

			if(attr & OAM_SPRITE_FLIP_HOR_F){
				offx = 7 - offx;
			}
			if(attr & OAM_SPRITE_FLIP_VER_F){
				offy = 7 - offy;
			}
			lsb = ((readPPU(pAdd + ti * 16 + offy    ) >> (7 - offx)) & 0x1) +
				  ((readPPU(pAdd + ti * 16 + offy + 8) >> (7 - offx)) & 0x1) * 2;

			// If it's not a zero color
			if(lsb != 0){
				// Either the sprite have priority over the background or it's transparent.
				if( (attr & OAM_SPRITE_PRIORITE_F) == 0 ||
				   ((attr & OAM_SPRITE_PRIORITE_F) && bgTransparent)){
					msb = attr & OAM_SPRITE_MSB_PALLET_F;
					color = 0x3F10 + (msb << 2) + lsb;
				}

				// Sprite 0 hit
				if(!bgTransparent && n == 0 && cycle != 255 && (ppu_reg.mask & MASK_ENABLE_BACKG_F)){
					ppu_reg.status |= STAT_SPRITE0_OCCU_F;
				}
			}
		}
	}
	return color;

}


void Ppu::tick(){

	// The PPU is hot enough to receive read/write from CPU
	if(cycleSinceReset > 29658)
		ppu_reg_enable_after_rst = true;
	else
		cycleSinceReset++;

	//Dot 1 of the pre-render line
	if(SL == -1 && cycle == 4){// doc said =1, blurr said =4
		ppu_reg.status &= ~STAT_SPRITE_OVERF_F; // Clear overflow
		ppu_reg.status &= ~STAT_SPRITE0_OCCU_F;	// Clear zero sprite
		// Clear Vblank flag
		if(ppu_reg_enable_after_rst)
			ppu_reg.status &= ~STAT_NMI_OCCU_F;
		nmi_enable = false;
	}// First dot of the post-render line
	else if(SL == 241 && cycle == 1){
		ppu_reg.status |= STAT_NMI_OCCU_F;		// Set   Vblank flag
		nmi_enable = false;

		// Draw current frame
		SDL_Rect offset;
		offset.x = 0;
		offset.y = 0;
		drawTilset();
		SDL_Flip( screen );

		memory->fetchKeyboardEvent();
	}//  The 257 dot is when horizontal(v) <= horizontal(t)
	else if(SL < VISIBLE_SL && cycle == 257){
		// TODO use defined mask everywhere
		if(ppu_reg.mask & (MASK_ENABLE_SPRITE_F | MASK_ENABLE_BACKG_F))
			ppu_reg.v = (ppu_reg.v & ~0x041F) | (ppu_reg.t & 0x041F);
	}// During pre-render scanline from 280 to 304 dot, t is repeatly copy to v
	else if(SL == -1 && 280 <= cycle && cycle <= 304){
		if(ppu_reg.mask & (MASK_ENABLE_SPRITE_F | MASK_ENABLE_BACKG_F))
			ppu_reg.v = (ppu_reg.v & 0x041F) | (ppu_reg.t & ~0x041F);
	}//  The 256 dot is when v is increment Verticaly
	else if(SL < VISIBLE_SL && SL != -1 && cycle == 256 ){
		incrementY(); // increase verticaly v
	}
	// We evaluate the sprite before the next line
	else if(SL < VISIBLE_SL && cycle == 339 ){
		// Do sprite evaluation when rendering is enable
		if(ppu_reg.mask & (MASK_ENABLE_SPRITE_F | MASK_ENABLE_BACKG_F))
			spriteEvaluation();
		ppu_reg.oma_add = 0;

	}// Draw the current pixel
	else if(SL < VISIBLE_SL && SL != -1 && cycle < CYC_VISIBLE){
		uint16_t bgPix = 0x3F00;
		uint16_t spPix = 0x3F00;
		if(ppu_reg.mask & MASK_ENABLE_BACKG_F  &&
		   ( cycle >= 8 || (ppu_reg.mask & MASK_SHOW_8px_BACKG_F) )){
			bgPix = getBackgroundColor();
		}

		if(ppu_reg.mask & MASK_ENABLE_SPRITE_F &&
		   ( cycle >= 8 || (ppu_reg.mask & MASK_SHOW_8px_SPRITE_F) )){
			spPix = getSpriteColor( (bgPix & 0x3) == 0);
		}

		if((spPix & 0x3) == 0)
			drawPixel32(screen, cycle, SL, nesColor[readPPU(bgPix)]);
		else
			drawPixel32(screen, cycle, SL, nesColor[readPPU(spPix)]);

		// Increase the horizontal value of the v register
		incrementX();
	}

	addCycle(1);
}



uint8_t Ppu::readReg(uint16_t a){
	switch(a % 8 ){
		case PPU_CTRL:		// 0x2000
			if(!ppu_reg_enable_after_rst)
				return 0;
			return  ppu_io_latch;
		case PPU_MASK:		// 0x2001
			if(!ppu_reg_enable_after_rst)
				return 0;
			return  ppu_io_latch;
		case PPU_STATUS:	// 0x2002
			// TODO use enum for readability
			toggle2005 = true; // Manual reset of the 0x2007 toggle to first write
			ppu_io_latch = (ppu_reg.status & 0xE0) | (ppu_io_latch & 0x1F); // Only update b7 b6 and b5 of the latch
			//Reading the status register will clear D7 mentioned above and also the address latch used by PPUSCROLL and PPUADDR. It does not clear the sprite 0 hit or overflow bit.
			ppu_reg.status &= ~STAT_NMI_OCCU_F;
			//ppu_reg.status |= 0x80;
			return  ppu_io_latch;
			break;
		case OAM_ADDR:		// 0x2003
			return  ppu_io_latch;
		case OAM_DATA:		// 0x2004
			// reads during vertical or forced blanking return the value from OAM at that address but do not increment.
			ppu_io_latch = ppu_oam1[ppu_reg.oma_add];
			return  ppu_io_latch;
			break;
		case PPU_SCROLL:	// 0x2005
			if(!ppu_reg_enable_after_rst)
				return 0;
			return  ppu_io_latch;
		case PPU_ADDR:		// 0x2006
			if(!ppu_reg_enable_after_rst)
				return 0;
			return  ppu_io_latch;
		case PPU_DATA:		// 0x2007
			//When the screen is turned off by disabling the background/sprite rendering flag with the PPUMASK or during vertical blank,
			//you can read or write data from VRAM through this port. Since accessing this register increments the VRAM address,
			//it should not be accessed outside vertical or forced blanking because it will cause graphical glitches, and if writing,
			//write to an unpredictable address in VRAM. However, two games are known to read from PPUDATA during rendering: see Tricky-to-emulate games.

			//VRAM reading and writing shares the same internal address register that rendering uses.
			//So after loading data into video memory, the program should reload the scroll position afterwards with PPUSCROLL writes in order to avoid wrong scrolling.
			//if((ppu_reg.mask & 0x0C) == 0){
				//ppu_io_latch = readBuffCPUPPU;

			uint8_t r;
			// Pallette address use internal bus, so no buffer
			if(add_cpu_ppu >= 0x3F00){
				// Doesn't read the palette, it reads the memory address bellow the pallette in VRAM
				readBuffCPUPPU = readPPU(add_cpu_ppu - 0x1000);
				r = readPPU(add_cpu_ppu);
			}
			else{
				r = readBuffCPUPPU;
				readBuffCPUPPU = readPPU(add_cpu_ppu);
			}

			if((ppu_reg.ctrl & CTRL_INCR_32_F))
				add_cpu_ppu += 32;
			else
				add_cpu_ppu++;

			return r;
	}
}


void Ppu::writeReg(uint16_t a, uint8_t v){
	switch(a){
		case PPU_CTRL:		// 0x2000
			//After power/reset, writes to this register are ignored for about 30000 cycles.
			if(!ppu_reg_enable_after_rst)
				return;
			//When turning on the NMI flag in bit 7, if the PPU is currently in vertical blank and the PPUSTATUS ($2002) vblank flag is set, an NMI will be generated immediately.
			//This can result in graphical errors (most likely a misplaced scroll) if the NMI routine is executed too late in the blanking period to finish on time.
			//To avoid this problem it is prudent to read $2002 immediately before writing $2000 to clear the vblank flag.
			ppu_reg.ctrl = v; // Set NMI_output to b7
			if(ppu_reg.ctrl & CTRL_8x16_SPRITE_F){
				printf("No 8x16 sprite support");
				exit(0);
			}

			// update 12b and 11b of the temporary address register
			ppu_reg.t = (ppu_reg.t & ~0x0C00)| ((v & 0x3) << 10);

			break;
		case PPU_MASK:		// 0x2001
			if(!ppu_reg_enable_after_rst)
				return;
			ppu_reg.mask = v;
			break;
		case PPU_STATUS:	// 0x2002
			// latch affected?
			break;
		case OAM_ADDR:		// 0x2003
			//OAMADDR is set to 0 during each of ticks 257-320 (the sprite tile loading interval) of the pre-render and visible scanlines.
			//The value of OAMADDR when sprite evaluation starts at tick 65 of the visible scanlines will determine where in OAM sprite evaluation starts,
			// and hence which sprite gets treated as sprite 0.
			ppu_reg.oma_add = v;
			break;
		case OAM_DATA:		// 0x2004
			// Writes to OAMDATA during rendering (on the pre-render line and the visible lines 0-239,
			// provided either sprite or background rendering is enabled) do not modify values in OAM, but do perform a glitchy increment of OAMADDR,
			ppu_oam1[(ppu_reg.oma_add)++] = v; // TODO add $E3 so b4 and b3 are always 0
			break;
		case PPU_SCROLL:	// 0x2005
			if(!ppu_reg_enable_after_rst)
				return;
			if(toggle2005){
				ppu_reg.scrollx = v;
				// Write b7 to b3 of the value into the temporary register
				ppu_reg.t = (ppu_reg.t & ~0x1F) | ((v >> 3) & 0x1F);
				// Write the 3 lsb to the x scrolling register
				ppu_reg.x = v & 0x7;
			}
			else{
				ppu_reg.scrolly = v;
				// Write b2 to b0 of the value into the b14 to b11 of the temporary register
				// Write b7 to b3 to b10 to b5 of t
				ppu_reg.t = (ppu_reg.t & (0xC00 | 0x1F)) // the b11 and b10 and the 5 lsb are not change
							| ((uint16_t)(v & 0b00000111) << 12)
							| ((uint16_t)(v & 0b11111000) << 2);
			}
			toggle2005 = !toggle2005;
			break;
		case PPU_ADDR:		// 0x2006
			if(!ppu_reg_enable_after_rst)
				return;
			if(toggle2005){
				add_cpu_ppu = (add_cpu_ppu & 0x00FF)
							  | ((uint16_t)(v & 0b00111111) << 8);
				// Write the first 6 bits of v into the t
				// Write b14 of t to 0
				ppu_reg.t = (ppu_reg.t & 0x00FF)
							| ((uint16_t)(v & 0b00111111) << 8);
			}
			else{
				add_cpu_ppu = (add_cpu_ppu & ~0x00FF)
							  | v;


				ppu_reg.t = (ppu_reg.t & ~0x00FF)
							| (uint16_t) v;
				// After t is updated, contents of t copied into v
				ppu_reg.v = ppu_reg.t;
			}
			toggle2005 = !toggle2005;
			//toggle2007 = !toggle2007;
			break;
		case PPU_DATA:		// 0x2007
			writePPU(add_cpu_ppu, v);
			// If it's not in the palette it's increment
			if((ppu_reg.ctrl & CTRL_INCR_32_F))
				add_cpu_ppu += 32;
			else
				add_cpu_ppu++;
			break;
		case OAM_DMA:		// 0x4014
			bytesLeft = DMA_BYTES_LEN;
			ppu_reg.oma_dma = v << 8;
			// Do copy?
			break;
	}
}

uint8_t Ppu::readPPU(uint16_t a){
	/*
$2000-$23FF 	$0400 	Nametable 0
$2400-$27FF 	$0400 	Nametable 1
$2800-$2BFF 	$0400 	Nametable 2
$2C00-$2FFF 	$0400 	Nametable 3
$3000-$3EFF 	$0F00 	Mirrors of $2000-$2EFF
$3F00-$3F1F 	$0020 	Palette RAM indexes
$3F20-$3FFF 	$00E0 	Mirrors of $3F00-$3F1F
	  */
	// Wrap around, the address bus is 14 bits
	a %= 0x4000;
	//$0000-1FFF is normally mapped by the cartridge to a CHR-ROM or CHR-RAM, often with a bank switching mechanism.
	if(a < 0x2000)
		return memory->readCHR(a);
	//$2000-2FFF is normally mapped to the 2kB NES internal VRAM,
	// providing 2 nametables with a mirroring configuration controlled by the cartridge,
	// but it can be partly or fully remapped to RAM on the cartridge,
	// allowing up to 4 simultaneous nametables.
	else if( a < 0x3F00){
		nametab_mirroring_t nm = memory->getNameTableMirroring();
		uint16_t am = a % 0x1000;// Mirroiring every 4k
		if(nm != HORIZONTAL){
			if(am >= 0x800)
				return vram[am % 0x400 + 0x800];
			else
				return vram[am % 0x400];
			//vram[am % 0x400 + 0x800 * (am / 0x800)] = v;
		}
		else if(nm != VERTICAL)
			if((am & 0x400) == 0)
				return vram[am % 0x800];			// L1: 0x2000, L3: 0x2800
			else
				return vram[(am & 0x3FF) + 0x400];  // L2: 0x2400, L4: 0x2C00
				//return vram[am % 0x7C0];
		else
			exit(1);
	}
	//$3F00-3FFF is not configurable, always mapped to the internal palette control.
	else{
		if((a & 0x3) == 0x0) // Every 4 bytes is a copy
			return ppu_pallette[0];
		else
			return ppu_pallette[(a - 0x3F00) % 0x20];
	}
	printf("Not mapped %X", a);
	exit(1);
}

void Ppu::writePPU(uint16_t a, uint8_t v){
	//72 150
	// 9 18
	// put 0x24
	// put 0xce
	// appear
	if(a == 0x2000 + 18 * 32 + 9){
		flagDMA  =false;
	}
	// Wrap around, the address bus is 14 bits
	a %= 0x4000;
	//$0000-1FFF is normally mapped by the cartridge to a CHR-ROM or CHR-RAM,
	// often with a bank switching mechanism.
	if(a < 0x2000)
		memory->writeCHR(a, v);
	//$2000-2FFF is normally mapped to the 2kB NES internal VRAM, providing 2
	// nametables with a mirroring configuration controlled by the cartridge,
	// but it can be partly or fully remapped to RAM on the cartridge, allowing
	// up to 4 simultaneous nametables.
	//$3000-3EFF is usually a mirror of the 2kB region from $2000-2EFF.
	// The PPU does not render from this address range, so this space has negligible utility.
	else if( a < 0x3F00){
		nametab_mirroring_t nm = memory->getNameTableMirroring();
		uint16_t am = a % 0x1000;// Mirroiring every 4k
		if(nm != HORIZONTAL){
			if(am >= 0x800)
				vram[am % 0x400 + 0x800] = v;
			else
				vram[am % 0x400] = v;
			//vram[am % 0x400 + 0x800 * (am / 0x800)] = v;
		}
		else if(nm != VERTICAL){
			//0x0000 xxxx xxxx = 0xX0XX
			//0x0100 xxxx xxxx = 0xX4XX
			//0x1000 xxxx xxxx = 0xX8XX
			//0x1100 xxxx xxxx = 0xXCXX
			if((am & 0x400) == 0)
				vram[am % 0x800] = v;			// L1: 0x2000, L3: 0x2800
			else
				vram[(am & 0x3FF) + 0x400] = v; // L2: 0x2400, L4: 0x2C00
		}
		else
			exit(1);
	}//$3F00-3FFF is not configurable, always mapped to the internal palette control.
	else{
		if((a & 0x13) == 0x10)
			ppu_pallette[(a - 0x3F00) % 0x10] = v;
		//if( (a & 0x3) == 0x00) // Every 4 bytes is a copy
		//	ppu_pallette[0] = v;
		else
			ppu_pallette[(a - 0x3F00) % 0x20] = v;
	}
}

uint8_t Ppu::readOAM(uint8_t a, uint8_t offset){

}

void Ppu::writeOAM2(uint8_t a, uint8_t offset, uint8_t v){

}


int Ppu::getCycle(){
	return cycle;
}

int Ppu::getScreenline(){
	return SL;
}

void Ppu::addCycle(int c){
	cycle += c;
	if( cycle >= CYC_PER_SL){
		SL++;
		if(SL == SL_PER_SCREEN)
			SL = -1;
		cycle = cycle % CYC_PER_SL;
	}
}

bool Ppu::wasCallDMA(){
	return bytesLeft != 0;
}

int Ppu::doDMA(){
	// oma_add set by $2003 offset the DMA copy and wrap around
	ppu_oam1[uint8_t(DMA_BYTES_LEN - bytesLeft + ppu_reg.oma_add)]
			=  memory->read( ppu_reg.oma_dma + DMA_BYTES_LEN - bytesLeft);
	bytesLeft--;
	if(bytesLeft == 0 && cycle % 2 == 0)
		return 3;  // 2 + 1 idle cycle
	else if(bytesLeft == 0 && cycle % 2 == 1)
		return 3;  // 2 + 1 idle cycle + 1 if odd
	else
		return 2;  // 2 cycles read + write
}

