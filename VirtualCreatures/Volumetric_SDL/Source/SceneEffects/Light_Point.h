#pragma once

#include <Renderer/SDL_OpenGL.h>
#include <Renderer/Shader/Shader.h>

#include <Scene/Scene.h>

#include <SceneEffects/Light.h>

#include <Constructs/Color3f.h>

class Light_Point :
	public Light
{
private:
	float m_intensity;

	float m_range;

public:
	static const float s_cutoffDistanceMultiplier;
	static const float s_rangeIntersectGeometryMultiplier;

	Color3f m_color;

	Light_Point();
	virtual ~Light_Point() {}

	void SetIntensity(float intensity);
	float GetIntensity();
	float GetRange();
	void SetCenter(const Vec3f &center);
	void IncCenter(const Vec3f &increment);
	const Vec3f &GetCenter();

	void CalculateRange();

	// Inherited from Light
	virtual void SetShader(Scene* pScene);
	void SetTransform(Scene* pScene);
	void RenderBoundingGeom();
	bool Intersects(const Vec3f &position);
	virtual LightType GetType();

	friend class SceneEffect_Lighting;
};

