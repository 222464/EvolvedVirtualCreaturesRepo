#include <SceneEffects/SceneEffect_HDRR.h>

#include <Renderer/RenderUtils.h>

#include <Utilities/UtilFuncs.h>

#include <Scene/Scene.h>

#include <assert.h>

SceneEffect_HDRR::SceneEffect_HDRR()
	: m_minLuminance(0.05f), m_maxLuminance(6.0f),
	m_luminanceMultiplier(0.8f),
	m_averageLuminance((m_minLuminance + m_maxLuminance) / 2.0f),
	m_eyeAdjustRate(0.1f), m_brightness(1.2f),
	m_bloomBlurPasses(3),
	m_bloomBlur(0.006142f), m_bloomIntensity(9.0f), m_bloomFalloff(22.0f)
{
}

SceneEffect_HDRR::~SceneEffect_HDRR()
{
	// Delete FBO's from chain
	for(unsigned int i = 0, size = m_downSampleChain.size(); i < size; i++)
		delete m_downSampleChain[i];
}

bool SceneEffect_HDRR::Create(const std::string &downSampleShaderName,
		const std::string &toneMappingShaderName,
		const std::string &bloomPortionRenderShaderName,
		const std::string &horizontalBlurShaderName,
		const std::string &verticalBlurShaderName,
		unsigned int downSampleChainSize)
{
	Scene* pScene = GetScene();

	assert(pScene != NULL);

	m_toneMapTempBuffer.Create(pScene->m_gBuffer.GetWidth(), pScene->m_gBuffer.GetHeight(), false, GL_RGB16F, GL_RGB, GL_FLOAT);

	// -------------------------------- Load Shaders ---------------------------------

	if(!m_downSampleShader.LoadAsset(downSampleShaderName))
		return false;

	if(!m_toneMapShader.LoadAsset(toneMappingShaderName))
		return false;

	if(!m_bloomPortionRenderShader.LoadAsset(bloomPortionRenderShaderName))
		return false;

	Asset* pAsset;

	if(!GetScene()->GetAssetManager_AutoCreate("shader", Shader::Asset_Factory)->GetAsset(horizontalBlurShaderName, pAsset))
		return false;

	m_pBlurShader_horizontal = static_cast<Shader*>(pAsset);

	if(!GetScene()->GetAssetManager("shader")->GetAsset(verticalBlurShaderName, pAsset))
		return false;

	m_pBlurShader_vertical = static_cast<Shader*>(pAsset);

	// Defaults - Down Sample
	m_downSampleShader.Bind();
	m_downSampleShader.SetShaderTexture("scene", 0, GL_TEXTURE_2D);

	m_downSampleShader.Unbind();

	GL_ERROR_CHECK();

	// Defaults - Tone mapping
	m_toneMapShader.Bind();

	m_toneMapShader.SetShaderTexture("scene", pScene->m_gBuffer.GetEffectTextureID(), GL_TEXTURE_2D);
	m_toneMapShader.SetUniformf("colorScalar", 1.0f);
	//m_toneMapShader.SetUniformf("maxBrightness", m_maxLuminance);

	m_toneMapShader.Unbind();

	GL_ERROR_CHECK();

	// Defaults - bloom portion render
	m_bloomPortionRenderShader.Bind();

	m_bloomPortionRenderShader.SetShaderTexture("scene", m_toneMapTempBuffer.GetTextureID(), GL_TEXTURE_2D);

	m_bloomPortionRenderShader.Unbind();

	m_blurPingPong.Create(GetScene()->m_pWin->GetPixelWidth() / 2, GetScene()->m_pWin->GetPixelHeight() / 2, false, GL_RGB8, GL_RGB, GL_UNSIGNED_BYTE);
	m_blurPingPong_multipass.Create(GetScene()->m_pWin->GetPixelWidth() / 2, GetScene()->m_pWin->GetPixelHeight() / 2, false, GL_RGB8, GL_RGB, GL_UNSIGNED_BYTE);
	
	GL_ERROR_CHECK();

	// --------------------------- Create downsample chain ---------------------------

	unsigned int width = pScene->m_gBuffer.GetWidth() / s_downSampleSize;
	unsigned int height = pScene->m_gBuffer.GetHeight() / s_downSampleSize;

	while(width >= s_downSampleSize || height >= s_downSampleSize)
	{
		FBO* newFBO = new FBO();

		m_downSampleChain.push_back(newFBO);

		newFBO->Create(width, height, false, GL_RGB16F, GL_RGB, GL_FLOAT);

		width /= s_downSampleSize;
		height /= s_downSampleSize;
	}

	// Always add 1x1 FBO
	FBO* newFBO = new FBO();

	m_downSampleChain.push_back(newFBO);

	newFBO->Create(1, 1, false, GL_RGB16F, GL_RGB, GL_FLOAT);

	GL_ERROR_CHECK();

	return true;
}

