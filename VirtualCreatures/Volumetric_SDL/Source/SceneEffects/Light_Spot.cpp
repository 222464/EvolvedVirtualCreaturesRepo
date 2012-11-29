#include <SceneEffects/Light_Spot.h>

#include <SceneEffects/SceneEffect_Lighting.h>

#include <Constructs/Matrix4x4f.h>

#include <Utilities/UtilFuncs.h>

#include <assert.h>

const float Light_Spot::s_cutoffDistanceMultiplier = 8.0f;
const float Light_Spot::s_rangeIntersectGeometryMultiplier = 1.1f;

Light_Spot::Light_Spot()
	: m_intensity(1.0f),
	m_color(1.0f, 1.0f, 1.0f),
	m_center(0.0f, 0.0f, 0.0f),
	m_direction(0.0f, -1.0f, 0.0f), // Defaults to pointing down
	m_spreadAngle(pif_over_4),
	m_spreadAngleCos(0.7f),
	m_lightSpotExponent(6.0f)
{
	RegenAABBAndTransform();
}

void Light_Spot::CalculateRange()
{
	float maxColor = std::max(std::max(m_color.r, m_color.g), m_color.b);
	m_range = s_cutoffDistanceMultiplier * sqrtf(maxColor * m_intensity);

	RegenAABBAndTransform();
}

void Light_Spot::RegenAABBAndTransform()
{
	m_spreadAngleCos = cosf(m_spreadAngle);
	m_coneEndRadius = m_range * tanf(m_spreadAngle);

	// X axis shifted unit AABB for transform
	m_aabb.SetHalfDims(Vec3f(0.5f, 1.0f, 1.0f));
	m_aabb.SetCenter(Vec3f(0.5f, 0.0f, 0.0f));

	// Update transform
	m_transform = Matrix4x4f::TranslateMatrix(m_center) * Matrix4x4f::DirectionMatrix_AutoUp(m_direction) * Matrix4x4f::ScaleMatrix(Vec3f(m_range, m_coneEndRadius, m_coneEndRadius));

	// Rotation
	m_aabb = m_aabb.GetTransformedAABB(m_transform);

	if(GetTree() != NULL)
		TreeUpdate();
}

void Light_Spot::SetShader(Scene* pScene)
{
	m_pLighting->m_spotLightEffectShader.Bind();

	m_pLighting->m_spotLightEffectShader.SetUniformv3f("lightPosition", pScene->GetViewMatrix() * m_center);
	m_pLighting->m_spotLightEffectShader.SetUniformv3f("lightColor", m_color);
	m_pLighting->m_spotLightEffectShader.SetUniformf("lightRange", m_range);
	m_pLighting->m_spotLightEffectShader.SetUniformv3f("lightSpotDirection", pScene->GetNormalMatrix() * m_direction);
	m_pLighting->m_spotLightEffectShader.SetUniformf("lightSpotExponent", m_lightSpotExponent);
	m_pLighting->m_spotLightEffectShader.SetUniformf("lightSpreadAngleCos", m_spreadAngleCos);
	m_pLighting->m_spotLightEffectShader.SetUniformf("lightIntensity", m_intensity);
}

void Light_Spot::SetTransform(Scene* pScene)
{
	pScene->SetWorldMatrix(m_transform);
}

void Light_Spot::SetIntensity(float intensity)
{
	m_intensity = intensity;

	CalculateRange();
}

float Light_Spot::GetIntensity() const
{
	return m_intensity;
}

float Light_Spot::GetRange() const
{
	return m_range;
}

void Light_Spot::SetCenter(const Vec3f &center)
{
	m_center = center;
	
	RegenAABBAndTransform();
}

void Light_Spot::IncCenter(const Vec3f &increment)
{
	m_center += increment;

	RegenAABBAndTransform();
}

const Vec3f &Light_Spot::GetCenter() const
{
	return m_center;
}

void Light_Spot::SetDirection(const Vec3f &direction)
{
	m_direction = direction.Normalize();

	RegenAABBAndTransform();
}

const Vec3f &Light_Spot::GetDirection() const
{
	return m_direction;
}

void Light_Spot::SetSpreadAngle(float angle)
{
	assert(m_spreadAngle < pif_over_2);

	m_spreadAngle = angle;

	RegenAABBAndTransform();
}

float Light_Spot::GetSpreadAngle() const
{
	return m_spreadAngle;
}

float Light_Spot::GetSpreadAngleCos() const
{
	return m_spreadAngleCos;
}


void Light_Spot::RenderBoundingGeom()
{
	m_pLighting->m_cone.Render();
}

void Light_Spot::DrawAABB()
{
	m_aabb.DebugRender();
}

bool Light_Spot::Intersects(const Vec3f &position)
{
	if((position - m_aabb.GetCenter()).Magnitude() < m_range * s_rangeIntersectGeometryMultiplier)
		return true;

	return false;
}

Light::LightType Light_Spot::GetType()
{
	return e_spot;
}