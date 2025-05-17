#pragma once

#include <glm/glm.hpp>
#include <vector>

#include "Shader.hpp"

// Material type enum
enum class MaterialType { BlinnPhong, PBR };

class Material {
public:
	virtual void bind(Shader const& shader) const = 0;
};
