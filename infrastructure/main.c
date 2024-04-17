#include <stdio.h>
#include <raylib.h>
#include <complex.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>
#include <stdbool.h>

#include "globals.h"
#include "../business/audioProcessing.h"
#include "../presentation/uiRendering.h"
#include "../persistence/libraryInitialization.h"

int screenWidth = 1200;
int screenHeight = 900;

Color OFFWHITE = {236, 235, 243, 255};
Color CUSTOMDARKGRAY = {46, 53, 50, 255};

float in_raw[FFT_SIZE];
float in_win[FFT_SIZE];
float complex out_raw[FFT_SIZE];
float  out_log[FFT_SIZE];
float out_smooth[FFT_SIZE];

SongNode* head = NULL;
SongNode* tail = NULL;
SongNode* currentSong = NULL;

SongQueue songQueue = {.front = -1, .rear = -1};

char songTitles[MAX_SONGS][256];
int songCount = 0;

Music currentMusic;
Music previousMusic;
int currentSongIndex = -1;
bool isPlaying = false;
bool showList = false;

VisualizerType currentVisualizer = VISUALIZER_BAR_CHART;

bool authorizedUser = false;
bool showLibrary = false;

Library userLibrary = {NULL, 0};

void enqueueSong(Music song, const char* title);
void PlayNextSongInQueue();
void processDroppedFiles();
void playSong(SongNode *song);
void PlayPause();
void SkipBackward();
void SkipForward();
void processAlbumDirectory(const char *albumPath, const char *albumName);
bool IsDirectory(const char *path);

int main(void) {

    InitWindow(screenWidth, screenHeight, "Bragi Beats");
    SetTargetFPS(60);

    Layout layout = CalculateLayout(screenWidth, screenHeight);

    InitAudioDevice();


   while(!WindowShouldClose()) {
        processDroppedFiles();

        size_t numberFftBins = ProcessFFT(in_raw, out_log, out_smooth);
        DrawUI(layout);
        RenderVisualizer(out_smooth, numberFftBins, layout.center.centerX, layout.center.centerY, layout.visualizerSpace);
    }

    CloseAudioDevice();
    CloseWindow();

    return 0;
}

void enqueueSong(Music song, const char* title) {
    SongNode* newNode = (SongNode*)malloc(sizeof(SongNode));
    newNode->song = song;
    strncpy(newNode->title, title, sizeof(newNode->title) - 1);
    newNode->title[sizeof(newNode->title) - 1] = '\0';
    newNode->next = NULL;
    newNode->prev = tail;
    
    if (tail != NULL) {
        tail->next = newNode;
    }
    
    tail = newNode;

    if (head == NULL) {
        head = newNode;
    }

    if (!isPlaying) {
        currentSong = newNode;
        PlayMusicStream(newNode->song);
        SetMusicVolume(newNode->song, 0.5f);
        AttachAudioStreamProcessor(newNode->song.stream, callback);
        isPlaying = true;
    }
}

void processDroppedFiles() {
    if (IsFileDropped()) {
        FilePathList droppedFiles = LoadDroppedFiles();

        for (unsigned int i = 0; i < droppedFiles.count; i++) {
            if (IsDirectory(droppedFiles.paths[i])) {
                const char *albumName = GetFileName(droppedFiles.paths[i]);
                AddAlbumToLibrary(albumName);
                processAlbumDirectory(droppedFiles.paths[i], albumName);
            } else if (IsFileExtension(droppedFiles.paths[i], ".wav")) {
                Music song = LoadMusicStream(droppedFiles.paths[i]);
                if (song.ctxData != NULL) {
                    const char* title = GetFileName(droppedFiles.paths[i]);
                    enqueueSong(song, title);

                }
            }
        }
        UnloadDroppedFiles(droppedFiles);
    }

    if (isPlaying && currentSong != NULL) {
        UpdateMusicStream(currentSong->song);

        if (GetMusicTimePlayed(currentSong->song) >= GetMusicTimeLength(currentSong->song)) {
            SkipForward();
            
            if (currentSong == NULL) {
                isPlaying = false;
                currentSong = head;
            } else {
                PlayMusicStream(currentSong->song);
                SetMusicVolume(currentSong->song, 0.5f);
                AttachAudioStreamProcessor(currentSong->song.stream, callback);
            }
        }
    }
}

void PlayPause() {
    printf("Play/Pause()\n");
    if (currentSong != NULL) {
        isPlaying = !isPlaying;
        if (isPlaying) {
            PlayMusicStream(currentSong->song);
        } else {
            PauseMusicStream(currentSong->song);
        }
    }
}

void SkipForward() {
    printf("SkipForward()\n");
    if (currentSong && currentSong->next) {
        UnloadMusicStream(currentSong->song);
        currentSong = currentSong->next;
        PlayMusicStream(currentSong->song);
        SetMusicVolume(currentSong->song, 0.5f);
        AttachAudioStreamProcessor(currentSong->song.stream, callback);
    }
}

void SkipBackward() {
    printf("SkipBackward()");
    if (currentSong && currentSong->prev) {
        UnloadMusicStream(currentSong->song);
        currentSong = currentSong->prev;
        PlayMusicStream(currentSong->song);
        SetMusicVolume(currentSong->song, 0.5f);
        AttachAudioStreamProcessor(currentSong->song.stream, callback);
    }
}

void processAlbumDirectory(const char *albumPath, const char *albumName) {
    DIR *dir;
    struct dirent *entry;
    if ((dir = opendir(albumPath)) != NULL) {
        while ((entry = readdir(dir)) != NULL) {
            if (entry->d_type == DT_REG && IsFileExtension(entry->d_name, ".wav")) {
                char filePath[1024];
                sprintf(filePath, "%s\%s", albumPath, entry->d_name);
                Music song = LoadMusicStream(filePath);
                if (song.ctxData != NULL) {
                    AddSongToAlbum(albumName, entry->d_name);
                }
            }
        }
        closedir(dir);
    } else {
        perror("Failed to open album directory");
    }
}

bool IsDirectory(const char *path) {
    struct stat statbuf;
    if (stat(path, &statbuf) != 0)
        return 0;
    return S_ISDIR(statbuf.st_mode);
}
