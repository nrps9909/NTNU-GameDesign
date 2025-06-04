#pragma once

#include <GLFW/glfw3.h>

class MainMenu {
public:
    static MainMenu& getInstance();
    
    bool isVisible() const { return visible_; }
    void show() { visible_ = true; }
    void hide() { visible_ = false; }
    
    void render();
    void processInput(GLFWwindow* window);
    
    // Returns true if game should start
    bool shouldStartGame() const { return startGame_; }
    void resetStartGame() { startGame_ = false; }
    
    // Returns true if game should exit
    bool shouldExit() const { return exitGame_; }

private:
    MainMenu() = default;
    ~MainMenu() = default;
    MainMenu(const MainMenu&) = delete;
    MainMenu& operator=(const MainMenu&) = delete;
    
    bool visible_ = true;  // Show menu by default
    bool startGame_ = false;
    bool exitGame_ = false;
    bool showInstructions_ = false;
};