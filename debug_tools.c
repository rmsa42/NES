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
int ct = 34;

const char *instr_enum_to_str(const byte instr);

int debugger_init(const char *bp_str, struct display *dp) {
	if (create_SDL_display("DebuggerNes", 640, 480, dp)) {
		return (1);
	}
	SDL_SetWindowPosition(dp->window, 200, 300);
	run_until_bp = false;
	if (bp_str != NULL) {
		bp = strtoul(bp_str, NULL, 16);
		run_until_bp = true;
	}
	return (0);
}

void general_debug_info(struct display *dp) {
	int win_width;
	int win_height;
	
	//printf("Inst %s running\n", instr_enum_to_str(bus_read(cpu.pc))); // Running Instruction
	SDL_GetWindowSize(dp->window, &win_width, &win_height);
	SDL_SetRenderDrawColor(dp->renderer, 0, 0, 255, SDL_ALPHA_OPAQUE);
	SDL_SetRenderScale(dp->renderer, 1.7, 1.7);

	byte memvl = bus_read(cpu.pc);
	SDL_SetRenderDrawColor(dp->renderer, 0, 0, 255, SDL_ALPHA_OPAQUE);
	text(dp, "%02x: %02x  %s", cpu.pc, memvl, instr_enum_to_str(memvl));
	SDL_SetRenderDrawColor(dp->renderer, 255, 255, 255, SDL_ALPHA_OPAQUE);
	for (unsigned int i = 0; i < 21; i++) {
		memvl = bus_read(cpu.pc + i);
		text(dp, "%02x: %02x  %s", cpu.pc + i, memvl, instr_enum_to_str(memvl));
	}

	start_panel(dp, "CPU Info", (fpoint){win_width / 4.0f, 0}, (fpoint){win_width - win_width / 4.0f, win_height}, NORMAL);
	//set_in_pos(dp, (fpoint){(win_width / 4) + 10, 0});
	text(dp, "PC: %02x", cpu.pc);
	text(dp, "ACC: %02x", cpu.acc);
	text(dp, "IX: %02x", cpu.ix);
	text(dp, "IY: %02x", cpu.iy);
	text(dp, "SP: %02x", cpu.sp);
	text(dp, "Processor Status");
	text(dp, "Negative: %d", check_bit(cpu.ps, NF));
	text(dp, "Overflow: %d", check_bit(cpu.ps, VF));
	text(dp, "Unused: 0");
	text(dp, "Break: %d", check_bit(cpu.ps, BF));
	text(dp, "Decimal: %d", check_bit(cpu.ps, DF));
	text(dp, "Interrupt: %d", check_bit(cpu.ps, IF));
	text(dp, "Zero: %d", check_bit(cpu.ps, ZF));
	text(dp, "Carry: %d", check_bit(cpu.ps, CF));
	text(dp, "PPU VRAM: %04x", ppu.v);

	text(dp, "Stack");
	start_panel(dp, "Stack", (fpoint){0, 0}, (fpoint){100, 50}, CURR_POS | WITH_BORDER);
	for (size_t i = cpu.sp + 1; i <= 0xFF; i++) {
		text(dp, "%02x,", bus_read(make_word(STACK_OFFSET, (byte)i)));
		text_options(dp, SAME_LINE);
	}
	end_panel(dp);

	text(dp, "Breakpoint: %04x", bp);
	end_panel(dp);

}

void memory_debug(struct display *dp) {
	SDL_SetRenderDrawColor(dp->renderer, 255, 255, 255, SDL_ALPHA_OPAQUE);
	SDL_SetRenderScale(dp->renderer, 2, 2);

	start_panel(dp, "PPU Memory", (fpoint){0, 0}, (fpoint){320, 240}, WITH_BORDER);
	text(dp, "PPU Memory");
	word addr = ppu.v - 10;
	for (unsigned int i = 1; i < 21; i++) {
		text(dp, "%04x: %02x   %02x   %02x   %02x", addr,
			ppu_mem[addr], ppu_mem[addr + 1], ppu_mem[addr + 2], ppu_mem[addr + 3]);
		addr += 4;
	}
	end_panel(dp);
}

bool update_debugger(struct display *dp) {
	SDL_Event e;
	bool advance = false;

	start_frame(dp);
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
			} else if (e.key.scancode == SDL_SCANCODE_P) {
				nmi(cpu.pc);
			}
		}
	}
	if (debug_panel == 0) {
		general_debug_info(dp);
	} else {
		memory_debug(dp);
	}
	SDL_RenderPresent(dp->renderer);
	if (cpu.pc == bp) {
		ct -= 1;
		if (ct == 0) {
			run_until_bp = false;
		} else {
			advance = true;
		}
	} else if (run_until_bp == true) {
		advance = true;
	}
	//if (cpu.pc == 0xAA66) {
	//	nmi(0xAA69);
	//} else if (cpu.pc == 0xAA50) {
	//	nmi(0xAA53);
	//}
	return (advance);
}

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

