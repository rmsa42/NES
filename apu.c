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

void read_pulse_data(pulse *square, word addr) {
	byte dlcv = cpu_mem[addr++];
	byte epns = cpu_mem[addr++];
	byte tl = cpu_mem[addr++];
	byte lth = cpu_mem[addr];

	square->duty = duty_table[get_duty(dlcv)];
	square->env_loop = get_evelope_loop(dlcv);
	square->timer = get_timer(tl, lth);
	square->len_counter_load = get_len_counter_load(lth);
}

void read_triangle_data(word addr) {
	byte cr = cpu_mem[addr++];
	byte unused = cpu_mem[addr++];
	byte tl = cpu_mem[addr++];
	byte lth = cpu_mem[addr];

	apu.triangle.control_flag = check_bit(cr, 7);
	apu.triangle.lin_counter_load = get_lin_counter_load(cr);
	apu.triangle.unused = unused;
	apu.triangle.timer = get_timer(tl, lth);
	apu.triangle.len_counter_load = get_len_counter_load(lth);
	apu.triangle.lin_reload_flag = 0; // Need to modify this
}

void pulse_debug(const int freq) {
	printf("Freq %d\n", freq);
	printf("Timer %d\n", apu.triangle.timer);
	printf("Duty %f\n", apu.square1.duty);
}

void clock_apu(SDL_AudioStream *stream) {
	int freq = 0;
	int freqt = 0;

	read_pulse_data(&apu.square1, SQUARE1_VECTOR);
	//read_pulse_data(&apu.square2, SQUARE2_VECTOR);
	read_triangle_data(TRIANGLE_VECTOR);
	freq = CPU_FREQ / (16 * (apu.square1.timer + 1));
	freqt = CPU_FREQ / (32 * (apu.triangle.timer + 1));

	pulse_debug(freqt);

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
		for (int i = 0; i < SDL_arraysize(samples); i++) {
			samples[i] = triangle_phases[(int)(wave_phase)] / 15.0f;
			wave_phase += (freqt / (float)SAMPLE_RATE) * 32;
			while (wave_phase >= 32.0f) {
				wave_phase -= 32.0f;
			}
		}
		SDL_SetAudioStreamGain(stream, 0.3f);
		SDL_PutAudioStreamData(stream, samples, sizeof(samples));
	}
}
