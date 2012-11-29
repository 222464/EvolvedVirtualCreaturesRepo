#pragma once

#include <Constructs/Vec3f.h>
#include <Constructs/Quaternion.h>

struct BaseFrame
{
	Vec3f m_pos;
	Quaternion m_orient;

	BaseFrame();
	BaseFrame(const Vec3f &pos, const Quaternion &orient);
};

