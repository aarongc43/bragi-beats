// visualizers.c

#include "visualizers.h"

#include <raylib.h>
#include <math.h>
#include "rlgl.h"

// Define colors
#define COLOR_ACCENT        (Color){ 0, 122, 255, 255 }   // Apple Blue
#define COLOR_BACKGROUND    (Color){ 245, 245, 245, 255 } // Light Gray
#define COLOR_SHADOW        (Color){ 0, 0, 0, 25 }        // Subtle Shadow
#define MAX_PARTICLES 1000
Particle particles[MAX_PARTICLES];

// Helper function to interpolate between two colors
static Color ColorInterpolate(Color c1, Color c2, float t) {
    return (Color){
        .r = (unsigned char)(c1.r + t * (c2.r - c1.r)),
        .g = (unsigned char)(c1.g + t * (c2.g - c1.g)),
        .b = (unsigned char)(c1.b + t * (c2.b - c1.b)),
        .a = (unsigned char)(c1.a + t * (c2.a - c1.a))
    };
}

Color ColorFromHSV(float hue, float saturation, float value) {
    float c = value * saturation;
    float x = c * (1.0f - fabsf(fmodf(hue / 60.0f, 2) - 1.0f));
    float m = value - c;
    float r, g, b;

    if (hue >= 0 && hue < 60) {
        r = c; g = x; b = 0;
    } else if (hue >= 60 && hue < 120) {
        r = x; g = c; b = 0;
    } else if (hue >= 120 && hue < 180) {
        r = 0; g = c; b = x;
    } else if (hue >= 180 && hue < 240) {
        r = 0; g = x; b = c;
    } else if (hue >= 240 && hue < 300) {
        r = x; g = 0; b = c;
    } else {
        r = c; g = 0; b = x;
    }

    Color color;
    color.r = (unsigned char)((r + m) * 255);
    color.g = (unsigned char)((g + m) * 255);
    color.b = (unsigned char)((b + m) * 255);
    color.a = 255;
    return color;
}

// Helper function to create a gradient color based on index
static Color GetGradientColor(size_t index, size_t total) {
    float t = (float)index / (float)(total - 1);
    Color startColor = COLOR_ACCENT;
    Color endColor = (Color){ 255, 255, 255, 255 }; // White
    return ColorInterpolate(startColor, endColor, t);
}

void UpdateAndDrawParticles(float dt) {
    for (int i = 0; i < MAX_PARTICLES; i++) {
        if (particles[i].life > 0) {
            // Update particle
            particles[i].position.x += particles[i].velocity.x * dt;
            particles[i].position.y += particles[i].velocity.y * dt;
            particles[i].life -= dt;
            particles[i].size *= 0.99f; // Gradually decrease size

            // Draw particle
            Color fadedColor = particles[i].color;
            fadedColor.a = (unsigned char)(255 * (particles[i].life / 1.0f)); // Fade out
            DrawCircleV(particles[i].position, particles[i].size, fadedColor);
        }
    }
}

// Function to draw a bar chart visualizer
void DrawBarChart(float out_smooth[], size_t numBins, Rectangle area) {
    float barWidth = area.width / (float)numBins;
    float maxBarHeight = area.height;
    
    for (size_t i = 0; i < numBins; ++i) {
        float amplitude = out_smooth[i];
        float barHeight = amplitude * maxBarHeight;
        Rectangle bar = {
            .x = area.x + i * barWidth,
            .y = area.y + (maxBarHeight - barHeight),
            .width = barWidth * 0.8f,
            .height = barHeight
        };

        // Apply rounded corners and subtle shadow
        Color barColor = GetGradientColor(i, numBins);
        DrawRectangleRounded(bar, 0.2f, 6, barColor);

        // Draw shadow
        Rectangle shadowBar = bar;
        shadowBar.y += 2;
        shadowBar.height -= 2;
        DrawRectangleRounded(shadowBar, 0.2f, 6, COLOR_SHADOW);
    }
}

