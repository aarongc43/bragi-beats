#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "../infrastructure/globals.h"
#include "uiRendering.h"
#include "visualizers.h"

VisualizerCenterPoint CalculateLayout(Rectangle* titleBar, Rectangle* queue, Rectangle* visualizerSpace, Rectangle* playbackControlPanel) {
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

    playbackControlPanel->x = 0;
    playbackControlPanel->y = screenHeight * 0.85;
    playbackControlPanel->width = screenWidth;
    playbackControlPanel->height = screenHeight * 0.15;

    return center;
}

void DrawLayout() {
    Rectangle titleBar, queue, visualizerSpace, playbackControlPanel;
    CalculateLayout(&titleBar, &queue, &visualizerSpace, &playbackControlPanel);

    DrawRectangleRec(titleBar, OFFWHITE);
    DrawRectangleRec(queue, OFFWHITE);
    DrawRectangleRec(playbackControlPanel, OFFWHITE);
    if (authorizedUser && showLibrary) {
        DrawLibrary();
    } else {
        DrawSongQueue(queue);
    }
}

void DrawLibrary(Rectangle libraryBounds) {
    int offsetY = 0;
    int itemHeight = 30;
    int indentSize = 30;

    for (int i = 0; i < userLibrary.albumCount; i++) {
        Album *album = &userLibrary.albums[i];

        Rectangle albumBounds = {libraryBounds.x, libraryBounds.y + offsetY, libraryBounds.width, itemHeight};

        if (DrawButton(albumBounds, album->name, 20)) {
            for (int j = 0; j < album->songCount; j++) {
                album->expanded = !album->expanded;

                Rectangle songBounds = {libraryBounds.x + indentSize, libraryBounds.y + offsetY, libraryBounds.width - indentSize, itemHeight};
                DrawText(album->songs[j].name, songBounds.x + 5, songBounds.y + 5, 18, BLACK);
                offsetY += itemHeight;
            }
        }
    }
    
    if (offsetY > libraryBounds.height) {
        // scrolling mechanism
    }

}

bool DrawButton(Rectangle bounds, const char* text, int fontSize) {
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

    int textWidth = MeasureText(text, fontSize);

    DrawText(text, bounds.x + (bounds.width / 2) - ((float)textWidth / 2), bounds.y + (bounds.height / 2) - (fontSize / 2), fontSize, BLACK);

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

void DrawPlaybackControls(Rectangle playbackControlPanel) {
    const int buttonHeight = 30;
    const int buttonWidth = 80;
    const int spacing = 30;

    float playPauseX = playbackControlPanel.x + (playbackControlPanel.width / 2) - ((float)buttonWidth / 2);
    float playPauseY = playbackControlPanel.y + (playbackControlPanel.height / 2) - ((float)buttonHeight / 2) - spacing;

    Rectangle playPauseBounds = {playPauseX, playPauseY, buttonWidth, buttonHeight};
    Rectangle skipBackBounds = {playPauseX - buttonWidth - spacing, playPauseY, buttonWidth, buttonHeight};
    Rectangle skipForwardBounds = {playPauseX + buttonWidth + spacing, playPauseY, buttonWidth, buttonHeight};

    bool hasSongInList = currentSong != NULL;

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

void DrawUI(bool *showList, int screenWidth, int screenHeight, Rectangle titleBar, Rectangle playbackControlPanel) {

    float timeFactor = (sin(GetTime()) + 1.0f) / 2.0f; // Normalize to [0, 1]

    Color backgroundColor = {
        .r = (unsigned char)(OFFWHITE.r + (CUSTOMDARKGRAY.r - OFFWHITE.r) * timeFactor),
        .g = (unsigned char)(OFFWHITE.g + (CUSTOMDARKGRAY.g - OFFWHITE.g) * timeFactor),
        .b = (unsigned char)(OFFWHITE.b + (CUSTOMDARKGRAY.b - OFFWHITE.b) * timeFactor),
        .a = 255
    };

    Rectangle visualizerButtonBounds = {
        .x = 20,
        .y = titleBar.y + (titleBar.height - 50) / 2,
        .width = 200,
        .height = 50
    };

    Rectangle loginButtonBounds = {
        .x = visualizerButtonBounds.x + visualizerButtonBounds.width + 20,
        .y = titleBar.y + (titleBar.height - 50) / 2,
        .width = 200,
        .height = 50
    };

    ClearBackground(backgroundColor);

    if (DrawButton(visualizerButtonBounds, "Visualizers", 20)) {
        *showList = !(*showList);
    }

    if (DrawButton(loginButtonBounds, "Login", 20)) {
        LoginUser();
    }

    DrawVisualizerSelection(showList, visualizerButtonBounds);

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

    DrawPlaybackControls(playbackControlPanel);
    DrawProgressBar(currentMusic, screenWidth, screenHeight);

}

void DrawVisualizerSelection(bool *showList, Rectangle buttonBounds) {
    const char* visualizerNames[] = {"Bar Chart", "Circle Star", "Wing"};
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

void LoginUser() {
    authorizedUser = true;
    showLibrary = true;
}
