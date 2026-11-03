#include "tests.h"

TEST_F(cpu_test, ASL_normal) {
	set_abs_inst(ASL_ABS, 0x0000, 0x01);
	EXPECT_EQ(cpu_mem[0x0000], 0x02);
	EXPECT_EQ(GET_BIT(cpu.ps, NF), 0);
	EXPECT_EQ(GET_BIT(cpu.ps, ZF), 0);
	EXPECT_EQ(GET_BIT(cpu.ps, CF), 0);
}

TEST_F(cpu_test, ASL_carry) {
	set_abs_inst(ASL_ABS, 0x0000, 0x80);
	EXPECT_EQ(cpu_mem[0x0000], 0x00);
	EXPECT_EQ(GET_BIT(cpu.ps, NF), 0);
	EXPECT_EQ(GET_BIT(cpu.ps, ZF), 1);
	EXPECT_EQ(GET_BIT(cpu.ps, CF), 1);
}

TEST_F(cpu_test, LSR_normal) {
	set_abs_inst(LSR_ABS, 0x0000, 0x02);
	EXPECT_EQ(cpu_mem[0x0000], 0x01);
	EXPECT_EQ(GET_BIT(cpu.ps, NF), 0);
	EXPECT_EQ(GET_BIT(cpu.ps, ZF), 0);
	EXPECT_EQ(GET_BIT(cpu.ps, CF), 0);
}

TEST_F(cpu_test, LSR_carry) {
	set_abs_inst(LSR_ABS, 0x0000, 0x01);
	EXPECT_EQ(cpu_mem[0x0000], 0x00);
	EXPECT_EQ(GET_BIT(cpu.ps, NF), 0);
	EXPECT_EQ(GET_BIT(cpu.ps, ZF), 1);
	EXPECT_EQ(GET_BIT(cpu.ps, CF), 1);
}

TEST_F(cpu_test, ROL_normal) {
	set_abs_inst(ROL_ABS, 0x0000, 0x01);
	EXPECT_EQ(cpu_mem[0x0000], 0x02);
	EXPECT_EQ(GET_BIT(cpu.ps, NF), 0);
	EXPECT_EQ(GET_BIT(cpu.ps, ZF), 0);
	EXPECT_EQ(GET_BIT(cpu.ps, CF), 0);
}

TEST_F(cpu_test, ROL_with_carry) {
	SET_BIT(cpu.ps, CF);
	set_abs_inst(ROL_ABS, 0x0000, 0x80);
	EXPECT_EQ(cpu_mem[0x0000], 0x01);
	EXPECT_EQ(GET_BIT(cpu.ps, NF), 0);
	EXPECT_EQ(GET_BIT(cpu.ps, ZF), 0);
	EXPECT_EQ(GET_BIT(cpu.ps, CF), 1);
}

TEST_F(cpu_test, ROR_normal) {
	set_abs_inst(ROR_ABS, 0x0000, 0x02);
	EXPECT_EQ(cpu_mem[0x0000], 0x01);
	EXPECT_EQ(GET_BIT(cpu.ps, NF), 0);
	EXPECT_EQ(GET_BIT(cpu.ps, ZF), 0);
	EXPECT_EQ(GET_BIT(cpu.ps, CF), 0);
}

TEST_F(cpu_test, ROR_with_carry) {
	SET_BIT(cpu.ps, CF);
	set_abs_inst(ROR_ABS, 0x0000, 0x01);
	EXPECT_EQ(cpu_mem[0x0000], 0x80);
	EXPECT_EQ(GET_BIT(cpu.ps, NF), 1);
	EXPECT_EQ(GET_BIT(cpu.ps, ZF), 0);
	EXPECT_EQ(GET_BIT(cpu.ps, CF), 1);
}
