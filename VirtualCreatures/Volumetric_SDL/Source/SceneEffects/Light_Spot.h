#pragma once

#include <Renderer/SDL_OpenGL.h>
#include <Renderer/Shader/Shader.h>

#include <Scene/Scene.h>

#include <Constructs/Color3f.h>

#include <SceneEffects/Light.h>

class Light_Spot :
	public Light
{
protected:
	float m_intensity;

	float m_range;

	Vec3f m_direction;
	Vec3f m_center;

	float m_spreadAngleCos;
	float m_spreadAngle;

	float m_coneEndRadius;

	void RegenAABBAndTransform();

	Matrix4x4f m_transform;

public:
	static const float s_cutoffDistanceMultiplier;
	static const float s_rangeIntersectGeometryMultiplier;

	float m_lightSpotExponent;
	
	Color3f m_color;

	Light_Spot();
	virtual ~Light_Spot() {}

	void SetIntensity(float intensity);
	float GetIntensity() const;
	float GetRange() const;
	void SetCenter(const Vec3f &center);
	void IncCenter(const Vec3f &increment);
	const Vec3f &GetCenter() const;
	void SetDirection(const Vec3f &direction);
	const Vec3f &GetDirection() const;
	void SetSpreadAngle(float angle);
	float GetSpreadAngle() const;
	float GetSpreadAngleCos() const;

	void CalculateRange();

	// Inherited from Light
	virtual void SetShader(Scene* pScene);
	void SetTransform(Scene* pScene);
	void RenderBoundingGeom();
	bool Intersects(const Vec3f &position);
	virtual LightType GetType();

	void DrawAABB();

	friend class SceneEffect_Lighting;
};

