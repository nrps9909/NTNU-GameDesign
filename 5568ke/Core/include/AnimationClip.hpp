#pragma once

#include <memory>
#include <string>
#include <vector>

#include <glm/glm.hpp>

// Forward declarations
namespace tinygltf {
class Model;
struct Animation;
struct AnimationChannel;
} // namespace tinygltf
class Node;
class AnimationChannel;

/**
 * @brief The animation clips in a model. A model can have many clips, each of which represents a piece of animation.
 *
 */
class AnimationClip {
public:
	AnimationClip(std::string const& name);

	void addChannel(tinygltf::Model const& model, tinygltf::Animation const& anim, tinygltf::AnimationChannel const& channel);
	void setAnimationFrame(std::vector<std::shared_ptr<Node>> const& nodes, float time);
	float getDuration() const;

	std::string clipName;

private:
	std::vector<std::shared_ptr<AnimationChannel>> channels_{};
};
