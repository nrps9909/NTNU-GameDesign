#include "DialogSystem.hpp"

#include <algorithm>
#include <iostream>
#include <limits>

#include <imgui.h>
#include <glm/gtc/matrix_transform.hpp>

#include "GameObject.hpp"
#include "Scene.hpp"

DialogSystem& DialogSystem::getInstance()
{
	static DialogSystem instance;
	return instance;
}

glm::vec2 DialogSystem::worldToScreen(glm::vec3 const& pos, Scene const& scene, int viewportW, int viewportH)
{
	glm::mat4 mvp = scene.cam.proj * scene.cam.view;
	glm::vec4 clipPos = mvp * glm::vec4(pos, 1.0f);

	if (clipPos.w <= 0.0f) {
		return glm::vec2(-1, -1);
	}

	glm::vec3 ndc = glm::vec3(clipPos) / clipPos.w;
	float screenX = (ndc.x + 1.0f) * 0.5f * viewportW;
	float screenY = (1.0f - ndc.y) * 0.5f * viewportH;

	return glm::vec2(screenX, screenY);
}

void DialogSystem::update(Scene& scene, float dt)
{
	std::shared_ptr<GameObject> player = nullptr;
	if (!scene.gameObjects.empty()) {
		player = scene.gameObjects[0];
	}

	if (!player)
		return;

	for (auto& npc : npcs_) {
		if (!npc.go || !npc.go->visible) {
			npc.showIcon = false;
			continue;
		}

		if (npc.inDialog) {
			npc.showIcon = false;
			continue;
		}

		float distance = player->distanceTo(*npc.go);
		float const INTERACTION_RANGE = 3.0f;

		if (distance <= INTERACTION_RANGE) {
			npc.showIcon = true;
		}
		else {
			npc.showIcon = false;
		}
	}
}

void DialogSystem::render(Scene const& scene)
{
	ImVec2 displaySize = ImGui::GetIO().DisplaySize;
	int viewportW = static_cast<int>(displaySize.x);
	int viewportH = static_cast<int>(displaySize.y);

	// 渲染互動圖標
	for (auto const& npc : npcs_) {
		if (npc.showIcon && npc.go) {
			glm::vec3 npcPos = npc.go->getWorldPosition();
			glm::vec3 iconPos = npcPos + glm::vec3(0.0f, 2.0f, 0.0f);

			glm::vec2 screenPos = worldToScreen(iconPos, scene, viewportW, viewportH);

			if (screenPos.x >= 0 && screenPos.x < viewportW && screenPos.y >= 0 && screenPos.y < viewportH) {

				ImDrawList* drawList = ImGui::GetBackgroundDrawList();
				ImVec2 iconCenter(screenPos.x, screenPos.y);
				float iconSize = 20.0f;

				// 繪製感嘆號圖標
				drawList->AddCircleFilled(iconCenter, iconSize, IM_COL32(255, 255, 0, 200));
				drawList->AddCircle(iconCenter, iconSize, IM_COL32(0, 0, 0, 255), 12, 2.0f);

				ImVec2 exclamationTop(iconCenter.x, iconCenter.y - 8);
				ImVec2 exclamationBottom(iconCenter.x, iconCenter.y + 2);
				drawList->AddLine(exclamationTop, exclamationBottom, IM_COL32(0, 0, 0, 255), 3.0f);

				ImVec2 dotPos(iconCenter.x, iconCenter.y + 6);
				drawList->AddCircleFilled(dotPos, 2.0f, IM_COL32(0, 0, 0, 255));

				ImVec2 textPos(iconCenter.x - 60, iconCenter.y + iconSize + 10);
				drawList->AddText(textPos, IM_COL32(255, 255, 255, 255), "Press E to interact");
			}
		}
	}

	renderDialogUI();
}

