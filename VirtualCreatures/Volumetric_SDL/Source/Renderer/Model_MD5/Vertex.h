#pragma once

#include <Constructs/Vec3f.h>
#include <Constructs/Vec2f.h>

struct Vertex
{
	Vec3f m_pos;
	Vec3f m_normal;
	Vec2f m_texCoord;
	int m_startWeight;
	int m_numWeights;

	Vertex();
	Vertex(const Vec3f &pos, const Vec3f &normal, const Vec2f &texCoord, int startWeight, int numWeights);
};

