// IMPORTANT: Review include order based on your project structure for GLAD
// If Scene.hpp -> include_5568ke.hpp includes system GL, GLAD error will persist.
// Ideally, glad.h is included very early in your main compilation flow.
#include <glad/glad.h>  // Attempt to include GLAD first in this TU
#include <GLFW/glfw3.h> // Then GLFW

#include "DialogSystem.hpp" // Your class definitions

#include <algorithm>
#include <iostream> // For std::cout
#include <limits>
#include <cmath> // For std::fmod
#include <cstdio> // For snprintf

#include <imgui.h>
#include <glm/gtc/matrix_transform.hpp>

// These headers provide full definitions needed for implementations
#include "Scene.hpp"         // For Scene, Camera
#include "GameObject.hpp"    // For GameObject (already in DialogSystem.hpp, but good for explicitness)
#include "Model.hpp"         // For Model definition
#include "AnimationClip.hpp" // For AnimationClip definition

// -------- Implementation of DialogSystem methods --------

DialogSystem& DialogSystem::getInstance()
{
	static DialogSystem instance;
	return instance;
}

NPC& DialogSystem::addNPC(std::shared_ptr<GameObject> go, std::vector<std::shared_ptr<DialogBase>> script)
{
	npcs_.push_back({
        go,                         // go
        std::move(script),          // dialogs
        false,                      // routeEnabled (will be set true by init functions)
        false,                      // showIcon
        false,                      // inDialog
        0,                          // scriptIndex
        0,                          // lineIndex
        0,                          // totalScore
        false,                      // isPlayingIdleAnimation
        0.0f,                       // idleAnimationTime
        -1                          // idleAnimationIndex
    });

    if (npcs_.back().go) {
        std::cout << "[DialogSystem] Added NPC. GameObject name: '" << std::string(npcs_.back().go->name) << "'. Initializing idle animation." << std::endl;
		initializeNPCIdleAnimation(npcs_.back());
	} else {
        std::cout << "[DialogSystem] Added NPC with a nullptr GameObject. Cannot initialize idle animation." << std::endl;
    }
	return npcs_.back();
}

glm::vec2 DialogSystem::worldToScreen(glm::vec3 const& pos, Scene const& scene, int viewportW, int viewportH)
{
	glm::mat4 mvp = scene.cam.proj * scene.cam.view; 
	glm::vec4 clipPos = mvp * glm::vec4(pos, 1.0f);
	if (clipPos.w <= 0.0f) { 
		return glm::vec2(-1.0f, -1.0f); 
	}
	glm::vec3 ndc = glm::vec3(clipPos) / clipPos.w;
	float screenX = (ndc.x + 1.0f) * 0.5f * static_cast<float>(viewportW);
	float screenY = (1.0f - ndc.y) * 0.5f * static_cast<float>(viewportH);
	return glm::vec2(screenX, screenY);
}

void DialogSystem::initializeNPCIdleAnimation(NPC& npc)
{
	if (!npc.go || !npc.go->getModel()) {
		return;
	}
	npc.idleAnimationIndex = findIdleAnimationIndex(npc.go);
	if (npc.idleAnimationIndex != -1) {
		startIdleAnimation(npc);
	}
}

int DialogSystem::findIdleAnimationIndex(std::shared_ptr<GameObject> const& go)
{
	if (!go || !go->getModel() || go->getModel()->animations.empty()) {
		return -1;
	}
	auto const& animations = go->getModel()->animations;
	for (size_t i = 0; i < animations.size(); ++i) {
        if (!animations[i]) continue; 
		std::string clipName = animations[i]->clipName;
		std::transform(clipName.begin(), clipName.end(), clipName.begin(),
			[](unsigned char c){ return std::tolower(c); }); 
		if (clipName.find("idle") != std::string::npos) {
			return static_cast<int>(i);
		}
	}
	if (!animations.empty()) {
        return 0; 
    }
	return -1;
}

void DialogSystem::startIdleAnimation(NPC& npc)
{
	if (npc.idleAnimationIndex == -1 || !npc.go || !npc.go->getModel()) {
		return;
	}
	auto model = npc.go->getModel();
	if (static_cast<size_t>(npc.idleAnimationIndex) >= model->animations.size() || !model->animations[npc.idleAnimationIndex]) {
		return;
	}
	npc.isPlayingIdleAnimation = true;
	npc.idleAnimationTime = 0.0f;
	model->animations[npc.idleAnimationIndex]->setAnimationFrame(model->nodes, 0.0f);
	model->updateLocalMatrices();
}

