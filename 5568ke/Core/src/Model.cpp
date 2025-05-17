#define GLM_ENABLE_EXPERIMENTAL

#include "Model.hpp"

#include <iostream>

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "AnimationClip.hpp"
#include "Mesh.hpp"
#include "Node.hpp"
#include "Shader.hpp"

Model::~Model() { cleanup(); }

void Model::draw(Shader const& shader, glm::mat4 const& modelMatrix) const
{
	// Set the model matrix
	shader.sendMat4("model", modelMatrix);

	// For skinned models, send joint matrices to shader
	if (!jointMatrices.empty() && animations.size() > 0) {
		// Enable skinning
		shader.sendBool("enableSkinning", true);

		// Send joint matrices to shader
		for (size_t i = 0; i < jointMatrices.size() && i < 100; i++) {
			std::string uniformName = "jointMatrices[" + std::to_string(i) + "]";
			shader.sendMat4(uniformName.c_str(), jointMatrices[i]);
		}
	}
	else {
		// Disable skinning for static meshes
		shader.sendBool("enableSkinning", false);
	}

	// Handle each mesh
	for (size_t i = 0; i < meshes.size(); i++) {
		glm::mat4 finalTransform = modelMatrix;

		// If this is a static mesh and has a node associated with it
		if (i < meshNodeIndices.size()) {
			int nodeIndex = meshNodeIndices[i];
			if (nodeIndex >= 0 && nodeIndex < nodes.size() && nodes[nodeIndex]) {
				// Apply node's transform to the model matrix
				finalTransform = modelMatrix * nodes[nodeIndex]->getNodeMatrix();
				shader.sendMat4("model", finalTransform);
			}
		}

		// Draw the mesh
		meshes[i].draw(shader);
	}
}
void Model::cleanup()
{
	// Clean up any dynamically allocated resources
	for (auto& mesh : meshes) {
		// Could add a cleanup method to Mesh class
		// mesh.cleanup();
	}
	meshes.clear();
	boundingBoxes.clear();
}

// Support animation functionality
void Model::updateJointMatrices()
{
	if (!rootNode) {
		return;
	}

	std::cout << "[Model] Updating joint matrices" << std::endl;

	// First update all local matrices
	for (auto& node : nodes) {
		if (node) {
			node->calculateLocalTRSMatrix();
		}
	}

	// Now update all global matrices in a hierarchical way
	NodeUtil::updateNodeMatricesRecursive(rootNode, glm::mat4(1.0f));

	// Finally, update the joint matrices
	updateJointMatricesFromNodes();
}

void Model::updateJointMatricesFromNodes()
{
	// Update joint matrices if this node is a joint
	for (auto const& node : nodes) {
		if (!node)
			continue;

		int nodeIndex = node->nodeNum;
		if (nodeIndex < nodeToJointMapping.size()) {
			int jointIndex = nodeToJointMapping[nodeIndex];
			if (jointIndex >= 0 && jointIndex < jointMatrices.size() && jointIndex < inverseBindMatrices.size()) {
				jointMatrices[jointIndex] = node->getNodeMatrix() * inverseBindMatrices[jointIndex];
			}
		}
	}
}

void Model::initializeDefaultPose()
{
	// Make sure all nodes have proper local matrices calculated
	for (auto& node : nodes) {
		if (node) {
			node->calculateLocalTRSMatrix();
		}
	}

	// Update global matrices starting from the root
	if (rootNode) {
		NodeUtil::updateNodeMatricesRecursive(rootNode, glm::mat4(1.0f));
	}

	// Update joint matrices based on the default pose
	updateJointMatricesFromNodes();

	std::cout << "[Model] Initialized default pose with " << jointMatrices.size() << " joint matrices" << std::endl;
}
