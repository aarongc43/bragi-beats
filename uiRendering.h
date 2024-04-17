#ifndef UI_RENDERING_H
#define UI_RENDERING_H

#include <raylib.h>
#include <stdlib.h>

extern Color OFFWHITE;
extern Color CUSTOMDARKGRAY;

typedef struct {
    int centerX;
    int centerY;
} VisualizerCenterPoint;

VisualizerCenterPoint CalculateLayout(Rectangle* titleBar, Rectangle* queue, Rectangle* visualizerSpace, Rectangle* playbackBar);
void DrawUI(Rectangle buttonBounds, bool *showList, int screenWidth, int screenHeight, Rectangle titleBar, Rectangle playbackControlPanel);
void RenderVisualizer(float out_smooth[], size_t numberFftBins, int centerX, int centerY, Rectangle visualizerSpace);
bool DrawButton(Rectangle bounds, const char* text, int fontSize);
void DrawTitleBar();
void DrawSongQueue(Rectangle queue);
void DrawBottomBar(int screenWidth, int screnHeight);
void DrawProgressBar(Music music, int screenHeight, int screenWidth);
void DrawVisualizerSelection(bool *showList, Rectangle buttonBounds);
void DrawLayout();

void DrawPlaybackControls(Rectangle playbackControlPanel);

#endif
