#include "DialogSystem.hpp"

#include <imgui.h>
#include <glm/gtc/matrix_transform.hpp>

DialogSystem& DialogSystem::getInstance()
{
	static DialogSystem instance;
	return instance;
}

void DialogSystem::addNPC(std::shared_ptr<GameObject> go, std::vector<std::string> lines, std::vector<std::string> options)
{
	if (!go)
		return;
	npcs_.push_back(NPC{go, std::move(lines), std::move(options), true, true});
}

void DialogSystem::update(Scene& scene, float /*dt*/)
{
	glm::vec3 camPos = scene.cam.pos;
	for (auto& npc : npcs_) {
		if (!npc.go)
			continue;
		float dist = glm::distance(npc.go->getWorldPosition(), camPos);
		npc.showIcon = dist < 3.0f;
	}
}

glm::vec2 DialogSystem::worldToScreen(glm::vec3 const& pos, Scene const& scene, int w, int h)
{
	glm::vec4 clip = scene.cam.proj * scene.cam.view * glm::vec4(pos, 1.0f);
	if (clip.w <= 0.0f)
		return {-1000.0f, -1000.0f};
	glm::vec3 ndc = glm::vec3(clip) / clip.w;
	return {(ndc.x * 0.5f + 0.5f) * w, (1.0f - (ndc.y * 0.5f + 0.5f)) * h};
}

void DialogSystem::render(Scene const& scene)
{
	ImDrawList* dl = ImGui::GetBackgroundDrawList();
	ImVec2 viewport = ImGui::GetIO().DisplaySize;
	for (auto& npc : npcs_) {
		if (!npc.go)
			continue;
		glm::vec3 worldPos = npc.go->getWorldPosition() + glm::vec3(0.0f, 2.0f, 0.0f);
		glm::vec2 screen = worldToScreen(worldPos, scene, (int)viewport.x, (int)viewport.y);
		if (npc.showIcon) {
			dl->AddCircleFilled(ImVec2(screen.x, screen.y), 10.0f, IM_COL32(255, 255, 0, 255));
			dl->AddText(ImVec2(screen.x - 4.0f, screen.y - 8.0f), IM_COL32(0, 0, 0, 255), "E");
			if (ImGui::IsKeyPressed(ImGuiKey_E)) {
				npc.showDialog = true;
			}
		}
		if (npc.showDialog) {
			ImGui::SetNextWindowPos(ImVec2(viewport.x * 0.5f - 150.0f, viewport.y * 0.7f), ImGuiCond_Always);
			ImGui::SetNextWindowSize(ImVec2(300, 0), ImGuiCond_Always);
			if (ImGui::Begin("Dialog", &npc.showDialog, ImGuiWindowFlags_NoCollapse)) {
				for (auto const& line : npc.lines)
					ImGui::TextWrapped("%s", line.c_str());
				if (!npc.options.empty()) {
					for (auto const& opt : npc.options) {
						if (ImGui::Button(opt.c_str())) {
							npc.showDialog = false;
						}
					}
				}
				else {
					if (ImGui::Button("Close"))
						npc.showDialog = false;
				}
			}
			ImGui::End();
		}
	}
}
