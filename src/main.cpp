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

enum AppState { STATE_LOGIN, STATE_REGISTER, STATE_MENU, STATE_CATALOG, STATE_VIEW_PRODUCTS, STATE_CART, STATE_ADD_PRODUCT, STATE_EDIT_PRODUCTS, STATE_EDIT_PRODUCT, STATE_USER_MANAGEMENT, STATE_OPTIONS, STATE_EXIT };

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

// User model (supports admin flag)
struct User { std::string name; std::string pass; bool isAdmin; };

// Function to load users from file. Format supported:
// username:password[:admin]
// where the optional third token equals "admin" to mark the account as admin.
std::vector<User> LoadUsers(const std::string& filename) {
    std::vector<User> users;
    std::ifstream file(filename);
    if (file.is_open()) {
        std::string line;
        while (std::getline(file, line)) {
            if (line.empty()) continue;
            // split at ':' up to 3 tokens
            std::vector<std::string> toks;
            size_t start = 0;
            while (true) {
                size_t p = line.find(':', start);
                if (p == std::string::npos) { toks.push_back(line.substr(start)); break; }
                toks.push_back(line.substr(start, p - start)); start = p + 1;
                if (toks.size() >= 2 && start >= line.size()) break;
            }
            std::string username = toks.size() > 0 ? toks[0] : std::string();
            std::string password = toks.size() > 1 ? toks[1] : std::string();
            bool admin = false;
            if (toks.size() > 2) {
                std::string t = toks[2];
                std::transform(t.begin(), t.end(), t.begin(), ::tolower);
                if (t == "admin" || t == "1" || t == "true") admin = true;
            }
            // Backwards compatibility: treat the account named "admin" as admin
            // even if older users.txt files don't include the optional :admin token.
            std::string userLower = username;
            std::transform(userLower.begin(), userLower.end(), userLower.begin(), ::tolower);
            if (!admin && userLower == "admin") admin = true;
            if (!username.empty()) users.push_back({username, password, admin});
        }
        file.close();
    }
    return users;
}

// Function to check login credentials
bool CheckLogin(const std::vector<User>& users, const std::string& username, const std::string& password) {
    for (const auto& user : users) {
        if (user.name == username && user.pass == password) return true;
    }
    return false;
}

// Global font used by scaled helpers
Font gFont = { 0 };

// --- Scaled text helpers (use these anywhere instead of raw DrawText/MeasureText) ---
static inline int ScaledFontSize(int baseFontSize) {
    // Reference UI built for 600px height; scale linearly with current screen height
    return std::max(8, (int)(baseFontSize * ((float)GetScreenHeight() / 600.0f)));
}
static inline void DrawTextScaled(const char *text, int x, int y, int baseFontSize, Color color) {
    Vector2 pos = {(float)x, (float)y};
    float fontSize = (float)ScaledFontSize(baseFontSize);
    float spacing = fontSize * 0.1f; // 10% of font size for proper spacing
    DrawTextEx(gFont, text, pos, fontSize, spacing, color);
}
static inline int MeasureTextScaled(const char *text, int baseFontSize) {
    float fontSize = (float)ScaledFontSize(baseFontSize);
    float spacing = fontSize * 0.1f; // Match spacing used in DrawTextScaled
    return (int)MeasureTextEx(gFont, text, fontSize, spacing).x;
}

bool DrawButton(const Rectangle &r, const char *text, Color baseColor, const ColorScheme &colors, int fontSize = 20) {
    Vector2 mouse = GetMousePosition();
    bool hovered = CheckCollisionPointRec(mouse, r);
    Color color = hovered ? Fade(baseColor, 0.8f) : baseColor;

    DrawRectangleRec(r, color);
    DrawRectangleLinesEx(r, 2, colors.primary);

    // Use the global TTF font via the scaled helpers so the button text uses Calibri
    int textWidth = MeasureTextScaled(text, fontSize);
    int textHeight = ScaledFontSize(fontSize);
    DrawTextScaled(text, (int)(r.x + (r.width - textWidth) / 2), (int)(r.y + (r.height - textHeight) / 2), fontSize, colors.text);

    if (hovered && IsMouseButtonReleased(MOUSE_LEFT_BUTTON)) return true;
    return false;
}

