#include "cpu.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

static struct cpu *cpu;
inst insts[0xFF];
void fill_insts();

int cpu_instance() {
	if (cpu != NULL) {
		return (1);
	}
	cpu = malloc(1 * sizeof(struct cpu));

	if (!cpu) { return (1); }
	memset(cpu, 0, sizeof(struct cpu));
	memset(memory, 0, sizeof(memory));
	fill_insts();
	cpu->sp = 0xFF;
	return (0);
}

void cpu_state() {
	printf("CPU State\n");
	printf("\tACC: %d\n", cpu->acc);
	printf("\tSP: %d\n", cpu->sp);
	printf("Processor Status\n");
	printf("\tNegative: %d\n", get_ps_bit(cpu->ps, NF));
	printf("\tOverflow: %d\n", get_ps_bit(cpu->ps, VF));
	printf("\tUnused: 0\n");
	printf("\tBreak: %d\n", get_ps_bit(cpu->ps, BF));
	printf("\tDecimal: %d\n", get_ps_bit(cpu->ps, DF));
	printf("\tInterrupt: %d\n", get_ps_bit(cpu->ps, IF));
	printf("\tZero: %d\n", get_ps_bit(cpu->ps, ZF));
	printf("\tCarry: %d\n", get_ps_bit(cpu->ps, CF));
}

byte fetch_byte() {
	return (memory[cpu->pc++]);
}

word fetch_word() {
	byte lb = fetch_byte();
	byte hb = fetch_byte();
	return (make_word(hb, lb));
}

// Address Modes

word impl_address_mode() { return (0); }

word imm_address_mode() { return (cpu->pc++); }

word acc_address_mode() { return (cpu->acc); }

word ind_address_mode() {
	word addr = fetch_word();
	return (make_word(memory[addr + 1], memory[addr]));
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
	byte addr = fetch_byte() + cpu->ix;
	return (addr);
}

word zp_y_address_mode() {
	byte addr = fetch_byte() + cpu->iy;
	return (addr);
}

word abs_address_mode() {
	word addr = fetch_word();
	return (addr);
}

word abs_x_address_mode() {
	word addr = fetch_word() + cpu->ix;
	return (addr);
}

word abs_y_address_mode() {
	word addr = fetch_word() + cpu->iy;
	return (addr);
}

//

// Stack Operations

void push_byte(byte vl) {
	memory[make_word(STACK_OFFSET, cpu->sp)] = vl;
	cpu->sp--;
}

void push_word(word vl) {
	byte hb = high_byte(vl);
	byte lb = low_byte(vl);

	push_byte(hb);
	push_byte(lb);
}

byte pop_byte() {
	byte ret = memory[make_word(STACK_OFFSET, cpu->sp)];

	cpu->sp++;
	return (ret);
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
		set_ps_bit(cpu->ps, ZF);
	} else {
		clear_ps_bit(cpu->ps, ZF);
	}
}

void infer_negative_flag(byte byte) {
	if (check_bit(byte, 7)) {
		set_ps_bit(cpu->ps, NF);
	} else {
		clear_ps_bit(cpu->ps, NF);
	}
}

void infer_overflow_flag(byte byte1, byte byte2) {
	byte bit1 = check_bit(byte1, 7);
	byte bit2 = check_bit(byte2, 7);
	byte carry6 = check_bit(((byte1 & CARRY_6_MASK) + (byte2 & CARRY_6_MASK)), 7);

	if ((bit1 && bit2 && !carry6) || (!bit1 && !bit2 && carry6)) {
		set_ps_bit(cpu->ps, VF);
	} else {
		clear_ps_bit(cpu->ps, VF);
	}
}

void infer_carry_flag(word res) {
	if (res > 0xFF) {
		set_ps_bit(cpu->ps, CF);
	} else {
		clear_ps_bit(cpu->ps, CF);
	}
}

//

// Instructions

// Logical

void And(word addr) {
	cpu->acc = cpu->acc & memory[addr];
	infer_zero_flag(cpu->acc);
	infer_negative_flag(cpu->acc);
}

void eor(word addr) {
	cpu->acc = cpu->acc ^ memory[addr];
	infer_zero_flag(cpu->acc);
	infer_negative_flag(cpu->acc);
}

void ora(word addr) {
	cpu->acc = cpu->acc | memory[addr];
	infer_zero_flag(cpu->acc);
	infer_negative_flag(cpu->acc);
}

void bit(word addr) { // Tenho de verificar esta função
	byte vl = memory[addr];

	//set_ps_bit(cpu->ps, ZF, ((cpu->acc & vl) == 0));
	//set_ps_bit(cpu->ps, VF, check_bit(vl, VF));
	//set_ps_bit(cpu->ps, NF, check_bit(vl, NF));
}

//

