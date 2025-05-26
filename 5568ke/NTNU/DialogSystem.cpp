#include "DialogSystem.hpp"

#include <algorithm>
#include <iostream>
#include <limits>

#include <imgui.h>
#include <glm/gtc/matrix_transform.hpp>

#include "AnimationClip.hpp"
#include "GameObject.hpp"
#include "Model.hpp"
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

void DialogSystem::initializeNPCIdleAnimation(NPC& npc)
{
	if (!npc.go || !npc.go->getModel()) {
		return;
	}

	// 尋找idle動畫
	npc.idleAnimationIndex = findIdleAnimationIndex(npc.go);

	if (npc.idleAnimationIndex != -1) {
		// 開始播放idle動畫
		startIdleAnimation(npc);
		std::cout << "[DialogSystem] Initialized idle animation for NPC at index " << npc.idleAnimationIndex << std::endl;
	}
	else {
		std::cout << "[DialogSystem] No idle animation found for NPC" << std::endl;
	}
}

int DialogSystem::findIdleAnimationIndex(std::shared_ptr<GameObject> const& go)
{
	if (!go || !go->getModel()) {
		return -1;
	}

	auto const& animations = go->getModel()->animations;

	// 嘗試尋找包含"idle"關鍵字的動畫（不區分大小寫）
	for (size_t i = 0; i < animations.size(); ++i) {
		std::string clipName = animations[i]->clipName;
		std::transform(clipName.begin(), clipName.end(), clipName.begin(), ::tolower);

		if (clipName.find("idle") != std::string::npos) {
			return static_cast<int>(i);
		}
	}

	// 如果沒找到idle動畫，嘗試使用第一個動畫作為默認
	if (!animations.empty()) {
		std::cout << "[DialogSystem] No idle animation found, using first animation as default" << std::endl;
		return 0;
	}

	return -1;
}

void DialogSystem::startIdleAnimation(NPC& npc)
{
	if (npc.idleAnimationIndex == -1 || !npc.go || !npc.go->getModel()) {
		return;
	}

	auto const& animations = npc.go->getModel()->animations;
	if (static_cast<size_t>(npc.idleAnimationIndex) >= animations.size()) {
		return;
	}

	npc.isPlayingIdleAnimation = true;
	npc.idleAnimationTime = 0.0f;

	// 設置初始幀
	animations[npc.idleAnimationIndex]->setAnimationFrame(npc.go->getModel()->nodes, 0.0f);
	npc.go->getModel()->updateLocalMatrices();
}

void DialogSystem::updateNPCIdleAnimation(NPC& npc, float dt)
{
	// 如果沒有在對話中且沒有播放idle動畫，開始播放
	if (!npc.isPlayingIdleAnimation && npc.idleAnimationIndex != -1) {
		startIdleAnimation(npc);
		return;
	}

	// 更新idle動畫
	if (npc.isPlayingIdleAnimation && npc.idleAnimationIndex != -1 && npc.go && npc.go->getModel()) {
		auto const& animations = npc.go->getModel()->animations;
		if (static_cast<size_t>(npc.idleAnimationIndex) < animations.size()) {
			auto const& idleClip = animations[npc.idleAnimationIndex];

			// 更新動畫時間
			npc.idleAnimationTime += dt;

			// 循環播放
			float duration = idleClip->getDuration();
			if (duration > 0.0f && npc.idleAnimationTime > duration) {
				npc.idleAnimationTime = std::fmod(npc.idleAnimationTime, duration);
			}

			// 應用動畫幀
			idleClip->setAnimationFrame(npc.go->getModel()->nodes, npc.idleAnimationTime);
			npc.go->getModel()->updateLocalMatrices();
		}
	}
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

		// 更新NPC的idle動畫
		updateNPCIdleAnimation(npc, dt);

		if (npc.inDialog) {
			npc.showIcon = false;
			continue;
		}

		float distance = player->distanceTo(*npc.go);
		float const INTERACTION_RANGE = 0.8f;

		if (npc.routeEnabled && distance <= INTERACTION_RANGE) {
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
			glm::vec3 iconPos = npcPos + glm::vec3(0.0f, 1.5f, 0.0f);

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
		// 對話結束時，重新開始idle動畫
		startIdleAnimation(*activeNPC);
		return;
	}

	DialogBase* currentDialog = activeNPC->dialogs[activeNPC->scriptIndex].get();

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
	ImGui::SetNextWindowSize(ImVec2(800, 400), ImGuiCond_FirstUseEver);
	ImGui::SetNextWindowPos(ImVec2(ImGui::GetIO().DisplaySize.x * 0.5f, ImGui::GetIO().DisplaySize.y * 0.8f), ImGuiCond_Always, ImVec2(0.5f, 1.0f));

	ImGuiWindowFlags flags = ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoTitleBar;

	if (ImGui::Begin("##Dialog", nullptr, flags)) {
		// 創建滾動區域來顯示對話內容
		ImGui::BeginChild("DialogContent", ImVec2(0, -60), true, ImGuiWindowFlags_AlwaysVerticalScrollbar);

		// 顯示從第一行到當前行的所有對話
		for (size_t i = 0; i <= npc.lineIndex && i < dialog.lines.size(); ++i) {
			std::string const& line = dialog.lines[i];

			// 檢查是否是旁白
			if (isNarrativeLine(line)) {
				// 旁白樣式 - 深色背景，灰色文字
				ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.85f, 0.85f, 0.85f, 1.0f));
				ImGui::TextWrapped("%s", line.c_str());
				ImGui::PopStyleColor();
			}
			else {
				// 角色對話 - 分離說話者和內容
				size_t colonPos = line.find("：");
				if (colonPos == std::string::npos) {
					colonPos = line.find(":");
				}

				if (colonPos != std::string::npos) {
					std::string speaker = line.substr(0, colonPos);
					std::string content = line.substr(colonPos + (line[colonPos] == ':' ? 1 : 3));

					ImGui::TextColored(ImVec4(0.8f, 0.8f, 1.0f, 1.0f), "%s:", speaker.c_str());
					ImGui::SameLine();
					ImGui::TextWrapped("%s", content.c_str());
				}
				else {
					ImGui::TextWrapped("%s", line.c_str());
				}
			}

			// 如果這是當前最新顯示的行，添加一些間距
			if (i == npc.lineIndex) {
				ImGui::Spacing();
				// 自動滾動到最新的對話
				ImGui::SetScrollHereY(1.0f);
			}
			else {
				ImGui::Spacing();
			}
		}

		ImGui::EndChild();

		ImGui::Separator();

		// 顯示進度和提示
		if (npc.lineIndex < dialog.lines.size() - 1) {
			ImGui::Text("Press E to continue...");
		}
		else {
			ImGui::Text("Press E to finish...");
		}
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
			// 對話結束時，重新開始idle動畫
			startIdleAnimation(npc);
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

	DialogBase* currentDialog = npc.dialogs[npc.scriptIndex].get();

	if (currentDialog->type == DialogType::DIALOG) {
		Dialog* dialog = static_cast<Dialog*>(currentDialog);

		npc.lineIndex++;

		if (npc.lineIndex >= dialog->lines.size()) {
			npc.scriptIndex++;
			npc.lineIndex = 0;
		}
	}
}