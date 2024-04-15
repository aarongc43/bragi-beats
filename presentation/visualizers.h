#ifndef VISUALIZERS_H
#define VISUALIZERS_H

#include <raylib.h>
#include <stdlib.h>

void circleStarVisual(float out_smooth[], size_t m, int centerX, int centerY);
void wingVisual(float out_smooth[], size_t m, int centerX, int centerY);
void barChartVisual(float out_smooth[], size_t m, Rectangle visualizerSpace);

#endif
