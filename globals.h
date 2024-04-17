#ifndef GLOBALS_H
#define GLOBALS_H

#include <complex.h>
#include <raylib.h>

#define FFT_SIZE (1<<15)
#define MAX_SONGS 100
#define ARRAY_LEN(xs) sizeof(xs)/sizeof(xs[0])


typedef struct {
    Music songs[MAX_SONGS];
    char titles[MAX_SONGS][256];
    int front, rear;
} SongQueue;

extern SongQueue songQueue;

typedef enum {
    VISUALIZER_BAR_CHART,
    VISUALIZER_CIRCLE,
    VISUALIZER_CIRCLE_STAR,
    VISUALIZER_WING,
    VISUALIZER_KALEIDOSCOPE
} VisualizerType;

extern VisualizerType currentVisualizer;

extern int screenWidth;
extern int screenHeight;
extern float in_raw[FFT_SIZE];
extern float in_win[FFT_SIZE];
extern float complex out_raw[FFT_SIZE];
extern float out_log[FFT_SIZE];
extern float out_smooth[FFT_SIZE];

extern Music currentMusic;
extern bool isPlaying;

extern Color OFFWHITE;
extern Color CUSTOMDARKGRAY;

#endif