void DialogSystem::updateNPCIdleAnimation(NPC& npc, float dt)
{
    if (!npc.go || !npc.go->getModel() || npc.idleAnimationIndex == -1) {
        return;
    }
    if (npc.inDialog) {
        if (npc.isPlayingIdleAnimation) {
            npc.isPlayingIdleAnimation = false; 
        }
        return; 
    }
	if (!npc.isPlayingIdleAnimation) { 
		startIdleAnimation(npc); 
        if (!npc.isPlayingIdleAnimation) return; 
	}
	if (npc.isPlayingIdleAnimation) {
		auto model = npc.go->getModel();
		if (static_cast<size_t>(npc.idleAnimationIndex) < model->animations.size() && model->animations[npc.idleAnimationIndex]) {
			auto const& idleClip = model->animations[npc.idleAnimationIndex];
			npc.idleAnimationTime += dt;
			float duration = idleClip->getDuration();
			if (duration > 0.0f) { 
				npc.idleAnimationTime = std::fmod(npc.idleAnimationTime, duration);
			} else {
                npc.idleAnimationTime = 0.0f; 
            }
			idleClip->setAnimationFrame(model->nodes, npc.idleAnimationTime);
			model->updateLocalMatrices();
		} else {
            npc.isPlayingIdleAnimation = false;
        }
	}
}

void DialogSystem::update(Scene& scene, float dt)
{
	std::shared_ptr<GameObject> player = nullptr;
    static bool playerSearchedAndWarned = false; 
    static bool playerFoundOnce = false; 

    if (!playerFoundOnce) { 
        for (const auto& go_ptr : scene.gameObjects) { 
            if (go_ptr && go_ptr->name == std::string_view("Player")) { 
                player = go_ptr;
                playerFoundOnce = true; 
                std::cout << "[DialogSystem::update] Player GameObject ('Player') found successfully. Name: '" << std::string(player->name) << "'" << std::endl;
                break;
            }
        }
    } else { 
         for (const auto& go_ptr : scene.gameObjects) { 
            if (go_ptr && go_ptr->name == std::string_view("Player")) { 
                player = go_ptr;
                break;
            }
        }
    }
	
    if (!player) {
        if (!playerSearchedAndWarned) { 
            std::cout << "[DialogSystem::update] WARNING: Player GameObject (named 'Player') not found in scene! Dialog interactions will not work." << std::endl;
            playerSearchedAndWarned = true; 
        }
        for (auto& npc_iter : npcs_) {
            if (npc_iter.go && npc_iter.go->visible) {
                updateNPCIdleAnimation(npc_iter, dt);
            }
            npc_iter.showIcon = false; 
        }
        return; 
    }

	for (auto& npc_iter : npcs_) { 
        std::cout << "[DS_Update] NPC Check: "; // This is the main debug print for NPC state
        if (npc_iter.go) {
            std::cout << "GO: '" << std::string(npc_iter.go->name) 
                      << "', Vis: " << npc_iter.go->visible
                      << ", RouteEn: " << npc_iter.routeEnabled
                      << ", InDlg: " << npc_iter.inDialog
                      << ", ShowIcon: " << npc_iter.showIcon
                      << ", ScriptIdx: " << npc_iter.scriptIndex;
            if (!npc_iter.dialogs.empty()) {
                 std::cout << ", Dialogs: " << npc_iter.dialogs.size();
            } else {
                 std::cout << ", Dialogs: 0";
            }
        } else {
            std::cout << "GO is nullptr.";
        }
        std::cout << std::endl;

		if (!npc_iter.go || !npc_iter.go->visible) {
            if (npc_iter.showIcon) { 
                npc_iter.showIcon = false;
            }
			continue;
		}
		updateNPCIdleAnimation(npc_iter, dt); 

		if (npc_iter.inDialog) {
            if (npc_iter.showIcon) { 
                npc_iter.showIcon = false;
            }
			continue;
		}
		
		if (npc_iter.routeEnabled) { 
            float distance = player->distanceTo(*npc_iter.go);
            std::cout << "[DS_Update] NPC '" << std::string(npc_iter.go->name) 
                      << "' (RouteEnabled): Dist to Player=" << distance;

            float const INTERACTION_RANGE = 1.0f; 
            if (distance <= INTERACTION_RANGE) {
                npc_iter.showIcon = true;
            } else {
                npc_iter.showIcon = false;
            }
            std::cout << ", NewShowIcon: " << npc_iter.showIcon << std::endl;
        } else { 
            if (npc_iter.showIcon) { 
                npc_iter.showIcon = false;
            }
        }
	}
}

