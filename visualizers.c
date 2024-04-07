#include "globals.h"
#include "visualizers.h"

#include <raylib.h>
#include <math.h>

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

