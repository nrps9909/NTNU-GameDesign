#define GLM_ENABLE_EXPERIMENTAL

#include "Application.hpp"

#include <cmath>
#include <iostream>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "AnimationClip.hpp"
#include "Collider.hpp"
#include "CollisionSystem.hpp"
#include "DialogSystem.hpp"
#include "Model.hpp"

Application::Application() {}
Application::~Application() { cleanup_(); }

int Application::run()
{
	initWindow_();
	initGL_();
	initImGui_();
	setupDefaultScene_();
	loop_();
	cleanup_();
	return 0;
}

void Application::initWindow_()
{
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	// Get primary monitor and its video mode for fullscreen
	GLFWmonitor* primaryMonitor = glfwGetPrimaryMonitor();
	const GLFWvidmode* mode = glfwGetVideoMode(primaryMonitor);
	
	// Create fullscreen window
	window_ = glfwCreateWindow(mode->width, mode->height, "教室的割布麟", primaryMonitor, nullptr);
	glfwMakeContextCurrent(window_);
	glfwSwapInterval(1);

	glfwSetInputMode(window_, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
	glfwSetWindowUserPointer(window_, this);

	glfwSetKeyCallback(window_, keyCallback_);
	glfwSetCursorPosCallback(window_, mouseCallback_);
	glfwSetScrollCallback(window_, scrollCallback_);
}

void Application::initImGui_() { ImGuiManagerRef.init(window_); }

void Application::keyCallback_(GLFWwindow* window, int key, int scancode, int action, int mode)
{
	Application* app = static_cast<Application*>(glfwGetWindowUserPointer(window));
	if (!app) return;

	if (key >= 0 && key < 1024) {
		if (action == GLFW_PRESS)
			app->keys_[key] = true;
		else if (action == GLFW_RELEASE)
			app->keys_[key] = false;
	}

	// ESC to toggle main menu (only if not in main menu already)
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
		if (app->mainMenuRef.isVisible()) {
			// If main menu is visible, exit game
			glfwSetWindowShouldClose(window, GLFW_TRUE);
		} else {
			// Show main menu
			app->mainMenuRef.show();
			// Reset cursor mode when returning to menu
			glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
		}
	}

	if (key == GLFW_KEY_TAB && action == GLFW_PRESS) {
		// Only allow tab switching when not in main menu
		if (!app->mainMenuRef.isVisible()) {
			int cursorMode = glfwGetInputMode(window, GLFW_CURSOR);
			if (cursorMode == GLFW_CURSOR_NORMAL) {
				glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
				Camera& cam = app->sceneRef.cam;
				cam.firstMouse = true;
			}
			else {
				glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
			}
		}
	}

	if (key == GLFW_KEY_F4 && action == GLFW_PRESS) {
		app->showAnimationUI_ = !app->showAnimationUI_;
	}
}

void Application::mouseCallback_(GLFWwindow* window, double xpos, double ypos)
{
	Application* app = static_cast<Application*>(glfwGetWindowUserPointer(window));
	int cursorMode = glfwGetInputMode(window, GLFW_CURSOR);
	if (cursorMode == GLFW_CURSOR_DISABLED) {
		app->sceneRef.cam.processMouse(xpos, ypos);
	}
}

void Application::scrollCallback_(GLFWwindow* window, double xoffset, double yoffset)
{
	// Application* app = static_cast<Application*>(glfwGetWindowUserPointer(window));
	// app->sceneRef.cam.processMouseScroll(static_cast<float>(yoffset)); // Example
}

void Application::initGL_()
{
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cerr << "Failed to initialize GLAD" << std::endl;
        // Consider exiting or throwing an exception
    }
}

