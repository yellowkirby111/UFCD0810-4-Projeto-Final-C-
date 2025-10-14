#include "raylib.h"
#include <iostream>
#include <string>

enum AppState { STATE_MENU, STATE_VIEW_PRODUCTS, STATE_ADD_PRODUCT, STATE_OPTIONS, STATE_EXIT };

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

    InitWindow(screenWidth, screenHeight, "Clothing Store App");
    SetTargetFPS(60);

    AppState state = STATE_MENU;
    int menuIndex = 0;

    while (!WindowShouldClose() && state != STATE_EXIT) {
        if (state == STATE_MENU) {
            if (IsKeyPressed(KEY_DOWN)) menuIndex = (menuIndex + 1) % 4;
            if (IsKeyPressed(KEY_UP)) menuIndex = (menuIndex + 3) % 4;
            if (IsKeyPressed(KEY_ENTER)) {
                if (menuIndex == 0) state = STATE_VIEW_PRODUCTS;
                else if (menuIndex == 1) state = STATE_ADD_PRODUCT;
                else if (menuIndex == 2) state = STATE_OPTIONS;
                else state = STATE_EXIT;
            }
        }

        BeginDrawing();
        ClearBackground(RAYWHITE);

        if (state == STATE_MENU) {
            DrawText("Clothing Store - Main Menu", 220, 60, 30, DARKBLUE);

            Rectangle btnView = { 300, 150, 200, 60 };
            Rectangle btnAdd = { 300, 230, 200, 60 };
            Rectangle btnOptions = { 300, 310, 200, 60 };
            Rectangle btnExit = { 300, 390, 200, 60 };

            if (DrawButton(btnView, "View Products", LIGHTGRAY, 20)) state = STATE_VIEW_PRODUCTS;
            if (DrawButton(btnAdd, "Add Product", LIGHTGRAY, 20)) state = STATE_ADD_PRODUCT;
            if (DrawButton(btnOptions, "Options", LIGHTGRAY, 20)) state = STATE_OPTIONS;
            if (DrawButton(btnExit, "Exit", LIGHTGRAY, 20)) state = STATE_EXIT;

            Rectangle selector = { 280.0f, 150.0f + menuIndex * 80.0f, 240.0f, 60.0f };
            DrawRectangleLinesEx(selector, 3, RED);

            DrawText("Use Up/Down and Enter or click with mouse", 200, 500, 16, GRAY);
        }
        else if (state == STATE_VIEW_PRODUCTS) {
            DrawText("Product List (placeholder)", 260, 200, 24, DARKGREEN);
            DrawText("Press ESC to return to menu", 260, 240, 18, GRAY);
            if (IsKeyPressed(KEY_ESCAPE)) state = STATE_MENU;
        }
        else if (state == STATE_ADD_PRODUCT) {
            DrawText("Add Product (placeholder)", 260, 200, 24, DARKGREEN);
            DrawText("Press ESC to return to menu", 260, 240, 18, GRAY);
            if (IsKeyPressed(KEY_ESCAPE)) state = STATE_MENU;
        }
        else if (state == STATE_OPTIONS) {
            DrawText("Options (placeholder)", 260, 200, 24, DARKBLUE);
            DrawText("Press ESC to return to menu", 260, 240, 18, GRAY);
            if (IsKeyPressed(KEY_ESCAPE)) state = STATE_MENU;
        }

        EndDrawing();
    }

    CloseWindow();
    std::cout << "Exiting application." << std::endl;
    return 0;
}