#include <SceneEffects/SceneEffect_GodRay.h>

#include <Scene/Scene.h>

#include <Renderer/SDL_OpenGL.h>

#include <assert.h>

SceneEffect_GodRay::SceneEffect_GodRay()
	: m_created(false)
{
}

bool SceneEffect_GodRay::Create(Window* pWin, const std::string &godRayShaderFileName)
{
	m_pWin = pWin;

	m_created = m_godRayShader.LoadAsset("NONE NONE data/shaders/godRay.frag");

	if(!m_created)
		return false;

	// Defaults
	m_godRayShader.Bind();
	m_godRayShader.SetUniformf("exposure", 0.01f);
	m_godRayShader.SetUniformf("decay", 0.99f);
	m_godRayShader.SetUniformf("density", 0.7f);
	m_godRayShader.SetUniformf("weight", 0.5f);
	m_godRayShader.Unbind();

	return true;
}

bool SceneEffect_GodRay::Created()
{
	return m_created;
}

void SceneEffect_GodRay::RunEffect()
{
	assert(m_created);

	// Set up orthogonal projection
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0.0f, 1.0f, 0.0f, 1.0f, -1.0f, 1.0f);
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();

	double modelView[16];
	glGetDoublev(GL_MODELVIEW_MATRIX, modelView);

	double projection[16];
	glGetDoublev(GL_PROJECTION_MATRIX, projection);

	double screenX, screenY, screenZ;

	int viewPort[] = {0, 0, static_cast<signed>(m_pWin->GetPixelWidth()), static_cast<signed>(m_pWin->GetPixelHeight())};
	
	gluProject(1000.0f, 1000.0f, 1000.0f, modelView, projection, viewPort, &screenX, &screenY, &screenZ);
	//float godDot = -pPlayer->GetViewVec().Normalize().Dot((Vec3f(100.0f, 100.0f, 100.0f) - pPlayer->GetPosition()).Normalize());
		
	m_godRayShader.Bind();

	//if(godDot <= 0.0f)
	//	m_godRayShader.SetUniformf("exposure", 0.0f);
	//else
	m_godRayShader.SetUniformf("exposure", 0.01f);

	m_godRayShader.SetShaderTexture("firstPass", GetScene()->m_gBuffer.GetTextureID(GBuffer::e_color), GL_TEXTURE_2D);
	m_godRayShader.SetUniformv2f("lightPositionOnScreen", static_cast<float>(screenX) / m_pWin->GetPixelWidth(), static_cast<float>(screenY) / m_pWin->GetPixelHeight());

	glDisable(GL_DEPTH_TEST);

	glEnable(GL_BLEND);

	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glBegin(GL_QUADS);
	glTexCoord2i(0, 0); glVertex2f(0.0f, 0.0f);
	glTexCoord2i(1, 0); glVertex2f(1.0f, 0.0f);
	glTexCoord2i(1, 1); glVertex2f(1.0f, 1.0f);
	glTexCoord2i(0, 1); glVertex2f(0.0f, 1.0f);
	glEnd();

	m_godRayShader.Unbind();

	glDisable(GL_BLEND);

	glEnable(GL_DEPTH_TEST);

	glPopMatrix();

	GL_ERROR_CHECK();
}