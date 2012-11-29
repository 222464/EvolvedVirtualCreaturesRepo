#include <Renderer/Animation_MD5/BaseFrame.h>

BaseFrame::BaseFrame()
{
}

BaseFrame::BaseFrame(const Vec3f &pos, const Quaternion &orient)
	: m_pos(pos), m_orient(orient)
{
}