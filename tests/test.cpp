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

	// Need to make this function for absolute addresses
	void set_inst(byte inst, word vl) {
		struct inst instr = insts[inst];

		memory[i++] = inst;
		memory[i++] = vl;
		fetch_byte();
		instr.run(instr.mode());
		cpu_state();
	}
};

TEST_F(cpu_test, ADC_normal) {
	set_inst(ADC_IMM, 0x01);
	EXPECT_EQ(cpu->acc, 0x01);
	EXPECT_EQ(get_ps_bit(cpu->ps, NF), 0);
	EXPECT_EQ(get_ps_bit(cpu->ps, VF), 0);
	EXPECT_EQ(get_ps_bit(cpu->ps, CF), 0);
	EXPECT_EQ(get_ps_bit(cpu->ps, ZF), 0);
}

TEST_F(cpu_test, ADC_with_carry) {
	cpu->acc = 0x06;
	set_ps_bit(cpu->ps, CF);
	set_inst(ADC_IMM, 0x06);
	EXPECT_EQ(cpu->acc, 0x0D);
	EXPECT_EQ(get_ps_bit(cpu->ps, NF), 0);
	EXPECT_EQ(get_ps_bit(cpu->ps, VF), 0);
	EXPECT_EQ(get_ps_bit(cpu->ps, CF), 0);
	EXPECT_EQ(get_ps_bit(cpu->ps, ZF), 0);
}

TEST_F(cpu_test, ADC_negative_overflow_pos) {
	cpu->acc = 0x06;
	set_inst(ADC_IMM, 0x7A);
	EXPECT_EQ(cpu->acc, 0x80);
	EXPECT_EQ(get_ps_bit(cpu->ps, NF), 1);
	EXPECT_EQ(get_ps_bit(cpu->ps, VF), 1);
	EXPECT_EQ(get_ps_bit(cpu->ps, CF), 0);
	EXPECT_EQ(get_ps_bit(cpu->ps, ZF), 0);
}

TEST_F(cpu_test, ADC_no_overflow_pos) {
	cpu->acc = 0x06;
	set_inst(ADC_IMM, 0x81);
	EXPECT_EQ(cpu->acc, 0x87);
	EXPECT_EQ(get_ps_bit(cpu->ps, NF), 1);
	EXPECT_EQ(get_ps_bit(cpu->ps, VF), 0);
	EXPECT_EQ(get_ps_bit(cpu->ps, CF), 0);
	EXPECT_EQ(get_ps_bit(cpu->ps, ZF), 0);
}

TEST_F(cpu_test, ADC_carry_overflow_neg) {
	cpu->acc = 0x80;
	set_inst(ADC_IMM, 0x81);
	EXPECT_EQ(cpu->acc, 0x01);
	EXPECT_EQ(get_ps_bit(cpu->ps, NF), 0);
	EXPECT_EQ(get_ps_bit(cpu->ps, VF), 1);
	EXPECT_EQ(get_ps_bit(cpu->ps, CF), 1);
	EXPECT_EQ(get_ps_bit(cpu->ps, ZF), 0);
}

TEST_F(cpu_test, ADC_no_overflow_neg) {
	cpu->acc = 0x80;
	set_inst(ADC_IMM, 0x04);
	EXPECT_EQ(cpu->acc, 0x84);
	EXPECT_EQ(get_ps_bit(cpu->ps, NF), 1);
	EXPECT_EQ(get_ps_bit(cpu->ps, VF), 0);
	EXPECT_EQ(get_ps_bit(cpu->ps, CF), 0);
	EXPECT_EQ(get_ps_bit(cpu->ps, ZF), 0);
}

TEST_F(cpu_test, ADC_carry_zero) {
	cpu->acc = 0x01;
	set_inst(ADC_IMM, 0xFF);
	EXPECT_EQ(cpu->acc, 0x00);
	EXPECT_EQ(get_ps_bit(cpu->ps, NF), 0);
	EXPECT_EQ(get_ps_bit(cpu->ps, VF), 0);
	EXPECT_EQ(get_ps_bit(cpu->ps, CF), 1);
	EXPECT_EQ(get_ps_bit(cpu->ps, ZF), 1);
}

int main() {
	testing::InitGoogleTest();
	return (RUN_ALL_TESTS());
}

