#pragma once

#include <Constructs/Vec3f.h>
#include <Constructs/Quaternion.h>
#include <Renderer/Animation_MD5/BaseFrame.h>

struct SkeletalJoint
{
	int m_parentID;
	Vec3f m_pos;
	Quaternion m_orient;

	SkeletalJoint();
	SkeletalJoint(const BaseFrame &baseFrame);
};

