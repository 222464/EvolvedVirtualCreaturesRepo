#pragma once

#include <Scene/SceneEffect.h>

#include <Renderer/Shader/Shader.h>

#include <AssetManager/Asset_Texture.h>

#include <Renderer/BufferObjects/FBO.h>

class SceneEffect_SSAO :
	public SceneEffect
{
private:
	FBO m_lowResBuffer;
	FBO m_blurPingPong;

	Shader m_ssao;
	Shader* m_pBlurShader_horizontal;
	Shader* m_pBlurShader_vertical;

	Asset_Texture m_randomTexture;

	bool m_created;

public:
	SceneEffect_SSAO();

	bool Create(const std::string &ssaoShaderName, const std::string &horizontalBlurShaderName, const std::string &verticalBlurShaderName, const std::string &randomTextureName);
	bool Created();

	// Inherited from SceneEffect
	void RunEffect();
};

