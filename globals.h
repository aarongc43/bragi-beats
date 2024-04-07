#ifndef GLOBALS_H
#define GLOBALS_H

#include <complex.h>

#define FFT_SIZE (1<<15)

extern int screenWidth;
extern int screenHeight;
extern float in_raw[FFT_SIZE];
extern float in_win[FFT_SIZE];
extern float complex out_raw[FFT_SIZE];
extern float out_log[FFT_SIZE];
extern float out_smooth[FFT_SIZE];

#endif