void DialogSystem::render(Scene const& scene)
{
    if (ImGui::GetCurrentContext() == nullptr) return; 
	ImVec2 displaySize = ImGui::GetIO().DisplaySize;
	int viewportW = static_cast<int>(displaySize.x);
	int viewportH = static_cast<int>(displaySize.y);
	ImDrawList* drawList = ImGui::GetBackgroundDrawList(); 

	for (auto const& npc_iter : npcs_) { 
		if (npc_iter.showIcon && npc_iter.go && npc_iter.go->visible) {
			glm::vec3 npcPos = npc_iter.go->getWorldPosition();
			glm::vec3 iconPos3D = npcPos + glm::vec3(0.0f, npc_iter.go->scale.y + 0.5f, 0.0f); 
			glm::vec2 screenPos = worldToScreen(iconPos3D, scene, viewportW, viewportH);

			if (screenPos.x >= 0.0f && screenPos.x < static_cast<float>(viewportW) && 
                screenPos.y >= 0.0f && screenPos.y < static_cast<float>(viewportH)) {
				ImVec2 iconCenter(screenPos.x, screenPos.y);
                float baseSize = 12.0f;
                ImVec2 rectMin = ImVec2(iconCenter.x - baseSize * 0.2f, iconCenter.y - baseSize * 0.7f);
                ImVec2 rectMax = ImVec2(iconCenter.x + baseSize * 0.2f, iconCenter.y + baseSize * 0.2f);
                drawList->AddRectFilled(rectMin, rectMax, IM_COL32(255, 255, 0, 220), 2.0f); 
                ImVec2 dotCenter = ImVec2(iconCenter.x, iconCenter.y + baseSize * 0.5f);
                drawList->AddCircleFilled(dotCenter, baseSize * 0.2f, IM_COL32(255, 255, 0, 220)); 
                ImVec2 textSize = ImGui::CalcTextSize("Press E to interact");
				ImVec2 textPos(iconCenter.x - textSize.x / 2, iconCenter.y + baseSize + 5.0f);
				drawList->AddText(textPos, IM_COL32(255, 255, 255, 255), "Press E to interact");
			}
		}
	}
	renderDialogUI();
}

void DialogSystem::renderDialogUI()
{
    if (ImGui::GetCurrentContext() == nullptr) return;
	NPC* activeNPC_ptr = nullptr; 
	for (auto& npc_iter : npcs_) { 
		if (npc_iter.inDialog) {
			activeNPC_ptr = &npc_iter;
			break;
		}
	}
	if (!activeNPC_ptr) return;

	if (activeNPC_ptr->scriptIndex >= activeNPC_ptr->dialogs.size()) {
		activeNPC_ptr->inDialog = false;
		return;
	}
	DialogBase* currentDialogBase = activeNPC_ptr->dialogs[activeNPC_ptr->scriptIndex].get();
    if (!currentDialogBase) { 
        activeNPC_ptr->inDialog = false; return;
    }

	if (currentDialogBase->type == DialogType::DIALOG) {
		renderDialog(*static_cast<Dialog*>(currentDialogBase), *activeNPC_ptr);
	}
	else if (currentDialogBase->type == DialogType::QUIZ) {
		renderQuiz(*static_cast<Quiz*>(currentDialogBase), *activeNPC_ptr);
	}
	else if (currentDialogBase->type == DialogType::GOODEND) {
		renderEnding(*static_cast<Dialog*>(currentDialogBase), *activeNPC_ptr, true);
	}
	else if (currentDialogBase->type == DialogType::BADEND) {
		renderEnding(*static_cast<Dialog*>(currentDialogBase), *activeNPC_ptr, false);
	}
}

namespace { 
    bool isNarrativeLineLocal(std::string const& line)
    {
        if (line.empty()) return false;
        if (line.rfind("（", 0) == 0 ) return true; 
        if (line[0] == '(' ) return true; 
        if (line.rfind("【", 0) == 0) return true; 
        return false;
    }
}

