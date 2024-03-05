#include <assert.h>
#include <stdio.h>
#include <raylib.h>
#include <complex.h>
#include <math.h>
#include <string.h>

// macro to calculate the length of an array
#define ARRAY_LEN(xs) sizeof(xs)/sizeof(xs[0])

// number of samples ot be processed by FFT
#define N 256
// input array for FFT
float in[N];
// output array for FFT
float complex out[N];
// variable to store the maximum amplitude found in the FFT result
float max_amp;
float pi;

// struct to represent stereo audio frame
typedef struct {
    float left;
    float right;
} Frame;

// void fft(float in[], size_t stride, float complex out[], size_t n);
// float amp(float complex z);
// void callback(void *bufferData, unsigned int frames);

// FFT implementaion is recursive and divides the input until bases cases are 
// reached and then combines the results
void fft(float in[], size_t stride, float complex out[], size_t n) {
    // ensure the input size is positive
    assert(n > 0);

    // base case if the array contains a single element, its own FFT
    if (n == 1) {
        out[0] = in[0];
        return;
    }

    // recusively call fft for even and odd indexed elements
    fft(in, stride*2, out, n/2);
    fft(in + stride, stride*2, out + n/2, n/2);

    // combine results of recursion
    for (size_t k = 0; k < n/2; ++k) {
        float t = (float)k/n;
        float complex v = cexp(-2*I*pi*t)*out[k + n/2];
        float complex e = out[k];
        out[k] = e + v;
        out[k + n/2] = e - v;
    }

}

// calculate the amplitude of a complex number as the max of its real and
// imaginary parts
float amp(float complex z) {
    float a = fabsf(crealf(z));
    float b = fabsf(cimagf(z));
    if (a < b) return b;
    return a;
}

// process incoming audio frames, applt FFT and update the maximum amplitude
void callback(void *bufferData, unsigned int frames) {
    // ensure there is always enough frames
    if (frames < N) frames = N;

    // cast buffer data to the Frame type
    Frame *fs = bufferData;

    // copy audio data to the input array
    for (size_t i = 0; i < frames; ++i) {
        // only using the left channel for now
        in[i] = fs[i].left;
    }

    // apply FFT to input array
    fft(in, 1, out, N);

    // find the maximum amplitude in the FFT results
    max_amp = 0.0001f;
    for (size_t i = 0; i < frames; ++i) {
        float a = amp(out[i]);
        // update max_amp if a larger amplitude is found
        if (max_amp < a) max_amp = a;
    }
}

Color HSVtoRGB(float h, float s, float v) {
    float r, g, b;

    int i = (int)floor(h * 6);
    float f = h * 6 - i;
    float p = v * (1 - s);
    float q = v * (1 - f * s);
    float t = v * (1 - (1 - f) * s);

    switch (i % 6) {
        case 0: r = v, g = t, b = p; break;
        case 1: r = q, g = v, b = p; break;
        case 2: r = p, g = v, b = t; break;
        case 3: r = p, g = q, b = v; break;
        case 4: r = t, g = p, b = v; break;
        case 5: r = v, g = p, b = q; break;
    }

    return (Color){(unsigned char)(r * 255), (unsigned char)(g * 255), (unsigned char)(b * 255), 255};
}

Color getColorForAmplitude(float amp) {
    float hue = amp;
    float saturation = 1.0f;
    float value = 1.0f;
    return HSVtoRGB(hue, saturation, value);
}

float smoothAmplitude (float currentAmp, float previousAmp, float smoothingFactor) {
    return previousAmp * smoothingFactor + currentAmp * (1 - smoothingFactor);
}

int main(void) {
    const int screenWidth = 800;
    const int screenHeight = 800;
    
    float previousAmps[N] = {0};
    pi = atan2f(1, 1)*4;

    InitWindow(screenWidth, screenHeight, "Bragi Beats");
    SetTargetFPS(60);
    InitAudioDevice();

    Music music;
    bool musicLoaded = false;

    while(!WindowShouldClose()) {
        // Handle drag-and-drop
        if (IsFileDropped()) {
            FilePathList droppedFiles = LoadDroppedFiles();

            if (droppedFiles.count > 0 && IsFileExtension(droppedFiles.paths[0], ".wav")) {
                if (musicLoaded) {
                    UnloadMusicStream(music);
                }
                music = LoadMusicStream(droppedFiles.paths[0]);

                assert(music.stream.sampleSize == 16);
                assert(music.stream.channels == 2);

                PlayMusicStream(music);
                SetMusicVolume(music, 0.5f);
                AttachAudioStreamProcessor(music.stream, callback);
                musicLoaded = true;
            }

            UnloadDroppedFiles(droppedFiles);
        }

        if (musicLoaded) {
            UpdateMusicStream(music);

            if (IsKeyPressed(KEY_SPACE)) {
                if (IsMusicStreamPlaying(music)) {
                    PauseMusicStream(music);
                } else {
                    ResumeMusicStream(music);
                }
            }
        }

        BeginDrawing();
        ClearBackground((Color){ GetTime()*10.0f, GetTime()*15.0f, GetTime()*20.0f, 255 });

        int w = GetRenderWidth();
        int h = GetRenderHeight();
        float cell_width = (float)w / N;

        for (size_t i = 0; i < N ; ++i) {
            float t = amp(out[i])/max_amp;
            float currentAmp = amp(out[i]) / max_amp;

            if (isnan(currentAmp)) {
                currentAmp = 0;
            }

            float smoothedAmp = smoothAmplitude(currentAmp, previousAmps[i], 0.5);

            if (isnan(smoothedAmp)) {
                smoothedAmp = t;
            }
            previousAmps[i] = smoothedAmp;

            Color color = getColorForAmplitude(smoothedAmp);

            DrawRectangle(i*cell_width, h/2 - h/2*smoothedAmp, cell_width, h/2*smoothedAmp, color);
        }

        if (musicLoaded) {
            DrawText("Playing music...", 10, 10, 20, LIGHTGRAY);
        } else {
            DrawText("Drag and Drop a Song to Start Playing", 200, 200, 20, LIGHTGRAY);
        }
        EndDrawing();
    }
     if (musicLoaded) {
        UnloadMusicStream(music);
    }

    CloseAudioDevice();
    CloseWindow();

    return 0;
}
