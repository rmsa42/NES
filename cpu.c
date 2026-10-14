#include "cpu.h"
#include "bus.h"
#include <string.h>
#include <stdio.h>
#include <stdbool.h>

struct cpu cpu;
inst insts[INSTS_NBR];
byte cpu_mem[CPU_MEM_SIZE];
bool nmi_flip = false;
int cpu_cycles;
int nbr_instr;

void fill_insts();

void cpu_init() {
	memset(&cpu, 0, sizeof(struct cpu));
	memset(cpu_mem, 0, sizeof(cpu_mem));
	fill_insts();
	cpu_cycles = 0;
	nbr_instr = 0;
	cpu.sp = STACK_VECTOR;
}

void cpu_reset() {
	memset(&cpu, 0, sizeof(struct cpu));
	cpu.sp = STACK_VECTOR;
}

inst get_inst_info(instr_type instr) { return (insts[instr]); }

void cpu_start() {
	cpu.pc = RES_VECTOR;
	cpu.pc = fetch_word();
}

byte clock_cpu() {
	byte byte = fetch_byte();
	inst instr = insts[byte];
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
	printf("\tNegative: %d\n", check_bit(cpu.ps, NF));
	printf("\tOverflow: %d\n", check_bit(cpu.ps, VF));
	printf("\tUnused: 0\n");
	printf("\tBreak: %d\n", check_bit(cpu.ps, BF));
	printf("\tDecimal: %d\n", check_bit(cpu.ps, DF));
	printf("\tInterrupt: %d\n", check_bit(cpu.ps, IF));
	printf("\tZero: %d\n", check_bit(cpu.ps, ZF));
	printf("\tCarry: %d\n", check_bit(cpu.ps, CF));
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
	return (make_word(hb, lb));
}

// Address Modes

word impl_address_mode() { return (0); }

word imm_address_mode() { return (cpu.pc++); }

word acc_address_mode() { return (cpu.acc); }

word ind_address_mode() {
	word addr = fetch_word();
	return (make_word(bus_read(addr + 1), bus_read(addr)));
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

	if (low_byte(addr) + cpu.ix > 0xFF) {
		cpu_cycles += 1;
	}
	return (addr + cpu.ix);
}

word abs_y_address_mode() {
	word addr = fetch_word();

	if (low_byte(addr) + cpu.iy > 0xFF) {
		printf("Cross: %x %x\n", low_byte(addr), cpu.iy);
		cpu_cycles += 1;
	}
	return (addr + cpu.iy);
}

word ind_x_address_mode() {
	byte ind_addr = fetch_byte();
	return (make_word(bus_read(ind_addr + cpu.ix + 1), bus_read(ind_addr + cpu.ix)));
}

word ind_y_address_mode() {
	byte ind_addr = fetch_byte();
	byte lb = bus_read(ind_addr) + cpu.iy;
	byte hb = bus_read(ind_addr + 1);

	if (bus_read(ind_addr) + cpu.iy > 0xFF) {
		hb += 1;
		cpu_cycles += 1;
	}
	return (make_word(hb, lb));
}

//

// Stack Operations

void push_byte(byte vl) {
	cpu_mem[make_word(STACK_OFFSET, cpu.sp)] = vl;
	cpu.sp--;
}

void push_word(word vl) {
	byte hb = high_byte(vl);
	byte lb = low_byte(vl);

	push_byte(hb);
	push_byte(lb);
}

byte pop_byte() {
	cpu.sp++;
	return (cpu_mem[make_word(STACK_OFFSET, cpu.sp)]);
}

word pop_word() {
	byte lb = pop_byte();
	byte hb = pop_byte();
	return (make_word(hb, lb));
}

//

// Processor Status Utilities

void infer_zero_flag(byte byte) {
	if (!byte) {
		set_bit(cpu.ps, ZF);
	} else {
		clear_bit(cpu.ps, ZF);
	}
}

void infer_negative_flag(byte byte) {
	if (check_bit(byte, 7)) {
		set_bit(cpu.ps, NF);
	} else {
		clear_bit(cpu.ps, NF);
	}
}

