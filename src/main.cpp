#include "raylib.h"
#include <iostream>
#include <string>
#include <fstream>
#include <vector>
#include <utility>
#include <algorithm>
#include <iomanip>
#include <sstream>

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

    // Products storage
    struct Product { std::string name; double price; bool hasPrice; std::string size; };
    std::vector<Product> products;
    bool productsLoaded = false;
    float productsScroll = 0.0f;

    auto LoadProducts = [&](const std::string &path) -> bool {
        products.clear();
        std::ifstream ifs(path);
        if (!ifs) return false;
        std::string line;
        while (std::getline(ifs, line)) {
            if (line.empty()) continue;
            size_t pos = line.find(';');
            if (pos != std::string::npos) {
                std::string name = line.substr(0, pos);
                std::string rest = line.substr(pos + 1);
                // rest can be price or price;size
                std::string priceStr = rest;
                std::string sizeStr;
                size_t pos2 = rest.find(';');
                if (pos2 != std::string::npos) {
                    priceStr = rest.substr(0, pos2);
                    sizeStr = rest.substr(pos2 + 1);
                }
                double price = 0.0;
                bool ok = false;
                try {
                    // remove possible currency symbols and spaces
                    size_t start = 0;
                    while (start < priceStr.size() && !((priceStr[start] >= '0' && priceStr[start] <= '9') || priceStr[start] == '.' || priceStr[start] == '-')) start++;
                    std::string trimmed = priceStr.substr(start);
                    price = std::stod(trimmed);
                    ok = true;
                } catch (...) { ok = false; }
                products.push_back({ name, price, ok, sizeStr });
            } else {
                products.push_back({ line, 0.0, false, std::string() });
            }
        }

        // Sort products: priced items first (ascending by price), then unpriced items
        std::sort(products.begin(), products.end(), [](const Product &a, const Product &b) {
            if (a.hasPrice != b.hasPrice) return a.hasPrice; // true before false
            if (!a.hasPrice && !b.hasPrice) return a.name < b.name;
            return a.price < b.price;
        });

        return true;
    };

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
            // Load products once when entering view (or when not loaded)
            if (!productsLoaded) productsLoaded = LoadProducts("data/products.txt");

            DrawText("Product List", 340, 30, 28, DARKBLUE);
            DrawText("Press ESC to return to menu", 260, 70, 16, GRAY);

            // Scrolling via mouse wheel and arrow keys
            float wheel = GetMouseWheelMove();
            productsScroll -= wheel * 20.0f; // wheel up -> move list up
            if (IsKeyDown(KEY_DOWN)) productsScroll -= 2.0f;
            if (IsKeyDown(KEY_UP)) productsScroll += 2.0f;

            // Clamp scroll based on content
            float contentHeight = (float)products.size() * 30.0f;
            float minScroll = std::min(0.0f, 420.0f - contentHeight);
            if (productsScroll < minScroll) productsScroll = minScroll;
            if (productsScroll > 0) productsScroll = 0;

            int startY = 120;
            if (products.empty()) {
                DrawText("No products found. Create 'data/products.txt' with one product per line (name;price).", 60, 180, 18, RED);
            } else {
                for (size_t i = 0; i < products.size(); ++i) {
                    float y = startY + i * 30 + productsScroll;
                    if (y < 100 - 30 || y > screenHeight) continue; // simple culling
                    const auto &p = products[i];
                    std::string line = p.name;
                    if (p.hasPrice) {
                        std::ostringstream ss;
                        ss.setf(std::ios::fixed); ss.precision(2);
                        ss << " - $" << p.price;
                        line += ss.str();
                    }
                    if (!p.size.empty()) {
                        line += " (Size: ";
                        line += p.size;
                        line += ")";
                    }
                    DrawText(line.c_str(), 120, (int)y, 20, BLACK);
                }
            }

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