#pragma once

#include "gtest/gtest.h"
#include "cpu.h"

extern inst insts[0xFF];
extern struct cpu cpu;
extern byte cpu_mem[0xFFFF];

class cpu_test : public testing::Test {
protected:

	int i;

	cpu_test() {
		cpu_init();
		i = 0;
	}
	~cpu_test() {}

	void SetUp() override {
		cpu_reset();
		i = 0;
	}

	// Need to make this function for absolute addresses
	void set_inst(byte inst, word vl) {
		struct inst instr = insts[inst];

		cpu_mem[i++] = inst;
		cpu_mem[i++] = vl;
		fetch_byte();
		instr.run(instr.mode());
		cpu_state();
	}

	void set_abs_inst(byte inst, word addr, byte vl) {
		struct inst instr = insts[inst];

		cpu_mem[i++] = inst;
		cpu_mem[i++] = (byte)addr;
		cpu_mem[i++] = (byte)(addr >> 8);
		cpu_mem[addr] = vl;
		fetch_byte();
		instr.run(instr.mode());
		cpu_state();
	}
};
