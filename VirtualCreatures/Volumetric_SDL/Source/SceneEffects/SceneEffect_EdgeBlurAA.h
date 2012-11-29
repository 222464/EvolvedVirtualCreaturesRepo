#pragma once

#include <Scene/SceneEffect.h>

#include <Renderer/Shader/Shader.h>

#include <Renderer/BufferObjects/FBO.h>

class SceneEffect_EdgeBlurAA :
	public SceneEffect
{
private:
	FBO m_tempBuffer;
	Shader m_aaShader;
	bool m_created;

public:
	SceneEffect_EdgeBlurAA();
	
	bool Create(const std::string &aaShaderName);
	bool Created();

	// Inherited from SceneEffect
	void RunEffect();
};

