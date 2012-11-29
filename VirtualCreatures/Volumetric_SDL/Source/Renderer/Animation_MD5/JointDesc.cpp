#include <Renderer/Animation_MD5/JointDesc.h>

JointDesc::JointDesc()
{
}

JointDesc::JointDesc(const std::string &name, int parentID, int flags, int startIndex)
	: m_name(name), m_parentID(parentID), m_flags(flags), m_startIndex(startIndex)
{
}