int main() {
    const int screenWidth = 800;
    const int screenHeight = 600;

    InitWindow(screenWidth, screenHeight, "Pepka");
    SetTargetFPS(60);

    // Load TTF from assets (args: filename, fontSize, glyphs pointer or NULL, glyphCount)
    // Use NULL,0 to load the default glyph set; use larger size (64) for better quality
    gFont = LoadFontEx("assets/Calibri.ttf", 64, NULL, 0);
    SetTextureFilter(gFont.texture, TEXTURE_FILTER_BILINEAR);

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
    std::vector<User> users;

    // Try current directory first
    users = LoadUsers("users.txt");
    if (users.empty()) {
        // Try src directory
        users = LoadUsers("src/users.txt");
    }

    // If still no users found, use defaults
    if (users.empty()) {
        std::cout << "No users.txt file found. Using default users:" << std::endl;
        users.push_back({"admin", "1234", true});
        users.push_back({"user", "password", false});
        std::cout << "Default users: admin/1234 and user/password" << std::endl;
    }

    // Debug: print all loaded users
    std::cout << "\n=== LOADED USERS DEBUG ===" << std::endl;
    for (size_t i = 0; i < users.size(); ++i) {
        const auto& u = users[i];
        std::cout << "User[" << i << "]: '" << u.name << "' / '" << u.pass << "' admin=" << (u.isAdmin?"1":"0") << std::endl;
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
    struct Product { std::string name; double price; bool hasPrice; double salePercent; bool hasSale; std::string size; std::string fabric; std::string sex; std::string description; int fileIndex; };
    std::vector<Product> products;
    std::vector<Product> filteredProducts; // For search/sort results
    bool productsLoaded = false;
    float productsScroll = 0.0f;
    
    // Search and sort variables
    char searchInput[64] = "";
    bool searchActive = false;
    int sortMode = 0; // 0=default, 1=price asc, 2=price desc, 3=size asc, 4=size desc
    bool needsResort = true;
    int selectedCategory = 0; // 0=All,1=Criança,2=Homem,3=Mulher,4=Bebê
    int editProductIndex = -1; // used by Edit Products screens
    bool editProductPopulateNeeded = false;

    auto LoadProducts = [&](const std::string &path) -> bool {
        products.clear();
        std::ifstream ifs(path);
        if (!ifs) return false;
        std::string line;
        int lineIndex = 0;
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
            double salePercent = 0.0;
            bool hasSale = false;
            // Support both old and new formats. Preferred new format:
            // name;price;size;fabric;sex;description (description may contain ';')
            if (tokens.size() >= 7) {
                fabricStr = tokens[3];
                sexStr = tokens[4];
                std::string saleStr = tokens[5];
                descStr = tokens[6];
                for (size_t i = 7; i < tokens.size(); ++i) descStr += ";" + tokens[i];
                try { salePercent = std::stod(saleStr); hasSale = true; } catch(...) { hasSale = false; salePercent = 0.0; }
            } else if (tokens.size() == 6) {
                // ambiguous: token[5] might be sale or description. Detect numeric -> sale, otherwise description
                fabricStr = tokens[3];
                sexStr = tokens[4];
                std::string t5 = tokens[5];
                bool looksNumeric = !t5.empty();
                for (char c : t5) if (!(isdigit((unsigned char)c) || c=='.' || c=='-' )) { looksNumeric = false; break; }
                if (looksNumeric) {
                    try { salePercent = std::stod(t5); hasSale = true; } catch(...) { hasSale = false; salePercent = 0.0; }
                    descStr.clear();
                } else {
                    descStr = t5;
                }
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
            products.push_back({ name, price, ok, salePercent, hasSale, sizeStr, fabricStr, sexStr, descStr, lineIndex });
            ++lineIndex;
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
        // helper: case-insensitive contains
        auto ciContains = [](const std::string &hay, const std::string &needle)->bool {
                std::string h = hay; std::string n = needle;
                std::transform(h.begin(), h.end(), h.begin(), ::tolower);
                std::transform(n.begin(), n.end(), n.begin(), ::tolower);
                return h.find(n) != std::string::npos;
        };

        // Size ranking helper: XXS, XS, S, M, L, XL, XXL (unknown sizes fall back to lexicographic but rank after known ones)
        auto sizeRank = [](const std::string &s)->int {
            std::string t = s;
            std::transform(t.begin(), t.end(), t.begin(), ::tolower);
            if (t == "xxs") return 0;
            if (t == "xs")  return 1;
            if (t == "s")   return 2;
            if (t == "m")   return 3;
            if (t == "l")   return 4;
            if (t == "xl")  return 5;
            if (t == "xxl" || t == "2xl") return 6;
            // Unknown sizes: put them after known sizes but keep a deterministic ordering
            int h = 100;
            for (char c : t) h = h * 31 + (int)c;
            return h;
        };

        // Filter by category then search term
        for (const auto& product : products) {
            bool categoryMatch = true;
            if (selectedCategory != 0) {
                // prefer explicit single-letter codes saved in product.sex (M/W/K/B)
                std::string sexLower = product.sex;
                std::transform(sexLower.begin(), sexLower.end(), sexLower.begin(), ::tolower);

                if (selectedCategory == 2) { // Homem
                    categoryMatch = (sexLower == "m") || ciContains(product.name, "men") || ciContains(product.description, "men");
                } else if (selectedCategory == 3) { // Mulher
                    categoryMatch = (sexLower == "w") || ciContains(product.name, "women") || ciContains(product.description, "women") || ciContains(product.name, "mulher") || ciContains(product.description, "mulher");
                } else if (selectedCategory == 4) { // Bebê
                    categoryMatch = (sexLower == "b") || ciContains(product.name, "bebe") || ciContains(product.name, "baby") || ciContains(product.description, "baby") || ciContains(product.description, "bebe");
                } else if (selectedCategory == 1) { // Criança
                    categoryMatch = (sexLower == "k") || ciContains(product.name, "kid") || ciContains(product.name, "crian") || ciContains(product.description, "kid") || ciContains(product.description, "crian");
                }
            }
            if (!categoryMatch) continue;

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
            case 3: // Size ascending (use sizeRank for natural ordering)
                std::sort(filteredProducts.begin(), filteredProducts.end(), [&](const Product &a, const Product &b) {
                    bool aHas = !a.size.empty();
                    bool bHas = !b.size.empty();
                    if (aHas != bHas) return aHas; // items with size first
                    if (!aHas && !bHas) return a.name < b.name;
                    int ra = sizeRank(a.size);
                    int rb = sizeRank(b.size);
                    if (ra != rb) return ra < rb;
                    return a.name < b.name;
                });
                break;
            case 4: // Size descending (reverse rank)
                std::sort(filteredProducts.begin(), filteredProducts.end(), [&](const Product &a, const Product &b) {
                    bool aHas = !a.size.empty();
                    bool bHas = !b.size.empty();
                    if (aHas != bHas) return aHas; // items with size first
                    if (!aHas && !bHas) return a.name < b.name;
                    int ra = sizeRank(a.size);
                    int rb = sizeRank(b.size);
                    if (ra != rb) return ra > rb;
                    return a.name < b.name;
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
        // Check if user already exists (by name)
        for (const auto& user : users) {
            if (user.name == username) {
                return false; // User already exists
            }
        }

        // Append to users.txt file (new users are non-admin by default)
        std::ofstream ofs("users.txt", std::ios::app);
        if (!ofs) return false;
        ofs << username << ":" << password << std::endl;
        ofs.close();

        // Add to current users list
        users.push_back({username, password, false});
        return true;
    };

    // Save the full users vector back to users.txt (overwrite)
    auto SaveAllUsers = [&]() -> bool {
        std::ofstream ofs("users.txt", std::ios::trunc);
        if (!ofs) return false;
        for (const auto &u : users) {
            ofs << u.name << ":" << u.pass;
            if (u.isAdmin) ofs << ":admin";
            ofs << "\n";
        }
        ofs.close();
        return true;
    };

    // Per-user cart persistence: file per user at data/cart_<username>.txt
    auto CartFilename = [&](const std::string &user)->std::string {
        std::string fn = "data/cart_" + user + ".txt";
        return fn;
    };

    auto LoadCart = [&](const std::string &user) {
        std::vector<std::pair<std::string,int>> cart;
        if (user.empty()) return cart;
        std::ifstream ifs(CartFilename(user));
        if (!ifs) return cart;
        std::string line;
        while (std::getline(ifs, line)) {
            if (line.empty()) continue;
            size_t p = line.find(';');
            std::string name = line.substr(0, p);
            int qty = 1;
            if (p != std::string::npos) {
                try { qty = std::stoi(line.substr(p+1)); } catch(...) { qty = 1; }
            }
            cart.push_back({name, qty});
        }
        return cart;
    };

    auto SaveCart = [&](const std::string &user, const std::vector<std::pair<std::string,int>> &cart)->bool {
        if (user.empty()) return false;
        std::ofstream ofs(CartFilename(user), std::ios::trunc);
        if (!ofs) return false;
        for (const auto &it : cart) ofs << it.first << ";" << it.second << "\n";
        return true;
    };

    // In-memory cart for currently logged user
    std::vector<std::pair<std::string,int>> currentCart;

    // Add this near the top of main(), after InitWindow:
    Texture2D logo = LoadTexture("assets/logo.png");

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
    auto RX = [&](float px)->float { return px * (float)sw; }; // relative x (0..1)
    auto RY = [&](float py)->float { return py * (float)sh; }; // relative y (0..1)
    auto RW = [&](float pw)->float { return pw * (float)sw; }; // relative width
    auto RH = [&](float ph)->float { return ph * (float)sh; }; // relative height
        int centerX = sw / 2;

        if (state == STATE_LOGIN) {
            // place title near top-center (scaled)
            DrawTextScaled("Login", centerX - MeasureTextScaled("Login", 48)/2, RY(0.12f), 48, colors.primary);

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
            DrawTextScaled("Username:", labelX, rowY, 24, colors.text);
            DrawRectangleRec(usernameRect, colors.inputBg);
            DrawTextScaled(username, (int)usernameRect.x + 6, (int)usernameRect.y + 6, 20, colors.text);
            if (inputFocus == 0) DrawRectangleLinesEx(usernameRect, 2, colors.accent);

            // Password field with show/hide button
            DrawTextScaled("Password:", labelX, passwordRowY, 24, colors.text);
            DrawRectangleRec(passwordRect, colors.inputBg);
            std::string passDisplay = showPassword ? password : std::string(strlen(password), '*');
            DrawTextScaled(passDisplay.c_str(), (int)passwordRect.x + 6, (int)passwordRect.y + 6, 20, colors.text);
            if (inputFocus == 1) DrawRectangleLinesEx(passwordRect, 2, colors.accent);

            // Add show/hide password button
            Rectangle showPassBtn = { passwordRect.x + passwordRect.width + 10, passwordRect.y, (float)RH(0.05f), passwordRect.height };
            if (DrawButton(showPassBtn, showPassword ? "Hide" : "Show", colors.buttonBg, colors, 14)) {
                showPassword = !showPassword;
            }

            // Handle mouse focus on inputs
            Vector2 mousePos = GetMousePosition();
            if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                if (CheckCollisionPointRec(mousePos, usernameRect)) inputFocus = 0;
                else if (CheckCollisionPointRec(mousePos, passwordRect)) inputFocus = 1;
                else inputFocus = -1;
            }

            // Remove the space key toggle for password visibility
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
                    // Determine admin flag from loaded users
                    isAdmin = false;
                    for (const auto &u : users) { if (u.name == currentUser) { isAdmin = u.isAdmin; break; } }
                    loginFailed = false;
                    // load user's cart
                    currentCart = LoadCart(currentUser);
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
            float inputH = (float)RH(0.05f);

            // Username field
            Rectangle usernameRect = { (float)rInputX, (float)(rY - 5), (float)rInputW, inputH };
            DrawTextScaled("Username:", rLabelX, rY, 24, colors.text);
            DrawRectangleRec(usernameRect, colors.inputBg);
            DrawTextScaled(regUsername, (int)usernameRect.x + 6, (int)usernameRect.y + 6, 20, colors.text);
            if (regInputFocus == 0) DrawRectangleLinesEx(usernameRect, 2, colors.accent);

            // Password field
            Rectangle passwordRect = { (float)rInputX, (float)(rY + RH(0.078f)), (float)rInputW, inputH };
            DrawTextScaled("Password:", rLabelX, rY + RH(0.083f), 24, colors.text);
            DrawRectangleRec(passwordRect, colors.inputBg);
            std::string passDisplay = regShowPassword ? regPassword : std::string(strlen(regPassword), '*');
            DrawTextScaled(passDisplay.c_str(), (int)passwordRect.x + 6, (int)passwordRect.y + 6, 20, colors.text);
            if (regInputFocus == 1) DrawRectangleLinesEx(passwordRect, 2, colors.accent);

            // Confirm Password field
            Rectangle confirmRect = { (float)rInputX, (float)(rY + RH(0.161f)), (float)rInputW, inputH };
            DrawTextScaled("Confirm Password:", rLabelX - RW(0.05f), rY + RH(0.166f), 20, colors.text);
            DrawRectangleRec(confirmRect, colors.inputBg);
            std::string confirmDisplay = regShowPassword ? regConfirmPassword : std::string(strlen(regConfirmPassword), '*');
            DrawTextScaled(confirmDisplay.c_str(), (int)confirmRect.x + 6, (int)confirmRect.y + 6, 20, colors.text);
            if (regInputFocus == 2) DrawRectangleLinesEx(confirmRect, 2, colors.accent);

            // Show/Hide password button
            Rectangle showPassBtn = { confirmRect.x + confirmRect.width + 10, passwordRect.y, (float)RH(0.05f), inputH };
            if (DrawButton(showPassBtn, regShowPassword ? "Hide" : "Show", colors.buttonBg, colors, 14)) {
                regShowPassword = !regShowPassword;
            }

            // Handle mouse focus on inputs
            Vector2 mousePos = GetMousePosition();
            if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                if (CheckCollisionPointRec(mousePos, usernameRect)) regInputFocus = 0;
                else if (CheckCollisionPointRec(mousePos, passwordRect)) regInputFocus = 1;
                else if (CheckCollisionPointRec(mousePos, confirmRect)) regInputFocus = 2;
                else regInputFocus = -1;
            }

            // Tab to switch fields
            if (IsKeyPressed(KEY_TAB)) {
                regInputFocus = (regInputFocus + 1) % 3;
            }

            // Input handling for active field
            int key = GetCharPressed();
            while (key > 0) {
                if (key >= 32 && key <= 125) {
                    if (regInputFocus == 0 && strlen(regUsername) < sizeof(regUsername) - 1) {
                        int len = strlen(regUsername);
                        regUsername[len] = (char)key;
                        regUsername[len + 1] = '\0';
                    } else if (regInputFocus == 1 && strlen(regPassword) < sizeof(regPassword) - 1) {
                        int len = strlen(regPassword);
                        regPassword[len] = (char)key;
                        regPassword[len + 1] = '\0';
                    } else if (regInputFocus == 2 && strlen(regConfirmPassword) < sizeof(regConfirmPassword) - 1) {
                        int len = strlen(regConfirmPassword);
                        regConfirmPassword[len] = (char)key;
                        regConfirmPassword[len + 1] = '\0';
                    }
                }
                key = GetCharPressed();
            }

            if (IsKeyPressed(KEY_BACKSPACE)) {
                if (regInputFocus == 0 && strlen(regUsername) > 0) regUsername[strlen(regUsername)-1] = '\0';
                else if (regInputFocus == 1 && strlen(regPassword) > 0) regPassword[strlen(regPassword)-1] = '\0';
                else if (regInputFocus == 2 && strlen(regConfirmPassword) > 0) regConfirmPassword[strlen(regConfirmPassword)-1] = '\0';
            }

            // Action buttons
            Rectangle backBtn = { (float)RX(0.30f), (float)RY(0.55f), (float)RW(0.15f), (float)RH(0.06f) };
            Rectangle registerBtn = { (float)RX(0.55f), (float)RY(0.55f), (float)RW(0.15f), (float)RH(0.06f) };

            if (DrawButton(backBtn, "< Back", colors.buttonBg, colors, 16)) {
                state = STATE_LOGIN;
            }
            if (DrawButton(registerBtn, "Register", colors.primary, colors, 16)) {
                // Registration validation logic
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

            // Show message (if any)
            if (!regMessage.empty()) {
                Color msgColor = regFailed ? RED : GREEN;
                DrawTextScaled(regMessage.c_str(), centerX - MeasureTextScaled(regMessage.c_str(), 18)/2, RY(0.70f), 18, msgColor);
            }
        }
        else if (state == STATE_MENU) {
            // Logout button in top-left (use margin)
            float margin = 0.025f;
            Rectangle logoutBtn = { (float)RX(margin), (float)RY(margin), (float)RW(0.125f), (float)RH(0.05f) };
            if (DrawButton(logoutBtn, "< Logout", colors.buttonBg, colors, 16)) {
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
             
            // Title and logo side by side, centered at the gap
            int titleWidth = MeasureTextScaled("Pepka", 60);
            float logoScale = 0.5f; // adjust this to make the logo smaller/larger
            int scaledLogoWidth = (int)(logo.width * logoScale);
            int gap = 20; // gap between text and logo
            
            // Calculate positions so the gap is at screen center
            int pepkaX = centerX - (titleWidth + gap/2); // Text ends at center minus half gap
            float textHeight = (float)ScaledFontSize(60);
            Vector2 logoPos = { (float)(centerX + gap/2), // Logo starts at center plus half gap
                               (float)RY(0.25f) + (textHeight - logo.height * logoScale) / 2.0f };
            
            DrawTextScaled("Pepka", pepkaX, RY(0.25f), 60, colors.primary);
            DrawTextureEx(logo, logoPos, 0.0f, logoScale, WHITE);

            // Menu buttons stack (aligned vertically with consistent spacing)
            float menuBaseY = RY(0.52f);
            float menuSpacing = RH(0.12f);
            Rectangle btnView = { (float)(centerX - RW(0.125f)), (float)menuBaseY, (float)RW(0.25f), (float)RH(0.1f) };
            if (DrawButton(btnView, "View Products", colors.buttonBg, colors, 28)) state = STATE_CATALOG;

            // Cart button (visible when logged in) placed under View
            Rectangle btnCart = { (float)(centerX - RW(0.125f)), (float)(menuBaseY + menuSpacing), (float)RW(0.25f), (float)RH(0.1f) };
            if (!currentUser.empty()) {
                if (DrawButton(btnCart, "Cart", colors.buttonBg, colors, 28)) state = STATE_CART;
            }

            // Only show Add Product button if admin (placed under Cart)
            if (isAdmin) {
                Rectangle btnAdd = { (float)(centerX - RW(0.125f)), (float)(menuBaseY + menuSpacing*2.0f), (float)RW(0.25f), (float)RH(0.1f) };
                if (DrawButton(btnAdd, "Add Product", colors.buttonBg, colors, 28)) state = STATE_ADD_PRODUCT;

                // Manage Accounts button (admin-only)
                Rectangle btnManage = { (float)(centerX - RW(0.125f)), (float)(menuBaseY + menuSpacing*3.0f), (float)RW(0.25f), (float)RH(0.1f) };
                if (DrawButton(btnManage, "Manage Accounts", colors.buttonBg, colors, 24)) state = STATE_USER_MANAGEMENT;

                // Move selector highlight based on menu index (uses uniform spacing)
                Rectangle selector = { btnView.x, btnView.y + menuIndex * menuSpacing, btnView.width, btnView.height };
                DrawRectangleLinesEx(selector, 3, DARK_ACCENT);
            } else {
                // Just highlight the view button for non-admin users
                DrawRectangleLinesEx(btnView, 3, DARK_ACCENT);
            }
         }
        else if (state == STATE_CATALOG) {
            // Simple category selector before viewing products
            DrawTextScaled("Choose a category", centerX - MeasureTextScaled("Choose a category", 28)/2, RY(0.12f), 28, colors.primary);
            float btnW = RW(0.28f); float btnH = RH(0.10f); float gap = RW(0.03f);
            float startX = centerX - (btnW*2 + gap)/2.0f;
            float y = RY(0.28f);
            Rectangle catKids = { startX, y, btnW, btnH };
            Rectangle catMen = { startX + (btnW + gap), y, btnW, btnH };
            Rectangle catWomen = { startX, y + btnH + RH(0.04f), btnW, btnH };
            Rectangle catBaby = { startX + (btnW + gap), y + btnH + RH(0.04f), btnW, btnH };
            if (DrawButton(catKids, "Kid", colors.buttonBg, colors, 28)) { selectedCategory = 1; productsLoaded = false; needsResort = true; state = STATE_VIEW_PRODUCTS; }
            if (DrawButton(catMen, "Man", colors.buttonBg, colors, 28)) { selectedCategory = 2; productsLoaded = false; needsResort = true; state = STATE_VIEW_PRODUCTS; }
            if (DrawButton(catWomen, "Women", colors.buttonBg, colors, 28)) { selectedCategory = 3; productsLoaded = false; needsResort = true; state = STATE_VIEW_PRODUCTS; }
            if (DrawButton(catBaby, "Baby", colors.buttonBg, colors, 28)) { selectedCategory = 4; productsLoaded = false; needsResort = true; state = STATE_VIEW_PRODUCTS; }

            Rectangle backBtn = { (float)RX(0.025f), (float)RY(0.025f), (float)RW(0.10f), (float)RH(0.05f) };
            if (DrawButton(backBtn, "< Back", colors.buttonBg, colors, 16)) state = STATE_MENU;
        }
        else if (state == STATE_VIEW_PRODUCTS) {
            // Load & sort once
            if (!productsLoaded) { productsLoaded = LoadProducts("data/products.txt"); needsResort = true; }
            if (needsResort) FilterAndSortProducts();

            // responsive layout for list
            float margin = 0.025f;
            Rectangle backBtn = { (float)RX(margin), (float)RY(margin), (float)RW(0.10f), (float)RH(0.05f) };
            if (DrawButton(backBtn, "< Back", colors.buttonBg, colors, 16)) state = STATE_CATALOG;

            // Use Calibri for headings
            DrawTextScaled("Product List", centerX - MeasureTextScaled("Product List", 40)/2, RY(0.05f), 40, DARKBLUE);

            // Search area
            DrawTextScaled("Search:", RX(0.025f), RY(0.12f), 18, colors.text);
            Rectangle searchRect = { (float)RX(0.12f), (float)RY(0.11f), (float)RW(0.30f), (float)RH(0.05f) };
            DrawRectangleRec(searchRect, LIGHTGRAY);
            DrawTextScaled(searchInput, (int)searchRect.x + 6, (int)searchRect.y + 6, 18, BLACK);
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
            Rectangle sortPriceAscBtn = { sortStartX, (float)RY(0.11f), sortW, sortH };
            Rectangle sortPriceDescBtn = { sortStartX + sortW + sortGap, (float)RY(0.11f), sortW, sortH };
            Rectangle sortSizeAscBtn = { sortStartX + (sortW+sortGap)*2, (float)RY(0.11f), sortW, sortH };
            Rectangle sortSizeDescBtn = { sortStartX + (sortW+sortGap)*3, (float)RY(0.11f), sortW, sortH };
            Color sortBtnColor = colors.buttonBg;
            if (DrawButton(sortPriceAscBtn, "Price ^", (sortMode==1)?LIME:sortBtnColor, colors, 14)) { sortMode=1; needsResort=true; }
            if (DrawButton(sortPriceDescBtn, "Price v", (sortMode==2)?LIME:sortBtnColor, colors, 14)) { sortMode=2; needsResort=true; }
            if (DrawButton(sortSizeAscBtn, "Size ^", (sortMode==3)?LIME:sortBtnColor, colors, 14)) { sortMode=3; needsResort=true; }
            if (DrawButton(sortSizeDescBtn, "Size v", (sortMode==4)?LIME:sortBtnColor, colors, 14)) { sortMode=4; needsResort=true; }

            // Info
            if (!searchInput[0] && sortMode == 0) DrawTextScaled("Showing all products", RX(0.025f), RY(0.17f), 16, GRAY);
            else {
                static std::string msg = "";
                std::string info = "Showing " + std::to_string(filteredProducts.size()) + " of " + std::to_string(products.size()) + " products";
                if (searchInput[0]) info += " (filtered)";
                DrawTextScaled(info.c_str(), RX(0.025f), RY(0.17f), 16, GRAY);
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
                DrawTextScaled("No products found. Create 'data/products.txt' with one product per line (name;price).", RX(0.05f), RY(0.35f), 18, RED);
            } else if (filteredProducts.empty()) {
                DrawTextScaled("No products match your search criteria.", centerX - MeasureTextScaled("No products match your search criteria.", 18)/2, RY(0.40f), 18, ORANGE);
            } else {
                static int viewDescriptionIndex = -1;
                for (size_t i = 0; i < filteredProducts.size(); ++i) {
                    float y = startY + i * rowH + productsScroll;
                    if (y < RY(0.20f) - rowH || y > sh) continue;
                    const auto &p = filteredProducts[i];
                    // Draw name
                    DrawTextScaled(p.name.c_str(), RX(0.03f), (int)y, 20, colors.text);
                    
                    // Price column starts after name
                    float priceX = RX(0.03f) + RW(0.25f);
                    if (p.hasPrice) {
                        if (p.hasSale) {
                            // Original price struck-through
                            std::ostringstream ss; ss.setf(std::ios::fixed); ss.precision(2); ss << "$" << p.price;
                            std::string orig = ss.str();
                            int origW = MeasureTextScaled(orig.c_str(), 18);
                            Color faded = Fade(colors.text, 0.6f);
                            DrawTextScaled(orig.c_str(), (int)priceX, (int)y, 18, faded);
                            float lineY = (float)y + ScaledFontSize(18) * 0.5f;
                            DrawLineEx(Vector2{ priceX, lineY }, Vector2{ priceX + origW, lineY }, 2.0f, colors.text);
                            
                            // Sale price right after
                            double salePrice = p.price * (1.0 - p.salePercent/100.0);
                            std::ostringstream sps; sps.setf(std::ios::fixed); sps.precision(2); sps << "$" << salePrice;
                            DrawTextScaled(sps.str().c_str(), (int)(priceX + origW + RW(0.01f)), (int)y, 18, colors.primary);
                        } else {
                            std::ostringstream ss; ss.setf(std::ios::fixed); ss.precision(2); ss << "$" << p.price;
                            DrawTextScaled(ss.str().c_str(), (int)priceX, (int)y, 18, colors.text);
                        }
                    }

                    // Size column starts after price
                    float sizeX = RX(0.45f); // Adjust this value to position size column
                    if (!p.size.empty()) {
                        DrawTextScaled(p.size.c_str(), (int)sizeX, (int)y, 16, Fade(colors.text, 0.8f));
                    }

                    // View button stays on the right
                    Rectangle viewBtn = { (float)(sw - RW(0.18f)), y - rowH*0.15f, (float)RW(0.14f), (float)(rowH*0.85f) };
                    if (DrawButton(viewBtn, "View", colors.buttonBg, colors, 14)) viewDescriptionIndex = (int)i;
                }

                if (viewDescriptionIndex >= 0 && viewDescriptionIndex < (int)filteredProducts.size()) {
                    const auto &p = filteredProducts[viewDescriptionIndex];
                    float modalW = (float)RW(0.75f), modalH = (float)RH(0.55f);
                    Rectangle modal; modal.x = (float)(centerX - modalW/2.0f); modal.y = (float)RY(0.18f); modal.width = modalW; modal.height = modalH;
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
                    Rectangle closeBtn; closeBtn.x = (float)(modal.x + modal.width - (float)RW(0.12f)); closeBtn.y = (float)(modal.y + modal.height - (float)RH(0.08f)); closeBtn.width = (float)RW(0.12f); closeBtn.height = (float)RH(0.08f);
                    if (DrawButton(closeBtn, "Close", colors.buttonBg, colors, 16)) viewDescriptionIndex = -1;
                    // Add to cart button (logged-in users)
                    if (!currentUser.empty()) {
                        Rectangle addCartBtn = { modal.x + 20.0f, modal.y + modal.height - (float)RH(0.08f), (float)RW(0.22f), (float)RH(0.08f) };
                        if (DrawButton(addCartBtn, "Add to Cart", colors.primary, colors, 16)) {
                            // add or increment
                            bool found = false;
                            for (auto &it : currentCart) {
                                if (it.first == p.name) { it.second += 1; found = true; break; }
                            }
                            if (!found) currentCart.push_back({p.name, 1});
                            SaveCart(currentUser, currentCart);
                        }
                    }
                }
            }
        }
        else if (state == STATE_ADD_PRODUCT) {
            // Back button
            Rectangle backBtn = { (float)RX(0.025f), (float)RY(0.025f), (float)RW(0.10f), (float)RH(0.05f) };
            if (DrawButton(backBtn, "< Back", colors.buttonBg, colors, 16)) state = STATE_MENU;

            // Quick link to Edit Products
            Rectangle editProductsBtn = { (float)(sw - RW(0.18f)), (float)RY(0.025f), (float)RW(0.16f), (float)RH(0.05f) };
            if (DrawButton(editProductsBtn, "Edit Products", colors.buttonBg, colors, 14)) state = STATE_EDIT_PRODUCTS;
            DrawTextScaled("Add Product", centerX - MeasureTextScaled("Add Product", 40)/2, RY(0.05f), 40, colors.primary);

            if (currentUser.empty() || !isAdmin) {
                DrawTextScaled("Admin privileges required to add products.", centerX - MeasureTextScaled("Admin privileges required to add products.", 20)/2, RY(0.20f), 20, colors.accent);
                DrawTextScaled("Please login with an admin account.", centerX - MeasureTextScaled("Please login with an admin account.", 18)/2, RY(0.26f), 18, colors.text);
                Rectangle btnToLogin; btnToLogin.x = (float)(centerX - (float)RW(0.15f)); btnToLogin.y = (float)RY(0.36f); btnToLogin.width = (float)RW(0.30f); btnToLogin.height = (float)RH(0.08f);
                if (DrawButton(btnToLogin, "Go to Login", colors.buttonBg, colors, 20)) { memset(username,0,sizeof(username)); memset(password,0,sizeof(password)); state = STATE_LOGIN; }
            } else {
                // Responsive, centered Add Product form
                static std::string nameInput, priceInput, sizeInput, removeInput, saleInput, msg;
                static int activeFieldAdd = 0; // 0=name,1=price,2=remove,3=sale
                static int editingIndex = -1; // index in products when editing, -1 = new

                // Layout metrics
                float topY = RY(0.10f);
                float labelX = RX(0.12f);
                float inputX = RX(0.30f);
                float fullW = RW(0.58f);
                float inputH = RH(0.06f);
                float gapV = RH(0.035f);

                Rectangle nameRect = { inputX, topY, fullW, inputH };
                Rectangle priceRect = { inputX, nameRect.y + inputH + gapV, fullW * 0.4f, inputH };
                // Category buttons will sit to the right of the price input
                float catX = priceRect.x + priceRect.width + RW(0.02f);
                float catBtnW = RW(0.06f);
                float catBtnH = inputH * 0.9f;
                float catGap = RW(0.015f);
                // reserve remaining width for category buttons area (not used as a rect here)
                float extraVerticalOffset = RH(0.08f); // adjust this value to move further up/down
                Rectangle sizeAreaRect = { inputX, priceRect.y + inputH + gapV + extraVerticalOffset, fullW, inputH };
                float descY = sizeAreaRect.y + inputH + gapV * 1.2f;
                float descH_add = (float)RH(0.18f);
                Rectangle descRect; descRect.x = inputX; descRect.y = descY; descRect.width = fullW; descRect.height = descH_add;

                // Draw labels and inputs
                DrawTextScaled("Name:", labelX, (int)nameRect.y + 6, 24, colors.text);
                DrawRectangleRec(nameRect, colors.inputBg);
                DrawTextScaled(nameInput.c_str(), (int)nameRect.x + 8, (int)nameRect.y + 6, 18, colors.text);
                if (activeFieldAdd == 0) DrawRectangleLinesEx(nameRect, 2, colors.accent);

                DrawTextScaled("Price:", labelX, (int)priceRect.y + 6, 24, colors.text);
                DrawRectangleRec(priceRect, colors.inputBg);
                DrawTextScaled(priceInput.c_str(), (int)priceRect.x + 8, (int)priceRect.y + 6, 18, colors.text);
                if (activeFieldAdd == 1) DrawRectangleLinesEx(priceRect, 2, colors.accent);

                // Sale % field (below price)
                Rectangle saleRect = { inputX, priceRect.y + priceRect.height + RH(0.02f), fullW * 0.2f, inputH };
                DrawTextScaled("Sale %:", labelX, (int)saleRect.y + 6, 20, colors.text);
                DrawRectangleRec(saleRect, colors.inputBg);
                DrawTextScaled(saleInput.c_str(), (int)saleRect.x + 8, (int)saleRect.y + 6, 18, colors.text);
                if (activeFieldAdd == 3) DrawRectangleLinesEx(saleRect, 2, colors.accent);

                // Category selection (M/W/K/B) placed to the right of Price (label removed)
                static int selectedCategoryAdd = 0; // 0=none,1=M,2=W,3=K,4=B
                const std::vector<std::string> catLabels = {"M","W","K","B"};
                for (size_t ci = 0; ci < catLabels.size(); ++ci) {
                    Rectangle cb = { catX + ci * (catBtnW + catGap), priceRect.y + (priceRect.height - catBtnH)/2.0f, catBtnW, catBtnH };
                    if (DrawButton(cb, catLabels[ci].c_str(), colors.buttonBg, colors, 18)) selectedCategoryAdd = (int)ci + 1;
                    if (selectedCategoryAdd == (int)ci + 1) DrawRectangleLinesEx(cb, 3, colors.accent);
                }

                // Size selection — buttons centered inside sizeAreaRect
                DrawTextScaled("Size:", labelX, (int)sizeAreaRect.y + 6, 24, colors.text);
                static const std::vector<std::string> sizeOptions = {"XS","S","M","L","XL","XXL"};
                float sbtnW = RW(0.09f), sbtnH = inputH * 0.9f, sGap = RW(0.02f);
                float totalS = sbtnW * (float)sizeOptions.size() + sGap * ((float)sizeOptions.size() - 1.0f);
                float startSx = inputX + (fullW - totalS) / 2.0f;
                float sBtnsY = sizeAreaRect.y + (sizeAreaRect.height - sbtnH) / 2.0f;
                Vector2 mouse = GetMousePosition();
                for (size_t si = 0; si < sizeOptions.size(); ++si) {
                    Rectangle sb = { startSx + si * (sbtnW + sGap), sBtnsY, sbtnW, sbtnH };
                    if (DrawButton(sb, sizeOptions[si].c_str(), colors.buttonBg, colors, 18)) { sizeInput = sizeOptions[si]; }
                    if (!sizeInput.empty() && sizeInput == sizeOptions[si]) DrawRectangleLinesEx(sb, 2, colors.accent);
                }

                // Load product input (for editing existing products)
                Rectangle removeRect = { inputX, descY + descH_add + gapV, fullW * 0.6f, inputH };
                // Remove input
                DrawTextScaled("Remove product:", labelX, (int)removeRect.y + 6, 20, colors.text);
                DrawRectangleRec(removeRect, colors.inputBg);
                DrawTextScaled(removeInput.c_str(), (int)removeRect.x + 8, (int)removeRect.y + 6, 18, colors.text);
                if (activeFieldAdd == 2) DrawRectangleLinesEx(removeRect, 2, colors.accent);

                // Small Load button to fetch an existing product by name into the form for editing
                float loadW = RW(0.12f);
                Rectangle loadBtn = { removeRect.x + removeRect.width + RW(0.02f), removeRect.y, loadW, removeRect.height };
                if (DrawButton(loadBtn, "Load", colors.buttonBg, colors, 16)) {
                    // Ensure products are loaded
                    if (!productsLoaded) { productsLoaded = LoadProducts("data/products.txt"); }
                    editingIndex = -1;
                    std::string target = removeInput;
                    auto toLower = [](const std::string &s){ std::string out=s; std::transform(out.begin(), out.end(), out.begin(), ::tolower); return out; };
                    std::string targetL = toLower(target);
                    for (size_t i = 0; i < products.size(); ++i) {
                        std::string nameL = toLower(products[i].name);
                        if (nameL == targetL) {
                            // populate form
                            nameInput = products[i].name;
                            if (products[i].hasPrice) {
                                std::ostringstream ss; ss.setf(std::ios::fixed); ss.precision(2); ss << products[i].price; priceInput = ss.str();
                            } else priceInput.clear();
                            sizeInput = products[i].size;
                            // map sex token to selectedCategoryAdd
                            std::string sex = products[i].sex; std::transform(sex.begin(), sex.end(), sex.begin(), ::tolower);
                            if (sex == "m") selectedCategoryAdd = 1;
                            else if (sex == "w") selectedCategoryAdd = 2;
                            else if (sex == "k") selectedCategoryAdd = 3;
                            else if (sex == "b") selectedCategoryAdd = 4;
                            else selectedCategoryAdd = 0;
                            editingIndex = (int)i;
                            msg = "Loaded product for edit: " + products[i].name;
                            break;
                        }
                    }
                    if (editingIndex == -1) msg = "No product with that name found";
                }

                // Click-to-focus
                if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                    if (CheckCollisionPointRec(mouse, nameRect)) activeFieldAdd = 0;
                    else if (CheckCollisionPointRec(mouse, priceRect)) activeFieldAdd = 1;
                    else if (CheckCollisionPointRec(mouse, removeRect)) activeFieldAdd = 2;
                    else if (CheckCollisionPointRec(mouse, saleRect)) activeFieldAdd = 3;
                    else activeFieldAdd = 0;
                }

                // Keyboard input for active field
                int cp = GetCharPressed();
                while (cp > 0) {
                    if (cp >= 32 && cp <= 125) {
                        if (activeFieldAdd == 0 && nameInput.size() < 200) nameInput.push_back((char)cp);
                        else if (activeFieldAdd == 1 && priceInput.size() < 64) priceInput.push_back((char)cp);
                        else if (activeFieldAdd == 2 && removeInput.size() < 200) removeInput.push_back((char)cp);
                        else if (activeFieldAdd == 3 && saleInput.size() < 10) saleInput.push_back((char)cp);
                    }
                    cp = GetCharPressed();
                }
                if (IsKeyPressed(KEY_BACKSPACE)) {
                    if (activeFieldAdd == 0 && !nameInput.empty()) nameInput.pop_back();
                    else if (activeFieldAdd == 1 && !priceInput.empty()) priceInput.pop_back();
                    else if (activeFieldAdd == 2 && !removeInput.empty()) removeInput.pop_back();
                    else if (activeFieldAdd == 3 && !saleInput.empty()) saleInput.pop_back();
                }
                if (IsKeyPressed(KEY_TAB)) activeFieldAdd = (activeFieldAdd + 1) % 4;

                // Action buttons centered (Save, Cancel)
                float actionY = sizeAreaRect.y + inputH + gapV;
                float actionW = RW(0.22f), actionH = RH(0.08f), actionGap = RW(0.04f);
                float totalActionW = actionW * 2 + actionGap;
                float actionStartX = centerX - totalActionW / 2.0f;
                Rectangle btnSave = { actionStartX, actionY, actionW, actionH };
                Rectangle btnCancel = { actionStartX + actionW + actionGap, actionY, actionW, actionH };

                if (DrawButton(btnSave, "Save", colors.primary, colors, 20)) {
                    double pr = 0.0; bool ok = false;
                    try {
                        size_t start = 0; while (start < priceInput.size() && !((priceInput[start] >= '0' && priceInput[start] <= '9') || priceInput[start] == '.' || priceInput[start] == '-')) start++;
                        std::string trimmed = priceInput.substr(start);
                        if (!trimmed.empty()) { pr = std::stod(trimmed); ok = true; }
                    } catch(...) { ok = false; }
                    double sp = 0.0; bool okSale = false;
                    if (!saleInput.empty()) {
                        try { sp = std::stod(saleInput); okSale = true; } catch(...) { okSale = false; }
                        if (okSale) { if (sp < 0) sp = 0; if (sp > 100) sp = 100; }
                    }
                    if (!ok) msg = "Invalid price";
                    else if (nameInput.empty()) msg = "Name required";
                    else {
                        // Build the product line to write (include sale token)
                        std::string sizeToken = sizeInput.empty() ? std::string() : sizeInput;
                        std::string fabricToken = std::string();
                        std::string sexToken;
                        if (selectedCategoryAdd == 1) sexToken = "M";
                        else if (selectedCategoryAdd == 2) sexToken = "W";
                        else if (selectedCategoryAdd == 3) sexToken = "K";
                        else if (selectedCategoryAdd == 4) sexToken = "B";
                        else sexToken = std::string();
                        std::string descToken = std::string();
                        std::ostringstream newline;
                        // format: name;price;size;fabric;sex;sale;description
                        newline << nameInput << ";" << pr << ";" << sizeToken << ";" << fabricToken << ";" << sexToken << ";" << (okSale ? std::to_string((int)sp) : "0") << ";" << descToken;

                        if (editingIndex >= 0) {
                            // update existing by index (same logic)...
                            std::ifstream ifs("data/products.txt");
                            if (!ifs) { msg = "Failed to open products file for update"; }
                            else {
                                std::vector<std::string> lines; std::string line;
                                while (std::getline(ifs, line)) lines.push_back(line);
                                ifs.close();
                                int fileIdx = -1;
                                if (editingIndex >= 0 && editingIndex < (int)products.size()) fileIdx = products[editingIndex].fileIndex;
                                if (fileIdx >= 0 && fileIdx < (int)lines.size()) {
                                    lines[fileIdx] = newline.str();
                                    std::ofstream ofs("data/products.txt", std::ios::trunc);
                                    if (!ofs) { msg = "Failed to write products file"; }
                                    else {
                                        for (auto &l : lines) ofs << l << "\n";
                                        ofs.close();
                                        msg = "Product updated";
                                        // reset form
                                        nameInput.clear(); priceInput.clear(); sizeInput.clear(); saleInput.clear(); selectedCategoryAdd = 0; editingIndex = -1; productsLoaded = false; needsResort = true;
                                    }
                                } else {
                                    msg = "Product index out of range";
                                }
                            }
                        } else {
                            std::ofstream ofs("data/products.txt", std::ios::app);
                            if (ofs) {
                                ofs << newline.str() << "\n";
                                ofs.close();
                                msg = "Product saved"; nameInput.clear(); priceInput.clear(); sizeInput.clear(); saleInput.clear(); selectedCategoryAdd = 0; productsLoaded = false; needsResort = true;
                            } else msg = "Failed to open file";
                        }
                    }
                }

                // (Remove button intentionally removed from Add Product screen)

                if (DrawButton(btnCancel, "Cancel", colors.buttonBg, colors, 20)) state = STATE_MENU;
                if (!msg.empty()) DrawTextScaled(msg.c_str(), centerX - MeasureTextScaled(msg.c_str(), 18)/2, RY(0.86f), 18, colors.accent);
            }
        }
        else if (state == STATE_CART) {
            // Ensure products loaded for price lookup
            if (!productsLoaded) { productsLoaded = LoadProducts("data/products.txt"); needsResort = true; }

            // Back button
            Rectangle backBtn = { (float)RX(0.025f), (float)RY(0.025f), (float)RW(0.10f), (float)RH(0.05f) };
            if (DrawButton(backBtn, "< Back", colors.buttonBg, colors, 16)) state = STATE_MENU;

            std::string totStr = "";
            if (currentUser.empty()) {
                // draw empty state lower to avoid overlapping header
                DrawTextScaled("Please login to view your cart.", centerX - MeasureTextScaled("Please login to view your cart.", 18)/2, RY(0.30f), 18, colors.accent);
            } else {
                // Layout columns (shifted slightly down so header stays visible)
                float marginX = RX(0.06f);
                // Card width is slightly smaller than full available width to leave room for the Remove button
                float cardW = RW(0.78f);
                float listX = marginX;
                float listY = RY(0.20f); // moved down a bit
                // internal column widths relative to card width
                float colNameW = cardW * 0.55f;
                float colQtyW = cardW * 0.15f;
                float colPriceW = cardW * 0.15f;
                float colSubW = cardW * 0.15f;
                float rowH = RH(0.09f);
                float gap = RH(0.015f);

                // Header row background
                Rectangle headerRect = { listX, listY - rowH*0.6f, RW(0.88f), rowH*0.7f };
                DrawRectangleRec(headerRect, Fade(colors.inputBg, 0.95f));
                DrawTextScaled("Item", (int)listX + 8, (int)(headerRect.y + 6), 18, colors.text);
                DrawTextScaled("Qty", (int)(listX + colNameW + 6), (int)(headerRect.y + 6), 18, colors.text);
                DrawTextScaled("Price", (int)(listX + colNameW + colQtyW + 6), (int)(headerRect.y + 6), 18, colors.text);
                DrawTextScaled("Subtotal", (int)(listX + colNameW + colQtyW + colPriceW + 6), (int)(headerRect.y + 6), 18, colors.text);

                double total = 0.0;
                float y = listY;
                if (currentCart.empty()) {
                    DrawTextScaled("Your cart is empty.", centerX - MeasureTextScaled("Your cart is empty.", 20)/2, RY(0.45f), 20, colors.accent);
                    DrawTextScaled("Browse products and click 'Add to Cart' to add items.", centerX - MeasureTextScaled("Browse products and click 'Add to Cart' to add items.", 16)/2, RY(0.50f), 16, colors.text);
                } else {
                    for (size_t i = 0; i < currentCart.size(); ++i) {
                        const auto &it = currentCart[i];
                        // card background (narrower than full width to leave room for Remove button)
                        Rectangle card = { listX, y, cardW, rowH };
                        DrawRectangleRec(card, Fade(colors.inputBg, 0.98f));
                        DrawRectangleLinesEx(card, 2, colors.primary);

                        // Name (wrap naive)
                        DrawTextScaled(it.first.c_str(), (int)(card.x + 8), (int)(card.y + 8), 18, colors.text);

                        // Price lookup
                        double price = 0.0; bool hasPrice = false;
                        double originalPrice = 0.0;
                        bool hasSaleLocal = false;
                        double salePercentLocal = 0.0;
                        for (const auto &prod : products) {
                            if (prod.name == it.first) {
                                originalPrice = prod.price;
                                hasPrice = prod.hasPrice;
                                hasSaleLocal = prod.hasSale;
                                salePercentLocal = prod.salePercent;
                                break;
                            }
                        }
                        if (hasPrice) {
                            if (hasSaleLocal) price = originalPrice * (1.0 - salePercentLocal/100.0);
                            else price = originalPrice;
                        } else price = 0.0;
                        std::ostringstream priceS; if (hasPrice) priceS << std::fixed << std::setprecision(2) << price; else priceS << "-";
                        std::string priceStr = priceS.str();

                        // Qty controls (inside card)
                        Rectangle qtyRect = { card.x + colNameW, card.y + rowH*0.15f, colQtyW, rowH*0.7f };
                        Rectangle minusBtn = { qtyRect.x, qtyRect.y, qtyRect.width*0.36f, qtyRect.height };
                        Rectangle qtyLabel = { qtyRect.x + qtyRect.width*0.36f, qtyRect.y, qtyRect.width*0.28f, qtyRect.height };
                        Rectangle plusBtn = { qtyRect.x + qtyRect.width*0.64f, qtyRect.y, qtyRect.width*0.36f, qtyRect.height };
                        if (DrawButton(minusBtn, "-", colors.buttonBg, colors, 18)) {
                            if (currentCart[i].second > 1) currentCart[i].second -= 1; else { currentCart.erase(currentCart.begin() + i); }
                            SaveCart(currentUser, currentCart);
                        }
                        DrawTextScaled(std::to_string(it.second).c_str(), (int)(qtyLabel.x + (qtyLabel.width - MeasureTextScaled(std::to_string(it.second).c_str(),18))/2), (int)(qtyLabel.y + 6), 18, colors.text);
                        if (DrawButton(plusBtn, "+", colors.buttonBg, colors, 18)) { currentCart[i].second += 1; SaveCart(currentUser, currentCart); }

                        // Price
                        float px = card.x + colNameW + colQtyW + 8;
                        if (hasPrice && hasSaleLocal) {
                            // original struck-through
                            std::ostringstream origs; origs.setf(std::ios::fixed); origs.precision(2); origs << "$" << originalPrice;
                            std::string orig = origs.str();
                            int origW = MeasureTextScaled(orig.c_str(), 16);
                            DrawTextScaled(orig.c_str(), (int)px, (int)(card.y + 8), 16, Fade(colors.text, 0.6f));
                            float lineY = card.y + 8 + ScaledFontSize(16) * 0.5f;
                            DrawLineEx(Vector2{ px, lineY }, Vector2{ px + origW, lineY }, 2.0f, colors.text);
                            // sale price after original (smaller gap)
                            double salePrice = originalPrice * (1.0 - salePercentLocal/100.0);
                            std::ostringstream sps; sps.setf(std::ios::fixed); sps.precision(2); sps << "$" << salePrice;
                            DrawTextScaled(sps.str().c_str(), (int)(px + origW + RW(0.005f)), (int)(card.y + 8), 16, colors.primary);
                        } else {
                            DrawTextScaled(priceStr.c_str(), (int)px, (int)(card.y + 8), 18, colors.text);
                        }

                        // Subtotal
                        double subtotal = hasPrice ? price * it.second : 0.0;
                        if (hasPrice) {
                            std::ostringstream ss; ss << std::fixed << std::setprecision(2) << subtotal;
                            DrawTextScaled((std::string("$") + ss.str()).c_str(), (int)(card.x + colNameW + colQtyW + colPriceW + 8), (int)(card.y + 8), 18, colors.text);
                            total += subtotal;
                        } else {
                            DrawTextScaled("-", (int)(card.x + colNameW + colQtyW + colPriceW + 8), (int)(card.y + 8), 18, colors.text);
                        }

                        // Remove small button on right (placed in the margin to avoid overlapping Subtotal)
                        Rectangle remBtn = { card.x + card.width + RW(0.02f), card.y + rowH*0.18f, RW(0.12f), rowH*0.64f };
                        if (DrawButton(remBtn, "Remove", (Color){220,80,80,255}, colors, 16)) { currentCart.erase(currentCart.begin() + i); SaveCart(currentUser, currentCart); }

                        y += rowH + gap;
                    }

                    // Compute totals string and defer drawing the totals box until after other UI so it isn't overlapped
                    std::ostringstream tot; tot << "Total: $" << std::fixed << std::setprecision(2) << total;
                    totStr = tot.str();
                    // store totStr in a local variable; drawing will happen after main content so it stays on top
                }
            }

            // Draw header last so it appears on top of cards
            DrawTextScaled("My Cart", centerX - MeasureTextScaled("My Cart", 36)/2, RY(0.08f), 36, colors.primary);

            // Draw totals box after everything else so it remains on top and not overlapped
            {
                Rectangle totBox = { (float)(centerX + RW(0.10f)), (float)RY(0.70f), (float)RW(0.34f), (float)RH(0.18f) };
                DrawRectangleRec(totBox, Fade(colors.inputBg, 0.98f)); DrawRectangleLinesEx(totBox, 2, colors.primary);
                DrawTextScaled(totStr.c_str(), (int)(totBox.x + 12), (int)(totBox.y + 12), 20, colors.primary);

                // Checkout and Clear buttons inside the totals box
                Rectangle checkoutBtn = { totBox.x + 12, totBox.y + totBox.height - RH(0.06f) - 8, totBox.width - 24, RH(0.06f) };
                if (DrawButton(checkoutBtn, "Checkout", colors.primary, colors, 20)) { currentCart.clear(); SaveCart(currentUser, currentCart); }
                Rectangle clearBtn = { totBox.x + 12, totBox.y + totBox.height - RH(0.06f)*2 - 16, totBox.width - 24, RH(0.06f) };
                if (DrawButton(clearBtn, "Clear Cart", (Color){200,70,70,255}, colors, 16)) { currentCart.clear(); SaveCart(currentUser, currentCart); }
            }
        }
        else if (state == STATE_EDIT_PRODUCTS) {
            // Ensure products loaded
            if (!productsLoaded) { productsLoaded = LoadProducts("data/products.txt"); }

            // Back to Add Product screen
            Rectangle backBtn = { (float)RX(0.025f), (float)RY(0.025f), (float)RW(0.10f), (float)RH(0.05f) };
            if (DrawButton(backBtn, "< Back", colors.buttonBg, colors, 16)) state = STATE_ADD_PRODUCT;

            DrawTextScaled("Edit Products", centerX - MeasureTextScaled("Edit Products", 28)/2, RY(0.08f), 28, colors.primary);

            // Simple scrollable list of products with Edit buttons
            float startY = RY(0.16f);
            float rowH = (float)RH(0.05f);
            float y = startY;
            for (size_t i = 0; i < products.size(); ++i) {
                if (y > RY(0.18f) + RY(0.70f)) break; // don't render past area
                const auto &p = products[i];
                std::ostringstream ss; ss << p.name; if (p.hasPrice) { ss << " - $" << std::fixed << std::setprecision(2) << p.price; }
                DrawTextScaled(ss.str().c_str(), RX(0.03f), (int)y, 18, colors.text);
                    float actionBtnW = (float)RW(0.14f);
                    float actionBtnH = (float)(rowH * 0.85f);
                    float editBtnX = (float)(sw - RW(0.18f));
                    Rectangle editBtn = { editBtnX, y - rowH*0.15f, actionBtnW, actionBtnH };
                    Rectangle removeBtn = { editBtnX - (actionBtnW + RW(0.02f)), y - rowH*0.15f, actionBtnW, actionBtnH };
                    if (DrawButton(editBtn, "Edit", colors.buttonBg, colors, 14)) {
                        editProductIndex = (int)i;
                        editProductPopulateNeeded = true;
                        state = STATE_EDIT_PRODUCT;
                    }
                    if (DrawButton(removeBtn, "Remove", colors.buttonBg, colors, 14)) {
                        // Remove product by index i from file and refresh
                        std::ifstream ifs("data/products.txt");
                        if (!ifs) {
                            // nothing to do
                        } else {
                            std::vector<std::string> lines; std::string line;
                            while (std::getline(ifs, line)) lines.push_back(line);
                            ifs.close();
                            if (i < lines.size()) {
                                lines.erase(lines.begin() + i);
                                std::ofstream ofs("data/products.txt", std::ios::trunc);
                                if (ofs) {
                                    for (auto &l : lines) ofs << l << "\n";
                                    ofs.close();
                                    productsLoaded = false; needsResort = true;
                                }
                            }
                        }
                    }
                y += rowH;
            }
        }
        else if (state == STATE_EDIT_PRODUCT) {
            // Edit a single product by index
            if (editProductIndex < 0 || editProductIndex >= (int)products.size()) { state = STATE_EDIT_PRODUCTS; }
            else {
                // Back button
                Rectangle backBtn = { (float)RX(0.025f), (float)RY(0.025f), (float)RW(0.10f), (float)RH(0.05f) };
                if (DrawButton(backBtn, "< Back", colors.buttonBg, colors, 16)) state = STATE_EDIT_PRODUCTS;

                DrawTextScaled("Edit Product", centerX - MeasureTextScaled("Edit Product", 28)/2, RY(0.05f), 28, colors.primary);

                // Form fields (populate from products[editProductIndex])
                static std::string editName, editPrice, editSize, editSale; // Added editSale here
                static int editCategory = 0;
                static bool populated = false;
                static std::string editDescription = "";
                static std::string origEditName = "";
                if (editProductPopulateNeeded || !populated) {
                    const auto &p = products[editProductIndex];
                    editName = p.name;
                    if (p.hasPrice) { std::ostringstream ss; ss.setf(std::ios::fixed); ss.precision(2); ss << p.price; editPrice = ss.str(); } else editPrice.clear();
                    editSize = p.size;
                    std::string s = p.sex; std::transform(s.begin(), s.end(), s.begin(), ::tolower);
                    if (s == "m") editCategory = 1; else if (s == "w") editCategory = 2; else if (s == "k") editCategory = 3; else if (s == "b") editCategory = 4; else editCategory = 0;
                    editDescription = p.description;
                    // sale populate
                    if (p.hasSale) {
                        std::ostringstream ssp; ssp << (int)p.salePercent;
                        editSale = ssp.str();
                    } else editSale.clear();
                    populated = true;
                    editProductPopulateNeeded = false;
                }

                // Layout like Add Product (simplified)
                float topY = RY(0.15f);
                float labelX = RX(0.12f);
                float inputX = RX(0.30f);
                float fullW = RW(0.58f);
                float inputH = RH(0.06f);
                float gapV = RH(0.035f);

                Rectangle nameRect = { inputX, topY, fullW, inputH };
                Rectangle priceRect = { inputX, nameRect.y + inputH + gapV, fullW * 0.4f, inputH };
                float catX = priceRect.x + priceRect.width + RW(0.02f);
                float catBtnW = RW(0.06f), catBtnH = inputH * 0.9f, catGap = RW(0.015f);
                Rectangle sizeAreaRect = { inputX, priceRect.y + inputH + gapV, fullW, inputH };

                DrawTextScaled("Name:", labelX, (int)nameRect.y + 6, 20, colors.text);
                DrawRectangleRec(nameRect, colors.inputBg);
                DrawTextScaled(editName.c_str(), (int)nameRect.x + 8, (int)nameRect.y + 6, 18, colors.text);

                DrawTextScaled("Price:", labelX, (int)priceRect.y + 6, 20, colors.text);
                DrawRectangleRec(priceRect, colors.inputBg);
                DrawTextScaled(editPrice.c_str(), (int)priceRect.x + 8, (int)priceRect.y + 6, 18, colors.text);

                // Draw sale input near price
                Rectangle saleRectEdit = { priceRect.x + priceRect.width + RW(0.02f), priceRect.y, RW(0.12f), priceRect.height };
                DrawTextScaled("Sale%:", (int)(saleRectEdit.x - MeasureTextScaled("Sale%:", 18) - 6), (int)saleRectEdit.y + 6, 18, colors.text);
                DrawRectangleRec(saleRectEdit, colors.inputBg);
                DrawTextScaled(editSale.c_str(), (int)saleRectEdit.x + 6, (int)saleRectEdit.y + 6, 18, colors.text);
                static bool saleFocus = false;
                if (saleFocus) DrawRectangleLinesEx(saleRectEdit, 2, colors.accent);

                // Category selection (M/W/K/B) placed to the right of Price (label removed)
                static int selectedCategoryAdd = 0; // 0=none,1=M,2=W,3=K,4=B
                const std::vector<std::string> catLabels = {"M","W","K","B"};
                for (size_t ci = 0; ci < catLabels.size(); ++ci) {
                    Rectangle cb = { catX + ci * (catBtnW + catGap), priceRect.y + (priceRect.height - catBtnH)/2.0f, catBtnW, catBtnH };
                    if (DrawButton(cb, catLabels[ci].c_str(), colors.buttonBg, colors, 18)) editCategory = (int)ci + 1;
                    if (editCategory == (int)ci + 1) DrawRectangleLinesEx(cb, 3, colors.accent);
                }

                // Size selection — buttons centered inside sizeAreaRect
                DrawTextScaled("Size:", labelX, (int)sizeAreaRect.y + 6, 20, colors.text);
                static const std::vector<std::string> sizeOptions = {"XS","S","M","L","XL","XXL"};
                float sbtnW = RW(0.09f), sbtnH = inputH * 0.9f, sGap = RW(0.02f);
                float totalS = sbtnW * (float)sizeOptions.size() + sGap * ((float)sizeOptions.size() - 1.0f);
                float startSx = inputX + (fullW - totalS) / 2.0f;
                float sBtnsY = sizeAreaRect.y + (sizeAreaRect.height - sbtnH) / 2.0f;
                for (size_t si = 0; si < sizeOptions.size(); ++si) {
                    Rectangle sb = { startSx + si * (sbtnW + sGap), sBtnsY, sbtnW, sbtnH };
                    if (DrawButton(sb, sizeOptions[si].c_str(), colors.buttonBg, colors, 18)) { editSize = sizeOptions[si]; }
                    if (!editSize.empty() && editSize == sizeOptions[si]) DrawRectangleLinesEx(sb, 2, colors.accent);
                }

                // Action buttons layout (compute early so description rect can be placed relative to them)
                // Use unique early names here to avoid redeclaration collisions later
                float actionY_early = sizeAreaRect.y + inputH + gapV;
                float actionW_early = RW(0.22f), actionH_early = RH(0.08f), actionGap_early = RW(0.04f);
                float totalActionW_early = actionW_early * 2 + actionGap_early;
                float actionStartX_early = centerX - totalActionW_early / 2.0f;
                Rectangle btnUpdate_early = { actionStartX_early, actionY_early, actionW_early, actionH_early };
                Rectangle btnCancel_early = { actionStartX_early + actionW_early + actionGap_early, actionY_early, actionW_early, actionH_early };

                // Description rectangle computed now so click handling can reference it
                // Compute description area using the early action layout values
                float descY = actionY_early + actionH_early + (float)RH(0.02f);
                float descH = (float)RH(0.18f);
                Rectangle descRect; descRect.x = inputX; descRect.y = descY; descRect.width = fullW; descRect.height = descH;

                // Click-to-focus and keyboard input for Name/Price/Description fields
                static int editFieldFocus = 0; // 0=name, 1=price, 2=none
                static bool descFocus = false;
                Vector2 mousePos = GetMousePosition();
                if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                    if (CheckCollisionPointRec(mousePos, nameRect)) { editFieldFocus = 0; descFocus = false; saleFocus = false; }
                    else if (CheckCollisionPointRec(mousePos, priceRect)) { editFieldFocus = 1; descFocus = false; saleFocus = false; }
                    else if (CheckCollisionPointRec(mousePos, descRect)) { descFocus = true; editFieldFocus = 2; saleFocus = false; }
                    else if (CheckCollisionPointRec(mousePos, saleRectEdit)) { saleFocus = true; editFieldFocus = 2; descFocus = false; }
                    else { editFieldFocus = 2; descFocus = false; saleFocus = false; }
                }
                // draw focus indicators
                if (editFieldFocus == 0) DrawRectangleLinesEx(nameRect, 2, colors.accent);
                if (editFieldFocus == 1) DrawRectangleLinesEx(priceRect, 2, colors.accent);
                // (desc focus border will be drawn after the description area is rendered so it is visible)

                // Text input handling routed by focus
                int ch = GetCharPressed();
                while (ch > 0) {
                    if (ch >= 32 && ch <= 125) {
                        if (descFocus && editDescription.size() < 2048) editDescription.push_back((char)ch);
                        else if (!descFocus && editFieldFocus == 0 && editName.size() < 200) editName.push_back((char)ch);
                        else if (!descFocus && editFieldFocus == 1 && editPrice.size() < 64) editPrice.push_back((char)ch);
                        else if (saleFocus && editSale.size() < 6) editSale.push_back((char)ch);
                    }
                    ch = GetCharPressed();
                }
                if (IsKeyPressed(KEY_BACKSPACE)) {
                    if (descFocus && !editDescription.empty()) editDescription.pop_back();
                    else if (!descFocus && editFieldFocus == 0 && !editName.empty()) editName.pop_back();
                    else if (!descFocus && editFieldFocus == 1 && !editPrice.empty()) editPrice.pop_back();
                    else if (saleFocus && !editSale.empty()) editSale.pop_back();
                }
                if (IsKeyPressed(KEY_TAB)) {
                    if (!descFocus) {
                        if (editFieldFocus == 0) editFieldFocus = 1; else editFieldFocus = 0;
                    }
                }

                // Action buttons: Update, Delete, Cancel
                float actionY = sizeAreaRect.y + inputH + gapV;
                float actionW = RW(0.22f), actionH = RH(0.08f), actionGap = RW(0.04f);
                float totalActionW = actionW * 3 + actionGap * 2;
                float actionStartX = centerX - totalActionW / 2.0f;
                Rectangle btnUpdate = { actionStartX, actionY, actionW, actionH };
                Rectangle btnDelete = { actionStartX + actionW + actionGap, actionY, actionW, actionH };
                Rectangle btnCancel = { actionStartX + (actionW + actionGap) * 2, actionY, actionW, actionH };

                if (DrawButton(btnUpdate, "Update", colors.primary, colors, 20)) {
                    // write update by finding the original product line by name and replacing it
                    std::ifstream ifs("data/products.txt");
                    if (!ifs) { /* fail */ }
                    else {
                        std::vector<std::string> lines; std::string line;
                        while (std::getline(ifs, line)) lines.push_back(line);
                        ifs.close();

                        std::string sizeToken = editSize;
                        std::string sexToken;
                        if (editCategory == 1) sexToken = "M"; else if (editCategory == 2) sexToken = "W"; else if (editCategory == 3) sexToken = "K"; else if (editCategory == 4) sexToken = "B";
                        std::ostringstream newline; newline << editName << ";" << editPrice << ";" << sizeToken << ";" << "" << ";" << sexToken << ";" << editDescription;
                        std::string newLine = newline.str();

                        bool replaced = false;
                        // Prefer matching by original product name
                        for (size_t li = 0; li < lines.size(); ++li) {
                            std::string l = lines[li];
                            size_t psep = l.find(';');
                            std::string lineName = (psep==std::string::npos) ? l : l.substr(0, psep);
                            if (!origEditName.empty() && lineName == origEditName) {
                                lines[li] = newLine; replaced = true; break;
                            }
                        }
                        // Fallback: use index if not found
                        if (!replaced && editProductIndex >= 0 && editProductIndex < (int)lines.size()) {
                            lines[editProductIndex] = newLine; replaced = true;
                        }

                        if (replaced) {
                            std::ofstream ofs("data/products.txt", std::ios::trunc);
                            if (ofs) {
                                for (auto &l : lines) ofs << l << "\n";
                                ofs.close();
                                productsLoaded = false; needsResort = true; state = STATE_EDIT_PRODUCTS; populated = false;
                            }
                        }
                    }
                }
                if (DrawButton(btnDelete, "Delete", (Color){220,80,80,255}, colors, 20)) {
                    // remove this product by index
                    std::ifstream ifs("data/products.txt");
                    if (ifs) {
                        std::vector<std::string> lines; std::string line;
                        while (std::getline(ifs, line)) lines.push_back(line);
                        ifs.close();
                        if (editProductIndex >= 0 && editProductIndex < (int)lines.size()) {
                            lines.erase(lines.begin() + editProductIndex);
                            std::ofstream ofs("data/products.txt", std::ios::trunc);
                            if (ofs) { for (auto &l : lines) ofs << l << "\n"; ofs.close(); }
                            productsLoaded = false; needsResort = true; populated = false; state = STATE_EDIT_PRODUCTS;
                        }
                    }
                }
                if (DrawButton(btnCancel, "Cancel", colors.buttonBg, colors, 20)) { state = STATE_EDIT_PRODUCTS; populated = false; }

                // Description textarea below action buttons (draw using descRect defined above)
                DrawTextScaled("Description:", labelX, (int)descRect.y - 18, 18, colors.text);
                DrawRectangleRec(descRect, colors.inputBg);
                // show text (simple multi-line naive wrap)
                int lineY = (int)descRect.y + 6;
                std::istringstream iss(editDescription);
                std::string tokenLine;
                while (std::getline(iss, tokenLine)) {
                    DrawTextScaled(tokenLine.c_str(), (int)descRect.x + 6, lineY, 16, colors.text);
                    lineY += 20;
                }

                // Debug overlay: show focus and input state to help diagnose why typing may not register
                {
                    bool mouseInDesc = CheckCollisionPointRec(mousePos, descRect);
                    std::ostringstream dbg; dbg << "descFocus=" << (descFocus?"1":"0") << " editFieldFocus=" << editFieldFocus << " descLen=" << editDescription.size() << " mouseInDesc=" << (mouseInDesc?"1":"0");
                    DrawTextScaled(dbg.str().c_str(), (int)inputX, (int)(descRect.y + descRect.height + 8), 14, colors.accent);
                }

                // Draw focus border for description (after the rect/content so it remains visible)
                if (descFocus) DrawRectangleLinesEx(descRect, 2, colors.accent);

                // (input handled above routed by focus)
            }
        }
         else if (state == STATE_USER_MANAGEMENT) {
            // Admin user management: list users, edit password, remove users
            if (!isAdmin) { state = STATE_MENU; }
            // Back button
            Rectangle backBtn = { (float)RX(0.025f), (float)RY(0.025f), (float)RW(0.10f), (float)RH(0.05f) };
            if (DrawButton(backBtn, "< Back", colors.buttonBg, colors, 16)) state = STATE_MENU;

            DrawTextScaled("Manage Accounts", centerX - MeasureTextScaled("Manage Accounts", 28)/2, RY(0.08f), 28, colors.primary);

            // Editable list with scroll/clipping to avoid overlap on large screens (F11)
            float startY = RY(0.16f);
            float visibleH = RY(0.70f);
            float rowH = (float)RH(0.06f);
            static float usersScroll = 0.0f;
            static int editUserIndex = -1;
            static bool editingUser = false;
            static std::string editUserNewPass = "";
            // Flag to indicate the edit modal was just opened so we can initialize focus and checkbox state
            static bool editUserJustOpened = false;

            // Scroll handling (mouse wheel + keyboard)
            float wheel = GetMouseWheelMove(); usersScroll -= wheel * RH(0.05f);
            if (IsKeyDown(KEY_DOWN)) usersScroll -= RH(0.01f);
            if (IsKeyDown(KEY_UP)) usersScroll += RH(0.01f);

            // clamp scroll to content height
            float contentH = (float)users.size() * rowH;
            float minScroll = std::min(0.0f, startY + visibleH - (startY + contentH)); // negative or zero
            if (usersScroll < minScroll) usersScroll = minScroll;
            if (usersScroll > 0) usersScroll = 0;

            float y = startY + usersScroll;
            for (size_t i = 0; i < users.size(); ++i) {
                const auto &u = users[i];

                // Clip: only render rows within visible area
                if (y + rowH < startY) { y += rowH; continue; }
                if (y > startY + visibleH) break;

                std::string uname = u.name;
                std::string displayName = uname + (u.isAdmin ? " (Admin)" : "");
                DrawTextScaled(displayName.c_str(), RX(0.03f), (int)y, 18, colors.text);

                float actionBtnW = (float)RW(0.18f);
                float actionBtnH = (float)(rowH * 0.85f);
                float editBtnX = (float)(sw - RW(0.18f));
                Rectangle editBtn = { editBtnX, y - rowH*0.15f, actionBtnW, actionBtnH };
                Rectangle removeBtn = { editBtnX - (actionBtnW + RW(0.02f)), y - rowH*0.15f, actionBtnW, actionBtnH };

                if (DrawButton(editBtn, "Edit", colors.buttonBg, colors, 14)) {
                    editUserIndex = (int)i;
                    editingUser = true;
                    // admin cannot view current password: leave new-pass empty and only update if admin types a new one
                    editUserNewPass = "";
                    // mark modal as just opened so we can initialize modal-local state
                    editUserJustOpened = true;
                }

                // Do not allow removing the main 'admin' account or the currently logged-in user
                if (u.name != "admin" && u.name != currentUser) {
                    if (DrawButton(removeBtn, "Remove", (Color){220,80,80,255}, colors, 14)) {
                        users.erase(users.begin() + i);
                        SaveAllUsers();
                        // after erase, contentH changed; clamp scroll
                        contentH = (float)users.size() * rowH;
                        minScroll = std::min(0.0f, startY + visibleH - (startY + contentH));
                        if (usersScroll < minScroll) usersScroll = minScroll;
                        // do not increment y (next item occupies same visual slot)
                        continue;
                    }
                } else {
                    // disabled remove button (draw as plain rect)
                    DrawRectangleRec(removeBtn, Fade(colors.inputBg, 0.98f));
                    DrawRectangleLinesEx(removeBtn, 2, colors.primary);
                    DrawTextScaled("-", (int)(removeBtn.x + removeBtn.width/2 - MeasureTextScaled("-",14)/2), (int)(removeBtn.y + 6), 14, colors.text);
                }

                y += rowH;
            }

            // Edit modal / inline area at bottom
            if (editingUser && editUserIndex >= 0 && editUserIndex < (int)users.size()) {
                float modalW = RW(0.72f);
                float modalH = RH(0.28f);
                // Center the modal inside the visible users area to avoid overlap on tall/fullscreen displays
                Rectangle modal; modal.x = centerX - modalW/2.0f; modal.y = startY + (visibleH - modalH) / 2.0f; modal.width = modalW; modal.height = modalH;
                DrawRectangleRec(modal, Fade(colors.inputBg, 0.98f)); DrawRectangleLinesEx(modal, 2, colors.accent);
                DrawTextScaled("Edit User", (int)modal.x + 12, (int)modal.y + 8, 20, colors.primary);

                // Username (readonly)
                std::string uname = users[editUserIndex].name;
                DrawTextScaled("Username:", (int)modal.x + 12, (int)modal.y + 40, 18, colors.text);
                DrawTextScaled(uname.c_str(), (int)modal.x + 130, (int)modal.y + 40, 18, colors.text);

                // Password input (masked — no reveal button: passwords must remain hidden)
                DrawTextScaled("Password:", (int)modal.x + 12, (int)modal.y + 72, 18, colors.text);
                Rectangle passRect = { modal.x + 130, modal.y + 68, modal.width - 150, RH(0.06f) };
                DrawRectangleRec(passRect, colors.inputBg);

                // Always display masked password (asterisks). Admin cannot reveal stored passwords.
                std::string passDisplay = editUserNewPass.empty() ? "(no change)" : std::string(editUserNewPass.size(), '*');
                DrawTextScaled(passDisplay.c_str(), (int)passRect.x + 6, (int)passRect.y + 6, 18, colors.text);

                // persistent focus flag for password input (declare before use)
                static bool userPassFocus = false;
                // admin grant toggle state (editable while in modal)
                static bool editUserGrantAdmin = false;
                // If the modal was just opened, initialize focus and checkbox state from the user record
                if (editUserJustOpened) {
                    userPassFocus = false;
                    editUserGrantAdmin = users[editUserIndex].isAdmin;
                    // clear the just-opened flag
                    editUserJustOpened = false;
                }
                // handle clicking into password field to focus
                Vector2 mpos = GetMousePosition();
                if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                    if (CheckCollisionPointRec(mpos, passRect)) userPassFocus = true; else userPassFocus = false;
                }

                // Admin grant checkbox UI
                Rectangle adminChk = { modal.x + 12, modal.y + 104, 20, 20 };
                // draw checkbox background depending on state
                DrawRectangleRec(adminChk, editUserGrantAdmin ? colors.primary : Fade(colors.inputBg, 0.98f));
                DrawRectangleLinesEx(adminChk, 2, colors.primary);
                DrawTextScaled("Grant admin rights", (int)(adminChk.x + adminChk.width + 8), (int)adminChk.y, 18, colors.text);
                if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                    if (CheckCollisionPointRec(mpos, adminChk) || CheckCollisionPointRec(mpos, (Rectangle){ adminChk.x + adminChk.width + 8, adminChk.y, 200, adminChk.height })) {
                        editUserGrantAdmin = !editUserGrantAdmin;
                    }
                }

                // Draw a visible focus border when the password input is active so it's clear the field has focus
                if (userPassFocus) DrawRectangleLinesEx(passRect, 2, colors.accent);
                if (userPassFocus) {
                    int c = GetCharPressed();
                    while (c > 0) {
                        if (c >= 32 && c <= 125 && editUserNewPass.size() < 128) editUserNewPass.push_back((char)c);
                        c = GetCharPressed();
                    }
                    if (IsKeyPressed(KEY_BACKSPACE) && !editUserNewPass.empty()) editUserNewPass.pop_back();
                }

                // Save / Cancel
                Rectangle saveBtn = { modal.x + 12, modal.y + modal.height - RH(0.08f) - 12, modal.width * 0.45f - 18, RH(0.06f) };
                Rectangle cancelBtn = { modal.x + modal.width * 0.55f + 6, modal.y + modal.height - RH(0.08f) - 12, modal.width * 0.45f - 18, RH(0.06f) };
                if (DrawButton(saveBtn, "Save", colors.primary, colors, 18)) {
                    // commit change: only update password if a new one was entered
                    if (!editUserNewPass.empty()) users[editUserIndex].pass = editUserNewPass;
                    // update admin flag according to checkbox
                    users[editUserIndex].isAdmin = editUserGrantAdmin;
                    SaveAllUsers();
                    editingUser = false; editUserIndex = -1; editUserNewPass.clear(); userPassFocus = false;
                }
                if (DrawButton(cancelBtn, "Cancel", colors.buttonBg, colors, 18)) {
                    editingUser = false; editUserIndex = -1; editUserNewPass.clear(); userPassFocus = false;
                }
            }

        }
         else if (state == STATE_OPTIONS) {
              // Back button
             Rectangle backBtn = { (float)RX(0.025f), (float)RY(0.025f), (float)RW(0.10f), (float)RH(0.05f) };
             if (DrawButton(backBtn, "< Back", colors.buttonBg, colors, 16)) {
                  state = STATE_MENU;
              }
             DrawTextScaled("Options", centerX - MeasureTextScaled("Options", 32)/2, RY(0.12f), 32, colors.primary);
             

             // Theme selection section (responsive layout)
             float sectionTop = RY(0.22f);
             DrawTextScaled("Theme:", centerX - MeasureTextScaled("Theme:", 24)/2, sectionTop, 24, colors.text);

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
            DrawTextScaled("Window Mode:", centerX - MeasureTextScaled("Window Mode:", 24)/2, winSectionY, 24, colors.text);
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

    // at exit, unload the font
    UnloadFont(gFont);
    UnloadTexture(logo);
    CloseWindow();
    std::cout << "Exiting application." << std::endl;
    return 0;
}