#include <assert.h>
#include <stdio.h>
#include <raylib.h>
#include <complex.h>
#include <math.h>
#include <string.h>

// number of samples to be processed by FFT
#define ARRAY_LEN(xs) sizeof(xs)/sizeof(xs[0])
// can't go too high becuase the width of the bars will become invisible
#define N (1<<13)
// input array for FFT holds the samples for the sound
float in1[N];
float in2[N];
// output array for FFT
float complex out[N];

// struct to represent stereo audio frame
typedef struct {
    float left;
    float right;
} Frame;

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
        float complex v = cexp(-2*I*PI*t)*out[k + n/2];
        float complex e = out[k];
        out[k] = e + v;
        out[k + n/2] = e - v;
    }

}

// calculate the amplitude of a complex number as the max of its real and
// imaginary parts
// to improve, use vectors on an imaginary plane
// need to use the magnitude of the vector which will represent the amplitude of
// the specific frequency
// direction represents phase
// length is how intense it is
float amp(float complex z) {
    float a = crealf(z);
    float b = cimagf(z);
    return logf(a*a + b*b);
}

// process incoming audio frames, applt FFT and update the maximum amplitude
// receive the samples in this callback periodically
// grabs a chunk of samples and calls the callback with that chunk
// push those samples into the input buffer from right to left
// turning the time domain into the frequency domain when we call fft in the
// main function
void callback(void *bufferData, unsigned int frames) {
    // ensure there is always enough frames
    if (frames > N) frames = N;

    // cast buffer data to the Frame type
    float (*fs)[2] = bufferData;

    // copy audio data to the input array
    for (size_t i = 0; i < frames; ++i) {
        // only using the left channel for now
        memmove(in1, in1 + 1, (N-1)*sizeof(in1[0]));
        in1[N-1] = fs[i][0];
    }
}

int main(void) {
    const int screenWidth = 800;
    const int screenHeight = 800;
    
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

        for (size_t i = 0; i < N; ++i) {
            float t = (float)i/(N-1);
            float hann = 0.5 - 0.5*cosf(2*PI*t);
            in2[i] = in1[i]*hann;
        }
        
        fft(in2, 1, out, N);

        float max_amp = 0.0f;
        for (size_t i = 0; i < N ; ++i) {
            float a = amp(out[i]);
            if (max_amp < a) max_amp = a;
        }
        
        float step = 1.06;
        float lowf = 1.0f;
        size_t m = 0;
        for (float f = lowf; (size_t) f < N/2; f = ceilf(f*step)) {
            m += 1;
        }

        float cell_width = (float)screenWidth/m;
        m = 0;
        for (float f = lowf; (size_t) f < N/2; f = ceilf(f*step)) {
            float f1 = ceilf(f*step);
            float a = 0.0f;
            for (size_t q = (size_t) f; q < N/2 && q < (size_t) f1; ++q) {
                float b = amp(out[q]);
                if (b > a) a = b;
            }
            // a /= (size_t) f1 - (size_t) f + 1;
            float t = a/max_amp;
            DrawRectangle(m*cell_width, screenHeight - screenHeight/2*t, cell_width, screenHeight/2*t, BLUE);
            m += 1;
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
