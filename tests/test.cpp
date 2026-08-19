#include "gtest/gtest.h"
#include "cpu.h"

extern inst insts[0xFF];
extern struct cpu *cpu;
extern byte memory[0xFFFF];

class cpu_test : public testing::Test {
protected:

	int i;

	cpu_test() {
		cpu_instance();
		i = 0;
	}
	~cpu_test() {}

	void SetUp() override {
		cpu_reset();
		i = 0;
	}

	void set_inst(byte inst, word vl) {
		struct inst instr = insts[inst];

		memory[i++] = inst;
		memory[i++] = vl;
		fetch_byte();
		instr.run(instr.mode());
	}
};

TEST_F(cpu_test, ADC) {
	set_inst(LDA_IMM, 0x01);
	set_inst(ADC_IMM, 0x01);
	EXPECT_EQ(cpu->acc, 0x02);

	set_inst(ADC_IMM, 0xFF);
	EXPECT_EQ(cpu->acc, (byte)(0x02 + 0xFF));
	EXPECT_EQ(get_ps_bit(cpu->ps, CF), 1);
}

int main() {
	testing::InitGoogleTest();
	return (RUN_ALL_TESTS());
}