void SceneEffect_HDRR::RunEffect()
{
	Scene* pScene = GetScene();

	glDisable(GL_DEPTH_TEST);

	// Ortho projection in normalized coordinates
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();

	m_downSampleShader.Bind();
	glBindTexture(GL_TEXTURE_2D, pScene->m_gBuffer.GetEffectTextureID());

	float offsets_x[s_downSampleSize];
	float offsets_y[s_downSampleSize];

	const int downSampleOffset = s_downSampleSize / 2;

	float gWidthInv = 1.0f / static_cast<float>(pScene->m_gBuffer.GetWidth());
	float gHeightInv = 1.0f / static_cast<float>(pScene->m_gBuffer.GetHeight());

	for(int i = 0; i < s_downSampleSize; i++)
	{
		offsets_x[i] = gWidthInv * (i - downSampleOffset + 0.5f);
		offsets_y[i] = gHeightInv * (i - downSampleOffset + 0.5f);
	}

	m_downSampleShader.SetUniform1fv("offsets_x", s_downSampleSize, offsets_x);
	m_downSampleShader.SetUniform1fv("offsets_y", s_downSampleSize, offsets_y);

	// Get scene luminance by downsample the effect texture
	m_downSampleChain[0]->Bind();
	m_downSampleChain[0]->SetViewport();

	DrawNormalizedQuad();

	glFlush();

	for(unsigned int i = 1, size = m_downSampleChain.size(); i < size; i++)
	{
		unsigned int prevIndex = i - 1;

		gWidthInv = 1.0f / static_cast<float>(m_downSampleChain[prevIndex]->GetWidth());
		gHeightInv = 1.0f / static_cast<float>(m_downSampleChain[prevIndex]->GetHeight());
		
		for(int j = 0; j < s_downSampleSize; j++)
		{
			offsets_x[j] = gWidthInv * (j - downSampleOffset + 0.5f);
			offsets_y[j] = gHeightInv * (j - downSampleOffset + 0.5f);
		}

		m_downSampleShader.SetUniform1fv("offsets_x", s_downSampleSize, offsets_x);
		m_downSampleShader.SetUniform1fv("offsets_y", s_downSampleSize, offsets_y);

		m_downSampleChain[i]->Bind();
		m_downSampleChain[i]->SetViewport();
		glBindTexture(GL_TEXTURE_2D, m_downSampleChain[prevIndex]->GetTextureID());

		DrawNormalizedQuad();

		glFlush();
	}

	Shader::Unbind();

	m_downSampleChain.back()->Bind_Read();

	// 1x1 FBO will still be bound, read from that
	float pixel[3];

	glReadPixels(0, 0, 1, 1, GL_RGB, GL_FLOAT, pixel);

	float luminance = sqrtf((pixel[0] * 0.299f + pixel[1] * 0.587f + pixel[2] * 0.114f)) * m_luminanceMultiplier;

	m_averageLuminance += (luminance - m_averageLuminance) * m_eyeAdjustRate * GetScene()->m_frameTimer.GetTimeMultiplier();

	m_averageLuminance = Clamp(m_averageLuminance, m_minLuminance, m_maxLuminance);

	float exposure = 1.0f / m_averageLuminance;
	float colorScalar = m_brightness * exposure / (exposure + 1.0f);

	// Render scene to temp buffer with tone mapping shader
	m_toneMapTempBuffer.Bind();

	m_toneMapTempBuffer.SetViewport();

	m_toneMapShader.Bind();
	m_toneMapShader.BindShaderTextures();
	m_toneMapShader.SetUniformf("colorScalar", colorScalar);

	DrawNormalizedQuad();

	m_toneMapShader.Unbind();

	// Copy tone map buffer to effect texture
	pScene->m_gBuffer.Bind_Draw();

	glBlitFramebuffer(0, 0, m_toneMapTempBuffer.GetWidth(), m_toneMapTempBuffer.GetHeight(),
		0, 0, m_toneMapTempBuffer.GetWidth(), m_toneMapTempBuffer.GetHeight(), GL_COLOR_BUFFER_BIT, GL_LINEAR);	

	// Calculate bloom using blur (horizontal is special bloom blur)
	m_blurPingPong.Bind();
	m_blurPingPong.SetViewport();

	m_bloomPortionRenderShader.Bind();
	m_bloomPortionRenderShader.BindShaderTextures();
	m_bloomPortionRenderShader.SetUniformf("blurSize", m_bloomBlur);
	m_bloomPortionRenderShader.SetUniformf("bloomIntensity", m_bloomIntensity);
	m_bloomPortionRenderShader.SetUniformf("bloomFalloff", m_bloomFalloff);

	DrawNormalizedQuad();

	m_bloomPortionRenderShader.Unbind();

	m_blurPingPong.Unbind();

	// Extra blur passes (already have 1, so start at 1)
	for(unsigned int p = 1; p < m_bloomBlurPasses; p++)
	{
		m_blurPingPong_multipass.Bind();
		m_pBlurShader_vertical->Bind();
		m_pBlurShader_vertical->SetShaderTexture("scene", m_blurPingPong.GetTextureID(), GL_TEXTURE_2D);
		m_pBlurShader_vertical->SetUniformf("blurSize", m_bloomBlur);
		m_pBlurShader_vertical->BindShaderTextures();

		DrawNormalizedQuad();	

		m_blurPingPong.Bind();
		m_pBlurShader_horizontal->Bind();
		m_pBlurShader_horizontal->SetShaderTexture("scene", m_blurPingPong_multipass.GetTextureID(), GL_TEXTURE_2D);
		m_pBlurShader_horizontal->SetUniformf("blurSize", m_bloomBlur);
		m_pBlurShader_horizontal->BindShaderTextures();

		DrawNormalizedQuad();
	}

	FBO::Unbind();

	// Additively blend bloom
	glEnable(GL_BLEND);

#ifdef HDRR_BLOOM_REPLACE_COLOR
	glBlendFunc(GL_ONE_MINUS_DST_COLOR, GL_ONE);
#else
	glBlendFunc(GL_ONE, GL_ONE);
#endif

	// Bind GBuffer again
	pScene->m_gBuffer.Bind();

	// Vertical blur
	m_pBlurShader_vertical->Bind();
	m_pBlurShader_vertical->SetShaderTexture("scene", m_blurPingPong.GetTextureID(), GL_TEXTURE_2D);
	m_pBlurShader_vertical->SetUniformf("blurSize", m_bloomBlur);

	m_pBlurShader_vertical->BindShaderTextures();

	pScene->m_pWin->SetViewport();

	DrawNormalizedQuad();

	glDisable(GL_BLEND);

	// Reset view matrix
	pScene->m_pWin->SetProjection();

	glEnable(GL_DEPTH_TEST);

	GL_ERROR_CHECK();
}