void Application::setupDefaultScene_()
{
	sceneRef.addLight(glm::vec3(1.0f, 7.0f, -4.0f), glm::vec3(1.0f), 2.0f);

	try {
		rendererRef.init();
		std::shared_ptr<GameObject> teacherGO = nullptr;

		{
			std::string const inaPath = "assets/models/smo_ina/scene.gltf";
			std::string const playerName = "Player";
			std::shared_ptr<Model> playerModel = registryRef.loadModel(inaPath, playerName);
			if (playerModel) {
				auto goPtr = registryRef.addModelToScene(sceneRef, playerModel);
				if (goPtr) {
					goPtr->position = {5.2f, 0.12f, -1.0f};
					goPtr->rotationDeg.y = 50;
					goPtr->updateTransformMatrix(); // IMPORTANT: Update transform after setting properties
					auto modelCol = std::make_shared<AABBCollider>(goPtr);
					collisionSysRef.add(modelCol);
					animStateRef.characterMoveMode = true;
				}
				animStateRef.gameObjectName = playerName;
				sceneRef.setupCameraToViewGameObject(playerName);
			}
		}

		{
			std::string const amePath = "assets/models/smo_ame/scene.gltf";
			std::string const ameName = "ame";
			std::shared_ptr<Model> ameModel = registryRef.loadModel(amePath, ameName);
			if (ameModel) {
				auto goPtr = registryRef.addModelToScene(sceneRef, ameModel);
				if (goPtr) {
					goPtr->position = {8.5f, 0.38f, 0.18f};
					goPtr->rotationDeg.y = -90;
					goPtr->updateTransformMatrix(); // IMPORTANT
					auto modelCol = std::make_shared<AABBCollider>(goPtr);
					collisionSysRef.add(modelCol);
					teacherGO = goPtr;
				}
			}
		}

		{
			std::string const calliPath = "assets/models/smo_calli/scene.gltf";
			std::string const calliName = "calli";
			std::shared_ptr<Model> calliModel = registryRef.loadModel(calliPath, calliName);
			if (calliModel) {
				auto goPtr = registryRef.addModelToScene(sceneRef, calliModel);
				if (goPtr) {
					goPtr->position = {6.369f, 0.12f, 2.834f};
					goPtr->scale = glm::vec3(0.35f);
					goPtr->rotationDeg.y = -161;
					goPtr->updateTransformMatrix(); // IMPORTANT
					auto modelCol = std::make_shared<AABBCollider>(goPtr);
					collisionSysRef.add(modelCol);
					initA(goPtr);
				}
			}
		}

		{
			std::string const kiaraPath = "assets/models/smo_kiara/scene.gltf";
			std::string const kiaraName = "kiara";
			std::shared_ptr<Model> kiaraModel = registryRef.loadModel(kiaraPath, kiaraName);
			if (kiaraModel) {
				auto goPtr = registryRef.addModelToScene(sceneRef, kiaraModel);
				if (goPtr) {
					goPtr->position = {7.38f, 0.12f, -1.538f};
					goPtr->rotationDeg.y = -42;
					goPtr->updateTransformMatrix(); // IMPORTANT
					auto modelCol = std::make_shared<AABBCollider>(goPtr);
					collisionSysRef.add(modelCol);
					initB(goPtr);
				}
			}
		}

		{
			std::string const guraPath = "assets/models/smo_gura/scene.gltf";
			std::string const guraName = "gura";
			std::shared_ptr<Model> guraModel = registryRef.loadModel(guraPath, guraName);
			if (guraModel) {
				auto goPtr = registryRef.addModelToScene(sceneRef, guraModel);
				if (goPtr) {
					goPtr->position = {7.744f, 0.12f, 2.284f};
					goPtr->scale = glm::vec3(0.35f);
					goPtr->rotationDeg.y = -141.503f;
					goPtr->updateTransformMatrix(); // IMPORTANT
					auto modelCol = std::make_shared<AABBCollider>(goPtr);
					collisionSysRef.add(modelCol);
					initC(goPtr);
				}
			}
		}

		{
			std::string const classRoomPath = "assets/models/japanese_classroom/scene.gltf";
			std::string const classRoomName = "classroom";
			std::shared_ptr<Model> classRoomModel = registryRef.loadModel(classRoomPath, classRoomName);
			if (classRoomModel) {
				auto goPtr = registryRef.addModelToScene(sceneRef, classRoomModel);
				if (goPtr) {
					goPtr->position = {8.4f, 0.0f, 7.0f};
					goPtr->scale = glm::vec3(2.6f);
					goPtr->updateTransformMatrix(); // IMPORTANT
				}
			}
		}
		if (teacherGO) {
			initBegin(teacherGO);
			std::cout << "[Application] Dialog system initialized with teacher and character routes" << std::endl;
		}

		// Add invisible walls around the classroom
		addInvisibleWalls_();

	} catch (std::runtime_error const& error) {
		std::cerr << "[Application::setupDefaultScene_] Exception: " << error.what() << std::endl;
	}
}

