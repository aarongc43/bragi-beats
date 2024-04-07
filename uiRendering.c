#include <string.h>
#include <math.h>

#include "globals.h"
#include "uiRendering.h"
#include "visualizers.h"

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
