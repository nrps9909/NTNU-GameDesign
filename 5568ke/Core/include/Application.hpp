#pragma once

#include <array>
#include <string>
#include <unordered_map>

#include "CollisionSystem.hpp"
#include "DialogSystem.hpp"
#include "GlobalAnimationState.hpp"
#include "ImGuiManager.hpp"
#include "MainMenu.hpp"
#include "ModelRegistry.hpp"
#include "Renderer.hpp"
#include "Scene.hpp"
#include "include_5568ke.hpp"

class Application {
public:
	Application();
	~Application();
	int run();

	// Singleton references
	Scene& sceneRef = Scene::getInstance();
	ImGuiManager& ImGuiManagerRef = ImGuiManager::getInstance();
	MainMenu& mainMenuRef = MainMenu::getInstance();
	ModelRegistry& registryRef = ModelRegistry::getInstance();
	Renderer& rendererRef = Renderer::getInstance();
	GlobalAnimationState& animStateRef = GlobalAnimationState::getInstance();
	CollisionSystem& collisionSysRef = CollisionSystem::getInstance();
	DialogSystem& dialogSysRef = DialogSystem::getInstance();

private:
	// Initialization methods
	void initWindow_();
	void initGL_();
	void initImGui_();
	// Scene setup methods
	void setupDefaultScene_();
	void addInvisibleWalls_();

	// Main loop methods
	void loop_();
	void processInput_(float dt);

	void tick_(float dt);
	void render_();

	// Cleanup
	void cleanup_();

	// Callbacks
	static void keyCallback_(GLFWwindow* window, int key, int scancode, int action, int mode);
	static void mouseCallback_(GLFWwindow* window, double xpos, double ypos);
	static void scrollCallback_(GLFWwindow* window, double xoffset, double yoffset);

private:
	GLFWwindow* window_{nullptr};
	double prevTime_{0.0};

	// ImGui management
	bool showSceneManager_{true};
	bool showAnimationUI_{true};
	bool showStatsWindow_{true};
	bool showSceneControlsWindow_{true};

	// Key state tracking
	std::array<bool, 1024> keys_{};
};
