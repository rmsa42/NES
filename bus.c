#include "bus.h"
#include "cpu.h"
#include "ppu.h"
#include "apu.h"

void bus_write(word addr, byte value) {
	if (addr >= 0x2000 && addr <= 0x2007) {
		ppu_write(addr, value);
	} else if (addr >= 0x4000 && addr <= 0x400B) {
		apu_write(addr, value);
	} else {
		cpu_write(addr, value);
	}
}

byte bus_read(word addr) {
	if (addr >= 0x2000 && addr <= 0x2007) {
		return (ppu_read(addr));
	} else {
		return (cpu_read(addr));
	}
	return (0);
}