void infer_overflow_flag(byte byte1, byte byte2) {
	byte bit1 = check_bit(byte1, 7);
	byte bit2 = check_bit(byte2, 7);
	byte carry6 = check_bit(((byte1 & CARRY_6_MASK) + (byte2 & CARRY_6_MASK)), 7);

	if ((bit1 && bit2 && !carry6) || (!bit1 && !bit2 && carry6)) {
		set_bit(cpu.ps, VF);
	} else {
		clear_bit(cpu.ps, VF);
	}
}

void infer_carry_flag(word res) {
	if (res > 0xFF) {
		set_bit(cpu.ps, CF);
	} else {
		clear_bit(cpu.ps, CF);
	}
}

//

void nmi() {
	push_word(cpu.pc);
	clear_bit(cpu.ps, BF);
	push_byte(cpu.ps);
	set_bit(cpu.ps, IF);
	cpu.pc = make_word(cpu_mem[NMI_VECTOR + 1], cpu_mem[NMI_VECTOR]);
	printf("NMI handler called\n");
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
	if (check_bit(memvl, 6)) {
		set_bit(cpu.ps, VF);
	} else {
		clear_bit(cpu.ps, VF);
	}
}

//

// Arithmetic

void adc(word addr) {
	byte memvl = bus_read(addr);
	word res = cpu.acc + memvl + check_bit(cpu.ps, CF);
	byte old_acc = cpu.acc;

	cpu.acc = (byte)res;
	infer_carry_flag(res);
	infer_overflow_flag(old_acc, memvl);
	infer_zero_flag(cpu.acc);
	infer_negative_flag(cpu.acc);
}

void sbc(word addr) {
	byte memvl = bus_read(addr);
	byte vl = memvl + (1 ^ check_bit(cpu.ps, CF));
	byte res = cpu.acc - vl;
	byte old_acc = cpu.acc;

	cpu.acc = res;
	if (old_acc >= vl) {
		set_bit(cpu.ps, CF);
	} else {
		clear_bit(cpu.ps, CF);
	}
	infer_overflow_flag(old_acc, ~(memvl));
	infer_zero_flag(cpu.acc);
	infer_negative_flag(cpu.acc);
}

void cmp(word addr) {
	byte memvl = bus_read(addr);
	byte res = cpu.acc - memvl;

	if (cpu.acc >= memvl) {
		set_bit(cpu.ps, CF);
	} else {
		clear_bit(cpu.ps, CF);
	}
	infer_zero_flag(res);
	infer_negative_flag(res);
}

void cpx(word addr) {
	byte memvl = bus_read(addr);
	byte res = cpu.ix - memvl;

	if (cpu.ix >= memvl) {
		set_bit(cpu.ps, CF);
	} else {
		clear_bit(cpu.ps, CF);
	}
	infer_zero_flag(res);
	infer_negative_flag(res);
}

