#include "MMC1.h"
#include "ppu.h"
#include <stdio.h>
#include <string.h>

struct MMC1 mmc1;
extern byte *chr_rom;
extern byte ppu_mem[PPU_MEM_SIZE];

void MMC1_init() {
	memset(&mmc1, 0, sizeof(struct MMC1));
}

void MMC1_write(word addr, byte value) {
	if (GET_BIT(value, 7)) {
		mmc1.shift_reg = 0;
	} else {
		if (mmc1.ct >= 4) {
			SET_BIT_TO(mmc1.shift_reg, 4, GET_BIT(value, 0));
			word raddr = 0x8000 + ((GET_BIT(addr, 14) << 14) + (GET_BIT(addr, 13) << 13));
			//printf("Addr: %x\n", raddr);
			//printf("Reg %x\n", mmc1.shift_reg);
			if (raddr == 0xA000) { // CHR bank 0
				memcpy(&ppu_mem[0x0000], &chr_rom[4096 * mmc1.shift_reg], 4096);
			} else if (raddr == 0xC000) { // CHR bank 1
				memcpy(&ppu_mem[0x1000], &chr_rom[4096 * mmc1.shift_reg], 4096);
			} else if (raddr == 0x8000) { // Control
				mmc1.control_reg = mmc1.shift_reg;
			}
			mmc1.shift_reg = 0;
			mmc1.ct = 0;
		} else {
			SET_BIT_TO(mmc1.shift_reg, 4, GET_BIT(value, 0));
			mmc1.shift_reg >>= 1;
			mmc1.ct++;
		}
	}
}
