#pragma once

#include <Renderer/ShadowMap/ShadowCubeMap.h>
#include <SceneEffects/Light_Point.h>

class Light_Point_Shadowed :
	public Light_Point
{
private:
	ShadowCubeMap m_shadowCubeMap;

public:
	Light_Point_Shadowed(unsigned int shadowMapSize);
	~Light_Point_Shadowed();

	// Inherited from Light_Spot
	//void SetShader(Scene* pScene);
	//LightType GetType();

	//void RenderToMap(Scene* pScene);

	//unsigned int GetColorTextureID();
};

