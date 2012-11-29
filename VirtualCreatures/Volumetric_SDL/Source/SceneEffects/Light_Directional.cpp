#include <SceneEffects/Light_Directional.h>

#include <SceneEffects/SceneEffect_Lighting.h>

#include <Constructs/Matrix4x4f.h>

Light_Directional::Light_Directional()
	: m_intensity(1.0f),
	m_color(1.0f, 1.0f, 1.0f),
	m_direction(0.0f, -1.0f, 0.0f)
{
}

void Light_Directional::SetShader(Scene* pScene)
{
	m_pLighting->m_directionalLightEffectShader.Bind();

	m_pLighting->m_directionalLightEffectShader.SetUniformv3f("lightColor", m_color);
	m_pLighting->m_directionalLightEffectShader.SetUniformv3f("lightDirection", pScene->GetNormalMatrix() * m_direction);
	m_pLighting->m_directionalLightEffectShader.SetUniformf("lightIntensity", m_intensity);
}

void Light_Directional::SetIntensity(float intensity)
{
	m_intensity = intensity;
}

float Light_Directional::GetIntensity()
{
	return m_intensity;
}

void Light_Directional::SetDirection(const Vec3f &direction)
{
	m_direction = direction.Normalize();
}

const Vec3f &Light_Directional::GetDirection() const
{
	return m_direction;
}