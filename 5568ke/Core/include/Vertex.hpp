#pragma once

#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>
#include <array>

/**
 * @brief Represents a single vertex in 3D space, with attributes used in shading and animation.
 */
struct Vertex {
	glm::vec3 position; // 3D position
	glm::vec3 normal;		// Normal vector for lighting
	glm::vec2 texcoord; // Texture coordinate (UV)

	// Skinning data
	std::array<int, 4> boneIds{}; // Indices of bones influencing the vertex
	glm::vec4 boneWeights{};			// Weights for each influencing bone
};
