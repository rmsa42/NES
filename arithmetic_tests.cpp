#include "tests.h"

TEST_F(cpu_test, ADC_normal) {
	set_inst(ADC_IMM, 0x01);
	EXPECT_EQ(cpu.acc, 0x01);
	EXPECT_EQ(check_bit(cpu.ps, NF), 0);
	EXPECT_EQ(check_bit(cpu.ps, VF), 0);
	EXPECT_EQ(check_bit(cpu.ps, CF), 0);
	EXPECT_EQ(check_bit(cpu.ps, ZF), 0);
}

TEST_F(cpu_test, ADC_with_carry) {
	cpu.acc = 0x06;
	set_bit(cpu.ps, CF);
	set_inst(ADC_IMM, 0x06);
	EXPECT_EQ(cpu.acc, 0x0D);
	EXPECT_EQ(check_bit(cpu.ps, NF), 0);
	EXPECT_EQ(check_bit(cpu.ps, VF), 0);
	EXPECT_EQ(check_bit(cpu.ps, CF), 0);
	EXPECT_EQ(check_bit(cpu.ps, ZF), 0);
}

TEST_F(cpu_test, ADC_negative_overflow_pos) {
	cpu.acc = 0x06;
	set_inst(ADC_IMM, 0x7A);
	EXPECT_EQ(cpu.acc, 0x80);
	EXPECT_EQ(check_bit(cpu.ps, NF), 1);
	EXPECT_EQ(check_bit(cpu.ps, VF), 1);
	EXPECT_EQ(check_bit(cpu.ps, CF), 0);
	EXPECT_EQ(check_bit(cpu.ps, ZF), 0);
}

TEST_F(cpu_test, ADC_no_overflow_pos) {
	cpu.acc = 0x06;
	set_inst(ADC_IMM, 0x81);
	EXPECT_EQ(cpu.acc, 0x87);
	EXPECT_EQ(check_bit(cpu.ps, NF), 1);
	EXPECT_EQ(check_bit(cpu.ps, VF), 0);
	EXPECT_EQ(check_bit(cpu.ps, CF), 0);
	EXPECT_EQ(check_bit(cpu.ps, ZF), 0);
}

TEST_F(cpu_test, ADC_carry_overflow_neg) {
	cpu.acc = 0x80;
	set_inst(ADC_IMM, 0x81);
	EXPECT_EQ(cpu.acc, 0x01);
	EXPECT_EQ(check_bit(cpu.ps, NF), 0);
	EXPECT_EQ(check_bit(cpu.ps, VF), 1);
	EXPECT_EQ(check_bit(cpu.ps, CF), 1);
	EXPECT_EQ(check_bit(cpu.ps, ZF), 0);
}

TEST_F(cpu_test, ADC_no_overflow_neg) {
	cpu.acc = 0x80;
	set_inst(ADC_IMM, 0x04);
	EXPECT_EQ(cpu.acc, 0x84);
	EXPECT_EQ(check_bit(cpu.ps, NF), 1);
	EXPECT_EQ(check_bit(cpu.ps, VF), 0);
	EXPECT_EQ(check_bit(cpu.ps, CF), 0);
	EXPECT_EQ(check_bit(cpu.ps, ZF), 0);
}

TEST_F(cpu_test, ADC_carry_zero) {
	cpu.acc = 0x01;
	set_inst(ADC_IMM, 0xFF);
	EXPECT_EQ(cpu.acc, 0x00);
	EXPECT_EQ(check_bit(cpu.ps, NF), 0);
	EXPECT_EQ(check_bit(cpu.ps, VF), 0);
	EXPECT_EQ(check_bit(cpu.ps, CF), 1);
	EXPECT_EQ(check_bit(cpu.ps, ZF), 1);
}

