#include <assert.h>
#include <stdio.h>
#include <raylib.h>
#include <complex.h>
#include <math.h>
#include <string.h>

#define ARRAY_LEN(xs) sizeof(xs)/sizeof(xs[0])
#define N (1<<15)
#define MAX_SONGS 100

int screenWidth = 800;
int screenHeight = 800;
float in_raw[N];
float in_win[N];
float complex out_raw[N];
float  out_log[N];
char songTitles[MAX_SONGS][256];
int songCount = 0;

typedef struct {
    float left;
    float right;
} Frame;

void fft(float in[], size_t stride, float complex out[], size_t n);
void ProcessFFT(float in_raw[], float out_log[], int cneterX, int centerY);
float amp(float complex z);
void callback(void *bufferData, unsigned int frames);
void circleStarVisual(float out_log[], size_t m, int centerX, int centerY);
bool DrawButton(Rectangle bounds, const char* text);
void DrawTitleBar();
void DrawSongQueue();

int main(void) {
    InitWindow(screenWidth, screenHeight, "Bragi Beats");
    SetTargetFPS(60);

    Rectangle buttonBounds = {10, 5, 100, 10};

    const char *visualizers[] = {"Visualizer 1", "Visualizer 2", "Visualizer 3"};
    int visualizerCount = sizeof(visualizers)/ sizeof(visualizers[0]);
    bool showList = false;

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

                if (songCount < MAX_SONGS) {
                    strncpy(songTitles[songCount], GetFileName(droppedFiles.paths[0]), 256);
                    songTitles[songCount][255] = '\0';

                    char* dot = strrchr(songTitles[songCount], '.');
                    if (dot != NULL) {
                        *dot = '\0';
                    }

                    songCount++;
                }

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

        int effectiveScreenWidth = screenWidth - 150;
        int effectiveScreenHeight = screenHeight - 20 - 100;

        int centerX = 150 + effectiveScreenWidth / 2;
        int centerY = 20 + effectiveScreenHeight / 2;

        BeginDrawing();
        ClearBackground((Color){ GetTime()*10.0f, GetTime()*15.0f, GetTime()*20.0f, 255 });

        DrawTitleBar();

        if (DrawButton(buttonBounds, "Visualizers")) {
            showList = !showList;
        }

        ProcessFFT(in_raw, out_log, centerX, centerY);

        if (showList) {
            int titleBarheight = 20;
            int maxVisualizerWidth = 0;
            int listStartY = titleBarheight + 10;
            int listItemHeight = 20;

            for (int i = 0; i < visualizerCount; i++) {
                int width = MeasureText(visualizers[i], 14);
                if (width > maxVisualizerWidth) {
                    maxVisualizerWidth = width;
                }
            }

            int listWidth = maxVisualizerWidth + 20;
            int listItemsX = 18;
            int listBackgroundHeight = visualizerCount * listItemHeight + 10;

            DrawRectangle(listItemsX - 10, listStartY - 10, listWidth, listBackgroundHeight, WHITE);

            for (int i = 0; i < visualizerCount; i++) {
                int itemY = listStartY + i * listItemHeight;
                int textWidth = MeasureText(visualizers[i], 13);
                int textX = listItemsX + (listWidth - textWidth) / 2;

                DrawText(visualizers[i], textX, itemY, 10, DARKGRAY);
            }
        }

        if (musicLoaded) {
            const char* text = "Playing music...";
            int fontSize = 20;
            int textWidth = MeasureText(text, fontSize);
            int textX = (screenWidth - textWidth) / 2;
            int textY = (20 - fontSize) / 2;

            DrawText(text, textX, textY, fontSize, LIGHTGRAY);
        } else {
            DrawText("Drag and Drop a Song to Start Playing", 200, 200, 20, LIGHTGRAY);
        }

        DrawSongQueue();

        EndDrawing();
    }
    if (musicLoaded) {
        UnloadMusicStream(music);
    }

    CloseAudioDevice();
    CloseWindow();

    return 0;
}

void fft(float in[], size_t stride, float complex out[], size_t n) {
    assert(n > 0);

    if (n == 1) {
        out[0] = in[0];
        return;
    }

    fft(in, stride*2, out, n/2);
    fft(in + stride, stride*2, out + n/2, n/2);

    for (size_t k = 0; k < n/2; ++k) {
        float t = (float)k / n;

        float complex v = cexp(-2 * I * PI * t) * out[k + n / 2];

        float complex e = out[k];

        out[k] = e + v;

        out[k + n / 2] = e - v;
    }
}

