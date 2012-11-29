#include <SceneEffects/SceneEffect_SSAO.h>

#include <Scene/Scene.h>

#include <assert.h>

SceneEffect_SSAO::SceneEffect_SSAO()
	: m_created(false)
{
}

bool SceneEffect_SSAO::Create(const std::string &ssaoShaderName, const std::string &horizontalBlurShaderName, const std::string &verticalBlurShaderName, const std::string &randomTextureName)
{
	if(!m_ssao.LoadAsset(ssaoShaderName))
		return false;

	if(!m_randomTexture.LoadAsset(randomTextureName))
		return false;

	Asset* pAsset;

	if(!GetScene()->GetAssetManager_AutoCreate("shader", Shader::Asset_Factory)->GetAsset(horizontalBlurShaderName, pAsset))
		return false;

	m_pBlurShader_horizontal = static_cast<Shader*>(pAsset);

	if(!GetScene()->GetAssetManager("shader")->GetAsset(verticalBlurShaderName, pAsset))
		return false;

	m_pBlurShader_vertical = static_cast<Shader*>(pAsset);

	m_blurPingPong.Create(GetScene()->m_pWin->GetPixelWidth() / 2, GetScene()->m_pWin->GetPixelHeight() / 2, false, GL_RGB8, GL_RGB, GL_UNSIGNED_BYTE);

	m_lowResBuffer.Create(GetScene()->m_pWin->GetPixelWidth() / 2, GetScene()->m_pWin->GetPixelHeight() / 2, false, GL_RGB8, GL_RGB, GL_UNSIGNED_BYTE);

	// Wrap random texture
	glBindTexture(GL_TEXTURE_2D, m_randomTexture.GetTextureID());
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	Scene* pScene = GetScene();

	// Set textures once
	m_ssao.Bind();

	m_ssao.SetShaderTexture("gNormal", pScene->m_gBuffer.GetTextureID(GBuffer::e_normal), GL_TEXTURE_2D);
	m_ssao.SetShaderTexture("gPosition", pScene->m_gBuffer.GetTextureID(GBuffer::e_position), GL_TEXTURE_2D);
	//m_ssao.SetShaderTexture("gDepth", 0, GL_TEXTURE_2D);
	m_ssao.SetShaderTexture("random", m_randomTexture.GetTextureID(), GL_TEXTURE_2D);

	m_ssao.SetUniformmat4("projection", Matrix4x4f::GL_Get_Projection());

	m_ssao.SetUniformf("radius", 0.6f);

	Shader::Unbind();

	m_created = true;

	return true;
}

bool SceneEffect_SSAO::Created()
{
	return m_created;
}

void SceneEffect_SSAO::RunEffect()
{
	assert(m_created);

	FBO::Unbind_Draw();
	FBO::Unbind_Read();

	glFlush();

	Scene* pScene = GetScene();

	m_lowResBuffer.Bind();
	m_lowResBuffer.SetViewport();

	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);

	m_ssao.Bind();
	m_ssao.BindShaderTextures();

	Matrix4x4f viewMatrix;
	pScene->m_camera.GetViewMatrix(viewMatrix);
	pScene->m_pWin->SetProjection();

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0, m_lowResBuffer.GetWidth(), 0, m_lowResBuffer.GetHeight(), -1, 1);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	glDisable(GL_DEPTH_TEST);
	glDisable(GL_CULL_FACE);

	float lowResBufferWidthf = static_cast<float>(m_lowResBuffer.GetWidth());
	float lowResBufferHeightf = static_cast<float>(m_lowResBuffer.GetHeight());

	glLoadIdentity();
	glBegin(GL_QUADS);
	glTexCoord2i(0, 0); glVertex3f(0.0f, 0.0f, 0.0f);
	glTexCoord2i(1, 0); glVertex3f(lowResBufferWidthf, 0.0f, 0.0f);
	glTexCoord2i(1, 1); glVertex3f(lowResBufferWidthf, lowResBufferHeightf, 0.0f);
	glTexCoord2i(0, 1); glVertex3f(0.0f, lowResBufferHeightf, 0.0f);
	glEnd();

	m_ssao.UnbindShaderTextures();
	Shader::Unbind();

	// ----------------------------- Blur ----------------------------

	// Bind ping pong FBO
	m_blurPingPong.Bind();
	m_blurPingPong.SetViewport();

	m_pBlurShader_horizontal->Bind();
	m_pBlurShader_horizontal->SetShaderTexture("scene", m_lowResBuffer.GetTextureID(), GL_TEXTURE_2D);
	m_pBlurShader_horizontal->BindShaderTextures();
	m_pBlurShader_horizontal->SetUniformf("blurSize", 0.003f); 

	// Fullscreen quad to blur
	glBegin(GL_QUADS);
	glTexCoord2i(0, 0); glVertex3f(0.0f, 0.0f, 0.0f);
	glTexCoord2i(1, 0); glVertex3f(lowResBufferWidthf, 0.0f, 0.0f);
	glTexCoord2i(1, 1); glVertex3f(lowResBufferWidthf, lowResBufferHeightf, 0.0f);
	glTexCoord2i(0, 1); glVertex3f(0.0f, lowResBufferHeightf, 0.0f);
	glEnd();

	glFlush();

	// Back to effect FBO
	FBO::Unbind();
	pScene->m_gBuffer.Bind();
	pScene->m_pWin->SetViewport();
	pScene->m_pWin->SetOrtho();

	m_pBlurShader_vertical->Bind();
	m_pBlurShader_vertical->SetShaderTexture("scene", m_blurPingPong.GetTextureID(), GL_TEXTURE_2D);
	m_pBlurShader_vertical->BindShaderTextures();
	m_pBlurShader_vertical->SetUniformf("blurSize", 0.003f); 

	glEnable(GL_BLEND);
	glBlendFunc(GL_DST_COLOR, GL_ZERO);
	
	// Fullscreen quad to blur
	pScene->m_gBuffer.DrawBufferSizedQuad();

	glDisable(GL_BLEND);
	glEnable(GL_CULL_FACE);
	glEnable(GL_DEPTH_TEST);

	pScene->m_pWin->SetProjection();

	Shader::Unbind();
	
	GL_ERROR_CHECK();
}