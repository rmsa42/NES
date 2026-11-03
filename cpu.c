#include "cpu.h"
#include "bus.h"
#include <string.h>
#include <stdio.h>
#include <stdbool.h>

struct Cpu cpu;
Inst insts[INSTS_NBR];
byte cpu_mem[CPU_MEM_SIZE];
bool nmi_flip = false;
int cpu_cycles;
int nbr_instr;

void fill_insts();

void cpu_init() {
	memset(&cpu, 0, sizeof(struct Cpu));
	memset(cpu_mem, 0, sizeof(cpu_mem));
	fill_insts();
	cpu_cycles = 0;
	nbr_instr = 0;
	cpu.sp = STACK_VECTOR;
}

void cpu_reset() {
	memset(&cpu, 0, sizeof(struct Cpu));
	cpu.sp = STACK_VECTOR;
}

Inst get_inst_info(enum instr_type instr) { return (insts[instr]); }

void cpu_start() {
	cpu.pc = RES_VECTOR;
	cpu.pc = fetch_word();
}

byte clock_cpu() {
	byte byte = fetch_byte();
	Inst instr = insts[byte];
	word data = instr.mode();
	instr.run(data);
	cpu_cycles += instr.cycles;
	nbr_instr += 1;
	return (instr.cycles);
}

void cpu_state() {
	printf("CPU State\n");
	printf("\tPC: %d\n", cpu.pc);
	printf("\tACC: %d\n", cpu.acc);
	printf("\tSP: %d\n", cpu.sp);
	printf("Processor Status\n");
	printf("\tNegative: %d\n", GET_BIT(cpu.ps, NF));
	printf("\tOverflow: %d\n", GET_BIT(cpu.ps, VF));
	printf("\tUnused: 0\n");
	printf("\tBreak: %d\n", GET_BIT(cpu.ps, BF));
	printf("\tDecimal: %d\n", GET_BIT(cpu.ps, DF));
	printf("\tInterrupt: %d\n", GET_BIT(cpu.ps, IF));
	printf("\tZero: %d\n", GET_BIT(cpu.ps, ZF));
	printf("\tCarry: %d\n", GET_BIT(cpu.ps, CF));
}

void cpu_write(word addr, byte value) {
	cpu_mem[addr] = value;
}

byte cpu_read(word addr) {
	return (cpu_mem[addr]);
}

byte fetch_byte() {
	return (bus_read(cpu.pc++));
}

word fetch_word() {
	byte lb = fetch_byte();
	byte hb = fetch_byte();
	return (MAKE_WORD(hb, lb));
}

// Address Modes

word impl_address_mode() { return (0); }

word imm_address_mode() { return (cpu.pc++); }

word acc_address_mode() { return (cpu.acc); }

word ind_address_mode() {
	word addr = fetch_word();
	return (MAKE_WORD(bus_read(addr + 1), bus_read(addr)));
}

word rel_address_mode() {
	byte offset = fetch_byte();
	return (offset);
}

word zp_address_mode() {
	byte addr = fetch_byte();
	return (addr);
}

word zp_x_address_mode() {
	byte addr = fetch_byte() + cpu.ix;
	return (addr);
}

word zp_y_address_mode() {
	byte addr = fetch_byte() + cpu.iy;
	return (addr);
}

word abs_address_mode() {
	word addr = fetch_word();
	return (addr);
}

word abs_x_address_mode() {
	word addr = fetch_word();

	if (GET_LOW_BYTE(addr) + cpu.ix > 0xFF) {
		cpu_cycles += 1;
	}
	return (addr + cpu.ix);
}

word abs_y_address_mode() {
	word addr = fetch_word();

	if (GET_LOW_BYTE(addr) + cpu.iy > 0xFF) {
		cpu_cycles += 1;
	}
	return (addr + cpu.iy);
}

word ind_x_address_mode() {
	byte ind_addr = fetch_byte();
	return (MAKE_WORD(bus_read(ind_addr + cpu.ix + 1), bus_read(ind_addr + cpu.ix)));
}

word ind_y_address_mode() {
	byte ind_addr = fetch_byte();
	byte lb = bus_read(ind_addr) + cpu.iy;
	byte hb = bus_read(ind_addr + 1);

	if (bus_read(ind_addr) + cpu.iy > 0xFF) {
		hb += 1;
		cpu_cycles += 1;
	}
	return (MAKE_WORD(hb, lb));
}

//

// Stack Operations

void push_byte(byte vl) {
	cpu_mem[MAKE_WORD(STACK_OFFSET, cpu.sp)] = vl;
	cpu.sp--;
}

