#pragma once

#include "utils.h"

#ifdef __cplusplus
extern "C" {
#endif

#define NMI_VECTOR 0xFFFA
#define RES_VECTOR 0xFFFC
#define IRQ_VECTOR 0xFFFE
#define STACK_VECTOR 0xFF
#define STACK_OFFSET 0x01

#define CPU_FREQ 1789773 // Hz

#define CARRY_6_MASK 0x7F

struct cpu {
	word pc;
	byte sp;
	byte acc;
	byte ix;
	byte iy;
	byte ps;
};

typedef enum {
	CF,
	ZF,
	IF,
	DF,
	BF,
	Unused,
	VF,
	NF
} status;

typedef struct inst {
	void (*run)(word);
	word (*mode)();
} inst;

void cpu_init();
void cpu_reset();
void clock_cpu();
void cpu_state();
byte fetch_byte();
word fetch_word();

typedef enum {
// Load/Store Instrutions
	LDA_IMM = 0xA9,
	LDA_ZPG = 0xA5,
	LDA_ZPGX = 0xB5,
	LDA_ABS = 0xAD,
	LDA_ABSX = 0xBD,
	LDA_ABSY = 0xB9,
	LDA_INDX = 0xA1,
	LDA_INDY = 0xB1,
	LDY_IMM = 0xA0,
	LDY_ZPG = 0xA4,
	LDY_ZPGX = 0xB4,
	LDY_ABS = 0xAC,
	LDY_ABSX = 0xBC,
	LDX_IMM = 0xA2,
	LDX_ZPG = 0xA6,
	LDX_ZPGY = 0xB6,
	LDX_ABS = 0xAE,
	LDX_ABSY = 0xBE,
	STA_ZPG = 0x85,
	STA_ZPGX = 0x95,
	STA_ABS = 0x8D,
	STA_ABSX = 0x9D,
	STA_ABSY = 0x99,
	STA_INDX = 0x81,
	STA_INDY = 0x91,
	STX_ZPG = 0x86,
	STX_ZPGY = 0x96,
	STX_ABS = 0x8E,
	STY_ZPG = 0x84,
	STY_ZPGX = 0x94,
	STY_ABS = 0x8C,
//
// Transfer Instructions
	TAX_IMPL = 0xAA,
	TAY_IMPL = 0xA8,
	TSX_IMPL = 0xBA,
	TXS_IMPL = 0x9A,
	TXA_IMPL = 0x8A,
	TYA_IMPL = 0x98,
//
// Stack Instructions
	PHA_IMPL = 0x48,
	PHP_IMPL = 0x08,
	PLA_IMPL = 0x68,
	PLP_IMPL = 0x28,
//
// Logical Instructions
	AND_IMM = 0x29,
	AND_ZPG = 0x25,
	AND_ZPGX = 0x35,
	AND_ABS = 0x2D,
	AND_ABSX = 0x3D,
	AND_ABSY = 0x39,
	AND_INDX = 0x21,
	AND_INDY = 0x31,
	EOR_IMM = 0x49,
	EOR_ZPG = 0x45,
	EOR_ZPGX = 0x55,
	EOR_ABS = 0x4D,
	EOR_ABSX = 0x5D,
	EOR_ABSY = 0x59,
	EOR_INDX = 0x41,
	EOR_INDY = 0x51,
	ORA_IMM = 0x09,
	ORA_ZPG = 0x05,
	ORA_ZPGX = 0x15,
	ORA_ABS = 0x0D,
	ORA_ABSX = 0x1D,
	ORA_ABSY = 0x19,
	ORA_INDX = 0x01,
	ORA_INDY = 0x11,
	BIT_ZPG = 0x24,
	BIT_ABS = 0x2C,
//
// Arithmetic Instructions
	ADC_IMM = 0x69,
	ADC_ZPG = 0x65,
	ADC_ZPGX = 0x75,
	ADC_ABS = 0x6D,
	ADC_ABSX = 0x7D,
	ADC_ABSY = 0x79,
	ADC_INDX = 0x61,
	ADC_INDY = 0x71,
	SBC_IMM = 0xE9,
	SBC_ZPG = 0xE5,
	SBC_ZPGX = 0xF5,
	SBC_ABS = 0xED,
	SBC_ABSX = 0xFD,
	SBC_ABSY = 0xF9,
	SBC_INDX = 0xE1,
	SBC_INDY = 0xF1,
	CMP_IMM = 0xC9,
	CMP_ZPG = 0xC5,
	CMP_ZPGX = 0xD5,
	CMP_ABS = 0xCD,
	CMP_ABSX = 0xDD,
	CMP_ABSY = 0xD9,
	CMP_INDX = 0xC1,
	CMP_INDY = 0xD1,
	CPX_IMM = 0xE0,
	CPX_ZPG = 0xE4,
	CPX_ABS = 0xEC,
	CPY_IMM = 0xC0,
	CPY_ZPG = 0xC4,
	CPY_ABS = 0xCC,
//
// Increments/Decrements Instructions
	INC_ZPG = 0xE6,
	INC_ZPGX = 0xF6,
	INC_ABS = 0xEE,
	INC_ABSX = 0xFE,
	INY_IMPL = 0xC8,
	INX_IMPL = 0xE8,
	DEC_ZPG = 0xC6,
	DEC_ZPGX = 0xD6,
	DEC_ABS = 0xCE,
	DEC_ABSX = 0xDE,
	DEX_IMPL = 0xCA,
	DEY_IMPL = 0x88,
//
// Bit Shifts Instructions
	ASL_ACC = 0x0A,
	ASL_ZPG = 0x06,
	ASL_ZPGX = 0x16,
	ASL_ABS = 0x0E,
	ASL_ABSX = 0x1E,
	LSR_ACC = 0x4A,
	LSR_ZPG = 0x46,
	LSR_ZPGX = 0x56,
	LSR_ABS = 0x4E,
	LSR_ABSX = 0x5E,
	ROL_ACC = 0x2A,
	ROL_ZPG = 0x26,
	ROL_ZPGX = 0x36,
	ROL_ABS = 0x2E,
	ROL_ABSX = 0x3E,
	ROR_ACC = 0x6A,
	ROR_ZPG = 0x66,
	ROR_ZPGX = 0x76,
	ROR_ABS = 0x6E,
	ROR_ABSX = 0x7E,
//
// Jump/Return Instructions
	JMP_ABS = 0x4C,
	JMP_IND = 0x6C,
	JSR_ABS = 0x20,
	RTS_IMPL = 0x60,
//
// Branches Instructions
	BCC_RLT = 0x90,
	BCS_RLT = 0xB0,
	BMI_RLT = 0x30,
	BNE_RLT = 0xD0,
	BPL_RLT = 0x10,
	BVC_RLT = 0x50,
	BVS_RLT = 0x70,
	BEQ_RLT = 0xF0,
//
// Status Flags Instructins
	CLC_IMPL = 0x18,
	CLD_IMPL = 0xD8,
	CLI_IMPL = 0x58,
	CLV_IMPL = 0xB8,
	SEC_IMPL = 0x38,
	SED_IMPL = 0xF8,
	SEI_IMPL = 0x78,
//
// System Instructions
	NOP_IMPL = 0xEA,
	RTI_IMPL = 0x40,
	BRK_IMPL = 0x00,
//
} instr_type;

#ifdef __cplusplus
}
#endif

