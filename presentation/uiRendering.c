#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../infrastructure/globals.h"
#include "uiRendering.h"
#include "visualizers.h"

Layout CalculateLayout(int screenWidth, int screenHeight) {
    Layout layout;
    layout.titleBar = (Rectangle){0, 0, screenWidth, screenHeight * 0.0875};
    layout.queue = (Rectangle){0, layout.titleBar.height, screenWidth * 0.20, screenHeight - layout.titleBar.height - (screenHeight * 0.15)};
    layout.library = layout.queue;
    layout.visualizerSpace = (Rectangle){layout.queue.width, layout.titleBar.height, screenWidth - layout.queue.width, screenHeight - layout.titleBar.height - (screenHeight * 0.15)};
    layout.playbackControlPanel = (Rectangle){0, screenHeight * 0.85, screenWidth, screenHeight * 0.15};
    layout.center.centerX = layout.visualizerSpace.x + layout.visualizerSpace.width / 2;
    layout.center.centerY = layout.visualizerSpace.y + layout.visualizerSpace.height / 2;

    return layout;
}

bool DrawButton(Rectangle bounds, const char* text, int fontSize) {
    Vector2 mousePoint = GetMousePosition();
    bool isHovering = CheckCollisionPointRec(mousePoint, bounds);
    bool clicked = false;

    if (isHovering) {
        DrawRectangleRec(bounds, IsMouseButtonDown(MOUSE_BUTTON_LEFT) ? CUSTOMDARKGRAY : LIGHTGRAY);
        if (IsMouseButtonDown(MOUSE_BUTTON_LEFT)) {
            clicked = true;
        }
    } else {
        DrawRectangleRec(bounds, GRAY);
    }

    int textWidth = MeasureText(text, fontSize);
    DrawText(text, bounds.x + (bounds.width / 2) - ((float)textWidth / 2), bounds.y + (bounds.height / 2) - (fontSize / 2), fontSize, BLACK);
    return clicked;
}

void DrawTextCentered(const char* text, Rectangle bounds, int fontSize, Color color) {
    int textWidth = MeasureText(text, fontSize);
    DrawText(text, bounds.x + (bounds.width - textWidth) / 2, bounds.y + (bounds.height - fontSize) / 2, fontSize, color);
}

void DrawTitleBar() {

}