void DialogSystem::renderDialog(Dialog const& dialog, NPC& npc)
{
	ImGui::SetNextWindowSize(ImVec2(ImGui::GetIO().DisplaySize.x * 0.8f, ImGui::GetIO().DisplaySize.y * 0.3f), ImGuiCond_Always);
	ImGui::SetNextWindowPos(ImVec2(ImGui::GetIO().DisplaySize.x * 0.5f, ImGui::GetIO().DisplaySize.y * 0.95f), ImGuiCond_Always, ImVec2(0.5f, 1.0f));
	ImGuiWindowFlags flags = ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse;

	if (ImGui::Begin("##DialogWindow", nullptr, flags)) {
        // 為按鈕稍微增加 footerHeight
        float footerHeight = ImGui::GetTextLineHeightWithSpacing() * 2.8f; // 根據按鈕大小調整
		ImGui::BeginChild("DialogContent", ImVec2(0, ImGui::GetContentRegionAvail().y - footerHeight), false, ImGuiWindowFlags_AlwaysVerticalScrollbar);

		for (size_t i = 0; i <= npc.lineIndex && i < dialog.lines.size(); ++i) {
			std::string const& line = dialog.lines[i];
			if (isNarrativeLineLocal(line)) {
				ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.7f, 0.7f, 0.7f, 1.0f));
                ImGui::TextWrapped("%s", line.c_str());
				ImGui::PopStyleColor();
			} else {
				size_t colonPos = line.find("："); // Full-width colon
				if (colonPos == std::string::npos) colonPos = line.find(":"); // Half-width colon
				if (colonPos != std::string::npos) {
					std::string speaker = line.substr(0, colonPos);
                    // Correctly handle colon length for substr
					std::string content = line.substr(colonPos + (line.substr(colonPos, 3) == "：" ? 3 : 1));
                    content.erase(0, content.find_first_not_of(" \t\n\r\f\v")); // Trim leading whitespace
                    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.9f, 0.9f, 0.2f, 1.0f)); // Speaker color
					ImGui::TextUnformatted(speaker.c_str());
                    ImGui::PopStyleColor();
					ImGui::SameLine();
					ImGui::TextWrapped("%s", content.c_str());
				} else {
					ImGui::TextWrapped("%s", line.c_str());
				}
			}
            ImGui::Dummy(ImVec2(0, ImGui::GetTextLineHeight() * 0.3f)); // Small spacing between lines
		}
        // Auto-scroll to bottom
        if (npc.lineIndex == 0 || ImGui::GetScrollY() >= ImGui::GetScrollMaxY() - ImGui::GetTextLineHeight() * 2.0f ) {
             ImGui::SetScrollHereY(1.0f);
        }
		ImGui::EndChild();
		ImGui::Separator();
        ImGui::SetCursorPosY(ImGui::GetCursorPosY() + ImGui::GetStyle().ItemSpacing.y);

        // --- Footer with Continue Text, Page Number, and Leave Button ---
        // Continue Text
		if (npc.lineIndex < dialog.lines.size() - 1) {
			ImGui::TextDisabled("按 E 繼續...");
		} else {
			ImGui::TextDisabled("按 E 結束...");
		}
        ImGui::SameLine(); // Keep "Press E" and other footer items on the same line if possible

        // Calculate positions for Page Number and Leave Button to be on the right
        const float leaveButtonWidth = 80.0f; // Adjust as needed
        const float padding = ImGui::GetStyle().FramePadding.x;
        const float spacing = ImGui::GetStyle().ItemSpacing.x;

        char page_buf[32];
        snprintf(page_buf, sizeof(page_buf), "(%zu/%zu)", npc.lineIndex + 1, dialog.lines.size());
        ImVec2 pageTextSize = ImGui::CalcTextSize(page_buf);

        // Position Leave Button to the far right
        float leaveButtonPosX = ImGui::GetWindowContentRegionMax().x - leaveButtonWidth - padding;
        
        // Position Page Number to the left of the Leave Button
        float pageNumPosX = leaveButtonPosX - pageTextSize.x - spacing;

        // Ensure "Press E" text doesn't overlap with page numbers or button
        // This is a simple check, might need more robust layout for very narrow windows
        if (ImGui::GetCursorPosX() < pageNumPosX - spacing) {
             // It's fine, elements are naturally spaced
        } else {
            // Not enough space, might need to rethink layout or make button smaller
            // For now, just let them be placed
        }

        ImGui::SameLine(pageNumPosX > ImGui::GetCursorPosX() ? pageNumPosX : 0); // Move to position for page number
        ImGui::TextDisabled("%s", page_buf);

        ImGui::SameLine(leaveButtonPosX > ImGui::GetCursorPosX() ? leaveButtonPosX : 0); // Move to position for leave button
		if (ImGui::Button("離開##DialogLeave", ImVec2(leaveButtonWidth, 0))) { // Using 0 for height makes it default
			npc.inDialog = false;
            // Optional: Reset dialog progress or disable the route for this NPC
            // npc.scriptIndex = 0;
            // npc.lineIndex = 0;
            // npc.routeEnabled = false; // If you want to prevent immediate re-interaction
            if (npc.go) {
                std::cout << "[DialogSystem] Player left dialog with NPC: " << npc.go->name.data() << std::endl;
            } else {
                std::cout << "[DialogSystem] Player left dialog with NPC (Unknown GO)." << std::endl;
            }
            //AudioManager::getInstance().playSoundEffect("ui_cancel_sfx"); // Example sfx
		}
		ImGui::End();
	}
}

