#pragma once

#include <Scene/SceneEffect.h>

#include <SceneEffects/SceneEffect_Lighting.h>

#include <Renderer/Shader/Shader.h>

#include <Renderer/BufferObjects/FBO.h>

#include <SceneEffects/Water/WaterGeomGrouping.h>

#include <Renderer/Octree/StaticOctree.h>

#include <AssetManager/Asset_Texture.h>

class SceneEffect_WaterRender :
	public SceneEffect
{
private:
	SceneEffect_Lighting* m_pLighting;

	std::list<WaterGeomGrouping*> m_pGeomGroupings;

	Asset_Texture m_bumpMap0, m_bumpMap1;

	Shader m_waterShader;

	FBO m_effectCopy;

public:
	float m_flowRate;

	Vec2f m_bumpMapOffset0, m_bumpMapOffset1;

	SceneEffect_WaterRender();
	~SceneEffect_WaterRender();

	bool Create(SceneEffect_Lighting* pLighting, const std::string &m_waterShaderName, const std::string &bumpMap0Name, const std::string &bumpMap1Name);

	void AddGrouping(WaterGeomGrouping* pGrouping);

	// Inherited from SceneEffect
	void RunEffect();
};

