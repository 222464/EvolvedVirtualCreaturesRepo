#include <Renderer/Model_MD5/Weight.h>

Weight::Weight()
{
}

Weight::Weight(int jointID, float bias, const Vec3f &pos)
	: m_jointID(jointID), m_bias(bias), m_pos(pos)
{
}