void DrawLibraryOrQueue(Layout layout) {
    if (authorizedUser && showLibrary) {
        DrawLibrary(layout.library);
    } else {
        DrawSongQueue(layout.queue);
    }
}
void DrawLibrary(Rectangle libraryBounds) {
    printf("Drawing Library");
    int offsetY = 0;
    int itemHeight = 30;
    int indentSize = 30;
    int maxViewableItems = libraryBounds.height / itemHeight;
    int currentItem = 0;

    Album *currentAlbum = userLibrary.albums;

    while (currentAlbum != NULL && currentItem < maxViewableItems) {

        Rectangle albumBounds = {libraryBounds.x, libraryBounds.y + offsetY, libraryBounds.width, itemHeight};
        bool albumClicked = DrawButton(albumBounds, currentAlbum->name, 20);
        offsetY += itemHeight;
        currentItem++;

        if (currentAlbum->expanded) {
            Song *currentSong = currentAlbum->songs;
            while (currentSong != NULL && currentItem < maxViewableItems) {
                Rectangle songBounds = {libraryBounds.x + indentSize, libraryBounds.y + offsetY, libraryBounds.width - indentSize, itemHeight};
                DrawText(currentSong->name, songBounds.x + 5, songBounds.y + 5, 18, BLACK);

                offsetY += itemHeight;
                currentItem++;
                currentSong = currentSong->next;
            }
        }
        if (albumClicked) {
            currentAlbum->expanded =! currentAlbum->expanded;
        }
        if (offsetY > libraryBounds.height) {
            // Scroll feature
        }
    }
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

void DrawPlaybackControls(Rectangle playbackControlPanel) {
    const int buttonHeight = 30;
    const int buttonWidth = 80;
    const int spacing = 30;

    float middleX = playbackControlPanel.x + (playbackControlPanel.width / 2);
    float playPauseX = middleX - (buttonWidth / 2);
    float playPauseY = playbackControlPanel.y + (playbackControlPanel.height / 2) - (buttonHeight / 2);

    Rectangle playPauseBounds = {playPauseX, playPauseY, buttonWidth, buttonHeight};
    Rectangle skipBackBounds = {playPauseX - buttonWidth - spacing, playPauseY, buttonWidth, buttonHeight};
    Rectangle skipForwardBounds = {playPauseX + buttonWidth + spacing, playPauseY, buttonWidth, buttonHeight};

    bool hasSongInList = currentSong != NULL;
    if (currentSong != NULL) {

        if (DrawButton(playPauseBounds, isPlaying ? "Pause" : "Play", 20) && hasSongInList) {
            printf("Play/Pause");
            PlayPause();
            printf("Play/Pause");
        }

        if (DrawButton(skipBackBounds, "<<", 20) && hasSongInList) {
            printf("skip backward");
            SkipBackward();
            printf("skip backward");
        }

        if (DrawButton(skipForwardBounds, ">>", 20) && hasSongInList) {
            printf("skip forward");
            SkipForward();
            printf("skip forward");
        }
    }
}

/*
void DrawProgressBar(Music music, int screenHeight, int screenWidth) {
    printf("Drawing Progress Bar\n");
    printf("Music pointer: %p\n", (void*)&music);

    float songLength = GetMusicTimeLength(music);
    float currentTime = GetMusicTimePlayed(music);
    float progress = currentTime / songLength;
    int progressBarHeight = 20;
    int progressBarWidth = screenWidth - 200;

    float progressBarActualWidth = progressBarWidth * progress;
    float minProgressBarWidth = 5.0f;

    progressBarActualWidth = (progressBarActualWidth < minProgressBarWidth) ? minProgressBarWidth : progressBarActualWidth;

    Rectangle progressBarRectangle = {100, screenHeight - 50, progressBarActualWidth, progressBarHeight};

    printf("Progress: %f, Song Length: %f, Current Time: %f\n", progress, songLength, currentTime);
    printf("Progress Bar Width: %f, Actual Width: %f, Screen Height: %d\n", progressBarWidth, progressBarActualWidth, screenHeight);

    DrawRectangleRec(progressBarRectangle, LIGHTGRAY);
    DrawRectangleLines(100, screenHeight - 50, progressBarWidth, progressBarHeight, BLACK);
}
*/

void DrawProgressBar(Music music, int screenHeight, int screenWidth) {
    if (music.stream.buffer == NULL) return;  // Ensure there's a song loaded

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

void DrawUI(Layout layout) {
    int screenWidth = GetScreenWidth();
    int screenHeight = GetScreenHeight();
    layout = CalculateLayout(screenWidth, screenHeight);

    Rectangle visualizerButtonBounds = {
        .x = 20,
        .y = layout.titleBar.y + (layout.titleBar.height - 50) /2,
        .width = 120,
        .height = 50
    };

    Rectangle loginButtonBounds = {
        .x = visualizerButtonBounds.x + visualizerButtonBounds.width + 20,
        .y = visualizerButtonBounds.y,
        .width = 120,
        .height = 50
    };

    static bool visualizerButtonPreviouslyPressed = false;
    static bool loginButtonPreviouslyPressed = false;

    BeginDrawing();
    ClearBackground(OFFWHITE);
    DrawPlaybackControls(layout.playbackControlPanel);
    if (currentSong != NULL) {
        DrawProgressBar(currentSong->song, screenHeight, screenWidth);
    }
    //DrawRectangleRec(layout.titleBar, OFFWHITE);
    DrawLibraryOrQueue(layout);

    if (DrawButton(visualizerButtonBounds, "Visualizers", 20) && !visualizerButtonPreviouslyPressed) {
        visualizerButtonPreviouslyPressed = true;
        showList = !showList;
    } else if (!IsMouseButtonDown(MOUSE_BUTTON_LEFT)) {
        visualizerButtonPreviouslyPressed = false;
    }

    if (DrawButton(loginButtonBounds, "Login", 20) && !loginButtonPreviouslyPressed) {
        loginButtonPreviouslyPressed = true;
        LoginUser();
    } else if (!IsMouseButtonDown(MOUSE_BUTTON_LEFT)) {
        loginButtonPreviouslyPressed = false;
    }

    DrawVisualizerSelection(&showList, visualizerButtonBounds);


    if (isPlaying) {
        const char* text = "Playing Music...";
        int fontSize = 30;
        int textWidth = MeasureText(text, fontSize);
        int textX = layout.titleBar.x + (layout.titleBar.width - textWidth) / 2;
        int textY = layout.titleBar.y + (layout.titleBar.height - fontSize) / 2;
        DrawText(text, textX, textY, fontSize, CUSTOMDARKGRAY);
    } else if (!isPlaying) {
        const char* text = "Drag and Drop a Song to Start Playing";
        int fontSize = 34;
        int textWidth = MeasureText(text, fontSize);
        int textX = 150 + (screenWidth - textWidth) / 2;
        int textY = 20 + (screenHeight / 2) - (fontSize / 2);
        DrawText(text, textX, textY, fontSize, CUSTOMDARKGRAY);
    }

    EndDrawing();
}

void DrawVisualizerSelection(bool *showList, Rectangle buttonBounds) {
    const char* visualizerNames[] = {"Bar Chart", "Circle", "Circle Star", "Wing", "Kaleidoscope"};
    int visualizerCount = ARRAY_LEN(visualizerNames);
    int paddingBetweenButtonAndList = 10;
    int buttonHeight = 30;

    if (*showList) {
        int listStartY = buttonBounds.y + buttonBounds.height + paddingBetweenButtonAndList;

        for (int i = 0; i < visualizerCount; i++) {
            Rectangle itemBounds = {buttonBounds.x, listStartY + i * (buttonHeight + paddingBetweenButtonAndList), buttonBounds.width, buttonHeight};

            if (DrawButton(itemBounds, visualizerNames[i], 20)) {
                currentVisualizer = (VisualizerType)i;
                *showList = false;
            }
        }
    }
}

void RenderVisualizer(float out_smooth[], size_t m, int centerX, int centerY, Rectangle visualIzerSpace) {
    switch (currentVisualizer) {
        case VISUALIZER_BAR_CHART:
            barChartVisual(out_smooth, m, visualIzerSpace);
            break;
        case VISUALIZER_CIRCLE:
            circleVisual(out_smooth, m, centerX, centerY);
            break;
        case VISUALIZER_CIRCLE_STAR:
            circleStarVisual(out_smooth, m, centerX, centerY);
            break;
        case VISUALIZER_WING:
            wingVisual(out_smooth, m, centerX, centerY);
            break;
        case VISUALIZER_KALEIDOSCOPE:
            kaleidoscopeVisual(out_smooth, m, centerX, centerY);
            break;
        default:
            break;
    }
}

void LoginUser() {
    authorizedUser = true;
    showLibrary = true;
}