TEST_F(cpu_test, SBC_normal_with_carry) {
	cpu.acc = 0x02;
	set_bit(cpu.ps, CF);
	set_inst(SBC_IMM, 0x01);
	EXPECT_EQ(cpu.acc, 0x01);
	EXPECT_EQ(check_bit(cpu.ps, NF), 0);
	EXPECT_EQ(check_bit(cpu.ps, VF), 0);
	EXPECT_EQ(check_bit(cpu.ps, CF), 1);
	EXPECT_EQ(check_bit(cpu.ps, ZF), 0);
}

TEST_F(cpu_test, SBC_normal_no_carry_zero) {
	cpu.acc = 0x02;
	set_inst(SBC_IMM, 0x01);
	EXPECT_EQ(cpu.acc, 0x00);
	EXPECT_EQ(check_bit(cpu.ps, NF), 0);
	EXPECT_EQ(check_bit(cpu.ps, VF), 0);
	EXPECT_EQ(check_bit(cpu.ps, CF), 1);
	EXPECT_EQ(check_bit(cpu.ps, ZF), 1);
}

TEST_F(cpu_test, SBC_no_overflow_zero) {
	cpu.acc = 0x7F;
	set_bit(cpu.ps, CF);
	set_inst(SBC_IMM, 0x7F);
	EXPECT_EQ(cpu.acc, 0x00);
	EXPECT_EQ(check_bit(cpu.ps, NF), 0);
	EXPECT_EQ(check_bit(cpu.ps, VF), 0);
	EXPECT_EQ(check_bit(cpu.ps, CF), 1);
	EXPECT_EQ(check_bit(cpu.ps, ZF), 1);
}

TEST_F(cpu_test, SBC_negative) {
	cpu.acc = 0x01;
	set_bit(cpu.ps, CF);
	set_inst(SBC_IMM, 0x02);
	EXPECT_EQ(cpu.acc, 0xFF);
	EXPECT_EQ(check_bit(cpu.ps, NF), 1);
	EXPECT_EQ(check_bit(cpu.ps, VF), 0);
	EXPECT_EQ(check_bit(cpu.ps, CF), 0);
	EXPECT_EQ(check_bit(cpu.ps, ZF), 0);
}

TEST_F(cpu_test, SBC_overflow) {
	cpu.acc = 0x81;
	set_bit(cpu.ps, CF);
	set_inst(SBC_IMM, 0x02);
	EXPECT_EQ(cpu.acc, 0x7F);
	EXPECT_EQ(check_bit(cpu.ps, NF), 0);
	EXPECT_EQ(check_bit(cpu.ps, VF), 1);
	EXPECT_EQ(check_bit(cpu.ps, CF), 1);
	EXPECT_EQ(check_bit(cpu.ps, ZF), 0);
}

TEST_F(cpu_test, CMP_greater) {
	cpu.acc = 0x02;
	set_inst(CMP_IMM, 0x01);
	EXPECT_EQ(check_bit(cpu.ps, NF), 0);
	EXPECT_EQ(check_bit(cpu.ps, VF), 0);
	EXPECT_EQ(check_bit(cpu.ps, CF), 1);
	EXPECT_EQ(check_bit(cpu.ps, ZF), 0);
}

TEST_F(cpu_test, CMP_equal) {
	cpu.acc = 0x02;
	set_inst(CMP_IMM, 0x02);
	EXPECT_EQ(check_bit(cpu.ps, NF), 0);
	EXPECT_EQ(check_bit(cpu.ps, VF), 0);
	EXPECT_EQ(check_bit(cpu.ps, CF), 1);
	EXPECT_EQ(check_bit(cpu.ps, ZF), 1);
}

TEST_F(cpu_test, CMP_under) {
	cpu.acc = 0x02;
	set_inst(CMP_IMM, 0x03);
	EXPECT_EQ(check_bit(cpu.ps, NF), 1);
	EXPECT_EQ(check_bit(cpu.ps, VF), 0);
	EXPECT_EQ(check_bit(cpu.ps, CF), 0);
	EXPECT_EQ(check_bit(cpu.ps, ZF), 0);
}

int main() {
	testing::InitGoogleTest();
	return (RUN_ALL_TESTS());
}