void DialogSystem::renderQuiz(Quiz const& quiz, NPC& npc)
{
	ImVec2 windowSize(std::min(ImGui::GetIO().DisplaySize.x * 0.7f, 900.0f), ImGui::GetIO().DisplaySize.y * 0.75f);
    ImGui::SetNextWindowSize(windowSize, ImGuiCond_Always);
	ImGui::SetNextWindowPos(ImVec2(ImGui::GetIO().DisplaySize.x * 0.5f, ImGui::GetIO().DisplaySize.y * 0.5f), ImGuiCond_Always, ImVec2(0.5f, 0.5f));
	ImGuiWindowFlags flags = ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoTitleBar;

	if (ImGui::Begin("##QuizWindow", nullptr, flags)) {
		ImGui::PushTextWrapPos(ImGui::GetCursorPos().x + ImGui::GetContentRegionAvail().x);
        ImGui::TextColored(ImVec4(1.0f, 0.9f, 0.3f, 1.0f), "問題:");
		ImGui::Separator(); ImGui::Spacing();
		ImGui::TextWrapped("%s", quiz.question.c_str());
		ImGui::PopTextWrapPos(); ImGui::Spacing(); ImGui::Separator(); ImGui::Spacing();

		if (quiz.userIndex == -1) { 
			ImGui::Text("請選擇你的答案:"); ImGui::Spacing();
            float buttonHeight = ImGui::GetTextLineHeightWithSpacing() * 2.0f; 
			for (size_t i = 0; i < quiz.options.size(); ++i) {
				if (ImGui::Button(quiz.options[i].c_str(), ImVec2(ImGui::GetContentRegionAvail().x, buttonHeight))) {
					quiz.userIndex = static_cast<int>(i);
					if (!quiz.v_score.empty() && i < quiz.v_score.size()) {
						npc.totalScore += quiz.v_score[i];
					}
				}
				ImGui::Spacing();
			}
		} else { 
			ImGui::TextColored(ImVec4(0.2f, 0.8f, 1.0f, 1.0f), "你的選擇:");
			ImGui::TextWrapped("%s", quiz.options[quiz.userIndex].c_str()); ImGui::Spacing();
			if (!quiz.feedback.empty() && static_cast<size_t>(quiz.userIndex) < quiz.feedback.size() && !quiz.feedback[quiz.userIndex].empty()) {
				ImGui::Separator(); ImGui::Spacing();
				ImGui::TextColored(ImVec4(0.9f, 0.9f, 0.9f, 1.0f), "回應:");
				ImGui::TextWrapped("%s", quiz.feedback[quiz.userIndex].c_str()); ImGui::Spacing();
			}
			ImGui::Separator(); ImGui::Spacing();
			ImGui::Text("當前哥布林指數 (目標：<25): %d", npc.totalScore); ImGui::Spacing();
            float buttonHeight = ImGui::GetTextLineHeightWithSpacing() * 2.0f;
			if (ImGui::Button("繼續", ImVec2(ImGui::GetContentRegionAvail().x, buttonHeight))) {
				if (quiz.question.find("選擇你的學習路線") != std::string::npos) {
					// This 'npc' is the teacher NPC.
					npc.inDialog = false; 
					npc.routeEnabled = false; // Teacher route is done.
                    std::cout << "[DialogSystem::renderQuiz] Teacher NPC '" << std::string(npc.go->name) << "' route disabled." << std::endl;
					
                    // The new character route will use the SAME GameObject as the teacher.
                    // This is based on your Application::setupDefaultScene_ where initA/B/C are called
                    // with "calli", "kiara", "gura" GameObjects, but then the character selection
                    // also calls initA/B/C, potentially with the "ame" (teacher) GameObject.
                    // For clarity, let's assume the intent is to re-use the GameObject that the current
                    // 'npc' (teacher) is using.
					std::shared_ptr<GameObject> characterGameObject = npc.go; 
                    std::cout << "[DialogSystem::renderQuiz] Character selection. Using GameObject: '" << std::string(characterGameObject->name) << "' for new route." << std::endl;

					switch (quiz.userIndex) {
						case 0: initA(characterGameObject); break; 
						case 1: initB(characterGameObject); break; 
						case 2: initC(characterGameObject); break; 
						default: initA(characterGameObject); break; 
					}
					ImGui::End(); 
					return; 
				}	
				bool isLastQuizInScript = true;
				for (size_t i = npc.scriptIndex + 1; i < npc.dialogs.size(); ++i) {
                    if (!npc.dialogs[i]) continue;
					if (npc.dialogs[i]->type == DialogType::QUIZ) { isLastQuizInScript = false; break; }
                    if (npc.dialogs[i]->type == DialogType::GOODEND || npc.dialogs[i]->type == DialogType::BADEND) break; 
				}

				if (isLastQuizInScript) { 
                    bool goodEndExists = false; size_t goodEndIdx = 0;
                    bool badEndExists = false;  size_t badEndIdx = 0;
                    for(size_t i = npc.scriptIndex + 1; i < npc.dialogs.size(); ++i) {
                        if (!npc.dialogs[i]) continue;
                        if (npc.dialogs[i]->type == DialogType::GOODEND) { goodEndExists = true; goodEndIdx = i; }
                        if (npc.dialogs[i]->type == DialogType::BADEND) { badEndExists = true; badEndIdx = i; }
                        if (goodEndExists && badEndExists) break; 
                    }
					bool shouldGetGoodEnding = (npc.totalScore <= 25); 
					
                    if (shouldGetGoodEnding && goodEndExists) npc.scriptIndex = goodEndIdx;
					else if (!shouldGetGoodEnding && badEndExists) npc.scriptIndex = badEndIdx;
                    else if (goodEndExists) npc.scriptIndex = goodEndIdx; 
                    else if (badEndExists) npc.scriptIndex = badEndIdx;  
                    else npc.scriptIndex++; 
					npc.lineIndex = 0;

                    if (npc.scriptIndex < npc.dialogs.size() && npc.dialogs[npc.scriptIndex] && npc.dialogs[npc.scriptIndex]->type == DialogType::QUIZ) {
                        static_cast<Quiz*>(npc.dialogs[npc.scriptIndex].get())->userIndex = -1;
                    } else if (npc.scriptIndex >= npc.dialogs.size()) {
                        npc.inDialog = false; 
                    }
				} else {
				    npc.scriptIndex++; npc.lineIndex = 0;
                    if (npc.scriptIndex < npc.dialogs.size() && npc.dialogs[npc.scriptIndex] && npc.dialogs[npc.scriptIndex]->type == DialogType::QUIZ) {
                        static_cast<Quiz*>(npc.dialogs[npc.scriptIndex].get())->userIndex = -1;
                    }
                }
			}
		}
		ImGui::End();
	}
}

