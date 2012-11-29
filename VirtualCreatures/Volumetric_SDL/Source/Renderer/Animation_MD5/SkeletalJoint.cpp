#include <Renderer/Animation_MD5/SkeletalJoint.h>

SkeletalJoint::SkeletalJoint()
	: m_parentID(-1), m_pos(0.0f, 0.0f, 0.0f)
{
}

SkeletalJoint::SkeletalJoint(const BaseFrame &baseFrame)
	: m_pos(baseFrame.m_pos), m_orient(baseFrame.m_orient)
{
}