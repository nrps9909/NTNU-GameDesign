#pragma once

#include <vector>

#include "Primitive.hpp"
#include "Vertex.hpp"

class Shader;

class Mesh {
public:
	/**
	 * @brief 'vertices' is the original vertex data of the Mesh.
	 * The entire buffer is uploaded to GPU via 'vbo_', and drawing units (e.g., primitives)
	 * reference into this data pool using indices.
	 *
	 * Each Vertex includes:
	 * - position: 3D coordinates
	 * - normal: normal vector used in lighting calculations
	 * - texcoord: 2D UV texture coordinate
	 * - boneIds: indices of bones affecting this vertex (up to 4)
	 * - boneWeights: influence weights corresponding to each bone
	 */
	std::vector<Vertex> vertices;

	/**
	 * @brief 'indices' define how to assemble triangles using vertices.
	 * This is the index buffer (EBO) and will be used with glDrawElements.
	 *
	 * For example, every group of 3 indices forms a triangle.
	 */
	std::vector<unsigned int> indices;

	/**
	 * @brief 'primitives' represent draw call units within the mesh.
	 * Each primitive defines:
	 * - a range in the index buffer to draw (indexOffset + indexCount)
	 * - an associated material to bind before rendering
	 *
	 * This enables a single mesh to contain multiple material regions.
	 */
	std::vector<Primitive> primitives;

	/**
	 * @brief Initializes OpenGL buffers (VAO, VBO, EBO) and uploads vertex/index data.
	 * Also configures vertex attribute pointers.
	 */
	void setup();

	/**
	 * @brief Renders the mesh using the given shader.
	 * Iterates through all primitives, binds their materials, and issues draw calls.
	 *
	 * @param shader The shader program to use for rendering.
	 */
	void draw(Shader const& shader) const;

private:
	// OpenGL object handles
	unsigned int vao_{}; // Vertex Array Object
	unsigned int vbo_{}; // Vertex Buffer Object (vertex data)
	unsigned int ebo_{}; // Element Buffer Object (index data)
};