void push_word(word vl) {
	byte hb = GET_HIGH_BYTE(vl);
	byte lb = GET_LOW_BYTE(vl);

	push_byte(hb);
	push_byte(lb);
}

byte pop_byte() {
	cpu.sp++;
	return (cpu_mem[MAKE_WORD(STACK_OFFSET, cpu.sp)]);
}

word pop_word() {
	byte lb = pop_byte();
	byte hb = pop_byte();
	return (MAKE_WORD(hb, lb));
}

//

// Processor Status Utilities

void infer_zero_flag(byte byte) {
	if (!byte) {
		SET_BIT(cpu.ps, ZF);
	} else {
		CLEAR_BIT(cpu.ps, ZF);
	}
}

void infer_negative_flag(byte byte) {
	if (GET_BIT(byte, 7)) {
		SET_BIT(cpu.ps, NF);
	} else {
		CLEAR_BIT(cpu.ps, NF);
	}
}

void infer_overflow_flag(byte byte1, byte byte2) {
	byte bit1 = GET_BIT(byte1, 7);
	byte bit2 = GET_BIT(byte2, 7);
	byte carry6 = GET_BIT(((byte1 & CARRY_6_MASK) + (byte2 & CARRY_6_MASK)), 7);

	if ((bit1 && bit2 && !carry6) || (!bit1 && !bit2 && carry6)) {
		SET_BIT(cpu.ps, VF);
	} else {
		CLEAR_BIT(cpu.ps, VF);
	}
}

void infer_carry_flag(word res) {
	if (res > 0xFF) {
		SET_BIT(cpu.ps, CF);
	} else {
		CLEAR_BIT(cpu.ps, CF);
	}
}

//

void nmi() {
	push_word(cpu.pc);
	CLEAR_BIT(cpu.ps, BF);
	push_byte(cpu.ps);
	SET_BIT(cpu.ps, IF);
	cpu.pc = MAKE_WORD(cpu_mem[NMI_VECTOR + 1], cpu_mem[NMI_VECTOR]);
}

// Instructions

// Logical

void And(word addr) {
	cpu.acc = cpu.acc & bus_read(addr);
	infer_zero_flag(cpu.acc);
	infer_negative_flag(cpu.acc);
}

void eor(word addr) {
	cpu.acc = cpu.acc ^ bus_read(addr);
	infer_zero_flag(cpu.acc);
	infer_negative_flag(cpu.acc);
}

void ora(word addr) {
	cpu.acc = cpu.acc | bus_read(addr);
	infer_zero_flag(cpu.acc);
	infer_negative_flag(cpu.acc);
}

void bit(word addr) {
	byte memvl = bus_read(addr);
	byte res = cpu.acc & memvl;

	infer_zero_flag(res);
	infer_negative_flag(memvl);
	if (GET_BIT(memvl, 6)) {
		SET_BIT(cpu.ps, VF);
	} else {
		CLEAR_BIT(cpu.ps, VF);
	}
}

//

// Arithmetic

void adc(word addr) {
	byte memvl = bus_read(addr);
	word res = cpu.acc + memvl + GET_BIT(cpu.ps, CF);
	byte old_acc = cpu.acc;

	cpu.acc = (byte)res;
	infer_carry_flag(res);
	infer_overflow_flag(old_acc, memvl);
	infer_zero_flag(cpu.acc);
	infer_negative_flag(cpu.acc);
}

void sbc(word addr) {
	byte memvl = bus_read(addr);
	byte vl = memvl + (1 ^ GET_BIT(cpu.ps, CF));
	byte res = cpu.acc - vl;
	byte old_acc = cpu.acc;

	cpu.acc = res;
	if (old_acc >= vl) {
		SET_BIT(cpu.ps, CF);
	} else {
		CLEAR_BIT(cpu.ps, CF);
	}
	infer_overflow_flag(old_acc, ~(memvl));
	infer_zero_flag(cpu.acc);
	infer_negative_flag(cpu.acc);
}

void cmp(word addr) {
	byte memvl = bus_read(addr);
	byte res = cpu.acc - memvl;

	if (cpu.acc >= memvl) {
		SET_BIT(cpu.ps, CF);
	} else {
		CLEAR_BIT(cpu.ps, CF);
	}
	infer_zero_flag(res);
	infer_negative_flag(res);
}

