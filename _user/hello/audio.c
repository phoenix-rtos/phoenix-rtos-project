/*
 * Adlib sound card example
 */

#include <stdio.h>
#include <errno.h>
#include <stdint.h>
#include <stdlib.h>
#include <sys/io.h>
#include <unistd.h>
#include "audio.h"


#define NO_DELAY                 1 /* On QEMU it's fine to have no delays, on real hardware maybe not */
#define USE_FREQUENCY_MODULATION 0 /* If 1, enable the second oscillator to get more rich sounds */
#define ADLIB_ADDR               ((void *)0x0388)
#define ADLIB_DATA               ((void *)0x0389)

#define OSC_SETUP      0x20 /* Basic setup, frequency multiplier*/
#define OSC_LEVEL      0x40 /* Level */
#define OSC_ATTDEC     0x60 /* Attack, decay times */
#define OSC_SUSREL     0x80 /* Sustain level, release time */
#define VOICE_FREQ_KEY 0xA0 /* Frequency and key on/off */
#define VOICE_FB       0xC0 /* Sustain level, release time */

#define VOICES        9
#define OSC_PER_VOICE 2


void adlib_write(uint8_t addr, uint8_t data)
{
	const int ADDR_DELAY = NO_DELAY ? 0 : 6;
	const int DATA_DELAY = NO_DELAY ? 0 : 35;
	outb(ADLIB_ADDR, addr);
	for (int i = 0; i < ADDR_DELAY; i++) {
		/* The only purpose of this read is to delay long enough (assuming 8 MHz ISA bus speed) */
		(void)inb(ADLIB_ADDR);
	}

	outb(ADLIB_DATA, data);
	for (int i = 0; i < DATA_DELAY; i++) {
		(void)inb(ADLIB_DATA);
	}
}


int adlib_detect(void)
{
	adlib_write(0x04, 0x60);
	adlib_write(0x04, 0x80);

	uint8_t status1 = inb(ADLIB_ADDR);

	adlib_write(0x02, 0xff);
	adlib_write(0x04, 0x21);

	usleep(100);

	uint8_t status2 = inb(ADLIB_ADDR);

	return ((status1 & 0xe0) == 0) && ((status2 & 0xe0) == 0xc0);
}


uint8_t adlib_oscLookup(int voice, int op)
{
	if (voice >= VOICES || op >= OSC_PER_VOICE) {
		return 0;
	}

	static const uint8_t lookup[OSC_PER_VOICE][VOICES] = {
		{ 0x00, 0x01, 0x02, 0x08, 0x09, 0x0A, 0x10, 0x11, 0x12 },
		{ 0x03, 0x04, 0x05, 0x0B, 0x0C, 0x0D, 0x13, 0x14, 0x15 },
	};

	return lookup[op][voice];
}


int adlib_init(void)
{
	if (!adlib_detect()) {
		return -ENODEV;
	}

	/* Zero out all registers */
	for (uint8_t i = 1; i <= 0xf5; i++) {
		adlib_write(i, 0);
	}

	adlib_write(0x01, 0x20); /* Set "Wave Select" bit to 1 - allows use or more waveforms */

	return 0;
}


void adlib_initVoice(uint8_t voice, int continuous)
{
	/* Changing these parameters will change the timbre of the voice.
	   This is just a simple setup to get something playing on the chip. */

	/* Set up oscillator 0 for voice 0 */
	uint8_t osc = adlib_oscLookup(voice, 0);
	adlib_write(OSC_SETUP + osc, 1 | (continuous ? (1 << 5) : 0)); /* Set frequency multiplier to 1, play continuously */
	adlib_write(OSC_LEVEL + osc, 0);                               /* Set level to maximum */
	adlib_write(OSC_ATTDEC + osc, 0xe4);                           /* Set some reasonable ADSR envelope */
	adlib_write(OSC_SUSREL + osc, 0x04);

#if USE_FREQUENCY_MODULATION
	/* Set up oscillator 1 for voice 0 */
	osc = adlib_oscLookup(voice, 1);
	adlib_write(OSC_SETUP + osc, 1 | (1 << 5)); /* Set frequency multiplier to 1, play continuously */
	adlib_write(OSC_LEVEL + osc, 0x1f);         /* Set level to 24.75 dB */
	adlib_write(OSC_ATTDEC + osc, 0xe4);        /* Set some reasonable ADSR envelope */
	adlib_write(OSC_SUSREL + osc, 0x04);
#else
	/* Put voice 0 into mixing instead of modulation mode to get pure sine wave */
	adlib_write(VOICE_FB + voice, 1);
#endif
}


void adlib_calcFreq(int frequency, uint16_t *f_out, uint8_t *oct_out)
{
	/* NOTE: you can change frequency to be a float if you want to */
	uint8_t oct = 4;
	const int mul = 1000;
	const int div = 758;

	/* Scale frequency to get it near the maximum range of the chip for more precision */
	while (frequency > (277 * 2) && oct < 7) {
		frequency /= 2;
		oct++;
	}

	while (frequency < (524 / 2) && oct > 0) {
		frequency *= 2;
		oct--;
	}

	*oct_out = oct;
	if ((frequency * mul) < (0x3ff * div)) {
		*f_out = (uint16_t)(frequency * mul / div);
	}
	else {
		*f_out = 0x3ff;
	}
}


void adlib_playVoice(uint8_t voice, uint16_t f_number, uint8_t octave)
{
	if (voice >= VOICES) {
		return;
	}

	adlib_write(VOICE_FREQ_KEY + voice, f_number & 0xff);
	uint8_t reg2 = ((f_number >> 8) & 0x3) | ((octave & 0x7) << 2) | (1 << 5);
	adlib_write(VOICE_FREQ_KEY + 0x10 + voice, reg2);
}


void adlib_stopVoice(uint8_t voice)
{
	if (voice >= VOICES) {
		return;
	}

	adlib_write(VOICE_FREQ_KEY + 0x10 + voice, 0x0);
}
