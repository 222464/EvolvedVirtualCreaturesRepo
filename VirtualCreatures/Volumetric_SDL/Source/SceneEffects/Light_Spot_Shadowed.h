#pragma once

#include <SceneEffects/Light_Spot.h>
#include <Renderer/BufferObjects/FBO.h>

class Light_Spot_Shadowed :
	public Light_Spot
{
private:
	FBO m_shadowMap;
	FBO m_blurPingPong;

	Matrix4x4f m_lightTransform;

public:
	Light_Spot_Shadowed(unsigned int shadowMap_width, unsigned int shadowMap_height);
	~Light_Spot_Shadowed();

	// Inherited from Light_Spot
	void SetShader(Scene* pScene);
	LightType GetType();

	void RenderToMap(Scene* pScene);

	unsigned int GetColorTextureID();
};