void Application::processInput_(float dt)
{
	int cursorMode = glfwGetInputMode(window_, GLFW_CURSOR);
	bool charMode = animStateRef.characterMoveMode;

	if (cursorMode == GLFW_CURSOR_DISABLED) {
		if (charMode && !animStateRef.gameObjectName.empty()) {
			auto goSharedPtr = sceneRef.findGameObject(animStateRef.gameObjectName);
			if (goSharedPtr) {
				GameObject& gameObject = *goSharedPtr;
				glm::vec3 worldForward = glm::normalize(glm::vec3(sceneRef.cam.front.x, 0.0f, sceneRef.cam.front.z));
				glm::vec3 worldRight = glm::normalize(glm::cross(worldForward, glm::vec3(0.0f, 1.0f, 0.0f)));
				glm::vec3 moveDirection(0.0f);
				float currentSpeed = animStateRef.camSpeed; // Assuming camSpeed is player speed

				if (glfwGetKey(window_, GLFW_KEY_W) == GLFW_PRESS) moveDirection += worldForward;
				if (glfwGetKey(window_, GLFW_KEY_S) == GLFW_PRESS) moveDirection -= worldForward;
				if (glfwGetKey(window_, GLFW_KEY_A) == GLFW_PRESS) moveDirection -= worldRight;
				if (glfwGetKey(window_, GLFW_KEY_D) == GLFW_PRESS) moveDirection += worldRight;

				bool isMoving = glm::length(moveDirection) > 0.01f; // Use a small threshold

				if (isMoving) {
					moveDirection = glm::normalize(moveDirection) * currentSpeed * dt;
					gameObject.position += moveDirection;
					if (glm::length(glm::vec2(moveDirection.x, moveDirection.z)) > 0.01f) {
						gameObject.rotationDeg.y = glm::degrees(atan2(moveDirection.x, moveDirection.z));
					}
					gameObject.updateTransformMatrix();
				}

				// Animation state handling
				if (gameObject.hasModel() && !gameObject.getModel()->animations.empty()) {
                    int idleAnimIndex = dialogSysRef.findIdleAnimationIndex(goSharedPtr); // Or a predefined idle index
                    int walkAnimIndex = 1; // Assuming 1 is a walk/move animation, adjust as needed
                    if (static_cast<size_t>(walkAnimIndex) >= gameObject.getModel()->animations.size() || !gameObject.getModel()->animations[walkAnimIndex]) {
                        walkAnimIndex = 0; // Fallback to a safe animation
                    }
                     if (idleAnimIndex == -1 || static_cast<size_t>(idleAnimIndex) >= gameObject.getModel()->animations.size() || !gameObject.getModel()->animations[idleAnimIndex]) {
                        idleAnimIndex = 0; // Fallback
                    }


					if (isMoving && !animStateRef.wasMoving) { // Started moving
						animStateRef.clipIndex = walkAnimIndex;
						animStateRef.play(animStateRef.clipIndex, 0.0f);
					} else if (!isMoving && animStateRef.wasMoving) { // Stopped moving
						animStateRef.stop();
                        animStateRef.clipIndex = idleAnimIndex; // Switch to idle animation clip
                        // Reset idle animation to its start if you want it to always restart
                        // This might conflict with DialogSystem's idle animation handling if not careful
                        if(gameObject.getModel()->animations[animStateRef.clipIndex]) {
						    gameObject.getModel()->animations[animStateRef.clipIndex]->setAnimationFrame(gameObject.getModel()->nodes, 0.0f);
						    gameObject.getModel()->updateLocalMatrices();
                        }
					}
				}
				animStateRef.wasMoving = isMoving;
			}
		} else { // Camera free movement
			sceneRef.cam.processKeyboard(dt, window_);
		}
	}

	// Update player animation if moving and animation is playing
	if (animStateRef.isAnimating && animStateRef.wasMoving && charMode && !animStateRef.gameObjectName.empty()) {
		auto goSharedPtr = sceneRef.findGameObject(animStateRef.gameObjectName);
		if (goSharedPtr && goSharedPtr->hasModel() && !goSharedPtr->getModel()->animations.empty()) {
			GameObject& gameObject = *goSharedPtr;
			int currentClipIdx = animStateRef.clipIndex;

			if (currentClipIdx >= 0 && static_cast<size_t>(currentClipIdx) < gameObject.getModel()->animations.size() && gameObject.getModel()->animations[currentClipIdx]) {
				auto& animClip = gameObject.getModel()->animations[currentClipIdx];
				animStateRef.currentTime += dt * animStateRef.getAnimateSpeed();
				float duration = animClip->getDuration();
				if (duration > 0.0f) {
					animStateRef.currentTime = std::fmod(animStateRef.currentTime, duration);
				} else {
					animStateRef.currentTime = 0.0f;
				}
				animClip->setAnimationFrame(gameObject.getModel()->nodes, animStateRef.currentTime);
				gameObject.getModel()->updateLocalMatrices(); // Ensure model matrices are updated after animation
			} else {
                 // std::cerr << "Player animation: Invalid clip index " << currentClipIdx << std::endl;
                 animStateRef.stop();
            }
		}
	}
}


