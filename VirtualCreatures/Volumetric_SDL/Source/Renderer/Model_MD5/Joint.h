#pragma once

#include <Constructs/Vec3f.h>
#include <Constructs/Quaternion.h>

#include <string>

struct Joint
{
	std::string m_name;
	int m_parentID;
	Vec3f m_pos;
	Quaternion m_orient;

	Joint();
	Joint(const std::string name, int parentID, const Vec3f &pos, const Quaternion &orient);
};