// Arithmetic

void adc(word addr) {
	byte vl = memory[addr];
	word res = cpu->acc + vl + get_ps_bit(cpu->ps, CF);
	byte old_acc = cpu->acc;

	cpu->acc = (byte)res;
	infer_carry_flag(res);
	infer_overflow_flag(old_acc, vl);
	infer_zero_flag(cpu->acc);
	infer_negative_flag(cpu->acc);
}

void sbc(word addr) {
	byte vl = memory[addr] + (1 ^ get_ps_bit(cpu->ps, CF));
	byte res = cpu->acc - vl;
	byte old_acc = cpu->acc;

	cpu->acc = res;
	if (old_acc >= vl) {
		set_ps_bit(cpu->ps, CF);
	} else {
		clear_ps_bit(cpu->ps, CF);
	}
	infer_overflow_flag(old_acc, vl);
	infer_zero_flag(cpu->acc);
	infer_negative_flag(cpu->acc);
}

void cmp(word addr) {
	byte vl = memory[addr];
	byte res = cpu->acc - vl;

	if (cpu->acc > vl) {
		set_ps_bit(cpu->ps, CF);
	} else {
		clear_ps_bit(cpu->ps, CF);
	}
	infer_zero_flag(res);
	infer_negative_flag(res);
}

//

// Load/Store

void lda(word addr) {
	cpu->acc = memory[addr];
	infer_zero_flag(cpu->acc);
	infer_negative_flag(cpu->acc);
}

void ldy(word addr) {
	cpu->iy = memory[addr];
	infer_zero_flag(cpu->iy);
	infer_negative_flag(cpu->iy);
}

void ldx(word addr) {
	cpu->ix = memory[addr];
	infer_zero_flag(cpu->ix);
	infer_negative_flag(cpu->ix);
}

void sta(word addr) {
	memory[addr] = cpu->acc;
}

void sty(word addr) {
	memory[addr] = cpu->iy;
}

void stx(word addr) {
	memory[addr] = cpu->ix;
}

// Transfers

void tax(word addr) {
	cpu->ix = cpu->acc;
	infer_zero_flag(cpu->ix);
	infer_negative_flag(cpu->ix);
}

void tay(word addr) {
	cpu->iy = cpu->acc;
	infer_zero_flag(cpu->iy);
	infer_negative_flag(cpu->iy);
}

void tsx(word addr) {
	cpu->ix = cpu->sp;
	infer_zero_flag(cpu->ix);
	infer_negative_flag(cpu->ix);
}

void txa(word addr) {
	cpu->acc = cpu->ix;
	infer_zero_flag(cpu->acc);
	infer_negative_flag(cpu->acc);
}

void txs(word addr) {
	cpu->sp = cpu->ix;
}

void tya(word addr) {
	cpu->acc = cpu->iy;
	infer_zero_flag(cpu->acc);
	infer_negative_flag(cpu->acc);
}

// Stack

void pha(word addr) {
	push_byte(cpu->acc);
}

void php(word addr) {
	push_byte(cpu->ps);
}

void pla(word addr) {
	cpu->acc = pop_byte();
	infer_zero_flag(cpu->acc);
	infer_negative_flag(cpu->acc);
}

void plp(word addr) {
	cpu->ps = pop_byte();
}

// Jumps
void jmp(word addr) {
	cpu->pc = addr;
}

void jsr(word addr) {
	push_word(cpu->pc);
	cpu->pc = addr;
}

void rts(word addr) {
	cpu->pc = pop_word();
}

// Branches
void bcc(word rdisp) {
	if (!get_ps_bit(cpu->ps, CF)) {
		cpu->pc += rdisp; 
	}
}

void bcs(word rdisp) {
	if (get_ps_bit(cpu->ps, CF)) {
		cpu->pc += rdisp;
	}
}

void beq(word rdisp) {
	if (get_ps_bit(cpu->ps, ZF)) {
		cpu->pc += rdisp;
	}
}

void bmi(word rdisp) {
	if (get_ps_bit(cpu->ps, NF)) {
		cpu->pc += rdisp;
	}
}

void bne(word rdisp) {
	if (!get_ps_bit(cpu->ps, ZF)) {
		cpu->pc += rdisp;
	}
}

void bpl(word rdisp) {
	if (!get_ps_bit(cpu->ps, NF)) {
		cpu->pc += rdisp;
	}
}

void bvc(word rdisp) {
	if (!get_ps_bit(cpu->ps, VF)) {
		cpu->pc += rdisp;
	}
}

void bvs(word rdisp) {
	if (get_ps_bit(cpu->ps, VF)) {
		cpu->pc += rdisp;
	}
}

// Status Flag Change
void clc(word np) {
	clear_ps_bit(cpu->ps, CF);
}

