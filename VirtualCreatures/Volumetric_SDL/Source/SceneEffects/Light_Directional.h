#pragma once

#include <Renderer/SDL_OpenGL.h>
#include <Renderer/Shader/Shader.h>

#include <Scene/Scene.h>

#include <SceneEffects/Light.h>

#include <Constructs/Color3f.h>

class Light_Directional // Does NOT inherit from Light, since it differs from the other sources in that it doesn't have a volume
{
protected:
	float m_intensity;

	Vec3f m_direction;

	class SceneEffect_Lighting* m_pLighting;

public:
	Color3f m_color;

	Light_Directional();
	virtual ~Light_Directional() {}

	void SetIntensity(float intensity);
	float GetIntensity();
	const Vec3f &GetCenter();
	void SetDirection(const Vec3f &direction);
	const Vec3f &GetDirection() const;

	virtual void SetShader(Scene* pScene);

	friend class SceneEffect_Lighting;
};

