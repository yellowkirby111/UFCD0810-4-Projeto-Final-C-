// Raylib-based graphical menu
#include "raylib.h"
#include <string>
#include <iostream>

enum AppState { STATE_MENU, STATE_PLAY, STATE_OPTIONS, STATE_EXIT };

// Draw a simple button and return true when clicked
bool DrawButton(const Rectangle &r, const char *text, Color baseColor, int fontSize = 20) {
    Vector2 mouse = GetMousePosition();
    bool hovered = CheckCollisionPointRec(mouse, r);
    Color color = hovered ? Fade(baseColor, 0.9f) : baseColor;

    DrawRectangleRec(r, color);
    DrawRectangleLinesEx(r, 2, BLACK);

    int textWidth = MeasureText(text, fontSize);
    DrawText(text, (int)(r.x + (r.width - textWidth) / 2), (int)(r.y + (r.height - fontSize) / 2), fontSize, BLACK);

    if (hovered && IsMouseButtonReleased(MOUSE_LEFT_BUTTON)) return true;
    return false;
}

int main() {
    const int screenWidth = 800;
    const int screenHeight = 600;

    InitWindow(screenWidth, screenHeight, "Menu - Raylib");
    SetTargetFPS(60);

    AppState state = STATE_MENU;
    int menuIndex = 0;

    while (!WindowShouldClose() && state != STATE_EXIT) {
        if (IsKeyPressed(KEY_DOWN)) menuIndex = (menuIndex + 1) % 3;
        if (IsKeyPressed(KEY_UP)) menuIndex = (menuIndex + 2) % 3;
        if (IsKeyPressed(KEY_ENTER)) {
            if (menuIndex == 0) state = STATE_PLAY;
            else if (menuIndex == 1) state = STATE_OPTIONS;
            else state = STATE_EXIT;
        }

        BeginDrawing();
        ClearBackground(RAYWHITE);

        if (state == STATE_MENU) {
            DrawText("Main Menu", 320, 60, 30, DARKBLUE);

            Rectangle btnPlay = { 300, 150, 200, 60 };
            Rectangle btnOptions = { 300, 230, 200, 60 };
            Rectangle btnExit = { 300, 310, 200, 60 };

            if (DrawButton(btnPlay, "Start Game", LIGHTGRAY, 20)) state = STATE_PLAY;
            if (DrawButton(btnOptions, "Options", LIGHTGRAY, 20)) state = STATE_OPTIONS;
            if (DrawButton(btnExit, "Exit", LIGHTGRAY, 20)) state = STATE_EXIT;

            Rectangle selector = { 280, 150 + menuIndex * 80, 240, 60 };
            DrawRectangleLinesEx(selector, 3, RED);

            DrawText("Use Up/Down and Enter or click with mouse", 200, 400, 16, GRAY);
        }

        else if (state == STATE_PLAY) {
            DrawText("Game running... (press ESC to return to menu)", 120, 200, 20, DARKGREEN);
            static float x = 100;
            x += 2.0f;
            if (x > screenWidth + 50) x = -50;
            DrawCircle((int)x, 350, 40, SKYBLUE);

            if (IsKeyPressed(KEY_ESCAPE)) state = STATE_MENU;
        }

        else if (state == STATE_OPTIONS) {
            DrawText("Options", 360, 60, 30, DARKBLUE);
            DrawText("(This is a placeholder. Press ESC to return)", 180, 120, 18, GRAY);

            static bool fullscreen = false;
            Rectangle toggleRect = { 320, 200, 160, 40 };
            DrawRectangleRec(toggleRect, LIGHTGRAY);
            DrawRectangleLinesEx(toggleRect, 2, BLACK);
            DrawText(fullscreen ? "Fullscreen: ON" : "Fullscreen: OFF", 330, 210, 18, BLACK);

            if (CheckCollisionPointRec(GetMousePosition(), toggleRect) && IsMouseButtonReleased(MOUSE_LEFT_BUTTON)) {
                fullscreen = !fullscreen;
                ToggleFullscreen();
            }

            if (IsKeyPressed(KEY_ESCAPE)) state = STATE_MENU;
        }

        EndDrawing();
    }

    CloseWindow();
    std::cout << "Exiting application." << std::endl;
    return 0;
}