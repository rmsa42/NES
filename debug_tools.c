#include "cpu.h"
#include "SDL3/SDL.h"
#include <stdio.h>

extern struct cpu *cpu;
extern byte memory[0xFFFF];
extern SDL_Renderer *renderer;
extern SDL_Window *window;

void cpu_debug() {
	SDL_FRect rect;
	int win_width;
	int win_height;
	int cpu_info_x;
	
	SDL_GetWindowSize(window, &win_width, &win_height);
	SDL_SetRenderDrawColor(renderer, 255, 255, 255, SDL_ALPHA_OPAQUE);
	SDL_SetRenderScale(renderer, 2, 2);

	cpu_info_x = (win_width / 4) + 10;
	SDL_RenderDebugTextFormat(renderer, cpu_info_x, 10, "CPU State");
	SDL_RenderDebugTextFormat(renderer, cpu_info_x, 20, "PC: %x", cpu->pc);
	SDL_RenderDebugTextFormat(renderer, cpu_info_x, 30, "ACC: %x", cpu->acc);
	SDL_RenderDebugTextFormat(renderer, cpu_info_x, 40, "IX: %x", cpu->ix);
	SDL_RenderDebugTextFormat(renderer, cpu_info_x, 50, "IY: %x", cpu->iy);
	SDL_RenderDebugTextFormat(renderer, cpu_info_x, 60, "SP: %x", cpu->sp);
	SDL_RenderDebugTextFormat(renderer, cpu_info_x, 80, "Processor Status");
	SDL_RenderDebugTextFormat(renderer, cpu_info_x, 90, "Negative: %d", get_bit(cpu->ps, NF));
	SDL_RenderDebugTextFormat(renderer, cpu_info_x, 100, "Overflow: %d", get_bit(cpu->ps, VF));
	SDL_RenderDebugTextFormat(renderer, cpu_info_x, 110, "Unused: 0");
	SDL_RenderDebugTextFormat(renderer, cpu_info_x, 120, "Break: %d", get_bit(cpu->ps, BF));
	SDL_RenderDebugTextFormat(renderer, cpu_info_x, 130, "Decimal: %d", get_bit(cpu->ps, DF));
	SDL_RenderDebugTextFormat(renderer, cpu_info_x, 140, "Interrupt: %d", get_bit(cpu->ps, IF));
	SDL_RenderDebugTextFormat(renderer, cpu_info_x, 150, "Zero: %d", get_bit(cpu->ps, ZF));
	SDL_RenderDebugTextFormat(renderer, cpu_info_x, 160, "Carry: %d", get_bit(cpu->ps, CF));

	SDL_SetRenderScale(renderer, 1, 1);
	rect.y = 5;
	rect.h = win_height - 10;
	rect.x = 5;
	rect.w = win_width / 2;
	SDL_RenderRect(renderer, &rect);
	SDL_SetRenderScale(renderer, 2, 2);
	for (unsigned int i = 0; i < 23; i++) {
		SDL_RenderDebugTextFormat(renderer, 6, 6 + (i * 10), "%x: %x", cpu->pc + i, memory[cpu->pc + i]);
	}
	SDL_RenderLine(renderer, 100, 9, 136, 9);
	SDL_RenderLine(renderer, 100, 9, 110, 16);
	SDL_RenderLine(renderer, 100, 9, 110, 2);
}
