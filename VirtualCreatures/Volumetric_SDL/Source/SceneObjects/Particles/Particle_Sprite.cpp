#include <SceneObjects/Particles/Particle_Sprite.h>

Particle_Sprite::Particle_Sprite()
	: m_pSprite(NULL), m_scale(1.0f), m_color(1.0f, 1.0f, 1.0f), m_age(0.0f),
	m_animationSpeed(1.0f)
{
}

bool Particle_Sprite::Logic()
{
	m_age += m_pScene->m_frameTimer.GetTimeMultiplier();

	if(m_age > m_despawnTime)
		return true;

	m_position += m_velocity * m_pScene->m_frameTimer.GetTimeMultiplier();

	return false;
}

void Particle_Sprite::Render(const Matrix4x4f &particleDirectionMatrix)
{
	m_pScene->SetWorldMatrix(Matrix4x4f::TranslateMatrix(m_position)
		* particleDirectionMatrix
		* Matrix4x4f::ScaleMatrix(Vec3f(m_scale, m_scale, m_scale)));

	m_pSprite->SetTime(m_age * m_animationSpeed);
	m_pSprite->Render();
}

float Particle_Sprite::GetRadius()
{
	return m_scale * m_radius;
}