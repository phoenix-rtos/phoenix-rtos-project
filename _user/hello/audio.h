#ifndef _AUDIO_H
#define _AUDIO_H

#include <stdio.h>
#include <errno.h>
#include <stdint.h>
#include <stdlib.h>
#include <sys/io.h>
#include <unistd.h>


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


void adlib_write(uint8_t addr, uint8_t data);

int adlib_detect(void);

uint8_t adlib_oscLookup(int voice, int op);

int adlib_init(void);

void adlib_initVoice(uint8_t voice, int continuous);

void adlib_calcFreq(int frequency, uint16_t *f_out, uint8_t *oct_out);

void adlib_playVoice(uint8_t voice, uint16_t f_number, uint8_t octave);

void adlib_stopVoice(uint8_t voice);

#endif /* _AUDIO_H */
