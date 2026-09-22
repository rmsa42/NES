#include "debug_tools.h"
#include "cpu.h"
#include <stdio.h>

extern struct cpu cpu;
extern byte memory[0xFFFF];

word bp = 0;
bool run_until_bp = false;

const char *instr_enum_to_str(const byte instr) {
	switch (instr) {
		case LDA_IMM:
		case LDA_ZPG:
		case LDA_ZPGX:
		case LDA_ABS:
		case LDA_ABSX:
		case LDA_ABSY:
			return ("LDA");
		case LDY_IMM:
		case LDY_ZPG:
		case LDY_ZPGX:
		case LDY_ABS:
		case LDY_ABSX:
			return ("LDY");
		case LDX_IMM:
		case LDX_ZPG:
		case LDX_ZPGY:
		case LDX_ABS:
		case LDX_ABSY:
			return ("LDX");
		case STA_ZPG:
		case STA_ZPGX:
		case STA_ABS:
		case STA_ABSX:
		case STA_ABSY:
		case STA_INDX:
		case STA_INDY:
			return ("STA");
		case STX_ZPG:
		case STX_ZPGY:
		case STX_ABS:
			return ("STX");
		case STY_ZPG:
		case STY_ZPGX:
		case STY_ABS:
			return ("STY");
		case TAX_IMPL:
			return ("TAX");
		case TAY_IMPL:
			return ("TAY");
		case TXA_IMPL:
			return ("TXA");
		case TYA_IMPL:
			return ("TYA");
		case TSX_IMPL:
			return ("TSX");
		case PHA_IMPL:
			return ("PHA");
		case PHP_IMPL:
			return ("PHP");
		case PLA_IMPL:
			return ("PLA");
		case PLP_IMPL:
			return ("PLP");
		case JMP_ABS:
		case JMP_IND:
			return ("JMP");
		case JSR_ABS:
			return ("JSR");
		case RTS_IMPL:
			return ("RTS");
		case BCC_RLT:
			return ("BCC");
		case BCS_RLT:
			return ("BCS");
		case BEQ_RLT:
			return ("BEQ");
		case BNE_RLT:
			return ("BNE");
		case BMI_RLT:
			return ("BMI");
		case BPL_RLT:
			return ("BPL");
		case BVC_RLT:
			return ("BVC");
		case BVS_RLT:
			return ("BVS");
		case INC_ABS:
		case INC_ABSX:
		case INC_ZPG:
		case INC_ZPGX:
			return ("INC");
		case DEC_ABS:
		case DEC_ABSX:
		case DEC_ZPG:
		case DEC_ZPGX:
			return ("DEC");
		case INX_IMPL:
			return ("INX");
		case INY_IMPL:
			return ("INY");
		case DEX_IMPL:
			return ("DEX");
		case DEY_IMPL:
			return ("DEY");
		case ASL_ACC:
		case ASL_ABS:
		case ASL_ABSX:
		case ASL_ZPG:
		case ASL_ZPGX:
			return ("ASL");
		case LSR_ACC:
		case LSR_ABS:
		case LSR_ABSX:
		case LSR_ZPG:
		case LSR_ZPGX:
			return ("LSR");
		case ROL_ACC:
		case ROL_ABS:
		case ROL_ABSX:
		case ROL_ZPG:
		case ROL_ZPGX:
			return ("ROL");
		case ROR_ACC:
		case ROR_ABS:
		case ROR_ABSX:
		case ROR_ZPG:
		case ROR_ZPGX:
			return ("ROR");
		case CLC_IMPL:
			return ("CLC");
		case SEC_IMPL:
			return ("SEC");
		case CLI_IMPL:
			return ("CLI");
		case SEI_IMPL:
			return ("SEI");
		case CLV_IMPL:
			return ("CLV");
		case CLD_IMPL:
			return ("CLD");
		case SED_IMPL:
			return ("SED");
		case NOP_IMPL:
			return ("NOP");
		case BRK_IMPL:
			return ("BRK");
		case RTI_IMPL:
			return ("RTI");

		default:
			return ("");
	}
}

