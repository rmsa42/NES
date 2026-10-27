#include "apu.h"
#include "cpu.h"
#include <stdio.h>

extern byte cpu_mem[CPU_MEM_SIZE];

struct apu apu;
float wave_phase = 0;
float duty_table[4] = { 0.125f, 0.25f, 0.50f, 0.75f };
int triangle_phases[32] = {
	15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0,
	0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15,
};

void apu_write(word addr, byte value) {
	switch (addr) {
		case SQUARE1:
			apu.square1.duty = duty_table[get_duty(value)];
			apu.square1.env_loop = get_evelope_loop(value);
			break;
		case SQUARE1 + 2:
			apu.square1.timer = get_timer(value, 0);
			break;
		case SQUARE1 + 3:
			apu.square1.timer = get_timer(apu.square1.timer, value);
			apu.square1.len_counter_load = get_len_counter_load(value);
			break;

		case TRIANGLE:
			apu.triangle.control_flag = check_bit(value, 7);
			apu.triangle.lin_counter_load = get_lin_counter_load(value);
			break;
		case TRIANGLE + 2:
			apu.triangle.timer = get_timer(value, 0);
			break;
		case TRIANGLE + 3:
			apu.triangle.timer = get_timer(apu.triangle.timer, value);
			break;

		default:
			break;
	}
}

void apu_read(word addr) {}

void pulse_debug(const int freq) {
	printf("Freq %d\n", freq);
	//printf("Timer %d\n", apu.square1.timer);
	//printf("Duty %f\n", apu.square1.duty);
}

void clock_apu(struct display *nes_dp) {
	int freq = 0;
	int freqt = 0;

	freq = CPU_FREQ / (16 * (apu.square1.timer + 1));
	freqt = CPU_FREQ / (32 * (apu.triangle.timer + 1));

	float samples[150];
	const int min_audio = (SAMPLE_RATE * sizeof(float)) / 2;

	if (SDL_GetAudioStreamQueued(nes_dp->stream) < min_audio) {
		//pulse_debug(freqt);
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
		//SDL_SetAudioStreamGain(nes_dp->stream, 0.5f);
		//SDL_PutAudioStreamData(nes_dp->stream, samples, sizeof(samples));
	}
}
