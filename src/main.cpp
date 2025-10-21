#include "raylib.h"
#include <iostream>
#include <string>
#include <fstream>
#include <vector>
#include <utility>
#include <algorithm>
#include <cctype>
#include <iomanip>
#include <sstream>
#include <cstring>

enum AppState { STATE_LOGIN, STATE_REGISTER, STATE_MENU, STATE_VIEW_PRODUCTS, STATE_ADD_PRODUCT, STATE_OPTIONS, STATE_EXIT };

// Theme colors
#define DARK_BACKGROUND (Color){15, 15, 15, 255}      // Very dark gray/black
#define DARK_PRIMARY (Color){138, 43, 226, 255}       // Blue-violet purple
#define DARK_SECONDARY (Color){75, 0, 130, 255}       // Indigo purple
#define DARK_ACCENT (Color){221, 160, 221, 255}       // Plum (light purple)
#define DARK_TEXT (Color){240, 240, 240, 255}         // Light gray text
#define DARK_INPUT_BG (Color){40, 40, 40, 255}        // Dark gray for inputs
#define DARK_BUTTON_BG (Color){60, 60, 60, 255}       // Medium gray for buttons

#define LIGHT_BACKGROUND WHITE
#define LIGHT_PRIMARY (Color){138, 43, 226, 255}      // Same purple for consistency
#define LIGHT_SECONDARY (Color){75, 0, 130, 255}      // Same indigo purple
#define LIGHT_ACCENT (Color){221, 160, 221, 255}      // Plum accent
#define LIGHT_TEXT BLACK                              // Black text on light
#define LIGHT_INPUT_BG LIGHTGRAY                     // Light gray for inputs
#define LIGHT_BUTTON_BG (Color){220, 220, 220, 255}  // Light gray for buttons

enum Theme { THEME_DARK, THEME_LIGHT };

// Theme management
struct ColorScheme {
    Color background;
    Color primary;
    Color secondary;
    Color accent;
    Color text;
    Color inputBg;
    Color buttonBg;
};

ColorScheme GetColorScheme(Theme theme) {
    if (theme == THEME_DARK) {
        return { DARK_BACKGROUND, DARK_PRIMARY, DARK_SECONDARY, DARK_ACCENT, DARK_TEXT, DARK_INPUT_BG, DARK_BUTTON_BG };
    } else {
        return { LIGHT_BACKGROUND, LIGHT_PRIMARY, LIGHT_SECONDARY, LIGHT_ACCENT, LIGHT_TEXT, LIGHT_INPUT_BG, LIGHT_BUTTON_BG };
    }
}

// Function to load users from file
std::vector<std::pair<std::string, std::string>> LoadUsers(const std::string& filename) {
    std::vector<std::pair<std::string, std::string>> users;
    std::ifstream file(filename);
    if (file.is_open()) {
        std::string line;
        while (std::getline(file, line)) {
            size_t pos = line.find(':');
            if (pos != std::string::npos) {
                std::string username = line.substr(0, pos);
                std::string password = line.substr(pos + 1);
                users.push_back({username, password});
            }
        }
        file.close();
    }
    return users;
}

// Function to check login credentials
bool CheckLogin(const std::vector<std::pair<std::string, std::string>>& users, 
               const std::string& username, const std::string& password) {
    for (const auto& user : users) {
        if (user.first == username && user.second == password) {
            return true;
        }
    }
    return false;
}

bool DrawButton(const Rectangle &r, const char *text, Color baseColor, const ColorScheme &colors, int fontSize = 20) {
    Vector2 mouse = GetMousePosition();
    bool hovered = CheckCollisionPointRec(mouse, r);
    Color color = hovered ? Fade(baseColor, 0.8f) : baseColor;

    DrawRectangleRec(r, color);
    DrawRectangleLinesEx(r, 2, colors.primary);

    // scale font relative to a 600px reference height
    int scaledFont = std::max(8, (int)(fontSize * ((float)GetScreenHeight() / 600.0f)));
    int textWidth = MeasureText(text, scaledFont);
    DrawText(text, (int)(r.x + (r.width - textWidth) / 2), (int)(r.y + (r.height - scaledFont) / 2), scaledFont, colors.text);

    if (hovered && IsMouseButtonReleased(MOUSE_LEFT_BUTTON)) return true;
    return false;
}

// --- Scaled text helpers (use these anywhere instead of raw DrawText/MeasureText) ---
static inline int ScaledFontSize(int baseFontSize) {
    // Reference UI built for 600px height; scale linearly with current screen height
    return std::max(8, (int)(baseFontSize * ((float)GetScreenHeight() / 600.0f)));
}
static inline void DrawTextScaled(const char *text, int x, int y, int baseFontSize, Color color) {
    DrawText(text, x, y, ScaledFontSize(baseFontSize), color);
}
static inline int MeasureTextScaled(const char *text, int baseFontSize) {
    return MeasureText(text, ScaledFontSize(baseFontSize));
}