void cpx(word addr) {
	byte memvl = bus_read(addr);
	byte res = cpu.ix - memvl;

	if (cpu.ix >= memvl) {
		SET_BIT(cpu.ps, CF);
	} else {
		CLEAR_BIT(cpu.ps, CF);
	}
	infer_zero_flag(res);
	infer_negative_flag(res);
}

void cpy(word addr) {
	byte memvl = bus_read(addr);
	byte res = cpu.iy - memvl;

	if (cpu.iy >= memvl) {
		SET_BIT(cpu.ps, CF);
	} else {
		CLEAR_BIT(cpu.ps, CF);
	}
	infer_zero_flag(res);
	infer_negative_flag(res);
}

//

// Load/Store

void lda(word addr) {
	cpu.acc = bus_read(addr);
	infer_zero_flag(cpu.acc);
	infer_negative_flag(cpu.acc);
}

void ldy(word addr) {
	cpu.iy = bus_read(addr);
	infer_zero_flag(cpu.iy);
	infer_negative_flag(cpu.iy);
}

void ldx(word addr) {
	cpu.ix = bus_read(addr);
	infer_zero_flag(cpu.ix);
	infer_negative_flag(cpu.ix);
}

void sta(word addr) {
	bus_write(addr, cpu.acc);
}

void sty(word addr) {
	bus_write(addr, cpu.iy);
}

void stx(word addr) {
	bus_write(addr, cpu.ix);
}

//

// Transfers

void tax(word addr) {
	cpu.ix = cpu.acc;
	infer_zero_flag(cpu.ix);
	infer_negative_flag(cpu.ix);
}

void tay(word addr) {
	cpu.iy = cpu.acc;
	infer_zero_flag(cpu.iy);
	infer_negative_flag(cpu.iy);
}

void tsx(word addr) {
	cpu.ix = cpu.sp;
	infer_zero_flag(cpu.ix);
	infer_negative_flag(cpu.ix);
}

void txa(word addr) {
	cpu.acc = cpu.ix;
	infer_zero_flag(cpu.acc);
	infer_negative_flag(cpu.acc);
}

void txs(word addr) {
	cpu.sp = cpu.ix;
}

void tya(word addr) {
	cpu.acc = cpu.iy;
	infer_zero_flag(cpu.acc);
	infer_negative_flag(cpu.acc);
}

//

// Stack

void pha(word addr) {
	push_byte(cpu.acc);
}

void php(word addr) {
	push_byte(cpu.ps);
}

void pla(word addr) {
	cpu.acc = pop_byte();
	infer_zero_flag(cpu.acc);
	infer_negative_flag(cpu.acc);
}

void plp(word addr) {
	cpu.ps = pop_byte();
}

//

// Jumps

void jmp(word addr) {
	cpu.pc = addr;
}

void jsr(word addr) {
	push_word(cpu.pc - 1);
	cpu.pc = addr;
}

void rts(word addr) {
	cpu.pc = pop_word();
	cpu.pc++;
}

//

// Branches

void bcc(word rdisp) {
	int8_t rdisp_s = rdisp;

	if (!GET_BIT(cpu.ps, CF)) {
		byte before_branch_hb = GET_HIGH_BYTE(cpu.pc);
		cpu.pc += rdisp_s; 
		if (GET_HIGH_BYTE(cpu.pc) != before_branch_hb) {
			cpu_cycles += 1;
		}
		cpu_cycles += 1;
	}
}

void bcs(word rdisp) {
	if (GET_BIT(cpu.ps, CF)) {
		byte before_branch_hb = GET_HIGH_BYTE(cpu.pc);
		cpu.pc += (int8_t)rdisp;
		if (GET_HIGH_BYTE(cpu.pc) != before_branch_hb) {
			cpu_cycles += 1;
		}
		cpu_cycles += 1;
	}
}

void beq(word rdisp) {
	if (GET_BIT(cpu.ps, ZF)) {
		byte before_branch_hb = GET_HIGH_BYTE(cpu.pc);
		cpu.pc += (int8_t)rdisp;
		if (GET_HIGH_BYTE(cpu.pc) != before_branch_hb) {
			cpu_cycles += 1;
		}
		cpu_cycles += 1;
	}
}

void bmi(word rdisp) {
	if (GET_BIT(cpu.ps, NF)) {
		byte before_branch_hb = GET_HIGH_BYTE(cpu.pc);
		cpu.pc += (int8_t)rdisp;
		if (GET_HIGH_BYTE(cpu.pc) != before_branch_hb) {
			cpu_cycles += 1;
		}
		cpu_cycles += 1;
	}
}

