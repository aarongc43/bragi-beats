#include <assert.h>
#include <stdio.h>
#include <raylib.h>
#include <complex.h>
#include <math.h>
#include <string.h>

#define ARRAY_LEN(xs) sizeof(xs)/sizeof(xs[0])
#define N (1<<15)
#define MAX_SONGS 100

int screenWidth = 1200;
int screenHeight = 1100;

float in_raw[N];
float in_win[N];
float complex out_raw[N];
float  out_log[N];
float out_smooth[N];
char songTitles[MAX_SONGS][256];
int songCount = 0;

typedef struct {
    float left;
    float right;
} Frame;

typedef struct {
    int centerX;
    int centerY;
} VisualizerCenterPoint;

typedef struct {
    Music songs[MAX_SONGS];
    char titles[MAX_SONGS][256];
    int front, rear;
} SongQueue;

typedef enum {
    VISUALIZER_BAR_CHART,
    VISUALIZER_CIRCLE_STAR,
    VISUALIZER_WING
} VisualizerType;

VisualizerType currentVisualizer = VISUALIZER_BAR_CHART;

SongQueue songQueue = {.front = -1, .rear = -1};
Music currentMusic;
bool isPlaying = false;

Color OFFWHITE = {236, 235, 243, 255};
Color CUSTOMDARKGRAY = {46, 53, 50, 255};

// audio processing functions
void fft(float in[], size_t stride, float complex out[], size_t n);
void ProcessFFT(float in_raw[], float out_log[], float out_smooth[], int centerX, int centerY, Rectangle visualizerSpace);
float amp(float complex z);
void callback(void *bufferData, unsigned int frames);

// visualizers
void circleStarVisual(float out_smooth[], size_t m, int centerX, int centerY);
void wingVisual(float out_smooth[], size_t m, int centerX, int centerY);
void barChartVisual(float out_smooth[], size_t m, Rectangle visualizerSpace);

// UI functions
void DrawUI(Rectangle buttonBounds, bool *showList, int screenWidth, int screenHeight, Rectangle titleBar);
bool DrawButton(Rectangle bounds, const char* text);
void DrawTitleBar();
void DrawSongQueue(Rectangle queue);
void DrawBottomBar(int screenWidth, int screnHeight);
void DrawProgressBar(Music music, int screenHeight, int screenWidth);
void enqueueSong(Music song, const char* title);
void DrawVisualizerSelection(bool *showList, Rectangle buttonBounds);
void RenderCurrentVisualizer(float out_smooth[], size_t m, int centerX, int centerY, Rectangle visualizerSpace);

// utility functions
void processDroppedFiles();
VisualizerCenterPoint CalculateLayout(Rectangle* titleBar, Rectangle* queue, Rectangle* visualizerSpace, Rectangle* playbackBar);
void DrawLayout();

