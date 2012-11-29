#pragma once

#include <Renderer/SDL_OpenGL.h>
#include <Renderer/Shader/Shader.h>

#include <Renderer/BufferObjects/FBO.h>

#include <Scene/Scene.h>

#include <SceneEffects/Light.h>
#include <SceneEffects/Light_Directional.h>

#include <Constructs/Color3f.h>

class Light_Directional_Shadowed :
	public Light_Directional // Does NOT inherit from Light, since it differs from the other sources in that it doesn't have a volume
{
public:
	static const unsigned int s_numCascades = 4;
private:
	FBO m_shadowMaps[s_numCascades];
	FBO m_shadowMapBlurPingPongs[s_numCascades];
	Matrix4x4f m_cascadeViews[s_numCascades];
	Matrix4x4f m_cascadeProjections[s_numCascades];
	Vec2f m_cascadeZBounds[s_numCascades];
	float m_splitDistances[s_numCascades];

	float GetSplitDistance(unsigned int cascade, float lambda, float zNear, float zFar);

public:
	Light_Directional_Shadowed(unsigned int shadowMapResolution, unsigned int cascadeResolutionDecreaseFactor, float maxDistance, float lambda, Scene* pScene);
	virtual ~Light_Directional_Shadowed() {}

	void UpdateProjection(Scene* pScene);
	void RenderToCascades(Scene* pScene);

	void DebugDrawCascade(Scene* pScene, unsigned int index);

	// Inherited from Light_Directional
	void SetShader(Scene* pScene);

	friend class SceneEffect_Lighting;
};

Vec3f GetOrientationVector(const Matrix4x4f &viewMatrix);
Vec3f GetUpVector(const Matrix4x4f &viewMatrix);
void GetFrustumCornerPoints(Scene* pScene, float zDistance, Vec3f points[4]);