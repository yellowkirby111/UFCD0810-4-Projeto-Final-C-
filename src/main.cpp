#include "raylib.h"
#include <iostream>
#include <string>
#include <fstream>
#include <vector>
#include <utility>
#include <cstring>

// Helper function to trim whitespace
std::string trim(const std::string& s) {
    size_t start = s.find_first_not_of(" \t\r\n");
    size_t end = s.find_last_not_of(" \t\r\n");
    return (start == std::string::npos) ? "" : s.substr(start, end - start + 1);
}

// Load users from file
std::vector<std::pair<std::string, std::string>> LoadUsers(const std::string& filename) {
    std::vector<std::pair<std::string, std::string>> users;
    std::ifstream file(filename);
    
    if (!file.is_open()) {
        std::cout << "Could not open file: " << filename << std::endl;
        return users; // Return empty vector to allow trying other locations
    }
    
    std::string line;
    while (std::getline(file, line)) {
        // Remove any trailing \r characters (Windows CRLF)
        if (!line.empty() && line.back() == '\r') {
            line.pop_back();
        }
        
        size_t sep = line.find(',');
        if (sep != std::string::npos && !line.empty()) {
            std::string user = trim(line.substr(0, sep));
            std::string pass = trim(line.substr(sep + 1));
            if (!user.empty() && !pass.empty()) {
                users.push_back({user, pass});
            }
        }
    }
    file.close();
    
    std::cout << "Successfully loaded " << users.size() << " users from " << filename << std::endl;
    return users;
}

// Check login credentials
bool CheckLogin(const std::vector<std::pair<std::string, std::string>>& users, const std::string& username, const std::string& password) {
    std::cout << "\n=== LOGIN CHECK DEBUG ===" << std::endl;
    std::cout << "Checking username: '" << username << "' (length: " << username.length() << ")" << std::endl;
    std::cout << "Checking password: '" << password << "' (length: " << password.length() << ")" << std::endl;
    
    for (size_t i = 0; i < users.size(); ++i) {
        const auto& u = users[i];
        std::cout << "Comparing with user[" << i << "]: '" << u.first << "' / '" << u.second << "'" << std::endl;
        
        bool userMatch = (u.first == username);
        bool passMatch = (u.second == password);
        
        std::cout << "  Username match: " << (userMatch ? "YES" : "NO") << std::endl;
        std::cout << "  Password match: " << (passMatch ? "YES" : "NO") << std::endl;
        
        if (userMatch && passMatch) {
            std::cout << "LOGIN SUCCESS!" << std::endl;
            return true;
        }
    }
    std::cout << "LOGIN FAILED - No matching user/password combination" << std::endl;
    return false;
}

enum AppState { STATE_LOGIN, STATE_MENU, STATE_VIEW_PRODUCTS, STATE_ADD_PRODUCT, STATE_OPTIONS, STATE_EXIT };

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

    // Try to load users from multiple possible locations
    std::vector<std::pair<std::string, std::string>> users;
    
    // Try current directory first
    users = LoadUsers("users.txt");
    if (users.empty()) {
        // Try src directory
        users = LoadUsers("src/users.txt");
    }
    
    // If still no users found, use defaults
    if (users.empty()) {
        std::cout << "No users.txt file found. Using default users:" << std::endl;
        users.push_back({"admin", "1234"});
        users.push_back({"user", "password"});
        std::cout << "Default users: admin/1234 and user/password" << std::endl;
    }

    // Debug: print all loaded users
    std::cout << "\n=== LOADED USERS DEBUG ===" << std::endl;
    for (size_t i = 0; i < users.size(); ++i) {
        const auto& u = users[i];
        std::cout << "User[" << i << "]: '" << u.first << "' / '" << u.second << "'" << std::endl;
    }
    std::cout << "===========================\n" << std::endl;

    AppState state = STATE_LOGIN;
    int menuIndex = 0;

    char username[32] = "";
    char password[32] = "";
    bool showPassword = false;
    bool loginFailed = false;
    int inputFocus = 0; // 0: username, 1: password

    while (!WindowShouldClose() && state != STATE_EXIT) {
        if (state == STATE_LOGIN) {
            if (IsKeyPressed(KEY_TAB)) inputFocus = 1 - inputFocus;
            if (IsKeyPressed(KEY_ENTER)) {
                std::cout << "\n=== LOGIN ATTEMPT ===" << std::endl;
                std::cout << "Username: '" << username << "'" << std::endl;
                std::cout << "Password: '" << password << "'" << std::endl;

                if (CheckLogin(users, std::string(username), std::string(password))) {
                    std::cout << "LOGIN SUCCESSFUL! Accessing main menu..." << std::endl;
                    state = STATE_MENU;
                    loginFailed = false;
                } else {
                    std::cout << "LOGIN FAILED! Please try again." << std::endl;
                    loginFailed = true;
                }
                std::cout << "====================\n" << std::endl;
            }
        }
        else if (state == STATE_MENU) {
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

        if (state == STATE_LOGIN) {
            DrawText("Login", 350, 100, 32, DARKBLUE);

            DrawText("Username:", 250, 200, 20, BLACK);
            DrawRectangle(370, 195, 200, 30, LIGHTGRAY);
            DrawText(username, 375, 200, 20, BLACK);
            if (inputFocus == 0) DrawRectangleLines(370, 195, 200, 30, RED);

            DrawText("Password:", 250, 250, 20, BLACK);
            DrawRectangle(370, 245, 200, 30, LIGHTGRAY);
            std::string passDisplay = showPassword ? password : std::string(strlen(password), '*');
            DrawText(passDisplay.c_str(), 375, 250, 20, BLACK);
            if (inputFocus == 1) DrawRectangleLines(370, 245, 200, 30, RED);

            DrawText("Press TAB to switch, ENTER to login", 250, 320, 18, GRAY);
            DrawText("Press SPACE to show/hide password", 250, 340, 18, GRAY);
            
            // Show test credentials
            DrawText("Test accounts:", 250, 380, 16, DARKGRAY);
            DrawText("admin / 1234", 250, 400, 16, DARKGRAY);
            DrawText("user / password", 250, 420, 16, DARKGRAY);

            if (loginFailed)
                DrawText("Login failed. Try again.", 280, 450, 20, RED);

            // Input handling
            int key = GetCharPressed();
            if (key >= 32 && key <= 125) {
                if (inputFocus == 0 && strlen(username) < 31) {
                    int len = strlen(username);
                    username[len] = (char)key;
                    username[len + 1] = '\0';
                }
                else if (inputFocus == 1 && strlen(password) < 31) {
                    int len = strlen(password);
                    password[len] = (char)key;
                    password[len + 1] = '\0';
                }
            }
            if (IsKeyPressed(KEY_BACKSPACE)) {
                if (inputFocus == 0 && strlen(username) > 0)
                    username[strlen(username) - 1] = '\0';
                else if (inputFocus == 1 && strlen(password) > 0)
                    password[strlen(password) - 1] = '\0';
            }
            if (IsKeyPressed(KEY_SPACE)) showPassword = !showPassword;
        }
        else if (state == STATE_MENU) {
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