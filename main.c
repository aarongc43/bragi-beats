#include <stdio.h>
#include <raylib.h>
#include <complex.h>
#include <string.h>

#include "globals.h"
#include "audioProcessing.h"
#include "uiRendering.h"

int screenWidth = 1200;
int screenHeight = 1100;

Color OFFWHITE = {236, 235, 243, 255};
Color CUSTOMDARKGRAY = {46, 53, 50, 255};

float in_raw[FFT_SIZE];
float in_win[FFT_SIZE];
float complex out_raw[FFT_SIZE];
float  out_log[FFT_SIZE];
float out_smooth[FFT_SIZE];

SongQueue songQueue = {.front = -1, .rear = -1};
char songTitles[MAX_SONGS][256];
int songCount = 0;

Music currentMusic;
bool isPlaying = false;

VisualizerType currentVisualizer = VISUALIZER_BAR_CHART;

void enqueueSong(Music song, const char* title);
void processDroppedFiles();

int main(void) {

    InitWindow(screenWidth, screenHeight, "Bragi Beats");
    SetTargetFPS(60);


    Rectangle titleBar, queue, visualizerSpace, playbackControlPanel;
    Rectangle visualizerButtonBounds = {
        .x = 10,
        .y = titleBar.y + (titleBar.height / 2) + 25,
        .width = 100,
        .height = 50
    };

    VisualizerCenterPoint center = CalculateLayout(&titleBar, &queue, &visualizerSpace, &playbackControlPanel);

    bool showList = false;

    InitAudioDevice();

    while(!WindowShouldClose()) {
        processDroppedFiles();

        BeginDrawing();
        DrawLayout();
        ProcessFFT(in_raw, out_log, out_smooth);
        DrawUI(visualizerButtonBounds, &showList, screenWidth, screenHeight, titleBar, playbackControlPanel);
        DrawVisualizerSelection(&showList, visualizerButtonBounds);

        size_t numberFftBins = ProcessFFT(in_raw, out_log, out_smooth);
        RenderVisualizer(out_smooth, numberFftBins, center.centerX, center.centerY, visualizerSpace);

        EndDrawing();
    }

    CloseAudioDevice();
    CloseWindow();

    return 0;
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
            UnloadMusicStream(currentMusic);

            songQueue.front++;

            if (songQueue.front > songQueue.rear) {
                isPlaying = false;
                songQueue.front = -1;
                songQueue.rear = -1;
            } else {
                currentMusic = songQueue.songs[songQueue.front];
                PlayMusicStream(currentMusic);
                SetMusicVolume(currentMusic, 0.5f);
                AttachAudioStreamProcessor(currentMusic.stream, callback);
            }
        }
    }
}
