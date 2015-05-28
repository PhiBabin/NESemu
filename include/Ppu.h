#ifndef PPU_H
#define PPU_H
#include <SDL/SDL.h>
#include <ctime>
#include <unistd.h>
#include "include/Memory.h"

#define CYC_VISIBLE 256
#define CYC_PER_SL  341
#define VISIBLE_SL 240
#define SL_PER_SCREEN 261

#define MICRO_SECOND_PER_FRAME 16639

#define DMA_BYTES_LEN 256



#define PPU_CTRL	0x000
#define PPU_MASK	0x001
#define PPU_STATUS  0x002
#define OAM_ADDR    0x003
#define OAM_DATA    0x004
#define PPU_SCROLL  0x005
#define PPU_ADDR	0x006
#define PPU_DATA	0x007
#define OAM_DMA		0x4014

// $2000 Control
#define CTRL_NAMETABLE_F				0x03	// (0 = $2000; 1 = $2400; 2 = $2800; 3 = $2C00)
#define CTRL_INCR_32_F					0x04	// (0: 1; 1: 32)
#define CTRL_PAT_TAB_SPRITE_F			0x08	// (0: $0000; 1: $1000; ignored in 8x16 mode)
#define CTRL_PAT_TAB_BACKG_F			0x10	// (0: $0000; 1: $1000)
#define CTRL_8x16_SPRITE_F				0x20	// (0: 8x8; 1: 8x16)
#define CTRL_SLAVE_F					0x40	// not used
#define CTRL_NMI_EN_F					0x80	// (0: off; 1: on)

// $2001 Mask
#define MASK_GRAYSCALE_F				0x01	// (0: color; 1: B/W)
#define MASK_SHOW_8px_BACKG_F			0x02	// (1: Show background in leftmost 8 pixels of screen; 0: Hide)
#define MASK_SHOW_8px_SPRITE_F			0x04	// (1: Show sprites in leftmost 8 pixels of screen; 0: Hide)
#define MASK_ENABLE_BACKG_F				0x08	// (1: show background)
#define MASK_ENABLE_SPRITE_F			0x10	// (1: show sprite)

// $2002 Status
#define STAT_SPRITE_OVERF_F				0x20 // More than 8 sprites on the same scranline (bug)
#define STAT_SPRITE0_OCCU_F				0x40 // Nonzero pixel of sprite 0 overlaps nonzero bg pixel
#define STAT_NMI_OCCU_F					0x80 //  (0: not in vblank; 1: in vblank)

#define OAM_SPRITE_MSB_PALLET_F			0x03
#define OAM_SPRITE_PRIORITE_F			0x20 // (0: in front of background; 1: behind background)
#define OAM_SPRITE_FLIP_HOR_F			0x40
#define OAM_SPRITE_FLIP_VER_F			0x80

typedef struct{
	uint8_t ctrl;
	uint8_t mask;
	uint8_t status;
	uint8_t oma_add; // For $2003/$2004 and DMA offset
	uint16_t ppu_add;// For $2006/$2007

	// PPU scrolling strange behavior
	uint16_t t; // 15 bits in reality
	uint16_t v; // 15 bits
	uint8_t x; //  3 bits

	uint16_t oma_dma;
}reg_ppu_t;

typedef struct{
	uint8_t y;
	uint8_t tile_id;
	uint8_t attribut;
	uint8_t x;
}reg_oma_t;


class Ppu
{
public:
	Ppu();
	~Ppu();
	void setMemory(Memory *m);
	void reset();
	void tick();

	void spriteEvaluation();
	uint16_t getBackgroundColor();
	uint16_t getSpriteColor(bool bgTransparent);
	void drawTilset();

	int getCycle();
	int getScreenline();
	void addCycle(const int c);

	uint8_t readPPU(uint16_t a);
	void writePPU(uint16_t a, uint8_t v);

	uint8_t readReg(uint16_t a);
	void writeReg(uint16_t a, uint8_t v);


	uint8_t readOAM(uint8_t a, uint8_t offset);
	void writeOAM2(uint8_t a, uint8_t offset, uint8_t v);

	uint8_t readPallette(uint8_t a);

	bool wasCallDMA();
	int doDMA();

	bool intNMI();

	void incrementX();
	void incrementY();

	void fetchPlayerInput();
private:
	Memory *memory;
	unsigned int cycleSinceReset;
	int cycle;
	int SL;

	reg_ppu_t ppu_reg;
	uint8_t vram[2048];
	uint8_t ppu_oam1[256];
	uint8_t ppu_oam2[32];
	uint8_t ppu_pallette[32];

	// TODO: Put in reg
	bool toggle2005;
	uint8_t ppu_io_latch;
	uint16_t add_cpu_ppu;
	uint8_t readBuffCPUPPU;


	uint8_t sprite_eval_buff;
	uint nbrSpriteCurrentSL;

	bool ppu_reg_enable_after_rst;

	int bytesLeft;

	bool nmi_enable;

	// Debuging
	clock_t end, begin;
	clock_t lastFrame;
	uint16_t frame;
	bool flagDMA; // unused TODO

	// SDL stuff to put in own class
	SDL_Surface *surface;
	SDL_Surface *screen;

	uint32_t nesColor[64];

};

#endif // PPU_H
