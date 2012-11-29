#pragma once

#include <World/World.h>

#include <AssetManager/Asset_Texture.h>

#include <Constructs/Vec3f.h>

#include <Scene/SceneEffect.h>

class SceneEffect_HUD :
	public SceneEffect
{
private:
	World* m_pWorld;
	FBO m_miniMapFBO;

public:
	SceneEffect_HUD(World* pWorld);
	~SceneEffect_HUD();

	// Inherited from SceneEffect
	void OnAdd();
	void RunEffect();
};

