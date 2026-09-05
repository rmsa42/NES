#include "apu.h"
#include "cpu.h"
#include <stdio.h>

extern byte memory[0xFFFF];

struct apu apu;
float wave_phase = 0;
float duty_table[4] = { 0.125f, 0.25f, 0.50f, 0.75f };

void read_pulse_data(pulse *square, word addr) {
	byte dlcv = memory[addr++];
	byte epns = memory[addr++];
	byte tl = memory[addr++];
	byte lth = memory[addr];

	square->duty = duty_table[get_duty(dlcv)];
	square->timer = get_timer(tl, lth);
	square->env_loop = get_evelope_loop(dlcv);
}

void pulse_debug(const int freq) {
	printf("Freq %d\n", freq);
	printf("Timer %d\n", apu.square1.timer);
	printf("Duty %f\n", apu.square1.duty);
}

void clock_apu(SDL_AudioStream *stream) {
	int freq = 0;

	read_pulse_data(&apu.square1, SQUARE1_VECTOR);

	freq = CPU_FREQ / (16 * (apu.square1.timer + 1));

	float samples[150];

	pulse_debug(freq);
	const int min_audio = (SAMPLE_RATE * sizeof(float)) / 2;
	if (SDL_GetAudioStreamQueued(stream) < min_audio) {
		for (int i = 0; i < SDL_arraysize(samples); i++) {
			samples[i] = (wave_phase < apu.square1.duty) ? 1 : -1;
			wave_phase += freq / (float)SAMPLE_RATE;
			if (wave_phase >= 1.0f) {
				wave_phase -= 1.0f;
			}
		}
		SDL_SetAudioStreamGain(stream, 0.01f);
		SDL_PutAudioStreamData(stream, samples, sizeof(samples));
	}
}
