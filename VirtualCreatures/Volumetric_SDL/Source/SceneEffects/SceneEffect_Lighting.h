#pragma once

#include <Scene/SceneEffect.h>

#include <Renderer/Octree.h>
#include <Renderer/Model_OBJ_VertexOnly.h>

#include <SceneEffects/Light.h>
#include <SceneEffects/Light_Point.h>
#include <SceneEffects/Light_Spot.h>
#include <SceneEffects/Light_Directional.h>

class SceneEffect_Lighting :
	public SceneEffect
{
private:
	StaticOctree m_lightSPT;

	std::unordered_set<Light*> m_lights;
	std::unordered_set<Light_Directional*> m_directionalLights; // Directional lights have separate treatment

	Shader m_pointLightEffectShader;
	Shader m_spotLightEffectShader;
	Shader m_spotLightShadowedEffectShader;
	Shader m_spotLightStoreMomentsShader;
	Shader m_VSMblurShader_horizontal;
	Shader m_VSMblurShader_vertical;
	Shader m_nullShader;
	Shader m_directionalLightEffectShader;
	Shader m_directionalLightShadowedEffectShader;
	Shader m_directionalLightStoreMomentsShader;

	// Point light shader attribute locations
	int m_pointLightPositionLocation;
	int m_pointLightColorLocation;
	int m_pointLightRangeLocation;
	int m_pointLightIntensityLocation;

	Model_OBJ_VertexOnly m_sphere;
	Model_OBJ_VertexOnly m_cone;

	Vec3f m_attenuation;

	unsigned char m_lightIndices[8];

	bool m_created;

public:
	Color3f m_ambient;

	SceneEffect_Lighting();
	~SceneEffect_Lighting();

	bool Create(const std::string &pointLightEffectShaderName,
		const std::string &spotLightEffectShaderName, const std::string &spotLightShadowedEffectShaderName, const std::string &spotLightStoreMomentsEffectShaderName,
		const std::string &VSMhorizontalBlurShaderName, const std::string &VSMverticalBlurShaderName,
		const std::string &directionalLightEffectShaderName, const std::string &directionalLightShadowedEffectShaderName, const std::string &directionalLightStoreMomentsEffectShaderName,
		const std::string &nullShaderName, const std::string &sphereModelName, const std::string &coneModelName);

	void AddLight(Light* pLight);
	void RemoveLight(Light* pLight);

	void AddLight_Directional(Light_Directional* pLight);
	void RemoveLight_Directional(Light_Directional* pLight);

	// Inherited from SceneEffect
	void RunEffect();

	void ClearLights();

	void BindPointLightShader();
	void BindSpotLightShader();

	void SetPointLightShininess(float shininess);
	void SetSpotLightShininess(float shininess);

	void RenderSphere();
	void RenderCone();

	void SetAttenuation(const Vec3f &attenuation);
	const Vec3f &GetAttenuation() const;

	void GetLightsInArea(const AABB &aabb, std::vector<Light*> &lights) const;

	friend class Light_Point;
	friend class Light_Point_Shadowed;
	friend class Light_Spot;
	friend class Light_Spot_Shadowed;
	friend class Light_Directional;
	friend class Light_Directional_Shadowed;
};

Matrix4x4f GetBias();