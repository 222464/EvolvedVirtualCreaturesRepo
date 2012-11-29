#include <SceneEffects/Light_Point.h>

#include <SceneEffects/SceneEffect_Lighting.h>

#include <Constructs/Matrix4x4f.h>

const float Light_Point::s_cutoffDistanceMultiplier = 8.0f;
const float Light_Point::s_rangeIntersectGeometryMultiplier = 1.2f;

Light_Point::Light_Point()
	: m_intensity(1.0f),
	m_color(1.0f, 1.0f, 1.0f)
{
	CalculateRange();
}

void Light_Point::CalculateRange()
{
	float maxColor = std::max(std::max(m_color.r, m_color.g), m_color.b);
	m_range = s_cutoffDistanceMultiplier * sqrtf(maxColor * m_intensity);

	m_aabb.SetHalfDims(Vec3f(m_range, m_range, m_range));
	
	if(GetTree() != NULL)
		TreeUpdate();
}

void Light_Point::SetShader(Scene* pScene)
{
	m_pLighting->m_pointLightEffectShader.Bind();

	m_pLighting->m_pointLightEffectShader.SetUniformv3f(m_pLighting->m_pointLightPositionLocation, pScene->GetViewMatrix() * m_aabb.GetCenter());
	m_pLighting->m_pointLightEffectShader.SetUniformv3f(m_pLighting->m_pointLightColorLocation, m_color);
	m_pLighting->m_pointLightEffectShader.SetUniformf(m_pLighting->m_pointLightRangeLocation, m_range);
	m_pLighting->m_pointLightEffectShader.SetUniformf(m_pLighting->m_pointLightIntensityLocation, m_intensity);
}

void Light_Point::SetTransform(Scene* pScene)
{
	pScene->SetWorldMatrix(Matrix4x4f::TranslateMatrix(m_aabb.GetCenter()) * Matrix4x4f::ScaleMatrix(Vec3f(m_range, m_range, m_range)));
}

void Light_Point::RenderBoundingGeom()
{
	m_pLighting->m_sphere.Render();
}

void Light_Point::SetIntensity(float intensity)
{
	m_intensity = intensity;

	CalculateRange();
}

float Light_Point::GetIntensity()
{
	return m_intensity;
}

float Light_Point::GetRange()
{
	return m_range;
}

void Light_Point::SetCenter(const Vec3f &center)
{
	m_aabb.SetCenter(center);

	if(GetTree() != NULL)
		TreeUpdate();
}

void Light_Point::IncCenter(const Vec3f &increment)
{
	m_aabb.IncCenter(increment);

	if(GetTree() != NULL)
		TreeUpdate();
}

const Vec3f &Light_Point::GetCenter()
{
	return m_aabb.GetCenter();
}

bool Light_Point::Intersects(const Vec3f &position)
{
	if((position - m_aabb.GetCenter()).Magnitude() < m_range * s_rangeIntersectGeometryMultiplier)
		return true;

	return false;
}

Light::LightType Light_Point::GetType()
{
	return e_point;
}