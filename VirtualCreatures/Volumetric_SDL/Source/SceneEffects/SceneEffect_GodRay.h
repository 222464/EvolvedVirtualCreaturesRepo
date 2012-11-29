#pragma once

#include <Scene/SceneEffect.h>
#include <Renderer/Shader/Shader.h>
#include <Renderer/Window.h>

class SceneEffect_GodRay :
	public SceneEffect
{
private:
	Shader m_godRayShader;

	Window* m_pWin;

	bool m_created;

public:
	SceneEffect_GodRay();

	bool Create(Window* pWin, const std::string &godRayShaderFileName);

	bool Created();

	// Inherited from SceneEffect
	void RunEffect();
};

