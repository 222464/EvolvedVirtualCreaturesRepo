#include <SceneEffects/SceneEffect_EdgeBlurAA.h>

#include <Scene/Scene.h>

#include <assert.h>

SceneEffect_EdgeBlurAA::SceneEffect_EdgeBlurAA()
	: m_created(false)
{
}

bool SceneEffect_EdgeBlurAA::Create(const std::string &aaShaderName)
{
	assert(!m_created);
	assert(GetScene() != NULL);

	if(!m_aaShader.LoadAsset(aaShaderName))
		return false;

	Scene* pScene = GetScene();

	m_tempBuffer.Create(pScene->m_gBuffer.GetWidth(), pScene->m_gBuffer.GetHeight(), false, GL_RGB8, GL_RGB, GL_UNSIGNED_BYTE);

	// Default
	m_aaShader.Bind();
	m_aaShader.SetShaderTexture("tex0", pScene->m_gBuffer.GetEffectTextureID(), GL_TEXTURE_2D);

	GL_ERROR_CHECK();

	m_created = true;

	return true;
}

bool SceneEffect_EdgeBlurAA::Created()
{
	return m_created;
}

void SceneEffect_EdgeBlurAA::RunEffect()
{
	Scene* pScene = GetScene();
	
	// Run shader back on main buffer
	m_aaShader.Bind();
	m_aaShader.BindShaderTextures();

	glDisable(GL_DEPTH_TEST);

	pScene->m_pWin->SetOrtho();
	glLoadIdentity();

	pScene->m_gBuffer.DrawBufferSizedQuad();

	Shader::Unbind();

	

	pScene->m_gBuffer.Bind();

	pScene->m_gBuffer.DrawBufferSizedQuad();

	pScene->m_pWin->SetProjection();

	glEnable(GL_DEPTH_TEST);

	

	GL_ERROR_CHECK();
}