void Application::tick_(float dt)
{
	processInput_(dt); // Handles player movement, animation state, and camera free-move

	dialogSysRef.update(sceneRef, dt); // Handles NPC logic, idle animations, interaction checks
	dialogSysRef.processInput(window_); // Handles player input for dialog progression

	// Other game logic updates can go here
	// For example, physics updates for all dynamic objects, AI updates not handled by DialogSystem etc.

	collisionSysRef.update(); // Handles collision detection and resolution

	// Update camera follow if in character mode
	if (animStateRef.characterMoveMode && !animStateRef.gameObjectName.empty()) {
		auto playerGO = sceneRef.findGameObject(animStateRef.gameObjectName);
		if (playerGO) {
			sceneRef.cam.updateFollow(playerGO->position, animStateRef.followDistance, animStateRef.followHeight);
		}
	}
	sceneRef.cam.updateMatrices(window_); // Update view/projection matrices
}

void Application::render_()
{
	int w, h;
	glfwGetFramebufferSize(window_, &w, &h);
	if (w == 0 || h == 0) return; // Avoid division by zero if window is minimized

	rendererRef.beginFrame(w, h, {0.1f, 0.11f, 0.13f});
	rendererRef.drawScene(sceneRef);
	rendererRef.endFrame();

	ImGuiManagerRef.newFrame();
	// ImGuiManagerRef.drawSceneGameObjectManager(sceneRef); // Optional UI
	// ImGuiManagerRef.drawAnimationControlPanel(sceneRef); // Optional UI
	// ImGuiManagerRef.drawStatusWindow(sceneRef); // Optional UI
	// ImGuiManagerRef.drawSceneControlWindow(sceneRef); // Optional UI
	dialogSysRef.render(sceneRef); // Dialog UI
	ImGuiManagerRef.render();

	glfwSwapBuffers(window_);
}

