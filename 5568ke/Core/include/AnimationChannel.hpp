#pragma once

#include <memory>
#include <vector>

#include <glm/glm.hpp>
#include <glm/gtx/quaternion.hpp>

#include "AnimationTypes.hpp"

// Forward declarations
namespace tinygltf {
class Model;
struct Animation;
struct AnimationChannel;
} // namespace tinygltf

/**
 * @brief The channel of the animation. Each channel contains a single transformation of a node for the entire duration of the animation.
 *
 */
class AnimationChannel {
public:
	void loadChannelData(tinygltf::Model const& model, tinygltf::Animation const& anim, tinygltf::AnimationChannel const& channel);

	glm::vec3 getScaling(float time) const;
	glm::vec3 getTranslation(float time) const;
	glm::quat getRotation(float time) const;
	float getMaxTime() const;

	int targetNode{-1};
	TargetPath targetPath = TargetPath::ROTATION;

private:
	InterpolationType interpolationType_ = InterpolationType::LINEAR;

	std::vector<float> timings_{}; // The time of the corresponding keyframe

	// Since there is only one path in a channel, only one of the three vectors below is not empty.
	std::vector<glm::vec3> scalings_{};
	std::vector<glm::vec3> translations_{};
	std::vector<glm::quat> rotations_{};
};
