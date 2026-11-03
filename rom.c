#include "rom.h"
#include "cpu.h"
#include "ppu.h"
#include <stdio.h>
#include <string.h>

extern byte cpu_mem[CPU_MEM_SIZE];
extern byte ppu_mem[PPU_MEM_SIZE];

byte *chr_rom;
byte pallete_rgb[PALLETE_SIZE];

void print_buff(byte *buff, const size_t size) {
	for (size_t i = 0; i < size; i++) {
		printf("%zx: %x\n", i, buff[i]);
	}
}

static void debug_header(Nes_header *hd) {
	printf("INES format: %d\n", hd->ines_format);
	printf("NES2 format: %d\n", hd->nes2_format);
	printf("Trainer flag: %d\n", hd->trainer_area_flag);
	printf("PRG-ROM size: %dKiB\n", hd->prg_rom_size);
	printf("CHR-ROM size: %dKiB\n", hd->chr_rom_size);
}

static void parse_header(FILE *rom, Nes_header *hd) {
	byte header[16];

	memset(header, 0, sizeof(header));
	fread(header, sizeof(header), 1, rom);
	if (header[0] == 'N' && header[1] == 'E' && header[2] == 'S' && header[3] == NES_EOF) {
		hd->ines_format = true;
	}
	if (hd->ines_format == true && (GET_BIT(header[7], 3) && !GET_BIT(header[7], 2))) {
		hd->nes2_format = true;
	}
	hd->trainer_area_flag = GET_BIT(header[6], 2);
	// Need to alter the low_byte and high_byte
	hd->prg_rom_size = MAKE_WORD(GET_LOW_BYTE(header[9]), header[4]) * 16;
	hd->chr_rom_size = MAKE_WORD(GET_HIGH_BYTE(header[9]), header[5]);
	chr_rom = malloc(hd->chr_rom_size * 8 * 1024 * sizeof(byte));
	print_buff(header, sizeof(header));
}

static void parse_trainer_area(FILE *rom, Nes_header *hd) {}

static void parse_prg_rom(FILE *rom, size_t prg_size) {
	size_t rsize = 0;
	size_t size = 16384;

	rsize = fread(&cpu_mem[0x8000], size, 1, rom);
	//printf("RSize: %zu\n", rsize);
	rsize = fread(&cpu_mem[0xC000], size, 1, rom);
	//print_buff(&cpu_mem[0x8000], 20);
}

static void parse_chr_rom(FILE *rom, size_t chr_size) {
	size_t rsize = 0;
	size_t size = 8192;

	rsize = fread(chr_rom, size, chr_size, rom);
	//memcpy(ppu_mem, &chr_rom[size], size);
	//print_buff(ppu_mem, size);
	//rsize = fread(&ppu_mem[0x0000], size, 1, rom);
	//printf("RSize: %zu\n", rsize);
}

static void parse_pallete() {
	FILE *pal;
	size_t size = 0x3F * 3;

	memset(&pallete_rgb, 0, sizeof(pallete_rgb));
	pal = fopen("../palettes/RP2C03.pal", "r");
	fread(&pallete_rgb[0], size, 1, pal);
	//print_buff(&pallete_rgb[0], size);
	fclose(pal);
}

int read_rom(const char *rom_name) {
	FILE *rom = NULL;
	Nes_header hd;

	printf("Parsing Rom Name: %s\n", rom_name);
	if (rom_name == NULL) {
		return (1);
	}
	if (strcmp((rom_name + strlen(rom_name) - 4), ".nes")) {
		return (1);
	}
	memset(&hd, 0, sizeof(hd));
	rom = fopen(rom_name, "rb");
	parse_header(rom, &hd);
	debug_header(&hd);
	if (hd.trainer_area_flag == true) {
		parse_trainer_area(rom, &hd);
	}
	parse_prg_rom(rom, hd.prg_rom_size);
	parse_chr_rom(rom, hd.chr_rom_size);
	fclose(rom);
	parse_pallete();
	printf("Parsed Rom\n");
	return (0);
}