void bne(word rdisp) {
	if (!GET_BIT(cpu.ps, ZF)) {
		byte before_branch_hb = GET_HIGH_BYTE(cpu.pc);
		cpu.pc += (int8_t)rdisp;
		if (GET_HIGH_BYTE(cpu.pc) != before_branch_hb) {
			cpu_cycles += 1;
		}
		cpu_cycles += 1;
	}
}

void bpl(word rdisp) {
	if (!GET_BIT(cpu.ps, NF)) {
		byte before_branch_hb = GET_HIGH_BYTE(cpu.pc);
		cpu.pc += (int8_t)rdisp;
		if (GET_HIGH_BYTE(cpu.pc) != before_branch_hb) {
			cpu_cycles += 1;
		}
		cpu_cycles += 1;
	}
}

void bvc(word rdisp) {
	if (!GET_BIT(cpu.ps, VF)) {
		byte before_branch_hb = GET_HIGH_BYTE(cpu.pc);
		cpu.pc += (int8_t)rdisp;
		if (GET_HIGH_BYTE(cpu.pc) != before_branch_hb) {
			cpu_cycles += 1;
		}
		cpu_cycles += 1;
	}
}

void bvs(word rdisp) {
	if (GET_BIT(cpu.ps, VF)) {
		byte before_branch_hb = GET_HIGH_BYTE(cpu.pc);
		cpu.pc += (int8_t)rdisp;
		if (GET_HIGH_BYTE(cpu.pc) != before_branch_hb) {
			cpu_cycles += 1;
		}
		cpu_cycles += 1;
	}
}

//

// Increment/Decrement

void inc(word addr) {
	bus_write(addr, bus_read(addr) + 1);
	infer_negative_flag(bus_read(addr));
	infer_zero_flag(bus_read(addr));
}

void inx(word addr) {
	cpu.ix += 1;
	infer_negative_flag(cpu.ix);
	infer_zero_flag(cpu.ix);
}

void iny(word addr) {
	cpu.iy += 1;
	infer_negative_flag(cpu.iy);
	infer_zero_flag(cpu.iy);
}

void dec(word addr) {
	bus_write(addr, bus_read(addr) - 1);
	infer_negative_flag(bus_read(addr));
	infer_zero_flag(bus_read(addr));
}

void dex(word addr) {
	cpu.ix -= 1;
	infer_negative_flag(cpu.ix);
	infer_zero_flag(cpu.ix);
}

void dey(word addr) {
	cpu.iy -= 1;
	infer_negative_flag(cpu.iy);
	infer_zero_flag(cpu.iy);
}

//

// Bit Shifts

void asl(word addr) {
	byte vl = bus_read(addr);
	
	if (GET_BIT(vl, 7)) {
		SET_BIT(cpu.ps, CF);
	} else {
		CLEAR_BIT(cpu.ps, CF);
	}
	vl = vl << 1;
	bus_write(addr, vl);
	infer_negative_flag(vl);
	infer_zero_flag(vl);
}

void asl_acc(word z) {
	if (GET_BIT(cpu.acc, 7)) {
		SET_BIT(cpu.ps, CF);
	} else {
		CLEAR_BIT(cpu.ps, CF);
	}
	cpu.acc = cpu.acc << 1;
	infer_negative_flag(cpu.acc);
	infer_zero_flag(cpu.acc);
}

void lsr(word addr) {
	byte vl = bus_read(addr);
	
	if (GET_BIT(vl, 0)) {
		SET_BIT(cpu.ps, CF);
	} else {
		CLEAR_BIT(cpu.ps, CF);
	}
	vl = vl >> 1;
	bus_write(addr, vl);
	infer_negative_flag(vl);
	infer_zero_flag(vl);
}

void lsr_acc(word z) {
	if (GET_BIT(cpu.acc, 0)) {
		SET_BIT(cpu.ps, CF);
	} else {
		CLEAR_BIT(cpu.ps, CF);
	}
	cpu.acc = cpu.acc >> 1;
	infer_negative_flag(cpu.acc);
	infer_zero_flag(cpu.acc);
}

void rol(word addr) {
	byte vl = bus_read(addr);
	byte res = (vl << 1);

	SET_BIT_TO(res, 0, GET_BIT(cpu.ps, CF));
	if (GET_BIT(vl, 7)) {
		SET_BIT(cpu.ps, CF);
	} else {
		CLEAR_BIT(cpu.ps, CF);
	}
	vl = res;
	bus_write(addr, vl);
	infer_negative_flag(vl);
	infer_zero_flag(vl);
}

