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
    
    // Login variables
    char username[32] = "";
    char password[32] = "";
    int inputFocus = 0;
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
        
        if (state == STATE_LOGIN) {
            if (IsKeyPressed(KEY_TAB)) inputFocus = 1 - inputFocus;
            if (IsKeyPressed(KEY_ENTER)) {
                std::cout << "\n=== LOGIN ATTEMPT ===" << std::endl;
                std::cout << "Username: '" << username << "'" << std::endl;
                std::cout << "Password: '" << password << "'" << std::endl;

                if (CheckLogin(users, std::string(username), std::string(password))) {
                    std::cout << "LOGIN SUCCESSFUL! Accessing main menu..." << std::endl;
                    currentUser = std::string(username);
                    isAdmin = (currentUser == "admin");
                    state = STATE_MENU;
                    loginFailed = false;
                } else {
                    std::cout << "LOGIN FAILED! Please try again." << std::endl;
                    loginFailed = true;
                }
                std::cout << "====================\n" << std::endl;
            }
        }
        else if (state == STATE_REGISTER) {
            if (IsKeyPressed(KEY_TAB)) regInputFocus = (regInputFocus + 1) % 3;
            if (IsKeyPressed(KEY_ENTER)) {
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
        }
        else if (state == STATE_MENU) {
            if (IsKeyPressed(KEY_DOWN)) menuIndex = (menuIndex + 1) % 3;
            if (IsKeyPressed(KEY_UP)) menuIndex = (menuIndex + 2) % 3;
            if (IsKeyPressed(KEY_ENTER)) {
                if (menuIndex == 0) state = STATE_VIEW_PRODUCTS;
                else if (menuIndex == 1) state = STATE_ADD_PRODUCT;
                else if (menuIndex == 2) state = STATE_OPTIONS;
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
            
            // Register button
            Rectangle registerBtn = { 250, 460, 150, 30 };
            if (DrawButton(registerBtn, "Register New User", SKYBLUE, 16)) {
                state = STATE_REGISTER;
                // Clear registration form
                strcpy(regUsername, "");
                strcpy(regPassword, "");
                strcpy(regConfirmPassword, "");
                regInputFocus = 0;
                regFailed = false;
                regMessage = "";
            }

            if (loginFailed)
                DrawText("Login failed. Try again.", 280, 500, 20, RED);

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
        else if (state == STATE_REGISTER) {
            DrawText("Register New User", 320, 100, 32, DARKBLUE);
            DrawText("Press ESC to return to login", 250, 140, 16, GRAY);
            
            // Handle text input for registration screen
            int key = GetCharPressed();
            if (key >= 32 && key <= 125) {
                if (regInputFocus == 0 && strlen(regUsername) < 31) {
                    int len = strlen(regUsername);
                    regUsername[len] = (char)key;
                    regUsername[len + 1] = '\0';
                }
                else if (regInputFocus == 1 && strlen(regPassword) < 31) {
                    int len = strlen(regPassword);
                    regPassword[len] = (char)key;
                    regPassword[len + 1] = '\0';
                }
                else if (regInputFocus == 2 && strlen(regConfirmPassword) < 31) {
                    int len = strlen(regConfirmPassword);
                    regConfirmPassword[len] = (char)key;
                    regConfirmPassword[len + 1] = '\0';
                }
            }
            if (IsKeyPressed(KEY_BACKSPACE)) {
                if (regInputFocus == 0 && strlen(regUsername) > 0)
                    regUsername[strlen(regUsername) - 1] = '\0';
                else if (regInputFocus == 1 && strlen(regPassword) > 0)
                    regPassword[strlen(regPassword) - 1] = '\0';
                else if (regInputFocus == 2 && strlen(regConfirmPassword) > 0)
                    regConfirmPassword[strlen(regConfirmPassword) - 1] = '\0';
            }
            if (IsKeyPressed(KEY_SPACE)) regShowPassword = !regShowPassword;

            DrawText("Username:", 250, 180, 20, BLACK);
            DrawRectangle(370, 175, 200, 30, LIGHTGRAY);
            DrawText(regUsername, 375, 180, 20, BLACK);
            if (regInputFocus == 0) DrawRectangleLines(370, 175, 200, 30, RED);

            DrawText("Password:", 250, 230, 20, BLACK);
            DrawRectangle(370, 225, 200, 30, LIGHTGRAY);
            std::string regPassDisplay = regShowPassword ? regPassword : std::string(strlen(regPassword), '*');
            DrawText(regPassDisplay.c_str(), 375, 230, 20, BLACK);
            if (regInputFocus == 1) DrawRectangleLines(370, 225, 200, 30, RED);
            
            DrawText("Confirm Password:", 220, 280, 20, BLACK);
            DrawRectangle(370, 275, 200, 30, LIGHTGRAY);
            std::string regConfirmDisplay = regShowPassword ? regConfirmPassword : std::string(strlen(regConfirmPassword), '*');
            DrawText(regConfirmDisplay.c_str(), 375, 280, 20, BLACK);
            if (regInputFocus == 2) DrawRectangleLines(370, 275, 200, 30, RED);

            DrawText("Press TAB to switch fields, ENTER to register", 200, 330, 18, GRAY);
            DrawText("Press SPACE to show/hide passwords", 220, 350, 18, GRAY);
            DrawText("Minimum 3 characters for username and password", 200, 370, 16, GRAY);
            
            // Back to login button
            Rectangle backToLoginBtn = { 200, 420, 120, 30 };
            if (DrawButton(backToLoginBtn, "← Back to Login", LIGHTGRAY, 16)) {
                state = STATE_LOGIN;
            }
            
            // Register button
            Rectangle regBtn = { 350, 420, 100, 30 };
            if (DrawButton(regBtn, "Register", LIME, 16)) {
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
                DrawText(regMessage.c_str(), 150, 470, 18, msgColor);
            }
        }
        else if (state == STATE_MENU) {
            // Logout button in top-left
            Rectangle logoutBtn = { 20, 20, 100, 30 };
            if (DrawButton(logoutBtn, "← Logout", LIGHTGRAY, 16)) {
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
            
            // Show current user info
            std::string userInfo = "Logged in as: " + currentUser;
            if (isAdmin) userInfo += " (Admin)";
            DrawText(userInfo.c_str(), 600, 25, 16, DARKGRAY);
            
            DrawText("Clothing Store - Main Menu", 220, 80, 30, DARKBLUE);

            Rectangle btnView = { 300, 170, 200, 60 };
            Rectangle btnAdd = { 300, 250, 200, 60 };
            Rectangle btnOptions = { 300, 330, 200, 60 };

            if (DrawButton(btnView, "View Products", LIGHTGRAY, 20)) state = STATE_VIEW_PRODUCTS;
            if (DrawButton(btnAdd, "Add Product", LIGHTGRAY, 20)) state = STATE_ADD_PRODUCT;
            if (DrawButton(btnOptions, "Options", LIGHTGRAY, 20)) state = STATE_OPTIONS;

            Rectangle selector = { 280.0f, 170.0f + menuIndex * 80.0f, 240.0f, 60.0f };
            DrawRectangleLinesEx(selector, 3, RED);

            DrawText("Use Up/Down and Enter or click with mouse", 200, 480, 16, GRAY);
        }
        else if (state == STATE_VIEW_PRODUCTS) {
            // Load products once when entering view (or when not loaded)
            if (!productsLoaded) productsLoaded = LoadProducts("data/products.txt");

            // Back button
            Rectangle backBtn = { 20, 20, 80, 30 };
            if (DrawButton(backBtn, "← Back", LIGHTGRAY, 16)) {
                state = STATE_MENU;
            }

            DrawText("Product List", 340, 30, 28, DARKBLUE);
            DrawText("Press ESC or click Back to return to menu", 220, 70, 16, GRAY);

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
        }
        else if (state == STATE_ADD_PRODUCT) {
            // Back button
            Rectangle backBtn = { 20, 20, 80, 30 };
            if (DrawButton(backBtn, "← Back", LIGHTGRAY, 16)) {
                state = STATE_MENU;
            }

            DrawText("Add Product", 320, 40, 28, DARKBLUE);

            if (currentUser.empty() || !isAdmin) {
                DrawText("Admin privileges required to add products.", 180, 120, 20, RED);
                DrawText("Please login with an admin account.", 220, 160, 18, GRAY);

                Rectangle btnToLogin = { 300, 220, 200, 50 };
                if (DrawButton(btnToLogin, "Go to Login", LIGHTGRAY, 20)) {
                    // clear previous inputs and go to login
                    memset(username, 0, sizeof(username));
                    memset(password, 0, sizeof(password));
                    state = STATE_LOGIN;
                }
            } else {
                static std::string nameInput;
                static std::string priceInput;
                static std::string sizeInput;
                static std::string removeInput;
                static std::string msg;
                // 0=name,1=price,2=size,3=remove
                static int activeFieldAdd = 0;
                int y = 130;

                Rectangle nameRect = { 240, (float)(y - 5), 420, 30 };
                Rectangle priceRect = { 240, (float)(y + 45), 200, 30 };
                Rectangle sizeRect = { 520, (float)(y + 45), 140, 30 };
                Rectangle removeRect = { 240, (float)(y + 95), 420, 30 };

                DrawText("Name:", 160, y, 20, BLACK);
                DrawRectangleRec(nameRect, LIGHTGRAY);
                DrawText(nameInput.c_str(), 250, y, 20, BLACK);
                if (activeFieldAdd == 0) DrawRectangleLinesEx(nameRect, 2, RED);

                DrawText("Price:", 160, y + 50, 20, BLACK);
                DrawRectangleRec(priceRect, LIGHTGRAY);
                DrawText(priceInput.c_str(), 250, y + 50, 20, BLACK);
                if (activeFieldAdd == 1) DrawRectangleLinesEx(priceRect, 2, RED);

                DrawText("Size:", 460, y + 50, 20, BLACK);
                // Draw selectable size buttons instead of text input
                static const std::vector<std::string> sizeOptions = {"XS","S","M","L","XL","XXL"};
                int btnW = 40;
                int btnH = 30;
                int gap = 8;
                for (size_t si = 0; si < sizeOptions.size(); ++si) {
                    Rectangle sb = { 520.0f + (float)si * (btnW + gap), (float)(y + 45), (float)btnW, (float)btnH };
                    // highlight selected
                    if (!sizeInput.empty() && sizeInput == sizeOptions[si]) DrawRectangleRec(sb, LIGHTGRAY);
                    if (DrawButton(sb, sizeOptions[si].c_str(), LIGHTGRAY, 18)) {
                        sizeInput = sizeOptions[si];
                        activeFieldAdd = 2;
                    }
                    // draw selection border
                    if (!sizeInput.empty() && sizeInput == sizeOptions[si]) DrawRectangleLinesEx(sb, 2, RED);
                }

                // Remove by name (admin)
                DrawText("Remove product (name):", 80, y + 100, 20, BLACK);
                DrawRectangleRec(removeRect, LIGHTGRAY);
                DrawText(removeInput.c_str(), 250, y + 100, 20, BLACK);
                if (activeFieldAdd == 3) DrawRectangleLinesEx(removeRect, 2, RED);

                // handle typing into the active field (includes remove field)
                int cp = GetCharPressed();
                while (cp > 0) {
                    if (cp >= 32 && cp <= 125) {
                        if (activeFieldAdd == 0 && nameInput.size() < 120) nameInput.push_back((char)cp);
                        else if (activeFieldAdd == 1 && priceInput.size() < 32) priceInput.push_back((char)cp);
                        // activeFieldAdd == 2 is the size selector (buttons) so don't accept typed chars
                        else if (activeFieldAdd == 3 && removeInput.size() < 120) removeInput.push_back((char)cp);
                    }
                    cp = GetCharPressed();
                }

                if (IsKeyPressed(KEY_TAB)) activeFieldAdd = (activeFieldAdd + 1) % 4;
                if (IsKeyPressed(KEY_BACKSPACE)) {
                    if (activeFieldAdd == 0 && !nameInput.empty()) nameInput.pop_back();
                    else if (activeFieldAdd == 1 && !priceInput.empty()) priceInput.pop_back();
                    else if (activeFieldAdd == 3 && !removeInput.empty()) removeInput.pop_back();
                    // size (activeFieldAdd==2) uses buttons; nothing to backspace
                }

                Rectangle btnSave = { 220, 300, 160, 40 };
                Rectangle btnRemove = { 400, 300, 160, 40 };
                Rectangle btnCancel = { 580, 300, 160, 40 };

                if (DrawButton(btnSave, "Save", LIGHTGRAY, 20)) {
                    double pr = 0.0; bool ok = false;
                    try {
                        size_t start = 0; while (start < priceInput.size() && !((priceInput[start] >= '0' && priceInput[start] <= '9') || priceInput[start]=='.' || priceInput[start]=='-')) start++;
                        std::string trimmed = priceInput.substr(start);
                        if (!trimmed.empty()) { pr = std::stod(trimmed); ok = true; }
                    } catch (...) { ok = false; }
                    if (!ok) msg = "Invalid price";
                    else if (nameInput.empty()) msg = "Name required";
                    else {
                        std::ofstream ofs("data/products.txt", std::ios::app);
                        if (ofs) {
                            ofs << nameInput << ";" << pr;
                            if (!sizeInput.empty()) ofs << ";" << sizeInput;
                            ofs << "\n";
                            ofs.close();
                            msg = "Product saved";
                            nameInput.clear(); priceInput.clear(); sizeInput.clear(); productsLoaded = false;
                        } else msg = "Failed to open file";
                    }
                }

                if (DrawButton(btnRemove, "Remove", LIGHTGRAY, 20)) {
                    // Remove products matching removeInput (case-insensitive name match)
                    if (removeInput.empty()) {
                        msg = "Enter a product name to remove";
                    } else {
                        std::ifstream ifs("data/products.txt");
                        if (!ifs) { msg = "Failed to open products file"; }
                        else {
                            std::vector<std::string> lines;
                            std::string line;
                            int removed = 0;
                            auto toLower = [](const std::string &s) {
                                std::string out = s;
                                std::transform(out.begin(), out.end(), out.begin(), [](unsigned char c){ return std::tolower(c); });
                                return out;
                            };
                            std::string target = toLower(removeInput);
                            while (std::getline(ifs, line)) {
                                if (line.empty()) continue;
                                std::string nameOnly = line;
                                size_t pos = line.find(';');
                                if (pos != std::string::npos) nameOnly = line.substr(0, pos);
                                if (toLower(nameOnly) == target) {
                                    removed++;
                                    continue; // skip this line
                                }
                                lines.push_back(line);
                            }
                            ifs.close();
                            std::ofstream ofs("data/products.txt", std::ios::trunc);
                            if (!ofs) { msg = "Failed to write products file"; }
                            else {
                                for (const auto &l : lines) ofs << l << "\n";
                                ofs.close();
                                if (removed > 0) {
                                    msg = "Removed " + std::to_string(removed) + " product(s)";
                                    removeInput.clear();
                                    productsLoaded = false;
                                } else msg = "No product matched that name";
                            }
                        }
                    }
                }

                if (DrawButton(btnCancel, "Cancel", LIGHTGRAY, 20)) {
                    state = STATE_MENU;
                }
                if (!msg.empty()) DrawText(msg.c_str(), 320, 360, 18, DARKGREEN);
            }
        }
        else if (state == STATE_OPTIONS) {
            // Back button
            Rectangle backBtn = { 20, 20, 80, 30 };
            if (DrawButton(backBtn, "← Back", LIGHTGRAY, 16)) {
                state = STATE_MENU;
            }

            DrawText("Options (placeholder)", 260, 200, 24, DARKBLUE);
            DrawText("Press ESC or click Back to return to menu", 200, 240, 18, GRAY);
        }

        EndDrawing();
    }

    CloseWindow();
    std::cout << "Exiting application." << std::endl;
    return 0;
}