int main() {
    const int screenWidth = 800;
    const int screenHeight = 600;

    InitWindow(screenWidth, screenHeight, "Pepka");
    SetTargetFPS(60);

    // Window mode handling: support Windowed, Windowed-Fullscreen (bordered window resized to monitor),
    // and Fullscreen (real fullscreen). Use ApplyWindowMode(...) to change modes.
    enum WindowMode { WM_WINDOWED = 0, WM_WINDOWED_FULLSCREEN = 1, WM_FULLSCREEN = 2 };
    // Start in windowed-fullscreen by default so the app fills the screen but remains a window (bordered)
    WindowMode currentWindowMode = WM_WINDOWED_FULLSCREEN;

    auto ApplyWindowMode = [&](WindowMode mode) {
        int mw = GetMonitorWidth(0);
        int mh = GetMonitorHeight(0);
        bool isFs = IsWindowFullscreen();

        if (mode == WM_FULLSCREEN) {
            if (!isFs) {
                ToggleFullscreen(); // switch to fullscreen mode
            }
            // Ensure fullscreen size matches monitor to avoid canvas scaling issues
            SetWindowSize(mw, mh);
            SetWindowPosition(0, 0);
        } else if (mode == WM_WINDOWED_FULLSCREEN) {
            // Make sure we're not in exclusive fullscreen
            if (isFs) ToggleFullscreen();
            // Resize window to cover the monitor (bordered window)
            SetWindowSize(mw, mh);
            SetWindowPosition(0, 0);
        } else { // WM_WINDOWED
            if (isFs) ToggleFullscreen();
            SetWindowSize(screenWidth, screenHeight);
            SetWindowPosition((mw - screenWidth) / 2, (mh - screenHeight) / 2);
        }
        currentWindowMode = mode;
    };

    // Apply initial windowed mode (ensures consistent start state)
    ApplyWindowMode(currentWindowMode);
 
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
    
    // Theme variables
    Theme currentTheme = THEME_DARK;  // Start with dark theme
    ColorScheme colors = GetColorScheme(currentTheme);
    
    // Login variables
    char username[32] = "";
    char password[32] = "";
    // start with no field focused; user must click a box to type
    int inputFocus = -1;
    bool showPassword = false;
    bool loginFailed = false;
    std::string currentUser;
    bool isAdmin = false;
    
    // Registration variables
    char regUsername[32] = "";
    char regPassword[32] = "";
    char regConfirmPassword[32] = "";
    int regInputFocus = 0; // 0=username, 1=password, 2=confirm
    bool regShowPassword = false;
    bool regFailed = false;
    std::string regMessage = "";

    // Products storage
    struct Product { std::string name; double price; bool hasPrice; std::string size; std::string fabric; std::string sex; std::string description; };
    std::vector<Product> products;
    std::vector<Product> filteredProducts; // For search/sort results
    bool productsLoaded = false;
    float productsScroll = 0.0f;
    
    // Search and sort variables
    char searchInput[64] = "";
    bool searchActive = false;
    int sortMode = 0; // 0=default, 1=price asc, 2=price desc, 3=size asc, 4=size desc
    bool needsResort = true;

    auto LoadProducts = [&](const std::string &path) -> bool {
        products.clear();
        std::ifstream ifs(path);
        if (!ifs) return false;
        std::string line;
        while (std::getline(ifs, line)) {
            if (line.empty()) continue;
            // Split by ';' into tokens: name;price;size;description (description optional, may contain semicolons)
            std::vector<std::string> tokens;
            size_t start = 0;
            while (true) {
                size_t p = line.find(';', start);
                if (p == std::string::npos) {
                    tokens.push_back(line.substr(start));
                    break;
                }
                tokens.push_back(line.substr(start, p - start));
                start = p + 1;
            }
            std::string name = tokens.size() > 0 ? tokens[0] : std::string();
            std::string priceStr = tokens.size() > 1 ? tokens[1] : std::string();
            std::string sizeStr = tokens.size() > 2 ? tokens[2] : std::string();
            std::string fabricStr;
            std::string sexStr;
            std::string descStr;
            // Support both old and new formats. Preferred new format:
            // name;price;size;fabric;sex;description (description may contain ';')
            if (tokens.size() >= 6) {
                fabricStr = tokens[3];
                sexStr = tokens[4];
                descStr = tokens[5];
                for (size_t i = 6; i < tokens.size(); ++i) descStr += ";" + tokens[i];
            } else if (tokens.size() == 5) {
                // name;price;size;fabric;description  (no sex provided)
                fabricStr = tokens[3];
                descStr = tokens[4];
            } else if (tokens.size() == 4) {
                // older format: name;price;size;description
                descStr = tokens[3];
            }

            double price = 0.0;
            bool ok = false;
            if (!priceStr.empty()) {
                try {
                    size_t s = 0;
                    while (s < priceStr.size() && !((priceStr[s] >= '0' && priceStr[s] <= '9') || priceStr[s] == '.' || priceStr[s] == '-')) s++;
                    std::string trimmed = priceStr.substr(s);
                    price = std::stod(trimmed);
                    ok = true;
                } catch (...) { ok = false; }
            }
            products.push_back({ name, price, ok, sizeStr, fabricStr, sexStr, descStr });
        }

        // Sort products: priced items first (ascending by price), then unpriced items
        std::sort(products.begin(), products.end(), [](const Product &a, const Product &b) {
            if (a.hasPrice != b.hasPrice) return a.hasPrice; // true before false
            if (!a.hasPrice && !b.hasPrice) return a.name < b.name;
            return a.price < b.price;
        });

        return true;
    };
    
    auto FilterAndSortProducts = [&]() {
        // Start with all products
        filteredProducts.clear();
        std::string searchTerm = searchInput;
        std::transform(searchTerm.begin(), searchTerm.end(), searchTerm.begin(), ::tolower);
        
        // Filter by search term
        for (const auto& product : products) {
            if (searchTerm.empty()) {
                filteredProducts.push_back(product);
            } else {
                std::string productName = product.name;
                std::transform(productName.begin(), productName.end(), productName.begin(), ::tolower);
                if (productName.find(searchTerm) != std::string::npos) {
                    filteredProducts.push_back(product);
                }
            }
        }
        
        // Sort filtered products
        switch (sortMode) {
            case 1: // Price ascending
                std::sort(filteredProducts.begin(), filteredProducts.end(), [](const Product &a, const Product &b) {
                    if (a.hasPrice != b.hasPrice) return a.hasPrice; // priced items first
                    if (!a.hasPrice && !b.hasPrice) return a.name < b.name;
                    return a.price < b.price;
                });
                break;
            case 2: // Price descending
                std::sort(filteredProducts.begin(), filteredProducts.end(), [](const Product &a, const Product &b) {
                    if (a.hasPrice != b.hasPrice) return a.hasPrice; // priced items first
                    if (!a.hasPrice && !b.hasPrice) return a.name < b.name;
                    return a.price > b.price;
                });
                break;
            case 3: // Size ascending
                std::sort(filteredProducts.begin(), filteredProducts.end(), [](const Product &a, const Product &b) {
                    if (a.size.empty() != b.size.empty()) return !a.size.empty(); // items with size first
                    if (a.size.empty() && b.size.empty()) return a.name < b.name;
                    return a.size < b.size;
                });
                break;
            case 4: // Size descending
                std::sort(filteredProducts.begin(), filteredProducts.end(), [](const Product &a, const Product &b) {
                    if (a.size.empty() != b.size.empty()) return !a.size.empty(); // items with size first
                    if (a.size.empty() && b.size.empty()) return a.name < b.name;
                    return a.size > b.size;
                });
                break;
            default: // Default sorting (original logic)
                std::sort(filteredProducts.begin(), filteredProducts.end(), [](const Product &a, const Product &b) {
                    if (a.hasPrice != b.hasPrice) return a.hasPrice; // true before false
                    if (!a.hasPrice && !b.hasPrice) return a.name < b.name;
                    return a.price < b.price;
                });
                break;
        }
        
        needsResort = false;
    };
    
    auto SaveUser = [&](const std::string &username, const std::string &password) -> bool {
        // Check if user already exists
        for (const auto& user : users) {
            if (user.first == username) {
                return false; // User already exists
            }
        }
        
        // Append to users.txt file
        std::ofstream ofs("users.txt", std::ios::app);
        if (!ofs) return false;
        ofs << username << ":" << password << std::endl;
        ofs.close();
        
        // Add to current users list
        users.push_back({username, password});
        return true;
    };

    while (!WindowShouldClose() && state != STATE_EXIT) {
        // Handle ESC key navigation
        if (IsKeyPressed(KEY_ESCAPE)) {
            if (state == STATE_REGISTER) {
                state = STATE_LOGIN;
            } else if (state != STATE_LOGIN && state != STATE_MENU) {
                state = STATE_MENU;
            }
        }
        // Global fullscreen toggle (F11) - use smart toggle to avoid stretched scaling
        if (IsKeyPressed(KEY_F11)) {
            // Toggle between exclusive fullscreen and windowed-fullscreen (our base mode)
            if (currentWindowMode == WM_FULLSCREEN) ApplyWindowMode(WM_WINDOWED_FULLSCREEN);
            else ApplyWindowMode(WM_FULLSCREEN);
        }

        // Begin frame / clear background (required so raylib input buffering and drawing work correctly)
        BeginDrawing();
        ClearBackground(colors.background);

        // Responsive helpers (used by the drawing/input code)
        int sw = GetScreenWidth();
        int sh = GetScreenHeight();
        auto RX = [&](float px)->int { return (int)(px * sw); }; // relative x (0..1)
        auto RY = [&](float py)->int { return (int)(py * sh); }; // relative y (0..1)
        auto RW = [&](float pw)->int { return (int)(pw * sw); }; // relative width
        auto RH = [&](float ph)->int { return (int)(ph * sh); }; // relative height
        int centerX = sw / 2;

        if (state == STATE_LOGIN) {
            // place title near top-center (scaled)
            DrawTextScaled("Login", centerX - MeasureTextScaled("Login", 32)/2, RY(0.12f), 32, colors.primary);

            // Username / Password layout (same positions used for input handling)
            // Define layout variables
            int labelX = RX(0.28f);
            int inputX = RX(0.4625f);
            int inputW = RW(0.25f);
            int rowY = RY(0.33f);
            int passwordRowY = rowY + RH(0.083f);
            
            // Create rectangles for input fields (compute RH/RW into floats to avoid narrowing warnings)
            float inputH = (float)RH(0.05f);
            float usernameRectX = (float)inputX;
            float usernameRectY = (float)(rowY - 5);
            float inputWf = (float)inputW;
            Rectangle usernameRect = { usernameRectX, usernameRectY, inputWf, inputH };
            Rectangle passwordRect = { (float)inputX, (float)(passwordRowY - 5), inputWf, inputH };
            DrawTextScaled("Username:", labelX, rowY, 20, colors.text);
            DrawRectangleRec(usernameRect, colors.inputBg);
            DrawTextScaled(username, (int)usernameRect.x + 6, (int)usernameRect.y + 6, 20, colors.text);
            if (inputFocus == 0) DrawRectangleLinesEx(usernameRect, 2, colors.accent);

            DrawTextScaled("Password:", labelX, passwordRowY, 20, colors.text);
            DrawRectangleRec(passwordRect, colors.inputBg);
            std::string passDisplay = showPassword ? password : std::string(strlen(password), '*');
            DrawTextScaled(passDisplay.c_str(), (int)passwordRect.x + 6, (int)passwordRect.y + 6, 20, colors.text);
            if (inputFocus == 1) DrawRectangleLinesEx(passwordRect, 2, colors.accent);

            // Handle mouse focus on inputs
            Vector2 mousePos = GetMousePosition();
            if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                if (CheckCollisionPointRec(mousePos, usernameRect)) inputFocus = 0;
                else if (CheckCollisionPointRec(mousePos, passwordRect)) inputFocus = 1;
                else inputFocus = -1;
            }

            // Tab to switch fields (must click first or press Tab to focus)
            if (IsKeyPressed(KEY_TAB)) {
                if (inputFocus < 0) inputFocus = 0;
                else inputFocus = (inputFocus + 1) % 2;
            }

            // Enter to attempt login (works from any focus)
            if (IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_KP_ENTER)) {
                std::string su = std::string(username);
                std::string sp = std::string(password);
                if (CheckLogin(users, su, sp)) {
                    currentUser = su;
                    isAdmin = (currentUser == "admin");
                    loginFailed = false;
                    state = STATE_MENU;
                    // clear sensitive buffer if you want:
                    // memset(password, 0, sizeof(password));
                } else {
                    loginFailed = true;
                }
            }

            // Typing / backspace for the focused field
            if (inputFocus == 0 || inputFocus == 1) {
                int key = GetCharPressed();
                while (key > 0) {
                    if (key >= 32 && key <= 125) {
                        if (inputFocus == 0 && strlen(username) < (int)(sizeof(username) - 1)) {
                            int len = strlen(username);
                            username[len] = (char)key; username[len+1] = '\0';
                        } else if (inputFocus == 1 && strlen(password) < (int)(sizeof(password) - 1)) {
                            int len = strlen(password);
                            password[len] = (char)key; password[len+1] = '\0';
                        }
                    }
                    key = GetCharPressed();
                }
                if (IsKeyPressed(KEY_BACKSPACE)) {
                    if (inputFocus == 0 && strlen(username) > 0) username[strlen(username)-1] = '\0';
                    if (inputFocus == 1 && strlen(password) > 0) password[strlen(password)-1] = '\0';
                }
            }

            // Toggle password visibility with SPACE (user hint shown in Register screen too)
            if (IsKeyPressed(KEY_SPACE)) showPassword = !showPassword;

            // Register button centered under inputs
            float registerBtnW = (float)RW(0.1875f);
            float registerBtnH = (float)RH(0.05f);
            Rectangle registerBtn = { (float)(centerX) - registerBtnW/2.0f, (float)RY(0.55f), registerBtnW, registerBtnH };
            if (DrawButton(registerBtn, "Register New User", colors.secondary, colors, 16)) {
                state = STATE_REGISTER;
                strcpy(regUsername, "");
                strcpy(regPassword, "");
                strcpy(regConfirmPassword, "");
                regInputFocus = 0;
                regFailed = false;
                regMessage = "";
            }

            if (loginFailed)
                DrawTextScaled("Login failed. Try again.", centerX - MeasureTextScaled("Login failed. Try again.", 20)/2, RY(0.85f), 20, RED);
        }
        else if (state == STATE_REGISTER) {
            DrawTextScaled("Register New User", centerX - MeasureTextScaled("Register New User", 32)/2, RY(0.12f), 32, colors.primary);
            DrawTextScaled("Press ESC to return to login", centerX - MeasureTextScaled("Press ESC to return to login", 16)/2, RY(0.17f), 16, colors.accent);
            
            // registration inputs - use same layout logic as login (centered)
            int rLabelX = RX(0.28f);
            int rInputX = RX(0.4625f);
            int rInputW = RW(0.25f);
            int rY = RY(0.28f);
            DrawText("Username:", rLabelX, rY, 20, colors.text);
            DrawRectangle(rInputX, rY - 5, rInputW, RH(0.05f), colors.inputBg);
            DrawTextScaled(regUsername, rInputX + 5, rY, 20, colors.text);
            if (regInputFocus == 0) DrawRectangleLines(rInputX, rY - 5, rInputW, RH(0.05f), colors.accent);

            DrawText("Password:", rLabelX, rY + RH(0.083f), 20, colors.text);
            DrawRectangle(rInputX, rY + RH(0.078f), rInputW, RH(0.05f), colors.inputBg);
            std::string regPassDisplay = regShowPassword ? regPassword : std::string(strlen(regPassword), '*');
            DrawTextScaled(regPassDisplay.c_str(), rInputX + 5, rY + RH(0.083f), 20, colors.text);
            if (regInputFocus == 1) DrawRectangleLines(rInputX, rY + RH(0.078f), rInputW, RH(0.05f), colors.accent);

            DrawText("Confirm Password:", rLabelX - RW(0.05f), rY + RH(0.166f), 20, colors.text);
            DrawRectangle(rInputX, rY + RH(0.161f), rInputW, RH(0.05f), colors.inputBg);
            std::string regConfirmDisplay = regShowPassword ? regConfirmPassword : std::string(strlen(regConfirmPassword), '*');
            DrawTextScaled(regConfirmDisplay.c_str(), rInputX + 5, rY + RH(0.166f), 20, colors.text);
            if (regInputFocus == 2) DrawRectangleLines(rInputX, rY + RH(0.161f), rInputW, RH(0.05f), colors.accent);

            DrawTextScaled("Press TAB to switch fields, ENTER to register", RX(0.33f), RY(0.55f), 18, colors.accent);
            DrawTextScaled("Press SPACE to show/hide passwords", RX(0.36f), RY(0.58f), 18, colors.accent);
            DrawTextScaled("Minimum 3 characters for username and password", RX(0.33f), RY(0.61f), 16, colors.accent);
            
            // Back to login button
            Rectangle backToLoginBtn = { 200, 420, 120, 30 };
            if (DrawButton(backToLoginBtn, "← Back to Login", colors.buttonBg, colors, 16)) {
                state = STATE_LOGIN;
            }
            
            // Register button
            Rectangle regBtn = { 350, 420, 100, 30 };
            if (DrawButton(regBtn, "Register", colors.primary, colors, 16)) {
                // Trigger the same logic as ENTER key
                std::string regUser = std::string(regUsername);
                std::string regPass = std::string(regPassword);
                std::string regConfirm = std::string(regConfirmPassword);
                
                if (regUser.empty() || regPass.empty()) {
                    regFailed = true;
                    regMessage = "Username and password are required!";
                } else if (regPass != regConfirm) {
                    regFailed = true;
                    regMessage = "Passwords do not match!";
                } else if (regPass.length() < 3) {
                    regFailed = true;
                    regMessage = "Password must be at least 3 characters!";
                } else if (regUser.length() < 3) {
                    regFailed = true;
                    regMessage = "Username must be at least 3 characters!";
                } else {
                    if (SaveUser(regUser, regPass)) {
                        regFailed = false;
                        regMessage = "Registration successful! You can now login.";
                        // Clear registration form
                        strcpy(regUsername, "");
                        strcpy(regPassword, "");
                        strcpy(regConfirmPassword, "");
                        regInputFocus = 0;
                    } else {
                        regFailed = true;
                        regMessage = "Username already exists!";
                    }
                }
            }

            // Show success/error messages
            if (!regMessage.empty()) {
                Color msgColor = regFailed ? RED : GREEN;
                DrawTextScaled(regMessage.c_str(), centerX - MeasureTextScaled(regMessage.c_str(), 18)/2, RY(0.78f), 18, msgColor);
            }
        }
        else if (state == STATE_MENU) {
            // Logout button in top-left (use margin)
            float margin = 0.025f;
            Rectangle logoutBtn = { (float)RX(margin), (float)RY(margin), (float)RW(0.125f), (float)RH(0.05f) };
            if (DrawButton(logoutBtn, "← Logout", colors.buttonBg, colors, 16)) {
                 // Clear user data and return to login
                 currentUser = "";
                 isAdmin = false;
                 strcpy(username, "");
                 strcpy(password, "");
                 inputFocus = 0;
                 showPassword = false;
                 loginFailed = false;
                 state = STATE_LOGIN;
             }
             
            // Options button top-right
            Rectangle optionsBtn = { (float)(sw - RW(0.13f)), (float)RY(margin), (float)RW(0.10f), (float)RH(0.05f) };
            if (DrawButton(optionsBtn, "Options", colors.buttonBg, colors, 16)) {
                state = STATE_OPTIONS;
            }
            
            // Show current user info to left of options button
            std::string userInfo = "Logged in as: " + currentUser;
            if (isAdmin) userInfo += " (Admin)";
            DrawTextScaled(userInfo.c_str(), (int)(optionsBtn.x - 10 - MeasureTextScaled(userInfo.c_str(), 16)), RY(margin) + 6, 16, colors.accent);
             
            // Title and centered menu buttons
            DrawTextScaled("Pepka", centerX - MeasureTextScaled("Pepka", 60)/2, RY(0.18f), 60, colors.primary);

            Rectangle btnView = { (float)(centerX - RW(0.125f)), (float)RY(0.45f), (float)RW(0.25f), (float)RH(0.1f) };
            if (DrawButton(btnView, "View Products", colors.buttonBg, colors, 20)) state = STATE_VIEW_PRODUCTS;

            // Only show Add Product button if admin
            if (isAdmin) {
                Rectangle btnAdd = { (float)(centerX - RW(0.125f)), (float)RY(0.62f), (float)RW(0.25f), (float)RH(0.1f) };
                if (DrawButton(btnAdd, "Add Product", colors.buttonBg, colors, 20)) state = STATE_ADD_PRODUCT;
                
                // Move selector highlight based on menu index
                Rectangle selector = { btnView.x, btnView.y + menuIndex * (btnAdd.y - btnView.y), btnView.width, btnView.height };
                DrawRectangleLinesEx(selector, 3, DARK_ACCENT);
            } else {
                // Just highlight the view button for non-admin users
                DrawRectangleLinesEx(btnView, 3, DARK_ACCENT);
            }
         }
        else if (state == STATE_VIEW_PRODUCTS) {
            // Load & sort once
            if (!productsLoaded) { productsLoaded = LoadProducts("data/products.txt"); needsResort = true; }
            if (needsResort) FilterAndSortProducts();

            // responsive layout for list
            float margin = 0.025f;
            Rectangle backBtn = { (float)RX(margin), (float)RY(margin), (float)RW(0.10f), (float)RH(0.05f) };
            if (DrawButton(backBtn, "← Back", colors.buttonBg, colors, 16)) state = STATE_MENU;

            DrawText("Product List", centerX - MeasureText("Product List", 28)/2, RY(0.05f), 28, DARKBLUE);

            // Search area
            DrawText("Search:", RX(0.025f), RY(0.12f), 18, colors.text);
            Rectangle searchRect = { (float)RX(0.12f), (float)RY(0.11f), (float)RW(0.30f), (float)RH(0.05f) };
            DrawRectangleRec(searchRect, LIGHTGRAY);
            DrawText(searchInput, (int)searchRect.x + 6, (int)searchRect.y + 6, 18, BLACK);
            if (searchActive) DrawRectangleLinesEx(searchRect, 2, BLUE);

            Vector2 mouse = GetMousePosition();
            if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) searchActive = CheckCollisionPointRec(mouse, searchRect);
            if (searchActive) {
                int key = GetCharPressed();
                while (key > 0) {
                    if (key >= 32 && key <= 125 && strlen(searchInput) < 63) {
                        int len = strlen(searchInput);
                        searchInput[len] = (char)key; searchInput[len+1] = '\0'; needsResort = true;
                    }
                    key = GetCharPressed();
                }
                if (IsKeyPressed(KEY_BACKSPACE) && strlen(searchInput) > 0) {
                    searchInput[strlen(searchInput)-1] = '\0'; needsResort = true;
                }
            }

            // Sorting buttons
            float sortStartX = RX(0.45f);
            float sortW = RW(0.12f); float sortH = RH(0.05f); float sortGap = RW(0.02f);
            Rectangle sortPriceAscBtn = { sortStartX, RY(0.11f), sortW, sortH };
            Rectangle sortPriceDescBtn = { sortStartX + sortW + sortGap, RY(0.11f), sortW, sortH };
            Rectangle sortSizeAscBtn = { sortStartX + (sortW+sortGap)*2, RY(0.11f), sortW, sortH };
            Rectangle sortSizeDescBtn = { sortStartX + (sortW+sortGap)*3, RY(0.11f), sortW, sortH };
            Color sortBtnColor = colors.buttonBg;
            if (DrawButton(sortPriceAscBtn, "Price ^", (sortMode==1)?LIME:sortBtnColor, colors, 14)) { sortMode=1; needsResort=true; }
            if (DrawButton(sortPriceDescBtn, "Price v", (sortMode==2)?LIME:sortBtnColor, colors, 14)) { sortMode=2; needsResort=true; }
            if (DrawButton(sortSizeAscBtn, "Size ^", (sortMode==3)?LIME:sortBtnColor, colors, 14)) { sortMode=3; needsResort=true; }
            if (DrawButton(sortSizeDescBtn, "Size v", (sortMode==4)?LIME:sortBtnColor, colors, 14)) { sortMode=4; needsResort=true; }

            // Info
            if (!searchInput[0] && sortMode == 0) DrawText("Showing all products", RX(0.025f), RY(0.17f), 16, GRAY);
            else {
                std::string info = "Showing " + std::to_string(filteredProducts.size()) + " of " + std::to_string(products.size()) + " products";
                if (searchInput[0]) info += " (filtered)";
                DrawText(info.c_str(), RX(0.025f), RY(0.17f), 16, GRAY);
            }

            // Scroll & list
            float wheel = GetMouseWheelMove(); productsScroll -= wheel * RH(0.05f);
            if (IsKeyDown(KEY_DOWN)) productsScroll -= RH(0.01f);
            if (IsKeyDown(KEY_UP)) productsScroll += RH(0.01f);
            float rowH = (float)RH(0.05f);
            float contentH = (float)filteredProducts.size() * rowH;
            float minScroll = std::min(0.0f, RY(0.78f) - contentH);
            if (productsScroll < minScroll) productsScroll = minScroll;
            if (productsScroll > 0) productsScroll = 0;

            float startY = RY(0.23f);
            if (products.empty()) {
                DrawText("No products found. Create 'data/products.txt' with one product per line (name;price).", RX(0.05f), RY(0.35f), 18, RED);
            } else if (filteredProducts.empty()) {
                DrawText("No products match your search criteria.", centerX - MeasureText("No products match your search criteria.", 18)/2, RY(0.40f), 18, ORANGE);
            } else {
                static int viewDescriptionIndex = -1;
                for (size_t i = 0; i < filteredProducts.size(); ++i) {
                    float y = startY + i * rowH + productsScroll;
                    if (y < RY(0.20f) - rowH || y > sh) continue;
                    const auto &p = filteredProducts[i];
                    std::string line = p.name;
                    if (p.hasPrice) { std::ostringstream ss; ss.setf(std::ios::fixed); ss.precision(2); ss << " - $" << p.price; line += ss.str(); }
                    if (!p.size.empty()) { line += " (Size: " + p.size + ")"; }
                    DrawTextScaled(line.c_str(), RX(0.03f), (int)y, 20, colors.text);

                    Rectangle viewBtn = { (float)(sw - RW(0.18f)), y - rowH*0.15f, (float)RW(0.14f), (float)(rowH*0.85f) };
                    if (DrawButton(viewBtn, "View", colors.buttonBg, colors, 14)) viewDescriptionIndex = (int)i;
                }

                if (viewDescriptionIndex >= 0 && viewDescriptionIndex < (int)filteredProducts.size()) {
                    const auto &p = filteredProducts[viewDescriptionIndex];
                    float modalW = RW(0.75f), modalH = RH(0.55f);
                    Rectangle modal = { (float)(centerX - modalW/2), RY(0.18f), modalW, modalH };
                    DrawRectangleRec(modal, Fade(colors.inputBg, 0.98f)); DrawRectangleLinesEx(modal, 2, colors.accent);
                    DrawTextScaled(p.name.c_str(), (int)modal.x + 20, (int)modal.y + 18, 24, colors.text);

                    // Show fabric and sex metadata if available
                    int metaY = (int)modal.y + 54;
                    if (!p.fabric.empty()) {
                        std::string fabricLine = std::string("Fabric: ") + p.fabric;
                        DrawTextScaled(fabricLine.c_str(), (int)modal.x + 20, metaY, 18, colors.text);
                        metaY += 22;
                    }
                    if (!p.sex.empty()) {
                        std::string sexLine = std::string("For: ") + p.sex;
                        DrawTextScaled(sexLine.c_str(), (int)modal.x + 20, metaY, 18, colors.text);
                        metaY += 22;
                    }

                    std::string desc = p.description.empty() ? "(No description)" : p.description;
                    int descY = metaY + 6;
                    int maxWidth = (int)modal.width - 40;
                    std::istringstream iss(desc);
                    std::string word;
                    std::string lineBuf;
                    while (iss >> word) {
                        std::string tryLine = lineBuf.empty() ? word : (lineBuf + " " + word);
                        if (MeasureTextScaled(tryLine.c_str(), 18) > maxWidth) { DrawTextScaled(lineBuf.c_str(), (int)modal.x + 20, descY, 18, colors.text); descY += 22; lineBuf = word; }
                        else lineBuf = tryLine;
                    }
                    if (!lineBuf.empty()) DrawTextScaled(lineBuf.c_str(), (int)modal.x + 20, descY, 18, colors.text);
                    Rectangle closeBtn = { modal.x + modal.width - RW(0.12f), modal.y + modal.height - RH(0.08f), RW(0.12f), RH(0.08f) };
                    if (DrawButton(closeBtn, "Close", colors.buttonBg, colors, 16)) viewDescriptionIndex = -1;
                }
            }
        }
        else if (state == STATE_ADD_PRODUCT) {
            Rectangle backBtn = { (float)RX(0.025f), (float)RY(0.025f), (float)RW(0.10f), (float)RH(0.05f) };
            if (DrawButton(backBtn, "← Back", colors.buttonBg, colors, 16)) state = STATE_MENU;
            DrawTextScaled("Add Product", centerX - MeasureTextScaled("Add Product", 28)/2, RY(0.05f), 28, colors.primary);

            if (currentUser.empty() || !isAdmin) {
                DrawText("Admin privileges required to add products.", centerX - MeasureText("Admin privileges required to add products.", 20)/2, RY(0.20f), 20, colors.accent);
                DrawText("Please login with an admin account.", centerX - MeasureText("Please login with an admin account.", 18)/2, RY(0.26f), 18, colors.text);
                Rectangle btnToLogin = { (float)(centerX - RW(0.15f)), RY(0.36f), (float)RW(0.30f), (float)RH(0.08f) };
                if (DrawButton(btnToLogin, "Go to Login", colors.buttonBg, colors, 20)) { memset(username,0,sizeof(username)); memset(password,0,sizeof(password)); state = STATE_LOGIN; }
            } else {
                // Responsive, centered Add Product form
                static std::string nameInput, priceInput, sizeInput, removeInput, msg;
                static int activeFieldAdd = 0; // 0=name,1=price,2=size,3=remove

                float sectionTop = RY(0.08f);
                DrawText("Add Product", centerX - MeasureText("Add Product", 28)/2, RY(0.05f), 28, colors.primary);

                float inputW = RW(0.70f);
                float inputH = RH(0.06f);
                float gapV = RH(0.04f);

                Rectangle nameRect = { centerX - inputW/2.0f, sectionTop + gapV, inputW, inputH };
                Rectangle priceRect = { centerX - RW(0.35f), nameRect.y + inputH + gapV, RW(0.28f), inputH };
                Rectangle removeRect = { centerX - inputW/2.0f, priceRect.y + inputH + gapV, inputW, inputH };

                // Draw name input
                DrawText("Name:", nameRect.x - RW(0.08f), nameRect.y + 4, 20, colors.text);
                DrawRectangleRec(nameRect, colors.inputBg);
                DrawText(nameInput.c_str(), (int)nameRect.x + 6, (int)nameRect.y + 6, 20, colors.text);
                if (activeFieldAdd == 0) DrawRectangleLinesEx(nameRect, 2, colors.accent);

                // Draw price input
                DrawText("Price:", priceRect.x - RW(0.08f), priceRect.y + 4, 20, colors.text);
                DrawRectangleRec(priceRect, colors.inputBg);
                DrawText(priceInput.c_str(), (int)priceRect.x + 6, (int)priceRect.y + 6, 20, colors.text);
                if (activeFieldAdd == 1) DrawRectangleLinesEx(priceRect, 2, colors.accent);

                // Size selection (centered row) — placed below the price input
                static const std::vector<std::string> sizeOptions = {"XS","S","M","L","XL","XXL"};
                float sbtnW = RW(0.10f), sbtnH = inputH, sGap = RW(0.02f);
                float totalS = sbtnW * (float)sizeOptions.size() + sGap * ((float)sizeOptions.size() - 1.0f);
                float startSx = centerX - totalS/2.0f;
                float sBtnsY = priceRect.y + inputH + RH(0.02f);

                // Size label above buttons
                DrawText("Size:", centerX - MeasureText("Size:", 20)/2, sBtnsY - RH(0.045f), 20, colors.text);

                for (size_t si = 0; si < sizeOptions.size(); ++si) {
                    Rectangle sb = { startSx + si * (sbtnW + sGap), sBtnsY, sbtnW, sbtnH };
                    if (DrawButton(sb, sizeOptions[si].c_str(), colors.buttonBg, colors, 18)) { sizeInput = sizeOptions[si]; activeFieldAdd = 2; }
                    if (!sizeInput.empty() && sizeInput == sizeOptions[si]) DrawRectangleLinesEx(sb, 2, RED);
                }

                // Remove input (moved below size buttons) - add extra vertical spacing
                removeRect.y = sBtnsY + sbtnH + RH(0.06f); // increased gap for clearer separation
                DrawText("Remove product:", removeRect.x - RW(0.18f), removeRect.y + 4, 20, colors.text);
                DrawRectangleRec(removeRect, LIGHTGRAY);
                DrawText(removeInput.c_str(), (int)removeRect.x + 6, (int)removeRect.y + 6, 20, BLACK);
                if (activeFieldAdd == 3) DrawRectangleLinesEx(removeRect, 2, RED);

                // Input handling (preserve behavior)
                int cp = GetCharPressed(); while (cp>0) { if (cp>=32 && cp<=125) { if (activeFieldAdd==0 && nameInput.size()<120) nameInput.push_back((char)cp); else if (activeFieldAdd==1 && priceInput.size()<32) priceInput.push_back((char)cp); else if (activeFieldAdd==3 && removeInput.size()<120) removeInput.push_back((char)cp); } cp=GetCharPressed(); }
                if (IsKeyPressed(KEY_TAB)) activeFieldAdd = (activeFieldAdd+1)%4;
                if (IsKeyPressed(KEY_BACKSPACE)) { if (activeFieldAdd==0 && !nameInput.empty()) nameInput.pop_back(); else if (activeFieldAdd==1 && !priceInput.empty()) priceInput.pop_back(); else if (activeFieldAdd==3 && !removeInput.empty()) removeInput.pop_back(); }

                // Action buttons centered
                float actionY = removeRect.y + inputH + gapV;
                float actionW = RW(0.22f), actionH = RH(0.08f), actionGap = RW(0.04f);
                float totalActionW = actionW * 3 + actionGap * 2;
                float actionStartX = centerX - totalActionW/2.0f;
                Rectangle btnSave = { actionStartX, actionY, actionW, actionH };
                Rectangle btnRemove = { actionStartX + actionW + actionGap, actionY, actionW, actionH };
                Rectangle btnCancel = { actionStartX + 2*(actionW + actionGap), actionY, actionW, actionH };

                if (DrawButton(btnSave, "Save", colors.primary, colors, 20)) {
                    double pr=0.0; bool ok=false;
                    try { size_t start=0; while (start<priceInput.size() && !((priceInput[start]>='0'&&priceInput[start]<='9')||priceInput[start]=='.'||priceInput[start]=='-')) start++; std::string trimmed = priceInput.substr(start); if (!trimmed.empty()) { pr = std::stod(trimmed); ok = true; } } catch(...) { ok=false; }
                    if (!ok) msg="Invalid price"; else if (nameInput.empty()) msg="Name required"; else {
                        std::ofstream ofs("data/products.txt", std::ios::app);
                        if (ofs) { ofs << nameInput << ";" << pr; if (!sizeInput.empty()) ofs << ";" << sizeInput; ofs << "\n"; ofs.close(); msg="Product saved"; nameInput.clear(); priceInput.clear(); sizeInput.clear(); removeInput.clear(); productsLoaded=false; needsResort=true; } else msg="Failed to open file";
                    }
                }
                if (DrawButton(btnRemove, "Remove", colors.buttonBg, colors, 20)) {
                    if (removeInput.empty()) msg="Enter a product name to remove";
                    else {
                        std::ifstream ifs("data/products.txt"); if (!ifs) msg="Failed to open products file"; else {
                            std::vector<std::string> lines; std::string line; int removed=0;
                            auto toLower = [](const std::string &s){ std::string out=s; std::transform(out.begin(),out.end(),out.begin(),::tolower); return out; };
                            std::string target = toLower(removeInput);
                            while (std::getline(ifs,line)) { if (line.empty()) continue; std::string nameOnly=line; size_t pos=line.find(';'); if (pos!=std::string::npos) nameOnly=line.substr(0,pos); if (toLower(nameOnly)==target) { removed++; continue; } lines.push_back(line); }
                            ifs.close();
                            std::ofstream ofs("data/products.txt", std::ios::trunc);
                            if (!ofs) msg="Failed to write products file"; else { for (auto &l:lines) ofs<<l<<"\n"; ofs.close(); if (removed>0) { msg = "Removed " + std::to_string(removed) + " product(s)"; removeInput.clear(); productsLoaded=false; needsResort=true; } else msg="No product matched that name"; }
                        }
                    }
                }
                if (DrawButton(btnCancel, "Cancel", colors.buttonBg, colors, 20)) state = STATE_MENU;
                if (!msg.empty()) DrawTextScaled(msg.c_str(), centerX - MeasureTextScaled(msg.c_str(), 18)/2, RY(0.58f), 18, colors.accent);
            }
         }
         else if (state == STATE_OPTIONS) {
              // Back button
             Rectangle backBtn = { (float)RX(0.025f), (float)RY(0.025f), (float)RW(0.10f), (float)RH(0.05f) };
             if (DrawButton(backBtn, "← Back", colors.buttonBg, colors, 16)) {
                  state = STATE_MENU;
              }
             DrawTextScaled("Options", centerX - MeasureTextScaled("Options", 32)/2, RY(0.12f), 32, colors.primary);
             DrawTextScaled("Press ESC or click Back to return to menu", centerX - MeasureTextScaled("Press ESC or click Back to return to menu", 16)/2, RY(0.17f), 16, colors.accent);
             

             // Theme selection section (responsive layout)
             float sectionTop = RY(0.22f);
             DrawText("Theme:", centerX - MeasureText("Theme:", 24)/2, sectionTop, 24, colors.text);

             float themeBtnW = RW(0.20f);
             float themeBtnH = RH(0.08f);
             float themeBtnGap = RW(0.04f);
             float themeY = sectionTop + RH(0.06f);
             Rectangle themeButtonDark = { centerX - themeBtnW - themeBtnGap/2.0f, themeY, themeBtnW, themeBtnH };
             Rectangle themeButtonLight = { centerX + themeBtnGap/2.0f, themeY, themeBtnW, themeBtnH };

             Color darkBtnColor = (currentTheme == THEME_DARK) ? colors.primary : colors.buttonBg;
             Color lightBtnColor = (currentTheme == THEME_LIGHT) ? colors.primary : colors.buttonBg;

             if (DrawButton(themeButtonDark, "Dark Mode", darkBtnColor, colors, 20)) {
                 currentTheme = THEME_DARK;
                 colors = GetColorScheme(currentTheme);
             }
             if (DrawButton(themeButtonLight, "Light Mode", lightBtnColor, colors, 20)) {
                 currentTheme = THEME_LIGHT;
                 colors = GetColorScheme(currentTheme);
             }

            // Window mode selection: Windowed, Windowed-Fullscreen, Fullscreen (spaced horizontally)
            float winSectionY = themeY + themeBtnH + RH(0.06f);
            DrawText("Window Mode:", centerX - MeasureText("Window Mode:", 24)/2, winSectionY, 24, colors.text);
            float winBtnsY = winSectionY + RH(0.06f);
            float winBtnW = RW(0.18f);
            float winBtnH = RH(0.08f);
            float totalWinW = winBtnW * 3 + themeBtnGap * 2;
            float startX = centerX - totalWinW/2.0f;
            Rectangle winBtn = { startX, winBtnsY, winBtnW, winBtnH };
            Rectangle winFsBtn = { startX + (winBtnW + themeBtnGap), winBtnsY, winBtnW, winBtnH };
            Rectangle fsBtn = { startX + 2*(winBtnW + themeBtnGap), winBtnsY, winBtnW, winBtnH };

             Color winColor = (currentWindowMode == WM_WINDOWED) ? colors.primary : colors.buttonBg;
             Color winFsColor = (currentWindowMode == WM_WINDOWED_FULLSCREEN) ? colors.primary : colors.buttonBg;
             Color fsColor = (currentWindowMode == WM_FULLSCREEN) ? colors.primary : colors.buttonBg;

             if (DrawButton(winBtn, "Windowed", winColor, colors, 20)) {
                 ApplyWindowMode(WM_WINDOWED);
             }
             if (DrawButton(winFsBtn, "Windowed-FS", winFsColor, colors, 20)) {
                 ApplyWindowMode(WM_WINDOWED_FULLSCREEN);
             }
             if (DrawButton(fsBtn, "Fullscreen", fsColor, colors, 20)) {
                 ApplyWindowMode(WM_FULLSCREEN);
             }
  
          }
 
         EndDrawing();
    }

    CloseWindow();
    std::cout << "Exiting application." << std::endl;
    return 0;
}