void DrawIridescentVisualizer(float out_smooth[], size_t numBins, Rectangle visualizerSpace) {
    float rotationAngle = 0.0f;
    // Calculate center point
    float centerX = visualizerSpace.x + visualizerSpace.width / 2;
    float centerY = visualizerSpace.y + visualizerSpace.height / 2;

    // Parameters for the visualizer
    float radius = (visualizerSpace.width < visualizerSpace.height ? visualizerSpace.width : visualizerSpace.height) / 2 - 50;
    float maxRadius = radius;
    float minRadius = radius / 2;
    float angleStep = 360.0f / numBins;

    static float hueOffset = 0.0f;

    for (size_t i = 0; i < numBins; ++i) {
        // Map amplitude to radius
        float amplitude = out_smooth[i];
        float normalizedAmplitude = amplitude; // Assuming amplitudes are normalized between 0 and 1

        float currentRadius = minRadius + normalizedAmplitude * (maxRadius - minRadius);

        // Calculate angle
        float angle = angleStep * i + rotationAngle;

        // Calculate hue based on angle and hue offset
        float hue = fmodf(angle + hueOffset, 360.0f);

        // Get color from HSV
        Color color = ColorFromHSV(hue, 1.0f, 1.0f);

        // Calculate start and end points of the arc
        float startAngle = angle;
        float endAngle = angle + angleStep;

        // Draw the arc or line
        float x1 = centerX + currentRadius * cosf(DEG2RAD * startAngle);
        float y1 = centerY + currentRadius * sinf(DEG2RAD * startAngle);
        float x2 = centerX + currentRadius * cosf(DEG2RAD * endAngle);
        float y2 = centerY + currentRadius * sinf(DEG2RAD * endAngle);

        int particlesToEmit = (int)(normalizedAmplitude * 10); // Adjust multiplier as needed
        for (int p = 0; p < particlesToEmit; p++) {
            // Find an inactive particle
            for (int j = 0; j < MAX_PARTICLES; j++) {
                if (particles[j].life <= 0) {
                    particles[j].position = (Vector2){x2, y2};
                    particles[j].velocity = (Vector2){
                        cosf(DEG2RAD * angle + GetRandomValue(-10, 10) * DEG2RAD) * GetRandomValue(50, 100),
                        sinf(DEG2RAD * angle + GetRandomValue(-10, 10) * DEG2RAD) * GetRandomValue(50, 100)
                    };
                    particles[j].color = color;
                    particles[j].life = 1.0f; // Particle life in seconds
                    particles[j].size = GetRandomValue(2, 5);
                    break;
                }
            }
        }

        rotationAngle += 0.1f;
        
        // Draw Bezier curve
        DrawLineBezier((Vector2){x1, y1}, (Vector2){x2, y2}, 5.0f, color);

        DrawCircleSector((Vector2){centerX, centerY}, currentRadius, startAngle, endAngle, 0, color);
    }

    // Increment hue offset for animation
    hueOffset += 0.5f; // Adjust speed as desired
    if (hueOffset >= 360.0f) {
        hueOffset -= 360.0f;
    }
    float dt = GetFrameTime();
    UpdateAndDrawParticles(dt);
}

void Draw3DTimeTunnelVisualizer(float out_smooth[], size_t numBins, Rectangle visualizerSpace) {
    static float cameraZ = 0.0f; // Camera position along the Z-axis
    static float rotationAngle = 0.0f;
    const float tunnelSpeed = 10.0f; // Speed of movement through the tunnel

    // Set up camera
    Camera camera = { 0 };
    camera.position = (Vector3){ 0.0f, 0.0f, cameraZ };
    camera.target = (Vector3){ 0.0f, 0.0f, cameraZ + 1.0f };
    camera.up = (Vector3){ 0.0f, 1.0f, 0.0f };
    camera.fovy = 90.0f;
    camera.projection = CAMERA_PERSPECTIVE;

    // Update camera position
    cameraZ -= tunnelSpeed * GetFrameTime();

    // Begin 3D mode
    BeginMode3D(camera);

    // Enable depth test for proper rendering
    rlDisableBackfaceCulling();
    rlEnableDepthTest();

    // Draw tunnel segments
    int segmentsAhead = 20; // Number of tunnel segments to draw ahead
    float segmentLength = 5.0f; // Length of each tunnel segment
    float baseRadius = 5.0f; // Base radius of the tunnel

    for (int i = 0; i < segmentsAhead; i++) {
        float zPos = cameraZ + i * segmentLength;

        // Calculate radius modulation based on audio data
        size_t binIndex = i % numBins;
        float amplitude = out_smooth[binIndex];
        float radius = baseRadius + amplitude * 2.0f; // Modulate radius

        // Create a torus (donut shape) for the tunnel segment
        Vector3 position = { 0.0f, 0.0f, zPos };

        // Calculate the rotation based on the current segment
        float segmentRotation = rotationAngle + i * 10.0f;

        // Determine the color based on audio data
        float hue = fmodf((amplitude * 360.0f) + (i * 10.0f), 360.0f);
        Color color = ColorFromHSV(hue, 1.0f, 1.0f);

        // Draw the tunnel segment
        // Since Raylib doesn't have a built-in torus, we'll draw a circle of quads
        int sides = 32;
        float angleStep = 360.0f / sides;
        for (int j = 0; j < sides; j++) {
            float angle1 = DEG2RAD * (j * angleStep + segmentRotation);
            float angle2 = DEG2RAD * ((j + 1) * angleStep + segmentRotation);

            Vector3 p1 = {
                position.x + radius * cosf(angle1),
                position.y + radius * sinf(angle1),
                position.z
            };
            Vector3 p2 = {
                position.x + radius * cosf(angle2),
                position.y + radius * sinf(angle2),
                position.z
            };
            Vector3 p3 = { p2.x, p2.y, position.z - segmentLength };
            Vector3 p4 = { p1.x, p1.y, position.z - segmentLength };

            // Draw quad
            rlBegin(RL_QUADS);
            rlColor4ub(color.r, color.g, color.b, color.a);
            rlVertex3f(p1.x, p1.y, p1.z);
            rlVertex3f(p2.x, p2.y, p2.z);
            rlVertex3f(p3.x, p3.y, p3.z);
            rlVertex3f(p4.x, p4.y, p4.z);
            rlEnd();
        }
    }

    // Disable depth test
    rlDisableDepthTest();

    // End 3D mode
    EndMode3D();

    // Update rotation angle for the twisting effect
    rotationAngle += 20.0f * GetFrameTime();

    // Optional: Draw overlay or additional effects here
}