float amp(float complex z) {
    float a = crealf(z);
    float b = cimagf(z);
    return logf(a*a + b*b);
}

void callback(void *bufferData, unsigned int frames) {
    if (frames > N) frames = N;

    float (*fs)[2] = bufferData;

    for (size_t i = 0; i < frames; ++i) {
        memmove(in_raw, in_raw + 1, (N-1)*sizeof(in_raw[0]));
        in_raw[N-1] = fs[i][0];
    }
}

bool DrawButton(Rectangle bounds, const char* text) {
    bool clicked = false;
    Vector2 mousePoint = GetMousePosition();
    bool isHovering = CheckCollisionPointRec(mousePoint, bounds);

    if (isHovering) {
        if (IsMouseButtonDown(MOUSE_BUTTON_LEFT)) {
            DrawRectangleRec(bounds, DARKGRAY);
        }
        else {
            DrawRectangleRec(bounds, LIGHTGRAY);
            if (IsMouseButtonReleased(MOUSE_BUTTON_LEFT)) {
                clicked = true;
            }
        }
    } else {
        DrawRectangleRec(bounds, GRAY);
    }

    int textWidth = MeasureText(text, 10);

    DrawText(text, bounds.x + (bounds.width / 2) - (textWidth / 2), bounds.y + (bounds.height / 2) - 5, 10, BLACK);

    return clicked;
}

void DrawTitleBar() {
    Rectangle titleBarBounds = {0, 0, GetScreenWidth(), 20};
    DrawRectangleRec(titleBarBounds, WHITE);
}

void ProcessFFT(float in_raw[], float out_log[], int centerX, int centerY) {

    for (size_t i = 0; i < N; ++i) {
        float t = (float)i/(N-1);
        float hann = 0.5 - 0.5*cosf(2*PI*t);
        in_win[i] = in_raw[i]*hann;
    }

    fft(in_win, 1, out_raw, N);

    float step = 1.05;
    float lowf = 1.0f;
    size_t m = 0;
    float max_amp = 1.0f;

    for (float f = lowf; (size_t) f < N/2; f = ceilf(f*step)) {
        float f1 = ceilf(f*step);
        float a = 0.0f;
        for (size_t q = (size_t) f; q < N/2 && q < (size_t) f1; ++q) {
            float b = amp(out_raw[q]);
            if (b > a) a = b;
        }

        if (max_amp < a) max_amp = a;
        out_log[m++] = a;
    }

    for (size_t i = 0; i < m; ++i) {
        out_log[i] /= max_amp;
    }

    circleStarVisual(out_log, m, centerX, centerY);

}

void circleStarVisual(float out_log[], size_t m, int centerX, int centerY) {
    float maxRadius = (screenHeight - 20 - 100) / 2;
    float angleStep = 360.0f / m;

    for (size_t i = 0; i < m; ++i) {

        float amplitude = out_log[i];
        float angle = angleStep * i;
        float lineThickness = 2.0f + (amplitude * 1);
        float radian = angle * (PI / 180.0f);

        Vector2 start = { (float)centerX, (float)centerY };
        Vector2 end = { centerX + cos(radian) * (amplitude * maxRadius),
            centerY + sin(radian) * (amplitude * maxRadius) };


        Color color = (Color){(unsigned char)(255 * sin(GetTime())), 128, (unsigned char)(255 * cos(GetTime())), 255};

        for (int j = 0; j < 5; j++) { // Example: Simulated glow effect
            float fadeFactor = (5 - j) / 5.0f; // Decrease opacity
            Color fadedColor = ColorAlpha(color, fadeFactor);
            DrawLineEx(start, end, lineThickness * fadeFactor, fadedColor);
        }
    }
}

void DrawSongQueue() {
    int sidebarWidth = 150;
    int sidebarHeight = 700;
    int sidebarX = 0;
    int sidebarY = 20;

    DrawRectangle(sidebarX, sidebarY, sidebarWidth, sidebarHeight, WHITE);

    int startY = sidebarY + 10;
    int padding = 5;

    for (int i = 0; i < songCount; i++) {
        int posY = startY + (i *(20 + padding));
        DrawText(songTitles[i], sidebarX + 5, startY + (i * (20 + padding)), 10, DARKGRAY);

        if (posY > sidebarY + sidebarHeight - 20) {
            break;
        }
    }
}
