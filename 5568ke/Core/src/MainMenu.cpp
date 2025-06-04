#include "MainMenu.hpp"
#include <imgui.h>
#include <iostream>
#include <algorithm>

MainMenu& MainMenu::getInstance()
{
    static MainMenu instance;
    return instance;
}

void MainMenu::render()
{
    if (!visible_) return;
    
    // Get display size for responsive design
    ImVec2 displaySize = ImGui::GetIO().DisplaySize;
    float scaleFactor = std::min(displaySize.x / 1920.0f, displaySize.y / 1080.0f);
    scaleFactor = std::max(scaleFactor, 0.5f); // Minimum scale factor
    
    if (showInstructions_) {
        // Instructions window
        ImVec2 instrWindowSize(displaySize.x * 0.7f, displaySize.y * 0.8f);
        ImGui::SetNextWindowSize(instrWindowSize, ImGuiCond_Always);
        ImGui::SetNextWindowPos(ImVec2(displaySize.x * 0.5f, displaySize.y * 0.5f), ImGuiCond_Always, ImVec2(0.5f, 0.5f));
        
        ImGuiWindowFlags instrFlags = ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | 
                                     ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoTitleBar;
        
        if (ImGui::Begin("##Instructions", nullptr, instrFlags)) {
            // Title
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.9f, 0.3f, 1.0f));
            ImGui::SetWindowFontScale(2.5f * scaleFactor);
            
            ImVec2 titleSize = ImGui::CalcTextSize("遊戲說明");
            ImGui::SetCursorPosX((instrWindowSize.x - titleSize.x) * 0.5f);
            ImGui::Text("遊戲說明");
            
            ImGui::SetWindowFontScale(1.0f);
            ImGui::PopStyleColor();
            
            ImGui::Spacing();
            ImGui::Separator();
            ImGui::Spacing();
            
            // Instructions content with larger text
            ImGui::SetWindowFontScale(1.4f * scaleFactor);
            
            ImGui::TextWrapped("歡迎來到「教室的割布麟」！");
            ImGui::Spacing();
            
            ImGui::Text("基本操作:");
            ImGui::BulletText("使用 W、A、S、D 鍵移動角色");
            ImGui::BulletText("使用滑鼠控制視角");
            ImGui::BulletText("按 Tab 鍵切換滑鼠模式（游戲/界面）");
            ImGui::BulletText("按 E 鍵與 NPC 互動或繼續對話");
            ImGui::BulletText("按 ESC 鍵返回主選單");
            
            ImGui::Spacing();
            ImGui::Text("遊戲目標:");
            ImGui::BulletText("探索校園環境");
            ImGui::BulletText("與各種 NPC 角色對話");
            ImGui::BulletText("體驗豐富的故事情節");
            ImGui::BulletText("發現隱藏的秘密和彩蛋");
            
            ImGui::Spacing();
            ImGui::Text("提示:");
            ImGui::BulletText("仔細聆聽每個角色的對話");
            ImGui::BulletText("探索每個角落，可能有驚喜");
            ImGui::BulletText("不同的選擇會帶來不同的結果");
            
            ImGui::SetWindowFontScale(1.0f);
            
            ImGui::Spacing();
            ImGui::Spacing();
            
            // Back button
            ImVec2 backButtonSize(150 * scaleFactor, 50 * scaleFactor);
            float backButtonPosX = (instrWindowSize.x - backButtonSize.x) * 0.5f;
            ImGui::SetCursorPosX(backButtonPosX);
            
            if (ImGui::Button("返回主選單", backButtonSize)) {
                showInstructions_ = false;
            }
        }
        ImGui::End();
    } else {
        // Main menu window
        ImVec2 windowSize(500 * scaleFactor, 400 * scaleFactor);
        ImGui::SetNextWindowSize(windowSize, ImGuiCond_Always);
        ImGui::SetNextWindowPos(ImVec2(displaySize.x * 0.5f, displaySize.y * 0.5f), ImGuiCond_Always, ImVec2(0.5f, 0.5f));
        
        ImGuiWindowFlags flags = ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | 
                                ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoTitleBar |
                                ImGuiWindowFlags_NoScrollbar;
        
        if (ImGui::Begin("##MainMenu", nullptr, flags)) {
            // Game title with enhanced styling
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.9f, 0.3f, 1.0f));
            ImGui::SetWindowFontScale(3.5f * scaleFactor);
            
            ImVec2 titleSize = ImGui::CalcTextSize("教室的割布麟");
            ImGui::SetCursorPosX((windowSize.x - titleSize.x) * 0.5f);
            ImGui::Text("教室的割布麟");
            
            ImGui::SetWindowFontScale(1.0f);
            ImGui::PopStyleColor();
            
            ImGui::Spacing();
            ImGui::Separator();
            ImGui::Spacing();
            ImGui::Spacing();
            
            // Menu buttons with responsive sizing
            ImVec2 buttonSize(250 * scaleFactor, 50 * scaleFactor);
            float buttonPosX = (windowSize.x - buttonSize.x) * 0.5f;
            
            // Enhanced button styling
            ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 8.0f);
            ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(10 * scaleFactor, 8 * scaleFactor));
            ImGui::SetWindowFontScale(1.2f * scaleFactor);
            
            ImGui::SetCursorPosX(buttonPosX);
            if (ImGui::Button("開始遊戲", buttonSize)) {
                startGame_ = true;
                visible_ = false;
            }
            
            ImGui::Spacing();
            
            ImGui::SetCursorPosX(buttonPosX);
            if (ImGui::Button("遊戲說明", buttonSize)) {
                showInstructions_ = true;
            }
            
            ImGui::Spacing();
            
            ImGui::SetCursorPosX(buttonPosX);
            if (ImGui::Button("離開遊戲", buttonSize)) {
                exitGame_ = true;
            }
            
            ImGui::SetWindowFontScale(1.0f);
            ImGui::PopStyleVar(2);
            
            ImGui::Spacing();
            ImGui::Spacing();
            
            // Quick instructions at bottom
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.7f, 0.7f, 0.7f, 1.0f));
            ImGui::SetWindowFontScale(0.9f * scaleFactor);
            
            ImGui::TextWrapped("快速提示: 使用 WASD 移動，E 鍵互動，Tab 切換滑鼠模式");
            ImGui::TextWrapped("按 Enter 開始遊戲，ESC 退出");
            
            ImGui::SetWindowFontScale(1.0f);
            ImGui::PopStyleColor();
        }
        ImGui::End();
    }
}

void MainMenu::processInput(GLFWwindow* window)
{
    if (!visible_) return;
    
    // ESC to exit or go back to main menu from instructions
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
        if (showInstructions_) {
            showInstructions_ = false;
        } else {
            exitGame_ = true;
        }
    }
    
    // Enter to start game (only from main menu, not instructions)
    if (!showInstructions_ && glfwGetKey(window, GLFW_KEY_ENTER) == GLFW_PRESS) {
        startGame_ = true;
        visible_ = false;
    }
}
