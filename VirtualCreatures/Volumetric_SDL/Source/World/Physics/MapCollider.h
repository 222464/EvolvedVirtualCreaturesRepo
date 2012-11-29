#pragma once

#include <Constructs/AABB.h>
#include <World/World.h>

#include <vector>

class MapCollider
{
private:
	AABB m_left;
	AABB m_right;
	AABB m_top;
	AABB m_bottom;
	AABB m_front;
	AABB m_back;

public:
	Vec3f m_center;
	Vec3f m_halfDims;

	World* m_pWorld;

	Vec3f m_vel;

	float m_boxThickness;

	MapCollider();

	// Returns if is hitting ground in y direction or not
	bool Update();
};