void cpy(word addr) {
	byte memvl = bus_read(addr);
	byte res = cpu.iy - memvl;

	if (cpu.iy >= memvl) {
		set_bit(cpu.ps, CF);
	} else {
		clear_bit(cpu.ps, CF);
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

	if (!check_bit(cpu.ps, CF)) {
		byte before_branch_hb = high_byte(cpu.pc);
		cpu.pc += rdisp_s; 
		if (high_byte(cpu.pc) != before_branch_hb) {
			cpu_cycles += 1;
		}
		cpu_cycles += 1;
	}
}

void bcs(word rdisp) {
	if (check_bit(cpu.ps, CF)) {
		byte before_branch_hb = high_byte(cpu.pc);
		cpu.pc += (int8_t)rdisp;
		if (high_byte(cpu.pc) != before_branch_hb) {
			cpu_cycles += 1;
		}
		cpu_cycles += 1;
	}
}

void beq(word rdisp) {
	if (check_bit(cpu.ps, ZF)) {
		byte before_branch_hb = high_byte(cpu.pc);
		cpu.pc += (int8_t)rdisp;
		if (high_byte(cpu.pc) != before_branch_hb) {
			printf("CrossPage\n");
			cpu_cycles += 1;
		}
		cpu_cycles += 1;
	}
}

void bmi(word rdisp) {
	if (check_bit(cpu.ps, NF)) {
		byte before_branch_hb = high_byte(cpu.pc);
		cpu.pc += (int8_t)rdisp;
		if (high_byte(cpu.pc) != before_branch_hb) {
			cpu_cycles += 1;
		}
		cpu_cycles += 1;
	}
}

void bne(word rdisp) {
	if (!check_bit(cpu.ps, ZF)) {
		byte before_branch_hb = high_byte(cpu.pc);
		cpu.pc += (int8_t)rdisp;
		if (high_byte(cpu.pc) != before_branch_hb) {
			cpu_cycles += 1;
		}
		cpu_cycles += 1;
	}
}

void bpl(word rdisp) {
	if (!check_bit(cpu.ps, NF)) {
		byte before_branch_hb = high_byte(cpu.pc);
		cpu.pc += (int8_t)rdisp;
		if (high_byte(cpu.pc) != before_branch_hb) {
			cpu_cycles += 1;
		}
		cpu_cycles += 1;
	}
}

void bvc(word rdisp) {
	if (!check_bit(cpu.ps, VF)) {
		byte before_branch_hb = high_byte(cpu.pc);
		cpu.pc += (int8_t)rdisp;
		if (high_byte(cpu.pc) != before_branch_hb) {
			cpu_cycles += 1;
		}
		cpu_cycles += 1;
	}
}

void bvs(word rdisp) {
	if (check_bit(cpu.ps, VF)) {
		byte before_branch_hb = high_byte(cpu.pc);
		cpu.pc += (int8_t)rdisp;
		if (high_byte(cpu.pc) != before_branch_hb) {
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
	
	if (check_bit(vl, 7)) {
		set_bit(cpu.ps, CF);
	} else {
		clear_bit(cpu.ps, CF);
	}
	vl = vl << 1;
	bus_write(addr, vl);
	infer_negative_flag(vl);
	infer_zero_flag(vl);
}

void asl_acc(word z) {
	if (check_bit(cpu.acc, 7)) {
		set_bit(cpu.ps, CF);
	} else {
		clear_bit(cpu.ps, CF);
	}
	cpu.acc = cpu.acc << 1;
	infer_negative_flag(cpu.acc);
	infer_zero_flag(cpu.acc);
}

void lsr(word addr) {
	byte vl = bus_read(addr);
	
	if (check_bit(vl, 0)) {
		set_bit(cpu.ps, CF);
	} else {
		clear_bit(cpu.ps, CF);
	}
	vl = vl >> 1;
	bus_write(addr, vl);
	infer_negative_flag(vl);
	infer_zero_flag(vl);
}

void lsr_acc(word z) {
	if (check_bit(cpu.acc, 0)) {
		set_bit(cpu.ps, CF);
	} else {
		clear_bit(cpu.ps, CF);
	}
	cpu.acc = cpu.acc >> 1;
	infer_negative_flag(cpu.acc);
	infer_zero_flag(cpu.acc);
}

void rol(word addr) {
	byte vl = bus_read(addr);
	byte res = (vl << 1);

	set_bit_to(res, 0, check_bit(cpu.ps, CF));
	if (check_bit(vl, 7)) {
		set_bit(cpu.ps, CF);
	} else {
		clear_bit(cpu.ps, CF);
	}
	vl = res;
	bus_write(addr, vl);
	infer_negative_flag(vl);
	infer_zero_flag(vl);
}

void rol_acc(word z) {
	byte res = (cpu.acc << 1);

	set_bit_to(res, 0, check_bit(cpu.ps, CF));
	if (check_bit(cpu.acc, 7)) {
		set_bit(cpu.ps, CF);
	} else {
		clear_bit(cpu.ps, CF);
	}
	cpu.acc = res;
	infer_negative_flag(cpu.acc);
	infer_zero_flag(cpu.acc);
}

void ror(word addr) {
	byte vl = bus_read(addr);
	byte res = (vl >> 1);

	set_bit_to(res, 7, check_bit(cpu.ps, CF));
	if (check_bit(vl, 0)) {
		set_bit(cpu.ps, CF);
	} else {
		clear_bit(cpu.ps, CF);
	}
	vl = res;
	bus_write(addr, vl);
	infer_negative_flag(vl);
	infer_zero_flag(vl);

}

void ror_acc(word z) {
	byte res = (cpu.acc >> 1);

	set_bit_to(res, 7, check_bit(cpu.ps, CF));
	if (check_bit(cpu.acc, 0)) {
		set_bit(cpu.ps, CF);
	} else {
		clear_bit(cpu.ps, CF);
	}
	cpu.acc = res;
	infer_negative_flag(cpu.acc);
	infer_zero_flag(cpu.acc);
}

//

// Status Flag

void clc(word np) {
	clear_bit(cpu.ps, CF);
}

void cld(word np) {
	clear_bit(cpu.ps, DF);
}

void cli(word np) {
	clear_bit(cpu.ps, IF);
}

void clv(word np) {
	clear_bit(cpu.ps, VF);
}

void sec(word np) {
	set_bit(cpu.ps, CF);
}

void sed(word np) {
	set_bit(cpu.ps, DF);
}

void sei(word np) {
	set_bit(cpu.ps, IF);
}

//

// System

void brk(word np) {
	push_word(cpu.pc);
	set_bit(cpu.ps, BF);
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

	insts[AND_IMM]	= (inst){ And, imm_address_mode,	2, 2 };
	insts[AND_ABS]	= (inst){ And, abs_address_mode,	3, 4 };
	insts[AND_ABSX]	= (inst){ And, abs_x_address_mode,	3, 4 };
	insts[AND_ABSY]	= (inst){ And, abs_y_address_mode,	3, 4 };
	insts[AND_ZPG]	= (inst){ And, zp_address_mode,		2, 3 };
	insts[AND_ZPGX]	= (inst){ And, zp_x_address_mode,	2, 4 };
    insts[AND_INDX]	= (inst){ And, ind_x_address_mode,	2, 6 };
    insts[AND_INDY]	= (inst){ And, ind_y_address_mode,	2, 5 };

    insts[EOR_IMM]	= (inst){ eor, imm_address_mode,	2, 2 };
    insts[EOR_ABS]	= (inst){ eor, abs_address_mode,	3, 4 };
    insts[EOR_ABSX]	= (inst){ eor, abs_x_address_mode,	3, 4 };
    insts[EOR_ABSY]	= (inst){ eor, abs_y_address_mode,	3, 4 };
    insts[EOR_ZPG]	= (inst){ eor, zp_address_mode,		2, 3 };
    insts[EOR_ZPGX]	= (inst){ eor, zp_x_address_mode,	2, 4 };
    insts[EOR_INDX]	= (inst){ eor, ind_x_address_mode,	2, 6 };
    insts[EOR_INDY]	= (inst){ eor, ind_y_address_mode,	2, 5 };

    insts[ORA_IMM]  = (inst){ ora, imm_address_mode,	2, 2 };
    insts[ORA_ABS]  = (inst){ ora, abs_address_mode,	3, 4 };
    insts[ORA_ABSX] = (inst){ ora, abs_x_address_mode,	3, 4 };
    insts[ORA_ABSY] = (inst){ ora, abs_y_address_mode,	3, 4 };
    insts[ORA_ZPG]  = (inst){ ora, zp_address_mode,		2, 3 };
    insts[ORA_ZPGX] = (inst){ ora, zp_x_address_mode,	2, 4 };
    insts[ORA_INDX] = (inst){ ora, zp_x_address_mode,	2, 6 };
    insts[ORA_INDY] = (inst){ ora, zp_x_address_mode,	2, 5 };

    insts[BIT_ABS] = (inst){ bit, abs_address_mode,		3, 4 };
    insts[BIT_ZPG] = (inst){ bit, zp_address_mode,		2, 3 };

	//

	// Arithmetic Instructions

	insts[ADC_IMM]	= (inst){ adc, imm_address_mode,	2, 2 };
	insts[ADC_ABS]	= (inst){ adc, abs_address_mode,	3, 4 };
	insts[ADC_ABSX]	= (inst){ adc, abs_x_address_mode,	3, 4 };
	insts[ADC_ABSY]	= (inst){ adc, abs_y_address_mode,	3, 4 };
	insts[ADC_ZPG]	= (inst){ adc, zp_address_mode,		2, 3 };
	insts[ADC_ZPGX]	= (inst){ adc, zp_x_address_mode,	2, 4 };
	insts[ADC_INDX]	= (inst){ adc, ind_x_address_mode,	2, 6 };
	insts[ADC_INDY]	= (inst){ adc, ind_y_address_mode,	2, 5 };

	insts[SBC_IMM]	= (inst){ sbc, imm_address_mode,	2, 2 };
	insts[SBC_ABS]	= (inst){ sbc, abs_address_mode,	3, 4 };
	insts[SBC_ABSX]	= (inst){ sbc, abs_x_address_mode,	3, 4 };
	insts[SBC_ABSY]	= (inst){ sbc, abs_y_address_mode,	3, 4 };
	insts[SBC_ZPG]	= (inst){ sbc, zp_address_mode,		2, 3 };
	insts[SBC_ZPGX]	= (inst){ sbc, zp_x_address_mode,	2, 4 };
	insts[SBC_INDX]	= (inst){ sbc, ind_x_address_mode,	2, 6 };
	insts[SBC_INDY]	= (inst){ sbc, ind_y_address_mode,	2, 5 };

	insts[CMP_IMM]	= (inst){ cmp, imm_address_mode,	2, 2 };
	insts[CMP_ABS]	= (inst){ cmp, abs_address_mode,	3, 4 };
	insts[CMP_ABSX]	= (inst){ cmp, abs_x_address_mode,	3, 4 };
	insts[CMP_ABSY]	= (inst){ cmp, abs_y_address_mode,	3, 4 };
	insts[CMP_ZPG]	= (inst){ cmp, zp_address_mode,		2, 3 };
	insts[CMP_ZPGX]	= (inst){ cmp, zp_x_address_mode,	2, 4 };
	insts[CMP_INDX]	= (inst){ cmp, ind_x_address_mode,	2, 6 };
	insts[CMP_INDY]	= (inst){ cmp, ind_y_address_mode,	2, 5 };

	insts[CPX_IMM] = (inst){ cpx, imm_address_mode,		2, 2 };
	insts[CPX_ABS] = (inst){ cpx, abs_address_mode,		3, 4 };
	insts[CPX_ZPG] = (inst){ cpx, zp_address_mode,		2, 3 };

	insts[CPY_IMM] = (inst){ cpy, imm_address_mode,		2, 2 };
	insts[CPY_ABS] = (inst){ cpy, abs_address_mode,		3, 4 };
	insts[CPY_ZPG] = (inst){ cpy, zp_address_mode,		2, 3 };

	//

	// Load/Store Instructions

    insts[LDA_IMM]  = (inst){ lda, imm_address_mode,	2, 2 };
    insts[LDA_ABS]  = (inst){ lda, abs_address_mode,	3, 4 };
    insts[LDA_ABSX] = (inst){ lda, abs_x_address_mode,	3, 4 };
    insts[LDA_ABSY] = (inst){ lda, abs_y_address_mode,	3, 4 };
    insts[LDA_ZPG]  = (inst){ lda, zp_address_mode,		2, 3 };
    insts[LDA_ZPGX] = (inst){ lda, zp_x_address_mode,	2, 4 };
    insts[LDA_INDX] = (inst){ lda, ind_x_address_mode,	2, 6 };
    insts[LDA_INDY] = (inst){ lda, ind_y_address_mode,	2, 5 };

    insts[LDX_IMM]  = (inst){ ldx, imm_address_mode,	2, 2 };
    insts[LDX_ABS]  = (inst){ ldx, abs_address_mode,	3, 4 };
    insts[LDX_ABSY] = (inst){ ldx, abs_y_address_mode,	3, 4 };
    insts[LDX_ZPG]  = (inst){ ldx, zp_address_mode,		2, 3 };
    insts[LDX_ZPGY] = (inst){ ldx, zp_y_address_mode,	2, 4 };

    insts[LDY_IMM]  = (inst){ ldy, imm_address_mode,	2, 2 };
    insts[LDY_ABS]  = (inst){ ldy, abs_address_mode,	3, 4 };
    insts[LDY_ABSX] = (inst){ ldy, abs_x_address_mode,	3, 4 };
    insts[LDY_ZPG]  = (inst){ ldy, zp_address_mode,		2, 3 };
    insts[LDY_ZPGX] = (inst){ ldy, zp_x_address_mode,	2, 4 };

    insts[STA_ABS]  = (inst){ sta, abs_address_mode,	3, 4 };
    insts[STA_ABSX] = (inst){ sta, abs_x_address_mode,	3, 5 };
    insts[STA_ABSY] = (inst){ sta, abs_y_address_mode,	3, 5 };
    insts[STA_ZPG]  = (inst){ sta, zp_address_mode,		2, 3 };
    insts[STA_ZPGX] = (inst){ sta, zp_x_address_mode,	2, 4 };
    insts[STA_INDX] = (inst){ sta, ind_x_address_mode,	2, 6 };
    insts[STA_INDY] = (inst){ sta, ind_y_address_mode,	2, 6 };

    insts[STX_ABS]  = (inst){ stx, abs_address_mode,	3, 4 };
    insts[STX_ZPG]  = (inst){ stx, zp_address_mode,		2, 3 };
    insts[STX_ZPGY] = (inst){ stx, zp_y_address_mode,	2, 4 };

    insts[STY_ABS]  = (inst){ sty, abs_address_mode,	3, 4 };
    insts[STY_ZPG]  = (inst){ sty, zp_address_mode,		2, 3 };
    insts[STY_ZPGX]	= (inst){ sty, zp_x_address_mode,	2, 4 };

	//

    // Transfers (implied)

    insts[TAX_IMPL]	= (inst){ tax, impl_address_mode,	1, 2 };
    insts[TAY_IMPL]	= (inst){ tay, impl_address_mode,	1, 2 };
    insts[TXA_IMPL]	= (inst){ txa, impl_address_mode,	1, 2 };
    insts[TYA_IMPL]	= (inst){ tya, impl_address_mode,	1, 2 };
    insts[TSX_IMPL]	= (inst){ tsx, impl_address_mode,	1, 2 };
    insts[TXS_IMPL]	= (inst){ txs, impl_address_mode,	1, 2 };

	//

    // Stack

    insts[PHA_IMPL]	= (inst){ pha, impl_address_mode,	1, 3 };
    insts[PHP_IMPL]	= (inst){ php, impl_address_mode,	1, 3 };
    insts[PLA_IMPL]	= (inst){ pla, impl_address_mode,	1, 4 };
    insts[PLP_IMPL]	= (inst){ plp, impl_address_mode,	1, 4 };

	//

    // Jumps

    insts[JMP_ABS]	= (inst){ jmp, abs_address_mode,	3, 3 };
    insts[JMP_IND]	= (inst){ jmp, ind_address_mode,	3, 5 };
    insts[JSR_ABS]	= (inst){ jsr, abs_address_mode,	3, 6 };
    insts[RTI_IMPL] = (inst){ rti, impl_address_mode,	1, 6 };
    insts[RTS_IMPL]	= (inst){ rts, impl_address_mode,	1, 6 };

	//

    // Branches (relative)

    insts[BCC_RLT]	= (inst){ bcc, rel_address_mode,	2, 2 };
    insts[BCS_RLT]	= (inst){ bcs, rel_address_mode,	2, 2 };
    insts[BEQ_RLT]	= (inst){ beq, rel_address_mode,	2, 2 };
    insts[BNE_RLT]	= (inst){ bne, rel_address_mode,	2, 2 };
    insts[BMI_RLT]	= (inst){ bmi, rel_address_mode,	2, 2 };
    insts[BPL_RLT]	= (inst){ bpl, rel_address_mode,	2, 2 };
    insts[BVC_RLT]	= (inst){ bvc, rel_address_mode,	2, 2 };
    insts[BVS_RLT]	= (inst){ bvs, rel_address_mode,	2, 2 };
	
	//

	// Increment/Decrement

    insts[INC_ABS]	= (inst){ inc, abs_address_mode,	3, 6 };
    insts[INC_ABSX]	= (inst){ inc, abs_x_address_mode,	3, 7 };
    insts[INC_ZPG]	= (inst){ inc, zp_address_mode,	2, 5 };
    insts[INC_ZPGX]	= (inst){ inc, zp_x_address_mode,	2, 6 };

    insts[INX_IMPL]	= (inst){ inx, impl_address_mode,	1, 2 };
    insts[INY_IMPL]	= (inst){ iny, impl_address_mode,	1, 2 };

    insts[DEC_ABS]	= (inst){ dec, abs_address_mode,	3, 6 };
    insts[DEC_ABSX]	= (inst){ dec, abs_x_address_mode,	3, 7 };
	insts[DEC_ZPG]	= (inst){ dec, zp_address_mode,		2, 5 };
    insts[DEC_ZPGX]	= (inst){ dec, zp_x_address_mode,	2, 6 };

    insts[DEX_IMPL]	= (inst){ dex, impl_address_mode,	1, 2 };
    insts[DEY_IMPL]	= (inst){ dey, impl_address_mode,	1, 2 };

	//

	// Bit Shifts

	insts[ASL_ACC]	= (inst){ asl_acc, impl_address_mode,	1, 2 };
	insts[ASL_ABS]	= (inst){ asl, abs_address_mode,		3, 6 };
	insts[ASL_ABSX]	= (inst){ asl, abs_x_address_mode,		3, 7 };
	insts[ASL_ZPG]	= (inst){ asl, zp_address_mode,			2, 5 };
	insts[ASL_ZPGX]	= (inst){ asl, zp_x_address_mode,		2, 6 };

	insts[LSR_ACC]	= (inst){ lsr_acc, impl_address_mode,	1, 2 };
	insts[LSR_ABS]	= (inst){ lsr, abs_address_mode,		3, 6 };
	insts[LSR_ABSX]	= (inst){ lsr, abs_x_address_mode,		3, 7 };
	insts[LSR_ZPG]	= (inst){ lsr, zp_address_mode,			2, 5 };
	insts[LSR_ZPGX]	= (inst){ lsr, zp_x_address_mode,		2, 6 };

	insts[ROL_ACC]	= (inst){ rol_acc, impl_address_mode,	1, 2 };
	insts[ROL_ABS]	= (inst){ rol, abs_address_mode,		3, 6 };
	insts[ROL_ABSX]	= (inst){ rol, abs_x_address_mode,		3, 7 };
	insts[ROL_ZPG]	= (inst){ rol, zp_address_mode,			2, 5 };
	insts[ROL_ZPGX]	= (inst){ rol, zp_x_address_mode,		2, 6 };

	insts[ROR_ACC]	= (inst){ ror_acc, impl_address_mode,	1, 2 };
	insts[ROR_ABS]	= (inst){ ror, abs_address_mode,		3, 6 };
	insts[ROR_ABSX]	= (inst){ ror, abs_x_address_mode,		3, 7 };
	insts[ROR_ZPG]	= (inst){ ror, zp_address_mode,			2, 5 };
	insts[ROR_ZPGX]	= (inst){ ror, zp_x_address_mode,		2, 6 };

	//

    // Status Flags

    insts[CLC_IMPL]	= (inst){ clc, impl_address_mode,	1, 2 };
    insts[SEC_IMPL]	= (inst){ sec, impl_address_mode,	1, 2 };
    insts[CLI_IMPL]	= (inst){ cli, impl_address_mode,	1, 2 };
    insts[SEI_IMPL]	= (inst){ sei, impl_address_mode,	1, 2 };
    insts[CLV_IMPL]	= (inst){ clv, impl_address_mode,	1, 2 };
    insts[CLD_IMPL]	= (inst){ cld, impl_address_mode,	1, 2 };
    insts[SED_IMPL]	= (inst){ sed, impl_address_mode,	1, 2 };

	//

    // System

    insts[NOP_IMPL]	= (inst){ nop, impl_address_mode,	1, 2 };
    insts[BRK_IMPL]	= (inst){ brk, impl_address_mode,	1, 7 };

	//
}

