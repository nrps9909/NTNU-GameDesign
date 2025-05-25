#pragma once

#include <glm/glm.hpp>

class Mesh;
class Model;

struct BoundingBox {
	glm::vec3 min;
	glm::vec3 max;
};

namespace BBoxUtil {
BoundingBox getMeshBBox(Mesh const& mesh);
glm::vec3 getBBoxCenter(BoundingBox const&);
void updateLocalBBox(Model& m);
bool isIntersectBBox(BoundingBox const& a, BoundingBox const& b);
BoundingBox mergeBBox(BoundingBox const& a, BoundingBox const& b);
} // namespace BBoxUtil
