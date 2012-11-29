#pragma once

#include <Constructs/Vec3f.h>

struct Weight
{
	int m_jointID;
	float m_bias;
	Vec3f m_pos;

	Weight();
	Weight(int jointID, float bias, const Vec3f &pos);
};

