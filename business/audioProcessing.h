#ifndef AUDIO_PROCESSING_H
#define AUDIO_PROCESSING_H

#include <stddef.h>
#include <complex.h>
#include <raylib.h>

#define FFT_SIZE (1<<15)

void fft(float in[], size_t stride, float complex out[], size_t n);
float amp(float complex z);
void callback(void *bufferData, unsigned int frames);
size_t ProcessFFT(float in_raw[], float out_log[], float out_smooth[]);

#endif
