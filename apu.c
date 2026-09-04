#include "apu.h"
#include "cpu.h"
#include <stdio.h>

extern byte memory[0xFFFF];

struct apu apu;
int cpu_cycles = 0;
byte phase = 0;
float wave_phase = 0;

void read_pulse_data(pulse *square, word addr) {
	square->dlcv = memory[addr++];
	square->epns = memory[addr++];
	square->tl = memory[addr++];
	square->lth = memory[addr];
}

void clock_apu(SDL_AudioStream *stream) {
	int freq = 0;
	word timer = 0;

	read_pulse_data(&apu.square1, SQUARE1_VECTOR);
	timer = get_timer_bits(apu.square1.tl, apu.square1.lth);

	freq = CPU_FREQ / (16 * (timer + 1));
	if (cpu_cycles-- <= 0) {
		phase = (phase + 1) & 7;
		cpu_cycles = (timer + 1) * 2;
	}

	float samples[150];

	const int min_audio = (SAMPLE_RATE * sizeof(float)) / 2;
	if (SDL_GetAudioStreamQueued(stream) < min_audio) {
		for (int i = 0; i < SDL_arraysize(samples); i++) {
			samples[i] = (wave_phase < 0.5f) ? 1 : -1;
			wave_phase += freq / (float)SAMPLE_RATE;
			if (wave_phase >= 1.0f) {
				wave_phase -= 1.0f;
			}
		}
		SDL_PutAudioStreamData(stream, samples, sizeof(samples));
	}
}
