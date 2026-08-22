#include "cpu.h"
#include <stdio.h>
#include <stdbool.h>

extern inst insts[0xFF];
extern byte memory[0xFFFF];

int main() {
	int res;
	bool run = true;

	res = cpu_instance();
	if (res) {
		printf("Error Init CPU\n");
		return (res);
	}

	int i = 0;
	memory[i++] = LDA_IMM;
	memory[i++] = 0xFF;
	memory[i++] = SEC_IMPL;
	memory[i++] = ROR_ACC;

	printf("Running CPU\n");
	for (int i = 0; i < 3; i++) {
		byte byte = fetch_byte();

		// Get the instruction
		inst instr = insts[byte];

		// Apply the address mode
		word vls = instr.mode();

		// Run the instruction
		instr.run(vls);
		
		printf("Inst %d runned\n", byte);
		cpu_state();
	}
}