void Application::loop_()
{
	prevTime_ = glfwGetTime();
	
	// Ensure main menu is shown at start
	mainMenuRef.show();
	
	while (!glfwWindowShouldClose(window_)) {
		double now = glfwGetTime();
		float dt = static_cast<float>(now - prevTime_);
		prevTime_ = now;

		if (dt <= 0.0f) dt = 0.00001f; // Ensure dt is positive and non-zero
		if (dt > 0.1f) dt = 0.1f;     // Clamp dt to prevent instability from large frame drops

		glfwPollEvents(); // Poll events first
		
		// Handle main menu
		if (mainMenuRef.isVisible()) {
			mainMenuRef.processInput(window_);
			
			// Only render main menu, skip game logic
			int w, h;
			glfwGetFramebufferSize(window_, &w, &h);
			if (w > 0 && h > 0) {
				rendererRef.beginFrame(w, h, {0.05f, 0.05f, 0.1f}); // Darker background for menu
				rendererRef.endFrame();
				
				ImGuiManagerRef.newFrame();
				mainMenuRef.render();
				ImGuiManagerRef.render();
				
				glfwSwapBuffers(window_);
			}
			
			// Check if user wants to exit or start game
			if (mainMenuRef.shouldExit()) {
				glfwSetWindowShouldClose(window_, GLFW_TRUE);
			} else if (mainMenuRef.shouldStartGame()) {
				mainMenuRef.resetStartGame();
				// Game starts, main menu is already hidden
				// Set cursor to disabled for game play
				glfwSetInputMode(window_, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
				sceneRef.cam.firstMouse = true;
			}
		} else {
			// Normal game loop
			tick_(dt);        // Update game state
			render_();        // Render the scene and UI
		}
	}
}

void Application::cleanup_()
{
	ImGuiManagerRef.cleanup();
	sceneRef.cleanup();
	rendererRef.cleanup();
	if (window_) {
		glfwDestroyWindow(window_);
		window_ = nullptr;
	}
	glfwTerminate();
}

void Application::addInvisibleWalls_()
{	// Classroom is positioned at {8.4f, 0.0f, 7.0f} with scale 2.6f
	// Need to match the actual classroom model dimensions more precisely
	glm::vec3 classroomCenter = {8.4f, 0.0f, 7.0f};
	float classroomSize = 16.0f; // Better match for scaled classroom (original * 2.6 scale factor)
	float wallThickness = 0.2f;  // Thinner walls for more precise boundaries
	float wallHeight = 8.0f;     // Taller for better coverage
	
	// Create invisible wall GameObjects with static physics properties
	auto createWall = [&](const std::string& name, glm::vec3 pos, glm::vec3 scale) {
		auto wallGO = std::make_shared<GameObject>();
		wallGO->name = name;
		wallGO->position = pos;
		wallGO->scale = scale;
		wallGO->visible = false;  // Make invisible
		wallGO->invMass = 0.0f;   // Static object (infinite mass)
		wallGO->restitution = 0.1f; // Low bounce
		wallGO->updateTransformMatrix();
				// Add to scene
		sceneRef.gameObjects.push_back(wallGO);
		
		// Create and add collider
		auto wallCollider = std::make_shared<AABBCollider>(wallGO);
		collisionSysRef.add(wallCollider);
		
		std::cout << "[Application] Added invisible wall: " << name << " at position (" 
		          << pos.x << ", " << pos.y << ", " << pos.z << ")" << std::endl;
	};
	
	// Create walls around the classroom
	// North wall (positive Z)
	createWall("wall_north", 
		{classroomCenter.x, wallHeight * 0.5f, classroomCenter.z + classroomSize * 0.5f + wallThickness * 0.5f},
		{classroomSize + wallThickness * 2.0f, wallHeight, wallThickness});
	
	// South wall (negative Z) 
	createWall("wall_south",
		{classroomCenter.x, wallHeight * 0.5f, classroomCenter.z - classroomSize * 0.5f - wallThickness * 0.5f},
		{classroomSize + wallThickness * 2.0f, wallHeight, wallThickness});
	
	// East wall (positive X)
	createWall("wall_east",
		{classroomCenter.x + classroomSize * 0.5f + wallThickness * 0.5f, wallHeight * 0.5f, classroomCenter.z},
		{wallThickness, wallHeight, classroomSize});
	
	// West wall (negative X)
	createWall("wall_west",
		{classroomCenter.x - classroomSize * 0.5f - wallThickness * 0.5f, wallHeight * 0.5f, classroomCenter.z},
		{wallThickness, wallHeight, classroomSize});
	
	std::cout << "[Application] Added 4 invisible walls around classroom" << std::endl;
}