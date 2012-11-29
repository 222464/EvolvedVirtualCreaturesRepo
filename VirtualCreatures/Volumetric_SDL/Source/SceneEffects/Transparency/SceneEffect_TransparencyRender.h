#pragma once

#include <Scene/SceneEffect.h>

#include <SceneEffects/Transparency/TransparentRenderable.h>
#include <SceneEffects/SceneEffect_Lighting.h>

#include <Renderer/Shader/Shader.h>

#include <vector>

class SceneEffect_TransparencyRender :
	public SceneEffect
{
private:
	SceneEffect_Lighting* m_pLighting;

	// List is cleared every frame
	std::vector<TransparentRenderable*> m_pTransparencyRenderables;

	bool m_rendering;

public:
	SceneEffect_TransparencyRender();

	bool Create(SceneEffect_Lighting* pLighting, const std::string &forwardLightingName);

	Shader m_forwardLightingShader;

	// Inherited from SceneEffect
	void RunEffect();

	void SetDiffuseColor(const Color3f &color);
	void SetSpecularColor(const Color3f &color);

	friend TransparentRenderable;
};