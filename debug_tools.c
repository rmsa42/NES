#include "debug_tools.h"
#include "cpu.h"
#include "ppu.h"
#include "bus.h"
#include <stdio.h>

extern struct cpu cpu;
extern struct ppu ppu;
extern byte ppu_mem[PPU_MEM_SIZE];

word bp = 0;
bool run_until_bp = false;
int debug_panel = 0;

const char *instr_enum_to_str(const byte instr) {
	switch (instr) {
		case AND_IMM:
		case AND_ABS:
		case AND_ABSX:
		case AND_ABSY:
		case AND_ZPG:
		case AND_ZPGX:
		case AND_INDX:
		case AND_INDY:
			return ("AND");
		case EOR_IMM:
		case EOR_ZPG:
		case EOR_ZPGX:
		case EOR_ABS:
		case EOR_ABSX:
		case EOR_ABSY:
			return ("EOR");
		case ORA_IMM:
		case ORA_ZPG:
		case ORA_ZPGX:
		case ORA_ABS:
		case ORA_ABSX:
		case ORA_ABSY:
			return ("ORA");
		case BIT_ZPG:
		case BIT_ABS:
			return ("BIT");
		case ADC_IMM:
		case ADC_ZPG:
		case ADC_ZPGX:
		case ADC_ABS:
		case ADC_ABSX:
		case ADC_ABSY:
		case ADC_INDX:
		case ADC_INDY:
			return ("ADC");
		case SBC_IMM:
		case SBC_ZPG:
		case SBC_ZPGX:
		case SBC_ABS:
		case SBC_ABSX:
		case SBC_ABSY:
		case SBC_INDX:
		case SBC_INDY:
			return ("SBC");
		case CMP_IMM:
		case CMP_ZPG:
		case CMP_ZPGX:
		case CMP_ABS:
		case CMP_ABSX:
		case CMP_ABSY:
		case CMP_INDX:
		case CMP_INDY:
			return ("CMP");
		case CPX_IMM:
		case CPX_ZPG:
		case CPX_ABS:
			return ("CPX");
		case CPY_IMM:
		case CPY_ZPG:
		case CPY_ABS:
			return ("CPY");
		case LDA_IMM:
		case LDA_ZPG:
		case LDA_ZPGX:
		case LDA_ABS:
		case LDA_ABSX:
		case LDA_ABSY:
		case LDA_INDX:
		case LDA_INDY:
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

void general_debug_info(struct display *dp) {
	SDL_FRect rect;
	int win_width;
	int win_height;
	int cpu_info_x;
	
	//printf("Inst %s running\n", instr_enum_to_str(bus_read(cpu.pc))); // Running Instruction
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
	SDL_RenderDebugTextFormat(dp->renderer, cpu_info_x, 190, "PPU VRAM: %x", ppu.v);
	SDL_RenderDebugTextFormat(dp->renderer, cpu_info_x, 200, "Stack");
	for (size_t i = cpu.sp + 1; i < 0xFF; i++) {
		SDL_RenderDebugTextFormat(dp->renderer, cpu_info_x - 20, 210, "%x", bus_read(make_word(STACK_OFFSET, (byte)i)));
		cpu_info_x += 20;
	}

	SDL_SetRenderScale(dp->renderer, 1, 1);
	rect.y = 5;
	rect.h = win_height - 10;
	rect.x = 5;
	rect.w = win_width / 2;
	SDL_RenderRect(dp->renderer, &rect);
	SDL_SetRenderScale(dp->renderer, 2, 2);
	for (unsigned int i = 0; i < 23; i++) {
		const byte memvl = bus_read(cpu.pc + i);
		SDL_RenderDebugTextFormat(dp->renderer, 6, 6 + (i * 10), "%x: %x", cpu.pc + i, memvl);
		SDL_RenderDebugTextFormat(dp->renderer, 80, 6 + (i * 10), "%s", instr_enum_to_str(memvl));
	}
	SDL_RenderLine(dp->renderer, 120, 9, 140, 9);
	SDL_RenderLine(dp->renderer, 120, 9, 130, 14);
	SDL_RenderLine(dp->renderer, 120, 9, 130, 4);
}

void memory_debug(struct display *dp) {
	SDL_SetRenderDrawColor(dp->renderer, 255, 255, 255, SDL_ALPHA_OPAQUE);
	SDL_SetRenderScale(dp->renderer, 2, 2);

	SDL_RenderDebugText(dp->renderer, 40, 0, "PPU Memory");
	word addr = ppu.v - 10;
	for (unsigned int i = 0; i < 21; i++) {
		SDL_RenderDebugTextFormat(dp->renderer, 10, 20 + (i * 10), "%02x:  %02x   %02x   %02x   %02x", addr,
			ppu_mem[addr], ppu_mem[addr + 1], ppu_mem[addr + 2], ppu_mem[addr + 3]);
		addr += 4;
	}
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
			} else if (e.key.scancode == SDL_SCANCODE_M) {
				debug_panel++;
				debug_panel = debug_panel % 2;
			}
		}
	}
	if (debug_panel == 0) {
		general_debug_info(dp);
	} else {
		memory_debug(dp);
	}
	SDL_RenderPresent(dp->renderer);
	if (run_until_bp == true && cpu.pc != bp) {
		advance = true;
	} else if (cpu.pc == bp) {
		run_until_bp = false;
	}
	if (cpu.pc == 0xAA66) {
		nmi();
	}
	return (advance);
}

