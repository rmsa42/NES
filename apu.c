#include "apu.h"
#include "cpu.h"
#include <stdio.h>

extern byte memory[0xFFFF];

struct apu apu;
float wave_phase = 0;
float duty_table[4] = { 0.125f, 0.25f, 0.50f, 0.75f };
int triangle_phases[32] = {
	15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0,
	0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15,
};
int noise_period_table[16] = {
	4, 8, 16, 32, 64, 96, 128, 160, 202, 254, 380, 508, 762, 1016, 2034, 4068
};
word shift = 1;

int apu_init() {
	memset(&apu, 0, sizeof(struct apu));
	apu.noise.shift_reg = 1;
	return (0);
}

void read_pulse_data(pulse *square, word addr) {
	byte dlcv = memory[addr++];
	byte epns = memory[addr++];
	byte tl = memory[addr++];
	byte lth = memory[addr];

	square->duty = duty_table[get_duty(dlcv)];
	square->env_loop = get_evelope_loop(dlcv);
	square->timer = get_timer(tl, lth);
	square->len_counter_load = get_len_counter_load(lth);
}

void read_triangle_data(word addr) {
	byte cr = memory[addr++];
	byte unused = memory[addr++];
	byte tl = memory[addr++];
	byte lth = memory[addr];

	apu.triangle.control_flag = check_bit(cr, 7);
	apu.triangle.lin_counter_load = get_lin_counter_load(cr);
	apu.triangle.unused = unused;
	apu.triangle.timer = get_timer(tl, lth);
	apu.triangle.len_counter_load = get_len_counter_load(lth);
	apu.triangle.lin_reload_flag = 0; // Need to modify this
}

void read_noise_data(word addr) {
	byte lcv = memory[addr++];
	byte unused = memory[addr++];
	byte mp = memory[addr++];
	byte lcl = memory[addr];

	apu.noise.env_loop = get_evelope_loop(lcv);
	apu.noise.unused = unused;
	apu.noise.mode = get_noise_mode(mp);
	apu.noise.period = noise_period_table[(int)get_noise_period(mp)];
	apu.noise.length_counter_load = get_len_counter_load(lcl);
}

void pulse_debug(const int freq) {
	printf("Freq %d\n", freq);
	printf("Timer %d\n", apu.triangle.timer);
	printf("Duty %f\n", apu.square1.duty);
}

word lfsr(word shift_reg, byte xor_bit) {
	byte xor_vl = check_bit(shift_reg, 0) ^ check_bit(shift_reg, xor_bit);

	//printf("XOR: %d\n", xor_vl);
	shift_reg = shift_reg >> 1;
	set_bit_to(shift_reg, 14, xor_vl);
	return (shift_reg);
}

void clock_apu(SDL_AudioStream *stream) {
	int freq = 0;
	int freqt = 0;
	int freqn = 0;

	read_pulse_data(&apu.square1, SQUARE1_VECTOR);
	//read_pulse_data(&apu.square2, SQUARE2_VECTOR);
	read_triangle_data(TRIANGLE_VECTOR);
	read_noise_data(NOISE_VECTOR);
	freq = CPU_FREQ / (16 * (apu.square1.timer + 1));
	freqt = CPU_FREQ / (32 * (apu.triangle.timer + 1));
	if (apu.noise.period != 0) {
		freqn = CPU_FREQ / apu.noise.period;
	}

	apu.noise.shift_reg = lfsr(apu.noise.shift_reg, 6);
	//printf("Freq: %d\n", freqn);
	//pulse_debug(freqt);

	float samples[150];
	const int min_audio = (SAMPLE_RATE * sizeof(float)) / 2;

	if (SDL_GetAudioStreamQueued(stream) < min_audio) {
		// Square Wave
		//for (int i = 0; i < SDL_arraysize(samples); i++) {
		//	samples[i] = (wave_phase < apu.square1.duty) ? 1 : -1;
		//	wave_phase += freq / (float)SAMPLE_RATE;
		//	if (wave_phase >= 1.0f) {
		//		wave_phase -= 1.0f;
		//	}
		//}
		// Triangle Wave
		//for (int i = 0; i < SDL_arraysize(samples); i++) {
		//	samples[i] = triangle_phases[(int)(wave_phase)] / 15.0f;
		//	wave_phase += (freqt / (float)SAMPLE_RATE) * 32;
		//	while (wave_phase >= 32.0f) {
		//		wave_phase -= 32.0f;
		//	}
		//}
		// Noise Wave
		for (int i = 0; i < SDL_arraysize(samples); i++) {
			samples[i] = (shift & 0x01) ? -1.0f : 1.0f;
			wave_phase += (freqn / (float)SAMPLE_RATE);
			while (wave_phase >= 1.0f) {
				wave_phase -= 1.0f;
				shift = lfsr(shift, 6);
			}
		}
		SDL_SetAudioStreamGain(stream, 0.3f);
		SDL_PutAudioStreamData(stream, samples, sizeof(samples));
	}
}
