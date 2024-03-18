#include <assert.h>
#include <stdio.h>
#include <raylib.h>
#include <complex.h>
#include <math.h>
#include <string.h>

// gets the size of an array
#define ARRAY_LEN(xs) sizeof(xs)/sizeof(xs[0])
// can't go too high becuase the width of the bars will become invisible
#define N (1<<15)

// store raw input samples
float in_raw[N];
// windowed input of raw samples
float in_win[N];
// stores the output of FFT, magnitude and phase
float complex out_raw[N];
// stores the logarithmically calculated values of the FFT
float  out_log[N];

// struct to for stereo audio frames
typedef struct {
    float left;
    float right;
} Frame;

// FFT implementaion is recursive and divides the input until bases cases are 
// reached and then combines the results
void fft(float in[], size_t stride, float complex out[], size_t n) {
    // ensure the input size of the array has something in it
    assert(n > 0);

    // base case: if the array only has one element which is a sample left then
    // it is done
    if (n == 1) {
        out[0] = in[0];
        return;
    }

    // fft recusive call to only calculate the even numbered indicies
    fft(in, stride*2, out, n/2);
    // fft recursive call to only calculate the odd numbered indicies
    fft(in + stride, stride*2, out + n/2, n/2);

    // this loop iterates over the first half of the out array. each iteration
    // combines pairs of elements from the first and second halves of the array
    for (size_t k = 0; k < n/2; ++k) {
        // calculate 't', a normalized frequency index for the current element 'k'.
        float t = (float)k / n;

        // Calculate the complex exponential for the current 'k'.
        float complex v = cexp(-2 * I * PI * t) * out[k + n / 2];

        // temporarily store the current 'k'th element from the first half of the
        // 'out' array. It is one of the two elements to be combined
        float complex e = out[k];

        // the result for the 'k'th position is obtained by adding 'e' and 'v'.
        // this combines the current element with its paired element from the
        // second half
        out[k] = e + v;

        // update the element in the second half of the
        // 'out' array by subtracting 'v' from 'e'
        out[k + n / 2] = e - v;
    }
}

// calculates the logarithmic amplitude of z
float amp(float complex z) {
    float a = crealf(z);
    float b = cimagf(z);
    return logf(a*a + b*b);
}

void callback(void *bufferData, unsigned int frames) {
    // ensure we don't process more frames than the function can handle
    if (frames > N) frames = N;

    // bufferData pointer holds incoming audio data
    // a pointer to an array of two floats
    float (*fs)[2] = bufferData;

    // iterate over each frame of audio data
    for (size_t i = 0; i < frames; ++i) {
        // shifts existing samples one position over, to make room for
        // new data, like a sliding window across audio data
        memmove(in_raw, in_raw + 1, (N-1)*sizeof(in_raw[0]));
        // insert newest sample at the end of the buffer, which keeps the
        // buffer up to data with the most recent audio data
        in_raw[N-1] = fs[i][0];
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

        // iterates over N sized array, size_t is used because array sizes are
        // non-negative
        for (size_t i = 0; i < N; ++i) {
            // calculates the normalized position of 't'
            float t = (float)i/(N-1);
            // applies hann window function to reduce spectral leakage
            float hann = 0.5 - 0.5*cosf(2*PI*t);
            // multiplies each sample by the hann value and stores in in_win
            in_win[i] = in_raw[i]*hann;
        }
        
        // perform fft on windowed input and stores in out_raw
        fft(in_win, 1, out_raw, N);

        // step in 6% increments, increase for low resolution, decrease for
        // higher resolution
        float step = 1.05;
        // initialize the starting frequency
        float lowf = 1.0f;
        // counter for logarithmic bins
        size_t m = 0;
        // tracker for max amplitude found
        float max_amp = 1.0f;

        // iterates over frequencies on a logarithmic scale
        // // stops halfway because of patterns, we only need to loop through
        // half of the frequencies
        for (float f = lowf; (size_t) f < N/2; f = ceilf(f*step)) {
            // start at 'f' and stop at 'f1' because this would be one bin and
            // then go to the next frequency step
            float f1 = ceilf(f*step);
            // hold maximum amplitude found in each bin
            float a = 0.0f;
            // calculates the amplitudes
            for (size_t q = (size_t) f; q < N/2 && q < (size_t) f1; ++q) {
                // b holds the frequency
                float b = amp(out_raw[q]);
                // if be is larger then a will be the max amplitude
                if (b > a) a = b;
            }

            // compare to overall max amplitude of the frequencies
            if (max_amp < a) max_amp = a;
            // the max ampltitude in the bin is stored in out_log and m which is
            // the bin number is incremented
            out_log[m++] = a;
        }

        // normalize frequecies by the max amplitude which will be from 0 to 1
        for (size_t i = 0; i < m; ++i) {
            out_log[i] /= max_amp;
        }

        // cell width is the screen width divided by the number of bins
        float cell_width = (float)screenWidth/m;

        for (size_t i = 0; i < m; ++i) {

            // using the normalize amplitude values in out_log
            float t = out_log[i];
            
            // DrawRectangle(i*cell_width, screenHeight - screenHeight*2/3*t, cell_width, screenHeight*2/3*t, BLUE);
            
            // values for the center of the screen
            int centerX = screenWidth / 2;
            int centerY = screenHeight / 2;
            // max radius of the visualizer
            float maxRadius = (screenHeight < screenWidth ? screenHeight : screenWidth) / 2;

            // Divide a full circle by the number of bins
            float angleStep = 360.0f / m;

            // iterate through each bin in out_log
            for (size_t i = 0; i < m; ++i) {

                // previously calculated amplitude for bin i, normalized
                float amplitude = out_log[i];
                // angle in degrees for each bin
                float angle = angleStep * i;

                // line thickness based on amplitude
                float lineThickness = 2.0f + (amplitude * 1);

                // convert angle to radians for math functions
                float radian = angle * (PI / 180.0f);

                // calculate the line's end point based on the amplitude
                Vector2 start = { (float)centerX, (float)centerY };
                Vector2 end = { centerX + cos(radian) * (amplitude * maxRadius),
                                centerY + sin(radian) * (amplitude * maxRadius) };

                // Choose a color based on the amplitude
                //Color color = (Color){(unsigned char)(255 * amplitude), 128, (unsigned char)(255 * (1 - amplitude)), 255};
                
                // smooth color transitions
                Color color = (Color){(unsigned char)(255 * sin(GetTime())), 128, (unsigned char)(255 * cos(GetTime())), 255};

                for (int j = 0; j < 5; j++) { // Example: Simulated glow effect
                    float fadeFactor = (5 - j) / 5.0f; // Decrease opacity
                    Color fadedColor = ColorAlpha(color, fadeFactor);
                    DrawLineEx(start, end, lineThickness * fadeFactor, fadedColor);
                }

                // Draw the line from the center to the calculated end point
                //DrawLineEx(start, end, lineThickness, color);
            }
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
