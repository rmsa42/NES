#include "MMC1.h"
#include "ppu.h"
#include "cpu.h"
#include <stdio.h>
#include <string.h>

MMC1 mmc1;
extern byte *chr_rom;
extern byte *prg_rom;

void MMC1_init() {
	memset(&mmc1, 0, sizeof(MMC1));
}

void change_bank(int start_addr, int end_addr, byte *mem, void (*write)(word, byte)) {
	for (int i = start_addr; i < end_addr; i++) {
		write(i, *(mem++));
	}
}

// Need to check if chr_rom is 0 bytes
void handle_chr_regs(word addr) {
	if (addr == CHR0) {
		if (!GET_BIT(mmc1.control_reg, 4)) {
			CLEAR_BIT(mmc1.shift_reg, 0);
			change_bank(0, 0x2000, chr_rom + (KIB4 * mmc1.shift_reg), ppu_write_mem);
		} else {
			change_bank(0, 0x1000, chr_rom + (KIB4 * mmc1.shift_reg), ppu_write_mem);
		}
	} else if (addr == CHR1) {
		change_bank(0x1000, 0x2000, chr_rom + (KIB4 * mmc1.shift_reg), ppu_write_mem);
	}
}

void handle_prg_reg(word addr) {
	byte bank_mode = PRG_BANK_MODE(mmc1.control_reg);

	// I don't know what to do with this bit, so best to clear it
	CLEAR_BIT(mmc1.shift_reg, 4);
	if (bank_mode <= 1) {
		CLEAR_BIT(mmc1.shift_reg, 0);
		change_bank(0x8000, 0x10000, prg_rom + (KIB16 * mmc1.shift_reg), cpu_write);
	} else {
		int start_addr = bank_mode == 2 ? 0xC000 : 0x8000;
		change_bank(
			start_addr,
			start_addr + KIB16,
			prg_rom + (KIB16 * mmc1.shift_reg),
			cpu_write
		);
	}
}

void MMC1_write(word addr, byte value) {
	if (GET_BIT(value, 7)) {
		mmc1.shift_reg = 0;
		mmc1.control_reg = 0x0C;
	} else {
		if (mmc1.ct >= 4) {
			SET_BIT_TO(mmc1.shift_reg, 4, GET_BIT(value, 0));
			word raddr = 0x8000 + ((GET_BIT(addr, 14) << 14) + (GET_BIT(addr, 13) << 13));
			handle_chr_regs(raddr);
			if (raddr == PRG0) {
				handle_prg_reg(raddr);
			} else if (raddr == CONTROL) {
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
