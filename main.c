#include <stdio.h>

#include <raylib.h>

int main(void) {
    const int screenWidth = 800;
    const int screenHeight = 800;

    InitWindow(screenWidth, screenHeight, "Raylib Test Window");

    SetTargetFPS(30);

    while(!WindowShouldClose()) {
        BeginDrawing();
            ClearBackground((Color){ GetTime()*10.0f, GetTime()*15.0f, GetTime()*20.0f, 255 });
            DrawText("Hello World!", 200, 200, 20, LIGHTGRAY);
        EndDrawing();

    }
    CloseWindow();

    return 0;
}