void DialogSystem::renderDialogUI()
{
	NPC* activeNPC = nullptr;
	for (auto& npc : npcs_) {
		if (npc.inDialog) {
			activeNPC = &npc;
			break;
		}
	}

	if (!activeNPC)
		return;

	if (activeNPC->scriptIndex >= activeNPC->dialogs.size()) {
		activeNPC->inDialog = false;
		activeNPC->scriptIndex = 0;
		activeNPC->lineIndex = 0;
		dialogChoice = DialogChoice::non;
		return;
	}

	DialogBase* currentDialog = activeNPC->dialogs[activeNPC->scriptIndex];

	if (currentDialog->type == DialogType::DIALOG) {
		renderDialog(*static_cast<Dialog*>(currentDialog), *activeNPC);
	}
	else if (currentDialog->type == DialogType::QUIZ) {
		renderQuiz(*static_cast<Quiz*>(currentDialog), *activeNPC);
	}
	else if (currentDialog->type == DialogType::GOODEND) {
		renderEnding(*static_cast<Dialog*>(currentDialog), *activeNPC, true);
	}
	else if (currentDialog->type == DialogType::BADEND) {
		renderEnding(*static_cast<Dialog*>(currentDialog), *activeNPC, false);
	}
}

// 檢查是否為旁白的輔助函數
bool isNarrativeLine(std::string const& line)
{
	// 檢查中文括號（3字節UTF-8）
	if (line.length() >= 3 && line.substr(0, 3) == "（") {
		return true;
	}
	// 檢查英文括號
	if (!line.empty() && line[0] == '(') {
		return true;
	}
	// 檢查【】標記
	if (line.length() >= 3 && line.substr(0, 3) == "【") {
		return true;
	}
	return false;
}

void DialogSystem::renderDialog(Dialog const& dialog, NPC& npc)
{
	ImGui::SetNextWindowSize(ImVec2(800, 200), ImGuiCond_FirstUseEver);
	ImGui::SetNextWindowPos(ImVec2(ImGui::GetIO().DisplaySize.x * 0.5f, ImGui::GetIO().DisplaySize.y * 0.8f), ImGuiCond_Always, ImVec2(0.5f, 1.0f));

	ImGuiWindowFlags flags = ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoTitleBar;

	if (ImGui::Begin("##Dialog", nullptr, flags)) {
		if (npc.lineIndex < dialog.lines.size()) {
			std::string currentLine = dialog.lines[npc.lineIndex];

			// 檢查是否是旁白
			if (isNarrativeLine(currentLine)) {
				// 旁白樣式 - 深色背景，灰色文字
				ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.85f, 0.85f, 0.85f, 1.0f));
				ImGui::TextWrapped("%s", currentLine.c_str());
				ImGui::PopStyleColor();
			}
			else {
				// 角色對話 - 分離說話者和內容
				size_t colonPos = currentLine.find("：");
				if (colonPos == std::string::npos) {
					colonPos = currentLine.find(":");
				}

				if (colonPos != std::string::npos) {
					std::string speaker = currentLine.substr(0, colonPos);
					std::string content = currentLine.substr(colonPos + (currentLine[colonPos] == ':' ? 1 : 3));

					ImGui::TextColored(ImVec4(0.8f, 0.8f, 1.0f, 1.0f), "%s:", speaker.c_str());
					ImGui::SameLine();
					ImGui::TextWrapped("%s", content.c_str());
				}
				else {
					ImGui::TextWrapped("%s", currentLine.c_str());
				}
			}
		}

		ImGui::Separator();
		ImGui::Text("Press E to continue...");
		ImGui::Text("(%zu/%zu)", npc.lineIndex + 1, dialog.lines.size());
	}
	ImGui::End();
}

