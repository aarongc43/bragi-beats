#ifndef VISUALIZERS_H
#define VISUALIZERS_H

#include <raylib.h>
#include <stdlib.h>

void barChartVisual(float out_smooth[], size_t m, Rectangle visualizerSpace);
void circleStarVisual(float out_smooth[], size_t m, int centerX, int centerY);
void circleVisual(float out_smooth[], size_t m, int centerX, int centerY);
void wingVisual(float out_smooth[], size_t m, int centerX, int centerY);
void kaleidoscopeVisual(float out_smooth[], size_t m, int centerX, int centerY);

#endif
