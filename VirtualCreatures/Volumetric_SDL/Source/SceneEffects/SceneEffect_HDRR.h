#pragma once

#include <Scene/SceneEffect.h>

#include <Renderer/Shader/Shader.h>

#include <Renderer/BufferObjects/FBO.h>

// Uncomment to change the blending of the bloom to replace
//#define HDRR_BLOOM_REPLACE_COLOR

class SceneEffect_HDRR :
	public SceneEffect
{
private:
	Shader m_downSampleShader;
	Shader m_toneMapShader;

	// Bloom portion render performs horizontal blur, so only need vertical blur shader
	Shader m_bloomPortionRenderShader;

	Shader* m_pBlurShader_horizontal;
	Shader* m_pBlurShader_vertical;

	std::vector<FBO*> m_downSampleChain;

	FBO m_toneMapTempBuffer;

	FBO m_blurPingPong;
	FBO m_blurPingPong_multipass;

	float m_averageLuminance;

public:
	static const unsigned int s_downSampleSize = 4;

	unsigned int m_bloomBlurPasses;

	float m_eyeAdjustRate;

	float m_minLuminance, m_maxLuminance;
	float m_luminanceMultiplier;

	float m_brightness;
	float m_bloomBlur;
	float m_bloomIntensity;
	float m_bloomFalloff;

	SceneEffect_HDRR();
	~SceneEffect_HDRR();

	bool Create(const std::string &downSampleShaderName,
		const std::string &toneMappingShaderName,
		const std::string &bloomPortionRenderShaderName,
		const std::string &horizontalBlurShaderName,
		const std::string &verticalBlurShaderName,
		unsigned int downSampleChainSize);

	// Inherited from SceneEffect
	void RunEffect();
};

