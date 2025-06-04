#include "MainMenu.hpp"
#include <imgui.h>
#include <iostream>

MainMenu& MainMenu::getInstance()
{
    static MainMenu instance;
    return instance;
}

void MainMenu::render()
{
    if (!visible_) return;
    
    // Center the menu window
    ImVec2 displaySize = ImGui::GetIO().DisplaySize;
    ImVec2 windowSize(400, 300);
    ImGui::SetNextWindowSize(windowSize, ImGuiCond_Always);
    ImGui::SetNextWindowPos(ImVec2(displaySize.x * 0.5f, displaySize.y * 0.5f), ImGuiCond_Always, ImVec2(0.5f, 0.5f));
    
    ImGuiWindowFlags flags = ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | 
                            ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoTitleBar |
                            ImGuiWindowFlags_NoScrollbar;
      if (ImGui::Begin("##MainMenu", nullptr, flags)) {
        // Game title
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.9f, 0.3f, 1.0f));
        ImGui::SetWindowFontScale(4.0f); // Increased to 4.0f for much larger title
        
        ImVec2 titleSize = ImGui::CalcTextSize("教室的割布麟");
        ImGui::SetCursorPosX((windowSize.x - titleSize.x) * 0.5f);
        ImGui::Text("教室的割布麟");
        
        ImGui::SetWindowFontScale(1.0f);
        ImGui::PopStyleColor();
        
        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();
        ImGui::Spacing();
        
        // Menu buttons
        ImVec2 buttonSize(200, 40);
        float buttonPosX = (windowSize.x - buttonSize.x) * 0.5f;
        
        ImGui::SetCursorPosX(buttonPosX);
        if (ImGui::Button("開始遊戲", buttonSize)) {
            startGame_ = true;
            visible_ = false;
        }
        
        ImGui::Spacing();
        
        ImGui::SetCursorPosX(buttonPosX);
        if (ImGui::Button("遊戲說明", buttonSize)) {
            // Show game instructions in future implementation
            std::cout << "[MainMenu] Game instructions clicked" << std::endl;
        }
        
        ImGui::Spacing();
        
        ImGui::SetCursorPosX(buttonPosX);
        if (ImGui::Button("離開遊戲", buttonSize)) {
            exitGame_ = true;
        }
        
        ImGui::Spacing();
        ImGui::Spacing();
        
        // Instructions
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.7f, 0.7f, 0.7f, 1.0f));
        ImGui::TextWrapped("使用 WASD 移動角色，按 E 與 NPC 互動");
        ImGui::TextWrapped("按 Tab 鍵切換滑鼠模式");
        ImGui::PopStyleColor();
    }
    ImGui::End();
}

void MainMenu::processInput(GLFWwindow* window)
{
    if (!visible_) return;
    
    // ESC to exit
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
        exitGame_ = true;
    }
    
    // Enter to start game
    if (glfwGetKey(window, GLFW_KEY_ENTER) == GLFW_PRESS) {
        startGame_ = true;
        visible_ = false;
    }
}
