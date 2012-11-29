#include <Renderer/Model_MD5/Joint.h>

Joint::Joint()
{
}

Joint::Joint(const std::string name, int parentID, const Vec3f &pos, const Quaternion &orient)
	: m_name(name), m_parentID(parentID), m_pos(pos), m_orient(orient)
{
}