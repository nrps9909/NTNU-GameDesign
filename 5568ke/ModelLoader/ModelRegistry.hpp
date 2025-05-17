#pragma once

#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

#include <glm/glm.hpp>

// Forward declarations
class Model;
class Scene;
class GltfLoader;

// Enum for supported model formats
enum class ModelFormat {
	UNSUPPORTED,
	GLTF,
	OBJ,
	FBX,
	AUTO_DETECT // Automatically detect format from file extension
};

// Central registry for managing all model loaders and models in the application
class ModelRegistry {
public:
	static ModelRegistry& getInstance();

	// Load a model with optional position parameters
	std::shared_ptr<Model> loadModel(std::string const& path, std::string const& name = "", glm::vec3 position = glm::vec3(0.0f),
																	 glm::vec3 rotation = glm::vec3(0.0f), float scale = 1.0f);

	// Get a previously loaded model by name
	std::shared_ptr<Model> getModel(std::string const& name);

	// Unload a model by name
	bool unloadModel(std::string const& name);

	// Add a model to a scene with a transform matrix
	void addModelToScene(Scene& scene, std::shared_ptr<Model> model, std::string const& name, glm::mat4 transform = glm::mat4(1.0f));

	// Add a model with automatic centering at a specified position
	void addModelToSceneCentered(Scene& scene, std::shared_ptr<Model> model, std::string const& name, glm::vec3 position = glm::vec3(0.0f),
															 glm::vec3 rotation = glm::vec3(0.0f), float scale = 1.0f);

	// Remove a model from a scene
	void removeModelFromScene(Scene& scene, std::string const& name);

	// Get a list of all registered model names (for UI)
	std::vector<std::string> const& getRegisteredModels() const;

	// Clean up all models
	void cleanup();

private:
	ModelRegistry();
	~ModelRegistry();

	// Detect format from file extension
	ModelFormat detectFormat_(std::string const& path);

	// Cache of loaded models
	std::unordered_map<std::string, std::shared_ptr<Model>> modelCache_;

	// List of registered model names (for UI reference)
	std::vector<std::string> registeredModels_;

	// Concrete loader instances
	std::unique_ptr<GltfLoader> gltfLoader_;
};