void rol_acc(word z) {
	byte res = (cpu.acc << 1);

	SET_BIT_TO(res, 0, GET_BIT(cpu.ps, CF));
	if (GET_BIT(cpu.acc, 7)) {
		SET_BIT(cpu.ps, CF);
	} else {
		CLEAR_BIT(cpu.ps, CF);
	}
	cpu.acc = res;
	infer_negative_flag(cpu.acc);
	infer_zero_flag(cpu.acc);
}

void ror(word addr) {
	byte vl = bus_read(addr);
	byte res = (vl >> 1);

	SET_BIT_TO(res, 7, GET_BIT(cpu.ps, CF));
	if (GET_BIT(vl, 0)) {
		SET_BIT(cpu.ps, CF);
	} else {
		CLEAR_BIT(cpu.ps, CF);
	}
	vl = res;
	bus_write(addr, vl);
	infer_negative_flag(vl);
	infer_zero_flag(vl);

}

void ror_acc(word z) {
	byte res = (cpu.acc >> 1);

	SET_BIT_TO(res, 7, GET_BIT(cpu.ps, CF));
	if (GET_BIT(cpu.acc, 0)) {
		SET_BIT(cpu.ps, CF);
	} else {
		CLEAR_BIT(cpu.ps, CF);
	}
	cpu.acc = res;
	infer_negative_flag(cpu.acc);
	infer_zero_flag(cpu.acc);
}

//

// Status Flag

void clc(word np) {
	CLEAR_BIT(cpu.ps, CF);
}

void cld(word np) {
	CLEAR_BIT(cpu.ps, DF);
}

void cli(word np) {
	CLEAR_BIT(cpu.ps, IF);
}

void clv(word np) {
	CLEAR_BIT(cpu.ps, VF);
}

void sec(word np) {
	SET_BIT(cpu.ps, CF);
}

void sed(word np) {
	SET_BIT(cpu.ps, DF);
}

void sei(word np) {
	SET_BIT(cpu.ps, IF);
}

//

// System

void brk(word np) {
	push_word(cpu.pc + 1);
	SET_BIT(cpu.ps, BF);
	push_byte(cpu.ps);
	cpu.pc = IRQ_VECTOR;
	cpu.pc = fetch_word();
}

void nop(word np) {}

void rti(word np) {
	cpu.ps = pop_byte();
	cpu.pc = pop_word();
}

//