int main(void) {

    InitWindow(screenWidth, screenHeight, "Bragi Beats");
    SetTargetFPS(60);


    Rectangle titleBar, queue, visualizerSpace, playbackBar;
    Rectangle visualizerButtonBounds = {
        .x = 10,
        .y = titleBar.y + (titleBar.height / 2) + 25,
        .width = 100,
        .height = 50
    };

    VisualizerCenterPoint center = CalculateLayout(&titleBar, &queue, &visualizerSpace, &playbackBar);

    bool showList = false;

    InitAudioDevice();

    while(!WindowShouldClose()) {
        processDroppedFiles();

        BeginDrawing();
        DrawLayout();
        ProcessFFT(in_raw, out_log, out_smooth, center.centerX, center.centerY, visualizerSpace);
        DrawUI(visualizerButtonBounds, &showList, screenWidth, screenHeight, titleBar);
        DrawVisualizerSelection(&showList, visualizerButtonBounds);
        EndDrawing();
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

void ProcessFFT(float in_raw[], float out_log[], float out_smooth[], int centerX, int centerY, Rectangle visualizerSpace) {

    float dt = GetFrameTime();

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

    float smoothness = 7.5f;
    for (size_t i = 0; i < m; ++i) {
        out_smooth[i] += (out_log[i] - out_smooth[i]) * smoothness * dt;

    }

    RenderCurrentVisualizer(out_smooth, m, centerX, centerY, visualizerSpace);
}

void enqueueSong(Music song, const char* title) {
    if (songQueue.rear == MAX_SONGS - 1) return;
    if (songQueue.front == -1) songQueue.front = 0;
    songQueue.songs[++songQueue.rear] = song;
    strncpy(songQueue.titles[songQueue.rear], title, sizeof(songQueue.titles[0]) - 1);
    songQueue.titles[songQueue.rear][sizeof(songQueue.titles[0]) - 1] = '\0';
}

void processDroppedFiles() {
    if (IsFileDropped()) {
        FilePathList droppedFiles = LoadDroppedFiles();

        if (droppedFiles.count > 0 && IsFileExtension(droppedFiles.paths[0], ".wav")) {
            Music song = LoadMusicStream(droppedFiles.paths[0]);
            if (song.ctxData != NULL) {
                const char* title = GetFileName(droppedFiles.paths[0]);
                enqueueSong(song, title);

                // play immediatly if it is not already playing
                if (!isPlaying) {
                    currentMusic = songQueue.songs[songQueue.front];
                    PlayMusicStream(currentMusic);
                    SetMusicVolume(currentMusic, 0.5f);
                    AttachAudioStreamProcessor(currentMusic.stream, callback);
                    isPlaying = true;
                }
            }
        }
        UnloadDroppedFiles(droppedFiles);
    }

    if (isPlaying) {
        UpdateMusicStream(currentMusic);
        if (GetMusicTimePlayed(currentMusic) >= GetMusicTimeLength(currentMusic)) {
            // song is finished, go play the next one
            songQueue.front++;
            if (songQueue.front <= songQueue.rear) {
                UnloadMusicStream(currentMusic);
                currentMusic = songQueue.songs[songQueue.front];
                PlayMusicStream(currentMusic);
            } else {
                // queue is empty
                isPlaying = false;
                songQueue.front = -1;
                songQueue.rear = -1;
            }
        }
    }
}

void barChartVisual(float out_smooth[], size_t m, Rectangle visualizerSpace) {
    float barWidth = visualizerSpace.width / (float)m;
    float maxHeight = visualizerSpace.height;

    for (size_t i = 0; i < m; ++i) {

        float amplitude = out_smooth[i] * maxHeight;
        float lineThickness = 3.0f;

        Vector2 start = {visualizerSpace.x + barWidth * i, visualizerSpace.y + visualizerSpace.height};
        Vector2 end = {visualizerSpace.x + barWidth * i, visualizerSpace.y + visualizerSpace.height - amplitude};

        Color color = (Color){(unsigned char)(255 * sin(GetTime())), 128, (unsigned char)(255 * cos(GetTime())), 255};

        for (int j = 0; j < 5; j++) { // Example: Simulated glow effect
            float fadeFactor = (5 - j) / 5.0f; // Decrease opacity
            Color fadedColor = ColorAlpha(color, fadeFactor);
            DrawLineEx(start, end, lineThickness * fadeFactor, fadedColor);
        }
    }

}

void circleStarVisual(float out_smooth[], size_t m, int centerX, int centerY) {
    Color color = (Color){(unsigned char)(255 * sin(GetTime())), 128, (unsigned char)(255 * cos(GetTime())), 255};
    float maxRadius = ((float)screenHeight - 20 - 100) / 2;
    // This ensures that we always draw a line back to the start point.
    float angleStep = 360.0f / (float)(m-1);  

    for (size_t i = 0; i < m; ++i) {
        float amplitude = out_smooth[i % m]; // Use modulo to ensure the last point connects to the start
        float angle = angleStep * i;
        float lineThickness = 2.0f;
        float radian = angle * (PI / 180.0f);

        Vector2 start = { (float)centerX, (float)centerY };
        Vector2 end = { centerX + cos(radian) * (amplitude * maxRadius),
                        centerY + sin(radian) * (amplitude * maxRadius) };

        Color color = (Color){(unsigned char)(255 * sin(GetTime())), 128, (unsigned char)(255 * cos(GetTime())), 255};

        DrawLineEx(start, end, lineThickness, color);
    }
    // Explicitly close the circle by drawing the last line to the start point
    Vector2 start = { (float)centerX, (float)centerY };
    Vector2 end = { centerX + cos(0) * (out_smooth[0] * maxRadius),
                    centerY + sin(0) * (out_smooth[0] * maxRadius) };
    DrawLineEx(start, end, 2.0f, color);
}

void wingVisual(float out_smooth[], size_t m, int centerX, int centerY) {
    float maxRadius = ((float)screenHeight - 20 - 100) / 2;
    float angleStep = 180.0f / m;

    for (size_t i = 0; i < m; ++i) {

        float amplitude = out_smooth[i];
        float angle = angleStep * i;
        float lineThickness = 2.0f;
        float radian = angle * (PI / 180.0f);

        Vector2 start = { (float)centerX, (float)centerY };
        Vector2 end = { centerX + sin(radian) * (amplitude * maxRadius),
            centerY + cos(radian) * (amplitude * maxRadius) };
        Vector2 end2 = { centerX + sin(-radian) * (amplitude * maxRadius),
            centerY + cos(-radian) * (amplitude * maxRadius) };

        Color color = (Color){(unsigned char)(255 * sin(GetTime())), 128, (unsigned char)(255 * cos(GetTime())), 255};

        for (int j = 0; j < 5; j++) { // Example: Simulated glow effect
            float fadeFactor = (5 - j) / 5.0f; // Decrease opacity
            Color fadedColor = ColorAlpha(color, fadeFactor);
            DrawLineEx(start, end, lineThickness * fadeFactor, fadedColor);
            DrawLineEx(start, end2, lineThickness * fadeFactor, fadedColor);
        }
    }
}

VisualizerCenterPoint CalculateLayout(Rectangle* titleBar, Rectangle* queue, Rectangle* visualizerSpace, Rectangle* playbackBar) {
    int screenWidth = GetScreenWidth();
    int screenHeight = GetScreenHeight();

    titleBar->x = 0;
    titleBar->y = 0;
    titleBar->width = screenWidth;
    titleBar->height = screenHeight * 0.0875;

    queue->x = 0;
    queue->y = titleBar->height;
    queue->width = screenWidth * 0.20;
    queue->height = screenHeight - titleBar->height - (screenHeight * 0.15);

    visualizerSpace->x = queue->width;
    visualizerSpace->y = titleBar->height;
    visualizerSpace->width = screenWidth - queue->width;
    visualizerSpace->height = screenHeight  - titleBar->height - (screenHeight *0.15);

    VisualizerCenterPoint center;
    center.centerX = visualizerSpace->x + (visualizerSpace->width / 2);
    center.centerY = visualizerSpace->y + (visualizerSpace->height / 2);

    playbackBar->x = 0;
    playbackBar->y = screenHeight * 0.85;
    playbackBar->width = screenWidth;
    playbackBar->height = screenHeight * 0.15;

    return center;
}

void DrawLayout() {
    Rectangle titleBar, queue, visualizerSpace, playbackBar;
    CalculateLayout(&titleBar, &queue, &visualizerSpace, &playbackBar);

    DrawRectangleRec(titleBar, OFFWHITE);
    DrawRectangleRec(queue, OFFWHITE);
    DrawSongQueue(queue);
    // DrawRectangleRec(visualizerSpace, DARKGRAY);
    DrawRectangleRec(playbackBar, OFFWHITE);
}

bool DrawButton(Rectangle bounds, const char* text) {
    bool clicked = false;
    Vector2 mousePoint = GetMousePosition();
    bool isHovering = CheckCollisionPointRec(mousePoint, bounds);

    if (isHovering) {
        if (IsMouseButtonDown(MOUSE_BUTTON_LEFT)) {
            DrawRectangleRec(bounds, CUSTOMDARKGRAY);
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

    DrawText(text, bounds.x + (bounds.width / 2) - ((float)textWidth / 2), bounds.y + (bounds.height / 2) - 5, 10, BLACK);

    return clicked;
}

void DrawSongQueue(Rectangle queue) {
    int sidebarWidth = queue.width;
    int sidebarHeight = queue.height;
    int sidebarX = queue.x;
    int sidebarY = queue.y;

    DrawRectangle(sidebarX, sidebarY, sidebarWidth, sidebarHeight, OFFWHITE);

    int startY = sidebarY + 10;
    int padding = 5;
    int maxSongsVisible = (sidebarHeight - 20) / (20 + padding);
    int songIndex = songQueue.front;

    for (int i = songQueue.front; i <= songQueue.rear && i < songQueue.front + maxSongsVisible; i++) {
        int posY = startY + ((i - songQueue.front) * (20 + padding));
        char songTitle[256];
        strcpy(songTitle, GetFileName(songQueue.titles[songIndex]));
        int fontSize = 12;

        DrawText(songQueue.titles[i], sidebarX + 5, posY, fontSize, CUSTOMDARKGRAY);

        if (posY + 20 > sidebarY + sidebarHeight) {
            break;
        }

    }
}

void DrawProgressBar(Music music, int screenHeight, int screenWidth) {
    float songLength = GetMusicTimeLength(music);
    float currentTime = GetMusicTimePlayed(music);
    float progress = currentTime / songLength;
    int progressBarHeight = 20;
    int progressBarWidth = screenWidth - 200;

    float progressBarActualWidth = progressBarWidth * progress;
    float minProgressBarWidth = 5.0f;

    progressBarActualWidth = (progressBarActualWidth < minProgressBarWidth) ? minProgressBarWidth : progressBarActualWidth;

    Rectangle progressBarRectangle = {100, screenHeight - 50, progressBarActualWidth, progressBarHeight};
    DrawRectangleRec(progressBarRectangle, LIGHTGRAY);
    DrawRectangleLines(100, screenHeight - 50, progressBarWidth, progressBarHeight, BLACK);
}

void DrawUI(Rectangle buttonBounds, bool *showList, int screenWidth, int screenHeight, Rectangle titleBar) {

    float timeFactor = (sin(GetTime()) + 1.0f) / 2.0f; // Normalize to [0, 1]
    Color backgroundColor = {
        .r = (unsigned char)(OFFWHITE.r + (CUSTOMDARKGRAY.r - OFFWHITE.r) * timeFactor),
        .g = (unsigned char)(OFFWHITE.g + (CUSTOMDARKGRAY.g - OFFWHITE.g) * timeFactor),
        .b = (unsigned char)(OFFWHITE.b + (CUSTOMDARKGRAY.b - OFFWHITE.b) * timeFactor),
        .a = 255
    };

    ClearBackground(backgroundColor);

    if (DrawButton(buttonBounds, "Visualizers")) {
        *showList = !(*showList);
    }

    DrawVisualizerSelection(showList, buttonBounds);

    if (isPlaying) {
        const char* text = "Playing music...";
        int fontSize = 30;
        int textWidth = MeasureText(text, fontSize);
        int textX = titleBar.x + (titleBar.width - textWidth) / 2;
        int textY = titleBar.y + (titleBar.height - fontSize) / 2;

        DrawText(text, textX, textY, fontSize, CUSTOMDARKGRAY);
    } else {
        const char* text = "Drag and Drop a Song to Start Playing";
        int fontSize = 34;
        int textWidth = MeasureText(text, fontSize);
        int textX = 150 + (screenWidth - textWidth) / 2;
        int textY = 20 + (screenHeight / 2) - (fontSize / 2);
        DrawText(text, textX, textY, fontSize, CUSTOMDARKGRAY);
    }
}

void DrawVisualizerSelection(bool *showList, Rectangle buttonBounds) {
    const char* visualizerNames[] = {"Bar Chart", "Circle Star", "Wing"};
    int visualizerCount = ARRAY_LEN(visualizerNames);
    int paddingBetweenButtonAndList = 5;

    if (*showList) {
        int listStartY = buttonBounds.y + buttonBounds.height + paddingBetweenButtonAndList;

        for (int i = 0; i < visualizerCount; i++) {
            Rectangle itemBounds = {buttonBounds.x, listStartY + i * 25, buttonBounds.width, 20};
            if (DrawButton(itemBounds, visualizerNames[i])) {
                currentVisualizer = (VisualizerType)i;
                *showList = false;
            }
        }
    }
}

void RenderCurrentVisualizer(float out_smooth[], size_t m, int centerX, int centerY, Rectangle visualIzerSpace) {
    switch (currentVisualizer) {
        case VISUALIZER_BAR_CHART:
            barChartVisual(out_smooth, m, visualIzerSpace);
            break;
        case VISUALIZER_CIRCLE_STAR:
            circleStarVisual(out_smooth, m, centerX, centerY);
            break;
        case VISUALIZER_WING:
            wingVisual(out_smooth, m, centerX, centerY);
            break;
        default:
            break;
    }
}
