#include "debug_tools.h"
#include "cpu.h"
#include "ppu.h"
#include "bus.h"
#include <stdio.h>
#include <stdlib.h>

extern Cpu cpu;
extern int cpu_cycles;
extern int nbr_instr;
extern Ppu ppu;
extern int scanline_nbr;

Debugger debug;

const char *instr_enum_to_str(const byte instr);

bool debug_block() {
	return (debug.run);
}

int debugger_init(const char *bp_str, Display *dp) {
	memset(&debug, 0, sizeof(Debugger));
	if (create_display("DebuggerNes", 640, 480, dp)) {
		return (1);
	}
	SDL_SetWindowPosition(dp->window, 200, 300);
	if (bp_str != NULL) {
		debug.breakpoint = strtoul(bp_str, NULL, 16);
	}
	debug.until_bp = true;
	return (0);
}

void print_debug_cpu_instr(Display *dp, word *addr) {
	byte instr_byte = bus_read(*addr);
	Inst instr = get_inst_info(instr_byte);

	//printf("Addr: %x, %d\n", *addr, instr.byte_size);
	text(dp, "%04x:", *addr);
	text_options(dp, SAME_LINE);
	text(dp, " %s", instr_enum_to_str(bus_read(*addr)));
	for (int i = 1; i < instr.byte_size; i++) {
		text(dp, " %02x", bus_read(*addr + i));
	}
	text_options(dp, NA);
	*addr += instr.byte_size;
}

void general_debug_info(Display *dp) {
	int win_width;
	int win_height;
	
	SDL_GetWindowSize(dp->window, &win_width, &win_height);
	SDL_SetRenderDrawColor(dp->renderer, 120, 120, 255, SDL_ALPHA_OPAQUE);
	SDL_SetRenderScale(dp->renderer, 2, 2);

	word addr = cpu.pc;
	print_debug_cpu_instr(dp, &addr);
	SDL_SetRenderDrawColor(dp->renderer, 255, 255, 255, SDL_ALPHA_OPAQUE);
	for (int i = 0; i < 15; i++) {
		print_debug_cpu_instr(dp, &addr);
	}

	start_panel(dp, "CPU Info", (FPoint){win_width / 4.0f, 0}, (FPoint){win_width - win_width / 4.0f, win_height}, NORMAL);
	text(dp, "PC: %02x", cpu.pc);
	text(dp, "ACC: %02x", cpu.acc);
	text(dp, "IX: %02x", cpu.ix);
	text(dp, "IY: %02x", cpu.iy);
	text(dp, "SP: %02x", cpu.sp);
	text(dp, "Processor Status");
	text(dp, "Negative: %d", GET_BIT(cpu.ps, NF));
	text(dp, "Overflow: %d", GET_BIT(cpu.ps, VF));
	text(dp, "Unused: 0");
	text(dp, "Break: %d", GET_BIT(cpu.ps, BF));
	text(dp, "Decimal: %d", GET_BIT(cpu.ps, DF));
	text(dp, "Interrupt: %d", GET_BIT(cpu.ps, IF));
	text(dp, "Zero: %d", GET_BIT(cpu.ps, ZF));
	text(dp, "Carry: %d", GET_BIT(cpu.ps, CF));
	text(dp, "PPU VRAM: %04x", ppu.v);
	text(dp, "Scanline: %d", scanline_nbr);

	start_panel(dp, "Stack", (FPoint){0, 0}, (FPoint){100, 50}, CURR_POS | WITH_BORDER);
	text_options(dp, CENTERED);
	text(dp, "Stack");
	text_options(dp, NA);
	for (size_t i = cpu.sp + 1; i <= 0xFF; i++) {
		text(dp, "%02x,", bus_read(MAKE_WORD(STACK_OFFSET, (byte)i)));
		text_options(dp, SAME_LINE);
	}
	end_panel(dp);

	text(dp, "Breakpoint: %04x", debug.breakpoint);
	text(dp, "CPU_Cycles: %d", cpu_cycles);
	text(dp, "Instr: %d", nbr_instr);
	end_panel(dp);
}

void memory_debug(Display *dp) {
	word addr = 0x2400;

	SDL_SetRenderDrawColor(dp->renderer, 255, 255, 255, SDL_ALPHA_OPAQUE);
	SDL_SetRenderScale(dp->renderer, 1.5, 1.5);

	start_panel(dp, "PPU Memory", (FPoint){0, 0}, (FPoint){640, 480}, NORMAL);
	text(dp, "PPU Memory");
	if (ppu.v >= 0x2600){
		addr = 0x2600;
	} 
	for (int i = 0; i < 33; i++) {
		text(dp, "%04x:", addr);
		text_options(dp, SAME_LINE);
		for (int k = 0; k < 16; k++) {
			text(dp, " %02x", ppu_read_mem(addr));
			addr++;
		}
		text_options(dp, NA);
	}
	text(dp, "");
	text(dp, "Palletes");
	text(dp, "%04x:", 0x3F00);
	text_options(dp, SAME_LINE);
	for (int i = 0; i < 0x10; i++) {
		text(dp, " %02x", ppu_read_mem(0x3F00 + i));
	}
	text_options(dp, NA);
	text(dp, "%04x:", 0x3F10);
	text_options(dp, SAME_LINE);
	for (int i = 0; i < 0x10; i++) {
		text(dp, " %02x", ppu_read_mem(0x3F10 + i));
	}
	text_options(dp, NA);
	end_panel(dp);
}

void debugger_handle_event(Display *dp, SDL_Event *e) {
	if (e->type == SDL_EVENT_KEY_DOWN) {
		if (e->key.scancode == SDL_SCANCODE_RETURN) {
			debug.step_into = true;
		} else if (e->key.scancode == SDL_SCANCODE_Q) {
			dp->run = false;
		} else if (e->key.scancode == SDL_SCANCODE_M) {
			debug.panel++;
			debug.panel = debug.panel % 2;
		} else if (e->key.scancode == SDL_SCANCODE_R) {
			debug.until_bp = true;
		}
	}
}

void update_debugger(Display *dp) {
	debug.run = debug.step_into | debug.until_bp;
	if (debug.until_bp == true && cpu.pc == debug.breakpoint) {
		debug.until_bp = false;
	} else if (debug.step_into == true) {
		debug.step_into = false;
	}
	if (debug.run == true) {
		return;
	}
	SDL_SetRenderDrawColor(dp->renderer, 0, 0, 0, SDL_ALPHA_OPAQUE);
	SDL_RenderClear(dp->renderer);
	start_frame(dp);
	if (debug.panel == 0) {
		general_debug_info(dp);
	} else {
		memory_debug(dp);
	}
	SDL_RenderPresent(dp->renderer);
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
		case TXS_IMPL:
			return ("TXS");
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

