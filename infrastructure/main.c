#include <stdio.h>
#include <raylib.h>
#include <complex.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>

#ifdef _WIN32
#include <windows.h>
#else
#include <dirent.h>
#include <sys/stat.h>
#endif

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
    
    if (songQueue.rear == MAX_SONGS - 1) {
        printf("Queue Full\n");
    } else {
        if (songQueue.front == -1) {
            songQueue.front = 0;
        }
        songQueue.rear++;
        strncpy(songQueue.titles[songQueue.rear], title, sizeof(songQueue.titles[songQueue.rear]) - 1);
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
    if (currentSong && currentSong->next) {
        UnloadMusicStream(currentSong->song);
        currentSong = currentSong->next;
        PlayMusicStream(currentSong->song);
        SetMusicVolume(currentSong->song, 0.5f);
        AttachAudioStreamProcessor(currentSong->song.stream, callback);
    }
}

void SkipBackward() {
    if (currentSong && currentSong->prev) {
        printf("SkipBackward() - Current song: %s\n", currentSong->title);
        UnloadMusicStream(currentSong->song);  // Correctly unload the current song
        currentSong = currentSong->prev;

        char fullPath[1024];  // Ensure the path is complete and correct
        sprintf(fullPath, "/path/to/music/directory/%s", currentSong->title);

        if (access(fullPath, F_OK) != -1) {  // Check if file exists
            currentSong->song = LoadMusicStream(fullPath);
            if (currentSong->song.stream.buffer != NULL) {
                PlayMusicStream(currentSong->song);
                SetMusicVolume(currentSong->song, 0.5f);
                AttachAudioStreamProcessor(currentSong->song.stream, callback);
            } else {
                printf("Error: Failed to load the previous song stream.\n");
            }
        } else {
            printf("Error: File does not exist [%s]\n", fullPath);
        }
    } else {
        printf("No previous song to play or failed to load song.\n");
        isPlaying = false;
    }
}

void processAlbumDirectory(const char *albumPath, const char *albumName) {
#ifdef _WIN32
    WIN32_FIND_DATA findFileData;
    HANDLE hFind = FindFirstFile(strcat(albumPath, "\\*"), &findFileData);

    if (hFind == INVALID_HANDLE_VALUE) {
        perror("Failed to open album directory");
        return;
    }

    do {
        if (findFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
            continue; // Skip directories
        } else {
            char filePath[1024];
            sprintf(filePath, "%s\\%s", albumPath, findFileData.cFileName);
            if (IsFileExtension(findFileData.cFileName, ".wav")) {
                Music song = LoadMusicStream(filePath);
                if (song.ctxData != NULL) {
                    AddSongToAlbum(albumName, findFileData.cFileName);
                }
            }
        }
    } while (FindNextFile(hFind, &findFileData) != 0);

    FindClose(hFind);
#else
    DIR *dir;
    struct dirent *entry;
    if ((dir = opendir(albumPath)) != NULL) {
        while ((entry = readdir(dir)) != NULL) {
            if (entry->d_type == DT_REG && IsFileExtension(entry->d_name, ".wav")) {
                char filePath[1024];
                sprintf(filePath, "%s/%s", albumPath, entry->d_name);
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
#endif
}

bool IsDirectory(const char *path) {
    struct stat statbuf;

    #ifdef _WIN32 
        if (_stat(path, &statbuf) != 0) {
        return false;
        }
        return (statbuf.st_mode & _S_IFDIR) != 0;
    #else 
        if (stat(path, &statbuf) != 0) {
            return 0;
        }
        return S_ISDIR(statbuf.st_mode);
    #endif
}

void LoginUser() {
    authorizedUser = true;
    showLibrary = true;
}