void DialogSystem::renderEnding(Dialog const& ending, NPC& npc, bool isGoodEnding)
{
	ImVec2 windowSize(ImGui::GetIO().DisplaySize.x * 0.7f, ImGui::GetIO().DisplaySize.y * 0.6f);
    ImGui::SetNextWindowSize(windowSize, ImGuiCond_Always);
	ImGui::SetNextWindowPos(ImVec2(ImGui::GetIO().DisplaySize.x * 0.5f, ImGui::GetIO().DisplaySize.y * 0.5f), ImGuiCond_Always, ImVec2(0.5f, 0.5f));
	ImGuiWindowFlags flags = ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoTitleBar;
	std::string windowTitle = isGoodEnding ? "##GoodEndingWindow" : "##BadEndingWindow";

	if (ImGui::Begin(windowTitle.c_str(), nullptr, flags)) {
		ImVec4 titleColor = isGoodEnding ? ImVec4(0.2f, 1.0f, 0.2f, 1.0f) : ImVec4(1.0f, 0.2f, 0.2f, 1.0f);
		const char* titleText = isGoodEnding ? "✅ 攻略成功!" : "❌ 攻略失敗";
        ImVec2 textSize = ImGui::CalcTextSize(titleText);
        ImGui::SetCursorPosX((ImGui::GetWindowSize().x - textSize.x) * 0.5f);
		ImGui::TextColored(titleColor, "%s", titleText);
		ImGui::Separator(); ImGui::Spacing();
        ImGui::BeginChild("EndingContent", ImVec2(0, ImGui::GetContentRegionAvail().y - ImGui::GetTextLineHeightWithSpacing() * 2.5f), false, ImGuiWindowFlags_AlwaysVerticalScrollbar);
		for (auto const& line : ending.lines) {
			if (isNarrativeLineLocal(line)) { 
				ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.7f, 0.7f, 0.7f, 1.0f));
				ImGui::TextWrapped("%s", line.c_str());
				ImGui::PopStyleColor();
			} else {
				ImGui::TextWrapped("%s", line.c_str());
			}
			ImGui::Spacing();
		}
        ImGui::EndChild(); ImGui::Separator(); ImGui::Spacing();
        float buttonHeight = ImGui::GetTextLineHeightWithSpacing() * 1.8f;
		if (ImGui::Button("結束", ImVec2(ImGui::GetContentRegionAvail().x, buttonHeight))) {
			npc.inDialog = false; 
            npc.routeEnabled = false; 
		}
		ImGui::End();
	}
}

