#pragma once

#include <functional>
#include <vector>

#include "BoundingBox.hpp"
#include "Collider.hpp"
#include "GameObject.hpp"

class CollisionSystem {
public:
	static CollisionSystem& getInstance()
	{
		static CollisionSystem instance;
		return instance;
	}

	using Callback = std::function<void(std::shared_ptr<GameObject>, std::shared_ptr<GameObject>)>;

	void add(std::shared_ptr<AABBCollider> c) { colliders_.push_back(c); }
	void remove(std::shared_ptr<AABBCollider> c);

	// Call once per frame AFTER all GameObject transforms have been updated
	void update()
	{
		std::size_t n = colliders_.size();
		for (std::size_t i = 0; i < n; ++i) {
			for (std::size_t j = i + 1; j < n; ++j) {
				auto a = colliders_[i];
				auto b = colliders_[j];
				if (BBoxUtil::isIntersectBBox(a->bounds(), b->bounds())) {
					onCollision_(a->owner(), b->owner());
				}
			}
		}
	}

private:
	CollisionSystem() = default;
	~CollisionSystem() = default;

	std::vector<std::shared_ptr<AABBCollider>> colliders_;
	void onCollision_(std::shared_ptr<GameObject>, std::shared_ptr<GameObject>);
};

inline void CollisionSystem::remove(std::shared_ptr<AABBCollider> c)
{
	colliders_.erase(std::remove(colliders_.begin(), colliders_.end(), c), colliders_.end());
}

inline void CollisionSystem::onCollision_(std::shared_ptr<GameObject> a, std::shared_ptr<GameObject> b)
{
	auto& A = *a;
	auto& B = *b;

	// Compute AABB overlap on each axis
	auto const& aMin = A.worldBBox.min;
	auto const& aMax = A.worldBBox.max;
	auto const& bMin = B.worldBBox.min;
	auto const& bMax = B.worldBBox.max;

	float overlapX = std::min(aMax.x - bMin.x, bMax.x - aMin.x);
	float overlapY = std::min(aMax.y - bMin.y, bMax.y - aMin.y);
	float overlapZ = std::min(aMax.z - bMin.z, bMax.z - aMin.z);

	// Find smallest penetration axis & its unit normal
	float penetration = std::min({overlapX, overlapY, overlapZ});
	glm::vec3 axisNormal;
	enum { AX_X, AX_Y, AX_Z } axis;
	if (penetration == overlapX) {
		axis = AX_X;
		axisNormal = {1, 0, 0};
	}
	else if (penetration == overlapY) {
		axis = AX_Y;
		axisNormal = {0, 1, 0};
	}
	else {
		axis = AX_Z;
		axisNormal = {0, 0, 1};
	}

	// Compute centers to know which side to push
	glm::vec3 centerA = BBoxUtil::getBBoxCenter(A.worldBBox);
	glm::vec3 centerB = BBoxUtil::getBBoxCenter(B.worldBBox);
	float side = (glm::dot(centerB - centerA, axisNormal) >= 0.0f ? 1.0f : -1.0f);
	glm::vec3 pushDir = axisNormal * side; // direction to push A out of B

	float invMassSum = A.invMass + B.invMass;
	if (invMassSum <= 0.0f) {
		// both static -> nothing to do
		return;
	}

	// Vertical contact hack: if Y-axis collision, full correction + zero Y-velocity
	if (axis == AX_Y) {
		// push A and B fully out of overlap along Y
		float corrA = (penetration * (A.invMass / invMassSum));
		float corrB = (penetration * (B.invMass / invMassSum));
		A.position -= pushDir * corrA;
		B.position += pushDir * corrB;

		// zero vertical velocities so they rest
		if (A.invMass > 0)
			A.velocity.y = 0.0f;
		if (B.invMass > 0)
			B.velocity.y = 0.0f;
	}
	else {
		// Fractional correction for non-vertical collisions (prevents jitter)
		float const k_slop = 0.01f; // small penetration allowance
		float const percent = 0.4f; // correct 40% per frame
		float correctionMag = std::max(penetration - k_slop, 0.0f) / invMassSum * percent;
		glm::vec3 correction = pushDir * correctionMag;
		A.position -= correction * A.invMass;
		B.position += correction * B.invMass;

		// Recompute transforms before impulse
		A.updateTransformMatrix();
		B.updateTransformMatrix();

		// Apply bounce impulse only if moving into each other
		glm::vec3 relVel = A.velocity - B.velocity;
		float vn = glm::dot(relVel, pushDir);
		if (vn < 0.0f) {
			float e = std::min(A.restitution, B.restitution);
			float j = -(1 + e) * vn / invMassSum;
			glm::vec3 impulse = pushDir * j;
			A.velocity += impulse * A.invMass;
			B.velocity -= impulse * B.invMass;
		}
	}

	// Final transforms & user callbacks
	A.updateTransformMatrix();
	B.updateTransformMatrix();
	a->onCollisionEnter(b);
	b->onCollisionEnter(a);
}
