#pragma once

#include <memory>
#include <string>
#include <vector>

#include <glm/glm.hpp>

#include "GameObject.hpp"
#include "Scene.hpp"

class DialogSystem {
public:
	struct NPC {
		std::shared_ptr<GameObject> go;
		std::vector<std::string> lines;
		std::vector<std::string> options;
		bool showIcon{false};
		bool showDialog{false};
	};

	static DialogSystem& getInstance();

	void addNPC(std::shared_ptr<GameObject> go, std::vector<std::string> lines, std::vector<std::string> options = {});

	void update(Scene& scene, float dt);
	void render(Scene const& scene);

private:
	DialogSystem() = default;

	static glm::vec2 worldToScreen(glm::vec3 const& pos, Scene const& scene, int viewportW, int viewportH);

	std::vector<NPC> npcs_;
};