void DialogSystem::processInput(GLFWwindow* window)
{
	static bool e_key_pressed_last_frame = false;
	bool e_key_down_this_frame = (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS);

	if (e_key_down_this_frame && !e_key_pressed_last_frame) {
		NPC* activeNPC_ptr = nullptr; 
		for (auto& npc_iter : npcs_) { 
			if (npc_iter.inDialog) { 
                activeNPC_ptr = &npc_iter; 
                break; 
            }
		}
		if (activeNPC_ptr) {
            if (activeNPC_ptr->scriptIndex < activeNPC_ptr->dialogs.size()) {
                DialogBase* currentDialogBase = activeNPC_ptr->dialogs[activeNPC_ptr->scriptIndex].get();
                if (currentDialogBase && currentDialogBase->type != DialogType::QUIZ) { 
                    handleDialogProgress(*activeNPC_ptr);
                } 
            }
		} else {
			for (auto& npc_iter : npcs_) { 
				if (npc_iter.showIcon && npc_iter.routeEnabled) { 
					npc_iter.inDialog = true; 
                    npc_iter.scriptIndex = 0; 
                    npc_iter.lineIndex = 0;
                    npc_iter.totalScore = 0; 
                    if (!npc_iter.dialogs.empty() && npc_iter.dialogs[0] && npc_iter.dialogs[0]->type == DialogType::QUIZ) {
                         static_cast<Quiz*>(npc_iter.dialogs[0].get())->userIndex = -1; 
                    }
					break; 
				}
			}
		}
	}
	e_key_pressed_last_frame = e_key_down_this_frame;
}

void DialogSystem::handleDialogProgress(NPC& npc)
{
	if (npc.scriptIndex >= npc.dialogs.size() || !npc.dialogs[npc.scriptIndex]) {
        npc.inDialog = false; return;
    }
	DialogBase* currentDialogBase = npc.dialogs[npc.scriptIndex].get();
	if (currentDialogBase->type == DialogType::DIALOG) {
		Dialog* dialog = static_cast<Dialog*>(currentDialogBase);
		npc.lineIndex++;
		if (npc.lineIndex >= dialog->lines.size()) {
			npc.scriptIndex++; npc.lineIndex = 0;
            if (npc.scriptIndex < npc.dialogs.size() && npc.dialogs[npc.scriptIndex] && 
                npc.dialogs[npc.scriptIndex]->type == DialogType::QUIZ) {
                static_cast<Quiz*>(npc.dialogs[npc.scriptIndex].get())->userIndex = -1;
            } else if (npc.scriptIndex >= npc.dialogs.size()) {
                npc.inDialog = false; 
            }
		}
	} 
}