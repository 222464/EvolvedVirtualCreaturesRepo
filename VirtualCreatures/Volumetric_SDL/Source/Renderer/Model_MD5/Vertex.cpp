#include <Renderer/Model_MD5/Vertex.h>

Vertex::Vertex()
{
}

Vertex::Vertex(const Vec3f &pos, const Vec3f &normal, const Vec2f &texCoord, int startWeight, int numWeights)
	: m_pos(pos), m_normal(normal), m_texCoord(texCoord), m_startWeight(startWeight), m_numWeights(numWeights)
{
}