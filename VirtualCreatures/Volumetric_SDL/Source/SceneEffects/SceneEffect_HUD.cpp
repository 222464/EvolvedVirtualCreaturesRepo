#include <SceneEffects/SceneEffect_HUD.h>

#include <Renderer/RenderUtils.h>

SceneEffect_HUD::SceneEffect_HUD(World* pWorld)
	: m_pWorld(pWorld)
{
}

SceneEffect_HUD::~SceneEffect_HUD()
{
}

void SceneEffect_HUD::OnAdd()
{
	// Render minimap
	m_miniMapFBO.Create(256, 256, false, GL_RGB8, GL_RGB, GL_UNSIGNED_BYTE);

	m_miniMapFBO.Bind();

	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);

	glDisable(GL_CULL_FACE);
	glDisable(GL_DEPTH_TEST);
	glDepthMask(false);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0, static_cast<double>(m_pWorld->GetChunksInX()) * Chunk::s_chunkSizeX, 0, static_cast<double>(m_pWorld->GetChunksInZ()) * Chunk::s_chunkSizeZ, -2000.0f, 2000.0f);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glDisable(GL_BLEND);

	Shader::Unbind();

	Matrix4x4f::CameraDirectionMatrix(Vec3f(0.0f, -1.0f, 0.0f), Vec3f(0.0f, 0.0f, 1.0f)).GL_Mult();
	Matrix4x4f::TranslateMatrix(Vec3f(0.0f, -1000.0f, 0.0f)).GL_Mult();

	glDisable(GL_TEXTURE_2D);

	glColor4f(0.0f, 1.0f, 0.0f, 1.0f);

	m_pWorld->RenderAllChunks();

	glColor4f(1.0f, 1.0f, 1.0f, 1.0f);

	

	glEnable(GL_DEPTH_TEST);
	glDepthMask(true);
	glEnable(GL_CULL_FACE);

	m_miniMapFBO.Unbind();

	GetScene()->m_pWin->SetProjection();
}

void SceneEffect_HUD::RunEffect()
{
	Scene* pScene = GetScene();

	glDisable(GL_DEPTH_TEST);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0, pScene->m_pWin->m_projected_width, 0, pScene->m_pWin->m_projected_height, -1.0f, 1.0f);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	// Draw HUD
	glEnable(GL_TEXTURE_2D);

	glBindTexture(GL_TEXTURE_2D, m_miniMapFBO.GetTextureID());

	glTranslatef(pScene->m_pWin->m_projected_width - 256.0f, pScene->m_pWin->m_projected_height - 256.0f, 0.0f);
	DrawQuadOriginBottomLeft(256.0f, 256.0f);

	glDisable(GL_TEXTURE_2D);

	GetScene()->m_pWin->SetProjection();

	glEnable(GL_DEPTH_TEST);
}