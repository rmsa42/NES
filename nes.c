#include "nes.h"
#include "cpu.h"
#include "ppu.h"
#include "apu.h"
#include "rom.h"
#include "bus.h"
#include <stdio.h>

extern byte pixels[256 * 240 * 4];

int window_width = 512;
int window_height = 480;
SDL_Texture *texture;
bool strobe = false;
int ct = 0;
byte input_shift_reg = 0;
int ct2 = 0;

int nes_init(struct display *nes_dp, const char *rom_name) {
	if (create_display("NES", window_width, window_height, nes_dp)) {
		return (1);
	}
	SDL_SetWindowPosition(nes_dp->window, 1000, 300);
	cpu_init();
	ppu_init();
	if (read_rom(rom_name)) {
		printf("Invalid ROM\n");
		return (1);
	}
	texture = SDL_CreateTexture(nes_dp->renderer, SDL_PIXELFORMAT_RGBA32, SDL_TEXTUREACCESS_STREAMING, 256, 240);
	cpu_start();
	printf("Nes Init\n");
	nes_dp->run = true;
	return (0);

}

void check_strobe() {
	bool strobe_high = check_bit(bus_read(0x4016), 0);

	if (!strobe && strobe_high) {
		strobe = true;
	} else if (strobe && !strobe_high) {
		strobe = false;
		ct = 7;
	}
	if (ct > 0) {
		if (ct == 4) {
			if (ct2 == 10000) {
				input_shift_reg = 0;
				ct2 = 0;
			}
			bus_write(0x4016, input_shift_reg);
			if (input_shift_reg == 1) {
				printf("Sending 1, %d\n", ct2);
				ct2++;
			}
		} else {
			bus_write(0x4016, 0);
		}
		ct--;
	}
}

void update_nes(struct display *nes_dp, int curr_time, int *last_time) {
	byte cycles = 0;
	bool render = curr_time > *last_time + 16;

	if (render) {
		SDL_SetRenderDrawColor(nes_dp->renderer, 0, 0, 0, SDL_ALPHA_OPAQUE);
		SDL_RenderClear(nes_dp->renderer);
	}
	cycles = clock_cpu();
	clock_ppu(nes_dp, cycles);
	check_strobe();
	//clock_apu(stream);
	if (render) {
		SDL_UpdateTexture(texture, NULL, (void *)pixels, 256 * 4);
		SDL_RenderTexture(nes_dp->renderer, texture, NULL, NULL);
		SDL_RenderPresent(nes_dp->renderer);
		*last_time = curr_time;
	}
}

void nes_handle_event(struct display *nes_dp, SDL_Event *e) {
	if (e->type == SDL_EVENT_KEY_DOWN) {
		if (e->key.scancode == SDL_SCANCODE_Q) {
			nes_dp->run = false;
		} else if (e->key.scancode == SDL_SCANCODE_RETURN) {
			input_shift_reg = 1;
		}
	} 
	//else if (e->type == SDL_EVENT_KEY_DOWN) {
	//	if (e->key.scancode == SDL_SCANCODE_RETURN) {
	//		input_shift_reg = 0;
	//	}
	//}
}
