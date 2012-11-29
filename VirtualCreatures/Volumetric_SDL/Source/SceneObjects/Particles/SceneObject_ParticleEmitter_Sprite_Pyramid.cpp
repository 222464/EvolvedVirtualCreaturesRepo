#include <SceneObjects/Particles/SceneObject_ParticleEmitter_Sprite_Pyramid.h>

#include <Utilities/UtilFuncs.h>

SceneObject_ParticleEmitter_Sprite_Pyramid::SceneObject_ParticleEmitter_Sprite_Pyramid()
	: m_minSpeed(0.1f), m_maxSpeed(0.2f), m_size(0.5f), m_direction(1.0f, 0.0f, 0.0f)
{
}

void SceneObject_ParticleEmitter_Sprite_Pyramid::SetParticle(Particle_Sprite* pParticle)
{
	// Random direction
	pParticle->m_position = m_position;

	pParticle->m_velocity = Vec3f(1.0f, Randf(-m_size, m_size), Randf(-m_size, m_size));

	// Zero vector check
	if(pParticle->m_velocity == Vec3f(0.0f, 0.0f, 0.0f))
		pParticle->m_velocity = Vec3f(1.0f, 0.0f, 0.0f);
	else
		pParticle->m_velocity.NormalizeThis();

	// Random speed
	pParticle->m_velocity *= Randf(m_minSpeed, m_maxSpeed);

	// Transform velocity to direction vector
	pParticle->m_velocity = Matrix4x4f::DirectionMatrix_AutoUp(m_direction) * pParticle->m_velocity;
}