void cpu_debug(struct display *dp) {
	SDL_FRect rect;
	int win_width;
	int win_height;
	int cpu_info_x;
	
	SDL_GetWindowSize(dp->window, &win_width, &win_height);
	SDL_SetRenderDrawColor(dp->renderer, 255, 255, 255, SDL_ALPHA_OPAQUE);
	SDL_SetRenderScale(dp->renderer, 2, 2);

	cpu_info_x = (win_width / 4) + 10;
	SDL_RenderDebugTextFormat(dp->renderer, cpu_info_x, 10, "CPU State");
	SDL_RenderDebugTextFormat(dp->renderer, cpu_info_x, 20, "PC: %x", cpu.pc);
	SDL_RenderDebugTextFormat(dp->renderer, cpu_info_x, 30, "ACC: %x", cpu.acc);
	SDL_RenderDebugTextFormat(dp->renderer, cpu_info_x, 40, "IX: %x", cpu.ix);
	SDL_RenderDebugTextFormat(dp->renderer, cpu_info_x, 50, "IY: %x", cpu.iy);
	SDL_RenderDebugTextFormat(dp->renderer, cpu_info_x, 60, "SP: %x", cpu.sp);
	SDL_RenderDebugTextFormat(dp->renderer, cpu_info_x, 80, "Processor Status");
	SDL_RenderDebugTextFormat(dp->renderer, cpu_info_x, 90, "Negative: %d", get_bit(cpu.ps, NF));
	SDL_RenderDebugTextFormat(dp->renderer, cpu_info_x, 100, "Overflow: %d", get_bit(cpu.ps, VF));
	SDL_RenderDebugTextFormat(dp->renderer, cpu_info_x, 110, "Unused: 0");
	SDL_RenderDebugTextFormat(dp->renderer, cpu_info_x, 120, "Break: %d", get_bit(cpu.ps, BF));
	SDL_RenderDebugTextFormat(dp->renderer, cpu_info_x, 130, "Decimal: %d", get_bit(cpu.ps, DF));
	SDL_RenderDebugTextFormat(dp->renderer, cpu_info_x, 140, "Interrupt: %d", get_bit(cpu.ps, IF));
	SDL_RenderDebugTextFormat(dp->renderer, cpu_info_x, 150, "Zero: %d", get_bit(cpu.ps, ZF));
	SDL_RenderDebugTextFormat(dp->renderer, cpu_info_x, 160, "Carry: %d", get_bit(cpu.ps, CF));
	SDL_RenderDebugTextFormat(dp->renderer, cpu_info_x, 180, "Breakpoint: %x", bp);

	SDL_SetRenderScale(dp->renderer, 1, 1);
	rect.y = 5;
	rect.h = win_height - 10;
	rect.x = 5;
	rect.w = win_width / 2;
	SDL_RenderRect(dp->renderer, &rect);
	SDL_SetRenderScale(dp->renderer, 2, 2);
	for (unsigned int i = 0; i < 23; i++) {
		const byte memvl = memory[cpu.pc + i];
		SDL_RenderDebugTextFormat(dp->renderer, 6, 6 + (i * 10), "%x: %x", cpu.pc + i, memvl);
		SDL_RenderDebugTextFormat(dp->renderer, 80, 6 + (i * 10), "%s", instr_enum_to_str(memvl));
	}
	SDL_RenderLine(dp->renderer, 120, 9, 140, 9);
	SDL_RenderLine(dp->renderer, 120, 9, 130, 14);
	SDL_RenderLine(dp->renderer, 120, 9, 130, 4);
}

int debugger_init(const char *bp_str, struct display *dp) {
	if (create_SDL_display("DebuggerNes", 640, 480, dp)) {
		return (1);
	}
	run_until_bp = false;
	if (bp_str != NULL) {
		bp = strtoul(bp_str, NULL, 16);
		run_until_bp = true;
	}
	return (0);
}

bool update_debugger(struct display *dp) {
	SDL_Event e;
	bool advance = false;

	SDL_SetRenderDrawColor(dp->renderer, 0, 0, 0, SDL_ALPHA_OPAQUE);
	SDL_RenderClear(dp->renderer);
	while (SDL_PollEvent(&e)) {
		if (e.type == SDL_EVENT_KEY_DOWN) {
			if (e.key.scancode == SDL_SCANCODE_RETURN) {
				advance = true;
			} else if (e.key.scancode == SDL_SCANCODE_Q) {
				dp->run = false;
			}
		}
	}
	cpu_debug(dp);
	SDL_RenderPresent(dp->renderer);
	if (run_until_bp == true && cpu.pc != bp) {
		advance = true;
	} else if (cpu.pc == bp) {
		run_until_bp = false;
	}
	return (advance);
}

