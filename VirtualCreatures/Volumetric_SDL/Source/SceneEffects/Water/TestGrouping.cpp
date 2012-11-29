#include <SceneEffects/Water/TestGrouping.h>

#include <Renderer/SDL_OpenGL.h>

TestGrouping::TestGrouping()
{
	m_aabb.SetCenter(Vec3f(0.0f, 16.0f, 0.0f));

	m_aabb.SetHalfDims(Vec3f(100.0f, 0.1f, 100.0f));
}

void TestGrouping::RenderGeomGrouping()
{
	GetScene()->SetWorldMatrix(Matrix4x4f::IdentityMatrix());

	glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
	glBegin(GL_QUADS);
	glNormal3f(0.0f, 1.0f, 0.0f);
	glTexCoord2i(0, 0); glVertex3f(-100.0f, 16.2f, 100.0f);
	glTexCoord2i(1, 0); glVertex3f(100.0f, 16.2f, 100.0f);
	glTexCoord2i(1, 1); glVertex3f(100.0f, 16.2f, -100.0f);
	glTexCoord2i(0, 1); glVertex3f(-100.0f, 16.2f, -100.0f);
	glEnd();
}