void cld(word np) {
	clear_ps_bit(cpu->ps, DF);
}

void cli(word np) {
	clear_ps_bit(cpu->ps, IF);
}

void clv(word np) {
	clear_ps_bit(cpu->ps, VF);
}

void sec(word np) {
	set_ps_bit(cpu->ps, CF);
}

void sed(word np) {
	set_ps_bit(cpu->ps, DF);
}

void sei(word np) {
	set_ps_bit(cpu->ps, IF);
}

// System Functions

void brk(word np) {
	push_word(cpu->pc);
	set_ps_bit(cpu->ps, BF);
	push_byte(cpu->ps);
	cpu->pc = IRQ_VECTOR;
	cpu->pc = fetch_word();
}

void nop(word np) {}

void rti(word np) {
	cpu->ps = pop_byte();
	cpu->pc = pop_word();
}

void fill_insts() {
	// Logical Instrucions

	insts[AND_IMM] = (inst){ And, imm_address_mode };
	insts[AND_ABS] = (inst){ And, abs_address_mode };
	insts[AND_ABSX] = (inst){ And, abs_x_address_mode };
	insts[AND_ABSY] = (inst){ And, abs_y_address_mode };
	insts[AND_ZPG] = (inst){ And, zp_address_mode };
	insts[AND_ZPGX] = (inst){ And, zp_x_address_mode };
    // insts[AND_INDX] = (inst){ and, ind_x_address_mode };
    // insts[AND_INDY] = (inst){ and, ind_y_address_mode };

    insts[EOR_IMM]  = (inst){ eor, imm_address_mode };
    insts[EOR_ZPG]  = (inst){ eor, zp_address_mode };
    insts[EOR_ZPGX] = (inst){ eor, zp_x_address_mode };
    insts[EOR_ABS]  = (inst){ eor, abs_address_mode };
    insts[EOR_ABSX] = (inst){ eor, abs_x_address_mode };
    insts[EOR_ABSY] = (inst){ eor, abs_y_address_mode };

    insts[ORA_IMM]  = (inst){ ora, imm_address_mode };
    insts[ORA_ZPG]  = (inst){ ora, zp_address_mode };
    insts[ORA_ZPGX] = (inst){ ora, zp_x_address_mode };
    insts[ORA_ABS]  = (inst){ ora, abs_address_mode };
    insts[ORA_ABSX] = (inst){ ora, abs_x_address_mode };
    insts[ORA_ABSY] = (inst){ ora, abs_y_address_mode };

    insts[BIT_ZPG] = (inst){ bit, zp_address_mode };
    insts[BIT_ABS] = (inst){ bit, abs_address_mode };

	//

	// Arithmetic Instructions
	insts[ADC_IMM] = (inst){ adc, imm_address_mode };
	insts[ADC_ZPG] = (inst){ adc, zp_address_mode };
	insts[ADC_ZPGX] = (inst){ adc, zp_x_address_mode };
	insts[ADC_ABS] = (inst){ adc, abs_address_mode };
	insts[ADC_ABSX] = (inst){ adc, abs_x_address_mode };
	insts[ADC_ABSY] = (inst){ adc, abs_y_address_mode };
	//insts[ADC_INDX] = (inst){ adc, ind_x_address_mode };
	//insts[ADC_INDY] = (inst){ adc, ind_y_address_mode };

	insts[SBC_IMM] = (inst){ sbc, imm_address_mode };
	insts[SBC_ZPG] = (inst){ sbc, zp_address_mode };
	insts[SBC_ZPGX] = (inst){ sbc, zp_x_address_mode };
	insts[SBC_ABS] = (inst){ sbc, abs_address_mode };
	insts[SBC_ABSX] = (inst){ sbc, abs_x_address_mode };
	insts[SBC_ABSY] = (inst){ sbc, abs_y_address_mode };
	//insts[SBC_INDX] = (inst){ sbc, ind_x_address_mode };
	//insts[SBC_INDY] = (inst){ sbc, ind_y_address_mode };

	insts[CMP_IMM] = (inst){ cmp, imm_address_mode };
	insts[CMP_ZPG] = (inst){ cmp, zp_address_mode };
	insts[CMP_ZPGX] = (inst){ cmp, zp_x_address_mode };
	insts[CMP_ABS] = (inst){ cmp, abs_address_mode };
	insts[CMP_ABSX] = (inst){ cmp, abs_x_address_mode };
	insts[CMP_ABSY] = (inst){ cmp, abs_y_address_mode };
	//insts[CMP_INDX] = (inst){ cmp, ind_x_address_mode };
	//insts[CMP_INDY] = (inst){ cmp, ind_y_address_mode };

	// Load/Store Instructions

    insts[LDA_IMM]  = (inst){ lda, imm_address_mode };
    insts[LDA_ZPG]  = (inst){ lda, zp_address_mode };
    insts[LDA_ZPGX] = (inst){ lda, zp_x_address_mode };
    insts[LDA_ABS]  = (inst){ lda, abs_address_mode };
    insts[LDA_ABSX] = (inst){ lda, abs_x_address_mode };
    insts[LDA_ABSY] = (inst){ lda, abs_y_address_mode };

    insts[LDY_IMM]  = (inst){ ldy, imm_address_mode };
    insts[LDY_ZPG]  = (inst){ ldy, zp_address_mode };
    insts[LDY_ZPGX] = (inst){ ldy, zp_x_address_mode };
    insts[LDY_ABS]  = (inst){ ldy, abs_address_mode };
    insts[LDY_ABSX] = (inst){ ldy, abs_x_address_mode };

    insts[LDX_IMM]  = (inst){ ldx, imm_address_mode };
    insts[LDX_ZPG]  = (inst){ ldx, zp_address_mode };
    insts[LDX_ZPGY] = (inst){ ldx, zp_y_address_mode };
    insts[LDX_ABS]  = (inst){ ldx, abs_address_mode };
    insts[LDX_ABSY] = (inst){ ldx, abs_y_address_mode };

    insts[STA_ZPG]  = (inst){ sta, zp_address_mode };
    insts[STA_ZPGX] = (inst){ sta, zp_x_address_mode };
    insts[STA_ABS]  = (inst){ sta, abs_address_mode };
    insts[STA_ABSX] = (inst){ sta, abs_x_address_mode };
    insts[STA_ABSY] = (inst){ sta, abs_y_address_mode };

    insts[STX_ZPG]  = (inst){ stx, zp_address_mode };
    insts[STX_ZPGY] = (inst){ stx, zp_y_address_mode };
    insts[STX_ABS]  = (inst){ stx, abs_address_mode };

    insts[STY_ZPG]  = (inst){ sty, zp_address_mode };
    insts[STY_ZPGX] = (inst){ sty, zp_x_address_mode };
    insts[STY_ABS]  = (inst){ sty, abs_address_mode };

	//

    // Transfers (implied)

    insts[TAX_IMPL] = (inst){ tax, impl_address_mode };
    insts[TAY_IMPL] = (inst){ tay, impl_address_mode };
    insts[TXA_IMPL] = (inst){ txa, impl_address_mode };
    insts[TYA_IMPL] = (inst){ tya, impl_address_mode };
    insts[TSX_IMPL] = (inst){ tsx, impl_address_mode };
    insts[TXS_IMPL] = (inst){ txs, impl_address_mode };

	//

    // Stack
    insts[PHA_IMPL] = (inst){ pha, impl_address_mode };
    insts[PHP_IMPL] = (inst){ php, impl_address_mode };
    insts[PLA_IMPL] = (inst){ pla, impl_address_mode };
    insts[PLP_IMPL] = (inst){ plp, impl_address_mode };

    // Jumps

    insts[JMP_ABS] = (inst){ jmp, abs_address_mode };
    insts[JMP_IND] = (inst){ jmp, ind_address_mode };
    insts[JSR_ABS] = (inst){ jsr, abs_address_mode };
    insts[RTS_IMPL]= (inst){ rts, impl_address_mode };

	//

    // Branches (relative)

    insts[BCC_RLT] = (inst){ bcc, rel_address_mode };
    insts[BCS_RLT] = (inst){ bcs, rel_address_mode };
    insts[BEQ_RLT] = (inst){ beq, rel_address_mode };
    insts[BNE_RLT] = (inst){ bne, rel_address_mode };
    insts[BMI_RLT] = (inst){ bmi, rel_address_mode };
    insts[BPL_RLT] = (inst){ bpl, rel_address_mode };
    insts[BVC_RLT] = (inst){ bvc, rel_address_mode };
    insts[BVS_RLT] = (inst){ bvs, rel_address_mode };
	
	//

    // Flags

    insts[CLC_IMPL] = (inst){ clc, impl_address_mode };
    insts[SEC_IMPL] = (inst){ sec, impl_address_mode };
    insts[CLI_IMPL] = (inst){ cli, impl_address_mode };
    insts[SEI_IMPL] = (inst){ sei, impl_address_mode };
    insts[CLV_IMPL] = (inst){ clv, impl_address_mode };
    insts[CLD_IMPL] = (inst){ cld, impl_address_mode };
    insts[SED_IMPL] = (inst){ sed, impl_address_mode };

	//

    // System

    insts[NOP_IMPL] = (inst){ nop, impl_address_mode };
    insts[BRK_IMPL] = (inst){ brk, impl_address_mode };
    insts[RTI_IMPL] = (inst){ rti, impl_address_mode };

	//
}