void fill_insts() {
	// Logical Instrucions

	insts[AND_IMM]	= (Inst){ And, imm_address_mode,	2, 2 };
	insts[AND_ABS]	= (Inst){ And, abs_address_mode,	3, 4 };
	insts[AND_ABSX]	= (Inst){ And, abs_x_address_mode,	3, 4 };
	insts[AND_ABSY]	= (Inst){ And, abs_y_address_mode,	3, 4 };
	insts[AND_ZPG]	= (Inst){ And, zp_address_mode,		2, 3 };
	insts[AND_ZPGX]	= (Inst){ And, zp_x_address_mode,	2, 4 };
    insts[AND_INDX]	= (Inst){ And, ind_x_address_mode,	2, 6 };
    insts[AND_INDY]	= (Inst){ And, ind_y_address_mode,	2, 5 };

    insts[EOR_IMM]	= (Inst){ eor, imm_address_mode,	2, 2 };
    insts[EOR_ABS]	= (Inst){ eor, abs_address_mode,	3, 4 };
    insts[EOR_ABSX]	= (Inst){ eor, abs_x_address_mode,	3, 4 };
    insts[EOR_ABSY]	= (Inst){ eor, abs_y_address_mode,	3, 4 };
    insts[EOR_ZPG]	= (Inst){ eor, zp_address_mode,		2, 3 };
    insts[EOR_ZPGX]	= (Inst){ eor, zp_x_address_mode,	2, 4 };
    insts[EOR_INDX]	= (Inst){ eor, ind_x_address_mode,	2, 6 };
    insts[EOR_INDY]	= (Inst){ eor, ind_y_address_mode,	2, 5 };

    insts[ORA_IMM]  = (Inst){ ora, imm_address_mode,	2, 2 };
    insts[ORA_ABS]  = (Inst){ ora, abs_address_mode,	3, 4 };
    insts[ORA_ABSX] = (Inst){ ora, abs_x_address_mode,	3, 4 };
    insts[ORA_ABSY] = (Inst){ ora, abs_y_address_mode,	3, 4 };
    insts[ORA_ZPG]  = (Inst){ ora, zp_address_mode,		2, 3 };
    insts[ORA_ZPGX] = (Inst){ ora, zp_x_address_mode,	2, 4 };
    insts[ORA_INDX] = (Inst){ ora, ind_x_address_mode,	2, 6 };
    insts[ORA_INDY] = (Inst){ ora, ind_y_address_mode,	2, 5 };

    insts[BIT_ABS] = (Inst){ bit, abs_address_mode,		3, 4 };
    insts[BIT_ZPG] = (Inst){ bit, zp_address_mode,		2, 3 };

	//

	// Arithmetic Instructions

	insts[ADC_IMM]	= (Inst){ adc, imm_address_mode,	2, 2 };
	insts[ADC_ABS]	= (Inst){ adc, abs_address_mode,	3, 4 };
	insts[ADC_ABSX]	= (Inst){ adc, abs_x_address_mode,	3, 4 };
	insts[ADC_ABSY]	= (Inst){ adc, abs_y_address_mode,	3, 4 };
	insts[ADC_ZPG]	= (Inst){ adc, zp_address_mode,		2, 3 };
	insts[ADC_ZPGX]	= (Inst){ adc, zp_x_address_mode,	2, 4 };
	insts[ADC_INDX]	= (Inst){ adc, ind_x_address_mode,	2, 6 };
	insts[ADC_INDY]	= (Inst){ adc, ind_y_address_mode,	2, 5 };

	insts[SBC_IMM]	= (Inst){ sbc, imm_address_mode,	2, 2 };
	insts[SBC_ABS]	= (Inst){ sbc, abs_address_mode,	3, 4 };
	insts[SBC_ABSX]	= (Inst){ sbc, abs_x_address_mode,	3, 4 };
	insts[SBC_ABSY]	= (Inst){ sbc, abs_y_address_mode,	3, 4 };
	insts[SBC_ZPG]	= (Inst){ sbc, zp_address_mode,		2, 3 };
	insts[SBC_ZPGX]	= (Inst){ sbc, zp_x_address_mode,	2, 4 };
	insts[SBC_INDX]	= (Inst){ sbc, ind_x_address_mode,	2, 6 };
	insts[SBC_INDY]	= (Inst){ sbc, ind_y_address_mode,	2, 5 };

	insts[CMP_IMM]	= (Inst){ cmp, imm_address_mode,	2, 2 };
	insts[CMP_ABS]	= (Inst){ cmp, abs_address_mode,	3, 4 };
	insts[CMP_ABSX]	= (Inst){ cmp, abs_x_address_mode,	3, 4 };
	insts[CMP_ABSY]	= (Inst){ cmp, abs_y_address_mode,	3, 4 };
	insts[CMP_ZPG]	= (Inst){ cmp, zp_address_mode,		2, 3 };
	insts[CMP_ZPGX]	= (Inst){ cmp, zp_x_address_mode,	2, 4 };
	insts[CMP_INDX]	= (Inst){ cmp, ind_x_address_mode,	2, 6 };
	insts[CMP_INDY]	= (Inst){ cmp, ind_y_address_mode,	2, 5 };

	insts[CPX_IMM] = (Inst){ cpx, imm_address_mode,		2, 2 };
	insts[CPX_ABS] = (Inst){ cpx, abs_address_mode,		3, 4 };
	insts[CPX_ZPG] = (Inst){ cpx, zp_address_mode,		2, 3 };

	insts[CPY_IMM] = (Inst){ cpy, imm_address_mode,		2, 2 };
	insts[CPY_ABS] = (Inst){ cpy, abs_address_mode,		3, 4 };
	insts[CPY_ZPG] = (Inst){ cpy, zp_address_mode,		2, 3 };

	//

	// Load/Store Instructions

    insts[LDA_IMM]  = (Inst){ lda, imm_address_mode,	2, 2 };
    insts[LDA_ABS]  = (Inst){ lda, abs_address_mode,	3, 4 };
    insts[LDA_ABSX] = (Inst){ lda, abs_x_address_mode,	3, 4 };
    insts[LDA_ABSY] = (Inst){ lda, abs_y_address_mode,	3, 4 };
    insts[LDA_ZPG]  = (Inst){ lda, zp_address_mode,		2, 3 };
    insts[LDA_ZPGX] = (Inst){ lda, zp_x_address_mode,	2, 4 };
    insts[LDA_INDX] = (Inst){ lda, ind_x_address_mode,	2, 6 };
    insts[LDA_INDY] = (Inst){ lda, ind_y_address_mode,	2, 5 };

    insts[LDX_IMM]  = (Inst){ ldx, imm_address_mode,	2, 2 };
    insts[LDX_ABS]  = (Inst){ ldx, abs_address_mode,	3, 4 };
    insts[LDX_ABSY] = (Inst){ ldx, abs_y_address_mode,	3, 4 };
    insts[LDX_ZPG]  = (Inst){ ldx, zp_address_mode,		2, 3 };
    insts[LDX_ZPGY] = (Inst){ ldx, zp_y_address_mode,	2, 4 };

    insts[LDY_IMM]  = (Inst){ ldy, imm_address_mode,	2, 2 };
    insts[LDY_ABS]  = (Inst){ ldy, abs_address_mode,	3, 4 };
    insts[LDY_ABSX] = (Inst){ ldy, abs_x_address_mode,	3, 4 };
    insts[LDY_ZPG]  = (Inst){ ldy, zp_address_mode,		2, 3 };
    insts[LDY_ZPGX] = (Inst){ ldy, zp_x_address_mode,	2, 4 };

    insts[STA_ABS]  = (Inst){ sta, abs_address_mode,	3, 4 };
    insts[STA_ABSX] = (Inst){ sta, abs_x_address_mode,	3, 5 };
    insts[STA_ABSY] = (Inst){ sta, abs_y_address_mode,	3, 5 };
    insts[STA_ZPG]  = (Inst){ sta, zp_address_mode,		2, 3 };
    insts[STA_ZPGX] = (Inst){ sta, zp_x_address_mode,	2, 4 };
    insts[STA_INDX] = (Inst){ sta, ind_x_address_mode,	2, 6 };
    insts[STA_INDY] = (Inst){ sta, ind_y_address_mode,	2, 6 };

    insts[STX_ABS]  = (Inst){ stx, abs_address_mode,	3, 4 };
    insts[STX_ZPG]  = (Inst){ stx, zp_address_mode,		2, 3 };
    insts[STX_ZPGY] = (Inst){ stx, zp_y_address_mode,	2, 4 };

    insts[STY_ABS]  = (Inst){ sty, abs_address_mode,	3, 4 };
    insts[STY_ZPG]  = (Inst){ sty, zp_address_mode,		2, 3 };
    insts[STY_ZPGX]	= (Inst){ sty, zp_x_address_mode,	2, 4 };

	//

    // Transfers (implied)

    insts[TAX_IMPL]	= (Inst){ tax, impl_address_mode,	1, 2 };
    insts[TAY_IMPL]	= (Inst){ tay, impl_address_mode,	1, 2 };
    insts[TXA_IMPL]	= (Inst){ txa, impl_address_mode,	1, 2 };
    insts[TYA_IMPL]	= (Inst){ tya, impl_address_mode,	1, 2 };
    insts[TSX_IMPL]	= (Inst){ tsx, impl_address_mode,	1, 2 };
    insts[TXS_IMPL]	= (Inst){ txs, impl_address_mode,	1, 2 };

	//

    // Stack

    insts[PHA_IMPL]	= (Inst){ pha, impl_address_mode,	1, 3 };
    insts[PHP_IMPL]	= (Inst){ php, impl_address_mode,	1, 3 };
    insts[PLA_IMPL]	= (Inst){ pla, impl_address_mode,	1, 4 };
    insts[PLP_IMPL]	= (Inst){ plp, impl_address_mode,	1, 4 };

	//

    // Jumps

    insts[JMP_ABS]	= (Inst){ jmp, abs_address_mode,	3, 3 };
    insts[JMP_IND]	= (Inst){ jmp, ind_address_mode,	3, 5 };
    insts[JSR_ABS]	= (Inst){ jsr, abs_address_mode,	3, 6 };
    insts[RTI_IMPL] = (Inst){ rti, impl_address_mode,	1, 6 };
    insts[RTS_IMPL]	= (Inst){ rts, impl_address_mode,	1, 6 };

	//

    // Branches (relative)

    insts[BCC_RLT]	= (Inst){ bcc, rel_address_mode,	2, 2 };
    insts[BCS_RLT]	= (Inst){ bcs, rel_address_mode,	2, 2 };
    insts[BEQ_RLT]	= (Inst){ beq, rel_address_mode,	2, 2 };
    insts[BNE_RLT]	= (Inst){ bne, rel_address_mode,	2, 2 };
    insts[BMI_RLT]	= (Inst){ bmi, rel_address_mode,	2, 2 };
    insts[BPL_RLT]	= (Inst){ bpl, rel_address_mode,	2, 2 };
    insts[BVC_RLT]	= (Inst){ bvc, rel_address_mode,	2, 2 };
    insts[BVS_RLT]	= (Inst){ bvs, rel_address_mode,	2, 2 };
	
	//

	// Increment/Decrement

    insts[INC_ABS]	= (Inst){ inc, abs_address_mode,	3, 6 };
    insts[INC_ABSX]	= (Inst){ inc, abs_x_address_mode,	3, 7 };
    insts[INC_ZPG]	= (Inst){ inc, zp_address_mode,	2, 5 };
    insts[INC_ZPGX]	= (Inst){ inc, zp_x_address_mode,	2, 6 };

    insts[INX_IMPL]	= (Inst){ inx, impl_address_mode,	1, 2 };
    insts[INY_IMPL]	= (Inst){ iny, impl_address_mode,	1, 2 };

    insts[DEC_ABS]	= (Inst){ dec, abs_address_mode,	3, 6 };
    insts[DEC_ABSX]	= (Inst){ dec, abs_x_address_mode,	3, 7 };
	insts[DEC_ZPG]	= (Inst){ dec, zp_address_mode,		2, 5 };
    insts[DEC_ZPGX]	= (Inst){ dec, zp_x_address_mode,	2, 6 };

    insts[DEX_IMPL]	= (Inst){ dex, impl_address_mode,	1, 2 };
    insts[DEY_IMPL]	= (Inst){ dey, impl_address_mode,	1, 2 };

	//

	// Bit Shifts

	insts[ASL_ACC]	= (Inst){ asl_acc, impl_address_mode,	1, 2 };
	insts[ASL_ABS]	= (Inst){ asl, abs_address_mode,		3, 6 };
	insts[ASL_ABSX]	= (Inst){ asl, abs_x_address_mode,		3, 7 };
	insts[ASL_ZPG]	= (Inst){ asl, zp_address_mode,			2, 5 };
	insts[ASL_ZPGX]	= (Inst){ asl, zp_x_address_mode,		2, 6 };

	insts[LSR_ACC]	= (Inst){ lsr_acc, impl_address_mode,	1, 2 };
	insts[LSR_ABS]	= (Inst){ lsr, abs_address_mode,		3, 6 };
	insts[LSR_ABSX]	= (Inst){ lsr, abs_x_address_mode,		3, 7 };
	insts[LSR_ZPG]	= (Inst){ lsr, zp_address_mode,			2, 5 };
	insts[LSR_ZPGX]	= (Inst){ lsr, zp_x_address_mode,		2, 6 };

	insts[ROL_ACC]	= (Inst){ rol_acc, impl_address_mode,	1, 2 };
	insts[ROL_ABS]	= (Inst){ rol, abs_address_mode,		3, 6 };
	insts[ROL_ABSX]	= (Inst){ rol, abs_x_address_mode,		3, 7 };
	insts[ROL_ZPG]	= (Inst){ rol, zp_address_mode,			2, 5 };
	insts[ROL_ZPGX]	= (Inst){ rol, zp_x_address_mode,		2, 6 };

	insts[ROR_ACC]	= (Inst){ ror_acc, impl_address_mode,	1, 2 };
	insts[ROR_ABS]	= (Inst){ ror, abs_address_mode,		3, 6 };
	insts[ROR_ABSX]	= (Inst){ ror, abs_x_address_mode,		3, 7 };
	insts[ROR_ZPG]	= (Inst){ ror, zp_address_mode,			2, 5 };
	insts[ROR_ZPGX]	= (Inst){ ror, zp_x_address_mode,		2, 6 };

	//

    // Status Flags

    insts[CLC_IMPL]	= (Inst){ clc, impl_address_mode,	1, 2 };
    insts[SEC_IMPL]	= (Inst){ sec, impl_address_mode,	1, 2 };
    insts[CLI_IMPL]	= (Inst){ cli, impl_address_mode,	1, 2 };
    insts[SEI_IMPL]	= (Inst){ sei, impl_address_mode,	1, 2 };
    insts[CLV_IMPL]	= (Inst){ clv, impl_address_mode,	1, 2 };
    insts[CLD_IMPL]	= (Inst){ cld, impl_address_mode,	1, 2 };
    insts[SED_IMPL]	= (Inst){ sed, impl_address_mode,	1, 2 };

	//

    // System

    insts[NOP_IMPL]	= (Inst){ nop, impl_address_mode,	1, 2 };
    insts[BRK_IMPL]	= (Inst){ brk, impl_address_mode,	1, 7 };

	//
}