void DialogSystem::renderQuiz(Quiz const& quiz, NPC& npc)
{
	ImGui::SetNextWindowSize(ImVec2(900, 400), ImGuiCond_FirstUseEver);
	ImGui::SetNextWindowPos(ImVec2(ImGui::GetIO().DisplaySize.x * 0.5f, ImGui::GetIO().DisplaySize.y * 0.5f), ImGuiCond_Always, ImVec2(0.5f, 0.5f));

	ImGuiWindowFlags flags = ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;

	if (ImGui::Begin("哥布林素質檢測", nullptr, flags)) {
		ImGui::TextColored(ImVec4(1.0f, 0.8f, 0.2f, 1.0f), "問題:");
		ImGui::TextWrapped("%s", quiz.question.c_str());
		ImGui::Spacing();

		if (quiz.userIndex == -1) {
			ImGui::Text("請選擇你的答案:");
			ImGui::Spacing();

			for (size_t i = 0; i < quiz.options.size(); ++i) {
				if (ImGui::Button(quiz.options[i].c_str(), ImVec2(-1, 0))) {
					const_cast<Quiz&>(quiz).userIndex = static_cast<int>(i);
				}
				ImGui::Spacing();
			}
		}
		else {
			ImGui::TextColored(ImVec4(0.2f, 1.0f, 0.2f, 1.0f), "你的選擇:");
			ImGui::TextWrapped("%s", quiz.options[quiz.userIndex].c_str());

			if (!quiz.feedback.empty() && static_cast<size_t>(quiz.userIndex) < quiz.feedback.size()) {
				ImGui::Spacing();
				ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.2f, 1.0f), "反饋:");
				ImGui::TextWrapped("%s", quiz.feedback[quiz.userIndex].c_str());
			}

			ImGui::Spacing();
			if (ImGui::Button("繼續", ImVec2(-1, 30))) {
				npc.scriptIndex++;
				npc.lineIndex = 0;
			}
		}
	}
	ImGui::End();
}

void DialogSystem::renderEnding(Dialog const& ending, NPC& npc, bool isGoodEnding)
{
	ImGui::SetNextWindowSize(ImVec2(800, 400), ImGuiCond_FirstUseEver);
	ImGui::SetNextWindowPos(ImVec2(ImGui::GetIO().DisplaySize.x * 0.5f, ImGui::GetIO().DisplaySize.y * 0.5f), ImGuiCond_Always, ImVec2(0.5f, 0.5f));

	ImGuiWindowFlags flags = ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;

	if (ImGui::Begin(isGoodEnding ? "Good Ending" : "Bad Ending", nullptr, flags)) {
		ImVec4 titleColor = isGoodEnding ? ImVec4(0.2f, 1.0f, 0.2f, 1.0f) : ImVec4(1.0f, 0.2f, 0.2f, 1.0f);

		ImGui::TextColored(titleColor, isGoodEnding ? "✅ 攻略成功!" : "❌ 攻略失敗");
		ImGui::Separator();

		for (auto const& line : ending.lines) {
			if (isNarrativeLine(line)) {
				ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.8f, 0.8f, 0.8f, 1.0f));
				ImGui::TextWrapped("%s", line.c_str());
				ImGui::PopStyleColor();
			}
			else {
				ImGui::TextWrapped("%s", line.c_str());
			}
			ImGui::Spacing();
		}

		if (ImGui::Button("關閉", ImVec2(-1, 30))) {
			npc.inDialog = false;
			npc.scriptIndex = 0;
			npc.lineIndex = 0;
		}
	}
	ImGui::End();
}

void DialogSystem::processInput(GLFWwindow* window)
{
	static bool eKeyPressed = false;
	bool eKeyDown = glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS;

	if (eKeyDown && !eKeyPressed) {
		eKeyPressed = true;

		NPC* activeNPC = nullptr;
		for (auto& npc : npcs_) {
			if (npc.inDialog) {
				activeNPC = &npc;
				break;
			}
		}

		if (activeNPC) {
			handleDialogProgress(*activeNPC);
		}
		else {
			for (auto& npc : npcs_) {
				if (npc.showIcon) {
					npc.inDialog = true;
					npc.scriptIndex = 0;
					npc.lineIndex = 0;
					break;
				}
			}
		}
	}

	if (!eKeyDown) {
		eKeyPressed = false;
	}
}

void DialogSystem::handleDialogProgress(NPC& npc)
{
	if (npc.scriptIndex >= npc.dialogs.size())
		return;

	DialogBase* currentDialog = npc.dialogs[npc.scriptIndex];

	if (currentDialog->type == DialogType::DIALOG) {
		Dialog* dialog = static_cast<Dialog*>(currentDialog);

		npc.lineIndex++;

		if (npc.lineIndex >= dialog->lines.size()) {
			npc.scriptIndex++;
			npc.lineIndex = 0;
		}
	}
}